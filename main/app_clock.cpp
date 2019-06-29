#include "app_clock.h"
#include <M5Stack.h>
#include <time.h>

void ClockApp::setup()
{
	memset(m_cache, 0, sizeof(m_cache));
}

void ClockApp::frame()
{
	time_t t;
	struct tm *tm;
	t = time(nullptr);
	tm = localtime(&t);

	char str[3][BufSize];
	const char *fmt[3] = { "%H:%M:%S", "%Y %m/%d", "%a" };
	for (int i = 0; i < 3; i++) {
		size_t len = strftime(str[i], sizeof(str[i]), fmt[i], tm);
		if (len == 0) {
			return;
		}
	}

	M5.Lcd.setTextSize(2);
	const uint32_t H = M5.Lcd.height();
	const uint32_t W = M5.Lcd.width();
	const uint32_t TextH = M5.Lcd.fontHeight();
	const uint32_t Pad = 4;
	M5.Lcd.setTextDatum(BR_DATUM);
	for (int i = 0; i < 3; i++) {
		if (strcmp(str[i], m_cache[i]) == 0) {
			continue;
		}
		strcpy(m_cache[i], str[i]);

		M5.Lcd.drawString(str[i],
			W - Pad,
			H - (3 - i - 1) * TextH - Pad);
	}
}

void ClockApp::redraw()
{
	const char *FileName = "/yappy_house_240.jpg";

	M5.Lcd.clear();

	M5.Lcd.setCursor(40, 0);
	M5.Lcd.printf("Error: %s", FileName);
	M5.Lcd.drawJpgFile(SD, FileName, 40, 0, 240, 240);

	// clear cache
	memset(m_cache, 0, sizeof(m_cache));
}
