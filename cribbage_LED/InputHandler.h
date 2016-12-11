#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

namespace IO
{
	const int NUM_PORTS = 4;
	// define the a type for a pointer to an IO control register
	typedef volatile unsigned char *const IO_REG;

	struct PORTS_T
	{
		IO_REG sel, ren, in, dir, out;
	};
	extern PORTS_T PORTS[NUM_PORTS];
	enum PULLUP {UP, DOWN, OFF};	// type of pullup
	class InputPin
	{
	public:
		InputPin(unsigned port, unsigned pin, bool polarity, PULLUP pull,
				unsigned debounceSamps, unsigned holdRepeat);
		// call in the setup routine
		void init();
		// return the current debounced state of the pin
		bool read();
		PORTS_T *reg;
		int bm;	// bitmask for the pin == 1<<pin
		// exit LMP when button press received
		bool exitLPMOnPress(bool enable = true);
		void setHoldRepeat(unsigned holdRepeat)
			{this->holdRepeat = holdRepeat; counts = 0;};
		// call in the timer ISR
		void debounce();
	private:
		bool polarity;				// active high or active low?
		unsigned debounceSamps;		// number of samples to debounce for
		unsigned counts;			// number of continuous samples in active state
		unsigned port;				// port
		unsigned pin;				// pin of port
		unsigned state;				// pin state w/o debouncing (on/off)
		unsigned debouncedState;	// debounced pin state (on/off)
		// how many more samples before interrupting again
		// handles press-and-hold
		unsigned holdRepeat;
		PULLUP pu;
	};
};
#endif
