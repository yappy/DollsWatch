#ifndef APP_INFO_H_
#define APP_INFO_H_

#include "app.h"
#include <esp_system.h>

class InfoApp : public App {
public:
	InfoApp() = default;
	virtual ~InfoApp() = default;

	void setup() override;
	void frame() override;
	void redraw() override;

private:
	int m_pos;

	int m_battery_level;
	bool m_is_charging;
	uint8_t m_mac[6];
	esp_chip_info_t m_chip_info;
	uint32_t m_cpu_freq_mhz;
	uint32_t m_heap_total, m_heap_free;
	uint32_t m_spiram_total, m_spiram_free;
	uint32_t m_flash_size, m_flash_speed;
};

#endif // APP_INFO_H_
