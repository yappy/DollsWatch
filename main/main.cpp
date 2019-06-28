#include <M5Stack.h>
#include <esp_system.h>

// The setup routine runs once when M5Stack starts up
void setup()
{
	// Initialize the M5Stack object
	// LCD, SD, Serial, I2C
	M5.begin(true, true, true, false);

	// LCD display
	M5.Lcd.setTextSize(2);
	M5.Lcd.println("Hello World");

	uint8_t mac[6] = { 0 };
	esp_efuse_mac_get_default(mac);
	M5.Lcd.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	esp_chip_info_t chip;
	esp_chip_info(&chip);
	M5.Lcd.printf("Rev: %d, Core: %d\n", chip.revision, chip.cores);

	M5.Lcd.printf("ESP-IDF: %s\n", esp_get_idf_version());
	M5.Lcd.printf("arduino-esp32: %s\n", ARDUINO_VER);
	M5.Lcd.printf("M5Stack: %s\n", M5STACK_VER);
	M5.Lcd.printf("App: %s\n", PROJECT_VER);
}

// The loop routine runs over and over again forever
void loop()
{
	M5.update();
}

// The arduino task
void loopTask(void *pvParameters)
{
	setup();
	for (;;) {
		loop();
	}
}

extern "C" void app_main()
{
	initArduino();
	xTaskCreateUniversal(loopTask, "loopTask", 8192, NULL, 1, NULL,
		CONFIG_ARDUINO_RUNNING_CORE);
}
