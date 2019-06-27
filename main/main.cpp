#include <M5Stack.h>
#include <esp_system.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// The setup routine runs once when M5Stack starts up
void setup(){

	// Initialize the M5Stack object
	M5.begin();

	// LCD display
	M5.Lcd.println("Hello World");

	M5.Lcd.print("ESP-IDF: ");
	M5.Lcd.println(esp_get_idf_version());
	M5.Lcd.print("App: ");
	M5.Lcd.println(PROJECT_VER);
}

// The loop routine runs over and over again forever
void loop() {

  M5.update();
}

// The arduino task
void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        micros(); //update overflow
        loop();
    }
}

extern "C" void app_main()
{
    initArduino();
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}
