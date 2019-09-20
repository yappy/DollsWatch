#ifndef APP_BT_CLIENT_H_
#define APP_BT_CLIENT_H_

#include "app.h"


enum class BtStatus {
	STOP,
	START,
};

class BleGattClientApp : public App {
public:
	BleGattClientApp() = default;
	virtual ~BleGattClientApp() = default;

	void setup() override;
	void frame() override;
	void redraw() override;

private:
	BtStatus m_status;

	void start();
	void stop();
};

#endif // APP_BT_CLIENT_H_
