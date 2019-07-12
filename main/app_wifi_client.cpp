#include "app_wifi_client.h"
#include "conf.h"
#include <M5Stack.h>
#include <esp_event_loop.h>

int WifiClientApp::event_handler(void *ctx, system_event_t *event)
{
	WifiClientApp *self = (WifiClientApp *)ctx;
	switch (event->event_id) {
	case SYSTEM_EVENT_SCAN_DONE:
		break;
	case SYSTEM_EVENT_STA_START:
		puts("STA_START");
		self->eh_sta_start();
		break;
	case SYSTEM_EVENT_STA_STOP:
		puts("STA_STOP");
		self->eh_sta_stop();
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		puts("STA_CONNECTED");
		self->eh_sta_connected();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		puts("STA_DISCONNECTED");
		self->eh_sta_disconnected();
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		puts("GOT IP");
		self->eh_sta_got_ip(&event->event_info.got_ip);
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		puts("LOST IP");
		self->eh_sta_lost_ip();
		break;
	case SYSTEM_EVENT_AP_START:
		puts("AP_START");
		self->eh_ap_start();
		break;
	case SYSTEM_EVENT_AP_STOP:
		puts("AP_STOP");
		self->eh_ap_stop();
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		puts("AP_CONNECTED");
		self->eh_ap_connected(&event->event_info.sta_connected);
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		puts("AP_DISCONNECTED");
		self->eh_ap_disconnected(&event->event_info.sta_disconnected);
		break;
	default:
		break;
	}
	return ESP_OK;
}

void WifiClientApp::eh_sta_start()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STA_STARTED);
	xSemaphoreGive(m_mtx);

	ESP_ERROR_CHECK(esp_wifi_connect());
}

void WifiClientApp::eh_sta_stop()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STOP);
	xSemaphoreGive(m_mtx);
}

void WifiClientApp::eh_sta_connected()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STA_CONNECTED);
	xSemaphoreGive(m_mtx);
}

void WifiClientApp::eh_sta_disconnected()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STA_STARTED);
	xSemaphoreGive(m_mtx);

	// auto reconnect
	ESP_ERROR_CHECK(esp_wifi_connect());
}

void WifiClientApp::eh_sta_got_ip(const system_event_sta_got_ip_t *event)
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STA_IP_OWNED);
	memcpy(m_status.ipaddr , &event->ip_info.ip.addr     , 4);
	memcpy(m_status.netmask, &event->ip_info.netmask.addr, 4);
	memcpy(m_status.gw     , &event->ip_info.gw.addr     , 4);
	xSemaphoreGive(m_mtx);

	// kick NTP
	configTime(WIFI_NTP_TZ, WIFI_NTP_DAYLIGHT, WIFI_NTP_SERVER);
	// HTTP server start
	m_http_server.start();
}

void WifiClientApp::eh_sta_lost_ip()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STA_CONNECTED);
	memset(m_status.ipaddr , 0, 4);
	memset(m_status.netmask, 0, 4);
	memset(m_status.gw     , 0, 4);
	xSemaphoreGive(m_mtx);

	// HTTP server stop
	m_http_server.stop();
}

void WifiClientApp::eh_ap_start()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::AP_STARTED);
	xSemaphoreGive(m_mtx);

	// HTTP server start
	m_http_server.start();
}

void WifiClientApp::eh_ap_stop()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.to(WifiState::STOP);
	xSemaphoreGive(m_mtx);

	// HTTP server stop
	m_http_server.stop();
}

void WifiClientApp::eh_ap_connected(const system_event_ap_staconnected_t *event)
{

}

void WifiClientApp::eh_ap_disconnected(
	const system_event_ap_stadisconnected_t *event)
{

}

