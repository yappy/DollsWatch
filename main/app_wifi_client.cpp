#include "app_wifi_client.h"
#include <M5Stack.h>
#include <esp_event_loop.h>

int WifiClientApp::event_handler(void *ctx, system_event_t *event)
{
	WifiClientApp *self = (WifiClientApp *)ctx;
	switch (event->event_id) {
	case SYSTEM_EVENT_SCAN_DONE:
		break;
	case SYSTEM_EVENT_STA_START:
		puts("START");
		self->eh_sta_start();
		break;
	case SYSTEM_EVENT_STA_STOP:
		puts("STOP");
		self->eh_sta_stop();
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		puts("CONNECTED");
		self->eh_sta_connected();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		puts("DISCONNECTED");
		self->eh_sta_disconnected();
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		puts("GOT IP");
		self->eh_sta_got_ip();
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		puts("LOST IP");
		self->eh_sta_lost_ip();
		break;
	default:
		break;
	}
	return ESP_OK;
}

void WifiClientApp::eh_sta_start()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = true;
	m_status.started = true;
	//m_status.connect_failed = false;
	xSemaphoreGive(m_mtx);

	ESP_ERROR_CHECK(esp_wifi_connect());
}

void WifiClientApp::eh_sta_stop()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = true;
	m_status.started = false;
	xSemaphoreGive(m_mtx);
}

void WifiClientApp::eh_sta_connected()
{
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = true;
	m_status.connected = true;
	xSemaphoreGive(m_mtx);
}

void WifiClientApp::eh_sta_disconnected()
{
	bool connect_failed = false;

	xSemaphoreTake(m_mtx, portMAX_DELAY);
	if (!m_status.connected) {
		m_status.connect_failed = true;
		connect_failed = true;
	}
	m_status.dirty = true;
	m_status.connected = false;
	xSemaphoreGive(m_mtx);

	if (connect_failed) {
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
}

void WifiClientApp::eh_sta_got_ip()
{

}

void WifiClientApp::eh_sta_lost_ip()
{

}

void WifiClientApp::setup()
{
	m_mtx = xSemaphoreCreateMutex();
	assert(m_mtx != nullptr);

	// LwIP (TCP/IP) stack init
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, this));

	// wifi init
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));
	strcpy((char *)wifi_config.sta.ssid, "?????");
	strcpy((char *)wifi_config.sta.password, "?????");

	// station (client) mode
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	// access point config
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	// start!
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiClientApp::frame()
{
	WifiStatus status;
	xSemaphoreTake(m_mtx, portMAX_DELAY);
	status = m_status;
	m_status.dirty = false;
	xSemaphoreGive(m_mtx);

	if (status.dirty) {
		M5.Lcd.setCursor(0, 100);
		M5.Lcd.printf("Started: %s\n", m_status.started ? "Yes" : "No");
		M5.Lcd.printf("Connected: %s\n", m_status.connected ? "Yes" : "No");
		M5.Lcd.printf("Connectt failed: %s\n", m_status.connect_failed ? "Yes" : "No");
	}
}

void WifiClientApp::redraw()
{
	M5.Lcd.clear();
	M5.Lcd.setTextSize(2);

	M5.Lcd.println("Wifi client");

	xSemaphoreTake(m_mtx, portMAX_DELAY);
	m_status.dirty = true;
	xSemaphoreGive(m_mtx);
}
