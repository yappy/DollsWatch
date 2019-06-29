#ifndef APP_CLOCK_H_
#define APP_CLOCK_H_

#include "app.h"

class ClockApp : public App {
public:
	ClockApp() = default;
	virtual ~ClockApp() = default;

	void setup() override;
	void frame() override;
	void redraw() override;

private:
	static const int BufSize = 16;
	char m_cache[3][BufSize];
};

#endif // APP_CLOCK_H_
