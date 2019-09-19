#include "app_bt_client.h"
#include "conf.h"
#include <M5Stack.h>


void BleGattClientApp::setup()
{
	// bt init
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

}

void BleGattClientApp::stop()
{

}