void WifiClientApp::setup()
{
	load_config_file();

	m_mtx = xSemaphoreCreateMutex();
	assert(m_mtx != nullptr);

	// LwIP (TCP/IP) stack init
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, this));

	// wifi init
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void WifiClientApp::start_sta()
{
	bool stop_state = false;
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	stop_state = (m_status.state == WifiState::STOP);
	if (stop_state) {
		m_status.to(WifiState::STA_STARTING);
	}
	xSemaphoreGive(m_mtx);
	if (!stop_state) {
		return;
	}

	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));
	if (m_conf_count > 0) {
		strcpy((char *)wifi_config.sta.ssid, m_conf[0].ssid);
		strcpy((char *)wifi_config.sta.password, m_conf[0].pass);
	}

	// station (client) mode
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	// access point config
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	// start!
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiClientApp::start_ap()
{
	bool stop_state = false;
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	stop_state = (m_status.state == WifiState::STOP);
	if (stop_state) {
		m_status.to(WifiState::AP_STARTING);
	}
	xSemaphoreGive(m_mtx);
	if (!stop_state) {
		return;
	}

	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));
	strlcpy((char *)wifi_config.ap.ssid, "shanghai-wifi",
		sizeof(wifi_config.ap.ssid));
	wifi_config.ap.max_connection = 4;
	if (strlen((char *)wifi_config.ap.password) == 0) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}
	else {
		wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	}

	// access point mode
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	// access point config
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	// start!
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiClientApp::stop()
{
	// can call multiple times
	ESP_ERROR_CHECK(esp_wifi_stop());
}

void WifiClientApp::frame()
{
	if (M5.BtnB.wasReleased()) {
		stop();
	}
	else if (M5.BtnA.wasReleased()) {
		start_ap();
	}
	else if (M5.BtnC.wasReleased()) {
		start_sta();
	}

	WifiStatus status;
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = false;
	status = m_status;
	xSemaphoreGive(m_mtx);

	if (!status.dirty) {
		return;
	}

	const char *main_desc = "Disabled";
	switch (status.state) {
	case WifiState::STOP:
		break;
	case WifiState::STA_STARTING:
	case WifiState::STA_STARTED:
	case WifiState::STA_CONNECTED:
	case WifiState::STA_IP_OWNED:
		main_desc = "Wifi Station";
		break;
	case WifiState::AP_STARTING:
	case WifiState::AP_STARTED:
		main_desc = "Access Point";
		break;
	}
	const char *sub_desc = "";
	switch (status.state) {
	case WifiState::STOP:
		break;
	case WifiState::STA_STARTING:
		sub_desc = "starting";
		break;
	case WifiState::STA_STARTED:
		sub_desc = "connecting";
		break;
	case WifiState::STA_CONNECTED:
		sub_desc = "obtaining ip";
		break;
	case WifiState::STA_IP_OWNED:
		sub_desc = "connected";
		break;
	case WifiState::AP_STARTING:
		sub_desc = "starting";
		break;
	case WifiState::AP_STARTED:
		sub_desc = "active";
		break;
	}

	M5.Lcd.setTextSize(2);
	M5.Lcd.setCursor(0, 20);
	M5.Lcd.printf("Mode: %12s\n", main_desc);
	M5.Lcd.printf("%12s\n", sub_desc);
	if (status.state == WifiState::STA_IP_OWNED) {
		M5.Lcd.printf("IP Addr: %3d.%3d.%3d.%3d\n",
			m_status.ipaddr[0], m_status.ipaddr[1],
			m_status.ipaddr[2], m_status.ipaddr[3]);
	}
	else if (status.state == WifiState::AP_STARTED) {
		M5.Lcd.printf("IP Addr: %3d.%3d.%3d.%3d\n", 192, 168, 4, 1);
	}
}

void WifiClientApp::redraw()
{
	M5.Lcd.clear();
	M5.Lcd.setTextSize(2);
	M5.Lcd.println("Wifi");

	M5.Lcd.setTextSize(1);
	for (int i = 0; i < m_conf_count; i++) {
		M5.Lcd.printf("%d: %s\n", i + 1, m_conf[i].ssid);
	}

	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = true;
	xSemaphoreGive(m_mtx);
}

void WifiClientApp::load_config_file()
{
	m_conf_count = 0;

	SDFile file = SD.open("/wifi.txt", FILE_READ);
	if (!file) {
		return;
	}
	auto read_line = [&file](char *buf, size_t bufsize) {
		size_t i = 0;
		int c;
		while ((c = file.read()) >= 0) {
			if (c == '\r') {
				continue;
			}
			if (c == '\n') {
				break;
			}
			// i < bufsize - 1
			if (i + 1 < bufsize) {
				buf[i] = c;
				i++;
			}
		}
		buf[i] = '\0';
	};
	while (m_conf_count < ConfigMax && file.available() > 0) {
		WifiConfig conf;
		read_line(conf.ssid, sizeof(conf.ssid));
		read_line(conf.pass, sizeof(conf.pass));

		m_conf[m_conf_count] = conf;
		m_conf_count++;
	}
}
