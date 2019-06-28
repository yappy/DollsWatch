#include <M5Stack.h>
#include <esp_system.h>

class Screen {
public:
	Screen() = default;
	virtual ~Screen() = default;

	virtual void setup() {}
	virtual void enter() {}
	virtual void leave() {}
	virtual void draw() {}
	virtual void input(int key) {}

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

	virtual void setup() override
	{
		esp_efuse_mac_get_default(m_mac);
		esp_chip_info(&m_chip_info);
	}
	virtual void draw()
	{
		M5.Lcd.setTextSize(2);

		M5.Lcd.println("Hello World");

		M5.Lcd.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
			m_mac[0], m_mac[1], m_mac[2], m_mac[3], m_mac[4], m_mac[5]);

		M5.Lcd.printf("Rev: %d, Core: %d\n",
			m_chip_info.revision, m_chip_info.cores);

		M5.Lcd.printf("ESP-IDF: %s\n", esp_get_idf_version());
		M5.Lcd.printf("arduino-esp32: %s\n", ARDUINO_VER);
		M5.Lcd.printf("M5Stack: %s\n", M5STACK_VER);
		M5.Lcd.printf("App: %s\n", PROJECT_VER);
	}

private:
	uint8_t m_mac[6];
	esp_chip_info_t m_chip_info;
};

namespace
{
	InfoScreen s_info;

	uint32_t s_screen_idx = 0;
	std::array<Screen *, 2> s_screen_list = {
		&s_info
	};
	inline Screen &cur_screen()
	{
		return *s_screen_list[s_screen_idx];
	}
} // namespace

// The main task
void mainTask(void *pvParameters)
{
	// Initialize the M5Stack object
	// LCD, SD, Serial, I2C
	M5.begin(true, true, true, false);
	cur_screen().repaint();
	while (1) {
		M5.update();
		if (cur_screen().isRepaintRequired()) {
			// x, y, font
			M5.Lcd.setCursor(0, 0, 1);
			cur_screen().draw();
			cur_screen().clearRepaint();
		}
	}
}

extern "C" void app_main()
{
	initArduino();
	xTaskCreateUniversal(mainTask, "mainTask", 8192, NULL, 1, NULL,
		CONFIG_ARDUINO_RUNNING_CORE);
}
