#ifndef APP_H_
#define APP_H_

class App {
public:
	App() = default;
	virtual ~App() = default;

	virtual void setup() {}
	virtual void frame() {}
	virtual void redraw() {}

	void repaint() { m_repaint = true; }
	void clearRepaint() { m_repaint = false; }
	bool isRepaintRequired() { return m_repaint; }

private:
	bool m_repaint;
};

#endif // APP_H_
