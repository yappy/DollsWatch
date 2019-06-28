#include <M5Stack.h>
#include <esp_system.h>

class Screen {
public:
	static const int INPUT_A = 1;
	static const int INPUT_B = 2;
	static const int INPUT_C = 3;

	Screen() = default;
	virtual ~Screen() = default;

	virtual void setup() {}
	virtual void input(int btn) {}
	virtual void draw() { M5.Lcd.clear(); }

	void repaint() { m_repaint = true; }
	void clearRepaint() { m_repaint = false; }
	bool isRepaintRequired() { return m_repaint; }

private:
	bool m_repaint;
};

class InfoScreen : public Screen {
public:
	InfoScreen() = default;
	virtual ~InfoScreen() = default;

	void setup() override
	{
		esp_efuse_mac_get_default(m_mac);
		esp_chip_info(&m_chip_info);
		m_cpu_freq_mhz = ESP.getCpuFreqMHz();
		m_heap_total = ESP.getHeapSize();
		m_heap_free = ESP.getFreeHeap();
		m_spiram_total = ESP.getPsramSize();
		m_spiram_free = ESP.getFreePsram();
		m_flash_size = ESP.getFlashChipSize();
		m_flash_speed = ESP.getFlashChipSpeed();
	}
	void input(int btn)
	{
		switch (btn) {
		case Screen::INPUT_B:
			setup();
			repaint();
			break;
		}
	}
	void draw() override
	{
		M5.Lcd.clear();
		M5.Lcd.setTextSize(2);

		M5.Lcd.println("System information");
		M5.Lcd.println("Press center to refresh");
		M5.Lcd.println();

		M5.Lcd.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
			m_mac[0], m_mac[1], m_mac[2], m_mac[3], m_mac[4], m_mac[5]);
		M5.Lcd.printf("Chip Rev: %d\nCore: %d, %u MHz\n",
			m_chip_info.revision, m_chip_info.cores, m_cpu_freq_mhz);
		M5.Lcd.println();

		M5.Lcd.println("Free Memory");
		M5.Lcd.printf("Heap   : %7u/%7u\n", m_heap_free, m_heap_total);
		M5.Lcd.printf("SPI RAM: %7u/%7u\n", m_spiram_free, m_spiram_total);
		M5.Lcd.println();

		M5.Lcd.printf("ESP-IDF: %s\n", esp_get_idf_version());
		M5.Lcd.printf("arduino-esp32: %s\n", ARDUINO_VER);
		M5.Lcd.printf("M5Stack: %s\n", M5STACK_VER);
		M5.Lcd.printf("App: %s\n", PROJECT_VER);
	}

private:
	uint8_t m_mac[6];
	esp_chip_info_t m_chip_info;
	uint32_t m_cpu_freq_mhz;
	uint32_t m_heap_total, m_heap_free;
	uint32_t m_spiram_total, m_spiram_free;
	uint32_t m_flash_size, m_flash_speed;
};

class ClockScreen : public Screen {
public:
	ClockScreen() = default;
	virtual ~ClockScreen() = default;

	void setup() override
	{
	}
	void draw() override
	{
		M5.Lcd.clear();

		M5.Lcd.setTextSize(2);

		M5.Lcd.println("Hello World");
	}
};

namespace
{
	InfoScreen s_info;
	ClockScreen s_clock;

	uint32_t s_screen_idx = 0;
	std::array<Screen *, 2> s_screen_list = {
		&s_info,
		&s_clock,
	};
	inline Screen &cur_screen()
	{
		return *s_screen_list[s_screen_idx];
	}
	inline void move_left()
	{
		s_screen_idx += s_screen_list.size();
		s_screen_idx--;
		s_screen_idx %= s_screen_list.size();
	}
	inline void move_right()
	{
		s_screen_idx++;
		s_screen_idx %= s_screen_list.size();
	}
} // namespace

// The main task
void mainTask(void *pvParameters)
{
	// Initialize the M5Stack object
	// LCD, SD, Serial, I2C
	M5.begin(true, true, true, false);

	const uint32_t W = M5.Lcd.width();
	const uint32_t H = M5.Lcd.height();
	// (?)
	const uint32_t TextH = M5.Lcd.fontHeight() * 2;

	for (auto &pscr : s_screen_list) {
		pscr->setup();
	}

	bool move_state = false;
	cur_screen().repaint();
	while (1) {
		// update GPIO state etc.
		M5.update();
		if (move_state) {
			// moveing state
			if (M5.BtnB.wasReleased()) {
				move_state = false;
				cur_screen().repaint();
			}
			else if (M5.BtnA.wasReleased()) {
				move_left();
				cur_screen().repaint();
			}
			else if (M5.BtnC.wasReleased()) {
				move_right();
				cur_screen().repaint();
			}
		}
		else {
			// normal state
			if (M5.BtnB.pressedFor(1000)) {
				move_state = true;
				cur_screen().repaint();
			}
			else {
				if (M5.BtnA.wasReleased()) {
					cur_screen().input(Screen::INPUT_A);
				}
				if (M5.BtnB.wasReleased()) {
					cur_screen().input(Screen::INPUT_B);
				}
				if (M5.BtnC.wasReleased()) {
					cur_screen().input(Screen::INPUT_C);
				}
			}
		}
		// reset x, y, font
		M5.Lcd.setCursor(0, 0, 1);
		if (cur_screen().isRepaintRequired()) {
			cur_screen().clearRepaint();
			cur_screen().draw();
			if (move_state) {
				M5.Lcd.setCursor(0, 0, 1);
				M5.Lcd.fillRect(0, H - TextH * 2, W, TextH * 2, TFT_GREEN);
				M5.Lcd.setTextDatum(BC_DATUM);
				char page_str[12];
				sprintf(page_str, "%u", s_screen_idx);
				M5.Lcd.drawString(page_str, M5.Lcd.width() / 2, H - TextH);
				M5.Lcd.drawString("<- moving ->", M5.Lcd.width() / 2, H);
			}
		}
		// vTaskDelay() - ms version
		delay(16);
	}
}

extern "C" void app_main()
{
	initArduino();
	xTaskCreateUniversal(mainTask, "mainTask", 8192, NULL, 1, NULL,
		CONFIG_ARDUINO_RUNNING_CORE);
}
