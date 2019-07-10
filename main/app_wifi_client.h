#ifndef APP_WIFI_CLIENT_H_
#define APP_WIFI_CLIENT_H_

#include "app.h"
#include "http_server.h"
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
	uint8_t ipaddr[4] = {0};
	uint8_t netmask[4] = {0};
	uint8_t gw[4] = {0};
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
	HttpServer m_http_server;

	void eh_sta_start();
	void eh_sta_stop();
	void eh_sta_connected();
	void eh_sta_disconnected();
	void eh_sta_got_ip(const system_event_sta_got_ip_t *event);
	void eh_sta_lost_ip();
	void eh_ap_start();
	void eh_ap_stop();
	void eh_ap_connected(const system_event_ap_staconnected_t *event);
	void eh_ap_disconnected(const system_event_ap_stadisconnected_t *event);

	void start_sta();
	void start_ap();
};

#endif // APP_WIFI_CLIENT_H_
