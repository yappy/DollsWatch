#include "app_bt_client.h"
#include "conf.h"
#include <M5Stack.h>
#include <esp_bt.h>
#include <esp_bt_main.h>


void BleGattClientApp::setup()
{
	// Create dependency to esp32-hal-bt.c
	// esp32-hal-bt.o has 'bool btInUse() = true'
	// esp32-hal-misc.o has weak 'bool btInUse() = false'
	// If not btInUse(), initArduino() releases all bt memory and
	// causes initialize errors
	btStarted();

	// Release memory for unused feature
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
}

void BleGattClientApp::frame()
{
	bool dirty = false;

	if (M5.BtnC.wasReleased()) {
		switch (m_status) {
		case BtStatus::STOP:
			start();
			break;
		case BtStatus::ACTIVE:
			stop();
			break;
		default:
			assert(0);
		}
		dirty = true;
	}

	if (dirty) {
		repaint();
	}
}

void BleGattClientApp::redraw()
{
	M5.Lcd.clear();
	M5.Lcd.setTextSize(2);

	const char *statstr = "";
	switch (m_status) {
	case BtStatus::STOP:
		statstr = "OFF";
		break;
	case BtStatus::ACTIVE:
		statstr = "ON";
		break;
	default:
		assert(0);
	}
	M5.Lcd.printf("Bluethooth %s\n", statstr);
}

void BleGattClientApp::start()
{
	// bt controller init
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
	// bt controller enable
	ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

	// bt stack init
	ESP_ERROR_CHECK(esp_bluedroid_init());
	// bt stack enable
	ESP_ERROR_CHECK(esp_bluedroid_enable());

	m_status = BtStatus::ACTIVE;
}

void BleGattClientApp::stop()
{
	ESP_ERROR_CHECK(esp_bluedroid_disable());
	ESP_ERROR_CHECK(esp_bluedroid_deinit());

	ESP_ERROR_CHECK(esp_bt_controller_disable());
	ESP_ERROR_CHECK(esp_bt_controller_deinit());

	m_status = BtStatus::STOP;
}
