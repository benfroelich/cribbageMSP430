#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

namespace IO
{
	const int NUM_PORTS = 4;
	// define the a type for a pointer to an IO control register
	typedef volatile unsigned char *const IO_REG;
	// pin history data type used to store pin level for debouncing
	//typedef unsigned pinHist_t;
	typedef unsigned char pinHist_t;
	struct PORTS_T
	{
		IO_REG sel, ren, in, dir, out;
	};
	extern PORTS_T PORTS[NUM_PORTS];
	namespace PULLUP
	{
		enum PULLUP {UP, DOWN, OFF};	// type of pullup
	}
	// return the state of the pin
	namespace PINSTATE
	{
		enum PINSTATE {OFF=0, ON, REPEAT};
	}
	class InputPin
	{
	public:
		InputPin(unsigned port, unsigned pin, bool polarity, PULLUP::PULLUP pull,
				unsigned activeSamps, unsigned holdRepeat);
		// call in the setup routine
		void init();
		// return the current debounced state of the pin
		PINSTATE::PINSTATE read();
		PORTS_T *reg;
		int bm;	// bitmask for the pin == 1<<pin
		// exit LMP when button press received
		bool exitLPMOnPress(bool enable = true);
		void setHoldRepeat(unsigned holdRepeat)
			{this->holdRepeat = holdRepeat; counts = 0;};
		// call in the timer ISR
		void debounce();
	private:
		bool checkForRepeat();		// check to see if the pin has been held
		bool checkForTransition();	// check for a transition on the pin
		bool polarity;				// active high or active low?
		pinHist_t dbncPtrn;			// pattern to match for a transition detection
		pinHist_t pinHist;			// bitwise pin samples
		unsigned counts;			// number of continuous samples in active state
		unsigned port;				// port
		unsigned pin;				// pin of port
		// internal pin state
		PINSTATE::PINSTATE state;
		// pin state that is reset when the user reads the pin state
		PINSTATE::PINSTATE dbncState;
		// how many more samples before interrupting again
		// handles press-and-hold
		unsigned holdRepeat;
		PULLUP::PULLUP pu;
	};
};
#endif
