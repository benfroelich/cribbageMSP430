#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

class InputPin
{
public:
	InputPin(unsigned bank, unsigned pin, unsigned debounceSamps) :
		bank(bank), pin(pin), debounceSamps(debounceSamps) {}
	bool read();
private:
	void debounce();
	unsigned debounceSamps;
	unsigned bank;
	unsigned pin;
};
class InputHandler
{
public:
	virtual void update();
private:

};

#endif
