#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

class InputPin
{
public:
	InputPin(unsigned bank, unsigned pin, unsigned debounceSamps,
			unsigned holdRepeat) :
		bank(bank), pin(pin), debounceSamps(debounceSamps),
		holdRepeat(holdRepeat) {};
	bool read();
private:
	void debounce();
	unsigned debounceSamps;
	unsigned bank;
	unsigned pin;
	unsigned state;
	unsigned holdRepeat;
};

#endif
