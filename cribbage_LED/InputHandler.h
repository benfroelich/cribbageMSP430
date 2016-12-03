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
	bool polarity;				// active high or active low?
	unsigned debounceSamps;		// number of samples to debounce for
	unsigned counts;			// number of continuous samples in active state
	unsigned bank;
	unsigned pin;
	unsigned state;				// on(1) or off(0)
	unsigned holdRepeat;		// how many more samples before incrementing again
};

#endif
