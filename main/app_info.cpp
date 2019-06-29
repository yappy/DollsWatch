#include "app_info.h"
#include <esp_system.h>
#include <M5Stack.h>

void InfoApp::setup()
{
	m_pos = 0;

	m_battery_level = M5.Power.getBatteryLevel();
	m_is_charging = M5.Power.isCharging();

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

void InfoApp::frame()
{
	if (M5.BtnB.wasReleased()) {
		setup();
		repaint();
	}
	if (M5.BtnA.wasReleased()) {
		m_pos++;
		repaint();
	}
	if (M5.BtnC.wasReleased()) {
		m_pos--;
		repaint();
	}
}

void InfoApp::redraw()
{
	M5.Lcd.clear();
	M5.Lcd.setTextSize(2);
	const int TextH = M5.Lcd.fontHeight();
	M5.Lcd.setCursor(0, m_pos * TextH);

	M5.Lcd.println("System information");
	M5.Lcd.println("Press center to refresh");
	M5.Lcd.println("Press left/right to move");
	M5.Lcd.println();

	M5.Lcd.printf("Battery: %d%%\n", m_battery_level);
	M5.Lcd.printf("Charging: %s\n", m_is_charging ? "Yes" : "No");
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
