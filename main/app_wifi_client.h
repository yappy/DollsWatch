#ifndef APP_WIFI_CLIENT_H_
#define APP_WIFI_CLIENT_H_

#include "app.h"
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <freertos/semphr.h>

// 32
const size_t SsidLen = sizeof(((wifi_sta_config_t *)nullptr)->ssid);
// 64
const size_t PassLen = sizeof(((wifi_sta_config_t *)nullptr)->password);

const int ConfigMax = 8;

struct WifiConfig {
	char ssid[SsidLen];
	char pass[PassLen];
};

struct WifiStatus {
	bool dirty = false;
	bool started = false;
	bool connected = false;
	bool connect_failed = false;
};

class WifiClientApp : public App {
public:
	WifiClientApp() = default;
	virtual ~WifiClientApp() = default;

	void setup() override;
	void frame() override;
	void redraw() override;

private:
	int m_conf_count;
	WifiConfig m_conf[ConfigMax];
	void load_config_file();

public:
	static int event_handler(void *ctx, system_event_t *event);

private:
	xSemaphoreHandle m_mtx;
	WifiStatus m_status;

	void eh_sta_start();
	void eh_sta_stop();
	void eh_sta_connected();
	void eh_sta_disconnected();
	void eh_sta_got_ip();
	void eh_sta_lost_ip();
};

#endif // APP_WIFI_CLIENT_H_
