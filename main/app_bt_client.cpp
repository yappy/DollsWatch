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
}

void BleGattClientApp::frame()
{
	if (M5.BtnC.wasReleased()) {
		start();
	}

	bool dirty = false;

	if (dirty) {
		repaint();
	}
}

void BleGattClientApp::redraw()
{
	M5.Lcd.clear();
	M5.Lcd.setTextSize(2);
	M5.Lcd.println("Bluethooth");
}

void BleGattClientApp::start()
{
	// Release memory for unused feature
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	// bt controller init
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
	// bt controller enable
	ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

	// bt stack init
	ESP_ERROR_CHECK(esp_bluedroid_init());
	// bt stack enable
	ESP_ERROR_CHECK(esp_bluedroid_enable());
}

void BleGattClientApp::stop()
{

}
