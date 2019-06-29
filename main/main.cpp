#include <M5Stack.h>
#include "conf.h"
#include "app_info.h"
#include "app_clock.h"

namespace {
	InfoApp s_info;
	ClockApp s_clock;

	uint32_t s_app_idx = 1;
	std::array<App *, 2> s_app_list = {
		&s_info,
		&s_clock,
	};

	inline App &cur_app()
	{
		return *s_app_list[s_app_idx];
	}

	inline void move_left()
	{
		s_app_idx += s_app_list.size();
		s_app_idx--;
		s_app_idx %= s_app_list.size();
	}

	inline void move_right()
	{
		s_app_idx++;
		s_app_idx %= s_app_list.size();
	}

	inline void reset_draw_state()
	{
		M5.Lcd.setCursor(0, 0);
		M5.Lcd.setTextSize(1);
		M5.Lcd.setTextDatum(0);
	}
} // namespace

// The main task
static void mainTask(void *pvParameters)
{
	// Initialize the M5Stack object
	// LCD, SD, Serial, I2C
	M5.begin(true, true, true, false);
	M5.Power.begin();

	const uint32_t W = M5.Lcd.width();
	const uint32_t H = M5.Lcd.height();
	const uint32_t TextH = M5.Lcd.fontHeight() * 2;

	for (auto &pscr : s_app_list) {
		pscr->setup();
	}

	bool move_state = false;
	cur_app().repaint();
	while (1) {
		// update GPIO state etc.
		M5.update();
		if (move_state) {
			// moveing state
			if (M5.BtnB.wasReleased()) {
				move_state = false;
				cur_app().repaint();
			}
			else if (M5.BtnA.wasReleased()) {
				move_left();
				cur_app().repaint();
			}
			else if (M5.BtnC.wasReleased()) {
				move_right();
				cur_app().repaint();
			}
		}
		else {
			// normal state
			if (M5.BtnB.pressedFor(1000)) {
				move_state = true;
				cur_app().repaint();
			}
			else {
				reset_draw_state();
				cur_app().frame();
			}
		}
		// reset x, y, font
		reset_draw_state();
		if (cur_app().isRepaintRequired()) {
			cur_app().clearRepaint();

			reset_draw_state();
			cur_app().redraw();

			if (move_state) {
				reset_draw_state();
				M5.Lcd.fillRect(0, H - TextH * 2, W, TextH * 2, TFT_GREEN);
				M5.Lcd.setTextSize(2);
				M5.Lcd.setTextDatum(BC_DATUM);
				char page_str[12];
				sprintf(page_str, "%u", s_app_idx);
				M5.Lcd.drawString(page_str, M5.Lcd.width() / 2, H - TextH);
				M5.Lcd.drawString("<- moving ->", M5.Lcd.width() / 2, H);
			}
		}
		// vTaskDelay() ms version
		delay(MAIN_LOOP_SLEEP);
	}
}

extern "C" void app_main()
{
	initArduino();
	xTaskCreateUniversal(mainTask, "mainTask",
		MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL,
		MAIN_TASK_CORE);
}
