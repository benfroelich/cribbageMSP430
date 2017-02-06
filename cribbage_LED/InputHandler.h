#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H
#include <cassert>

namespace IO
{
	struct PORTS_T;
	namespace PULLUP
	{
		enum PULLUP {UP, DOWN, OFF};	// type of pullup
	}
	// return the state of the pin
	namespace PINSTATE
	{
		enum PINSTATE {OFF=0, ON, REPEAT};
	}
	// this class configures and debounces input pins
	class InputPin
	{
	public:
		InputPin(unsigned port, unsigned pin, bool polarity, PULLUP::PULLUP pull,
				double tOn_ms, double tOff_ms, double tRepeat_ms, double tInt_ms);
		// no timing setup, moved to init
		InputPin(unsigned port, unsigned pin, bool polarity, PULLUP::PULLUP pull);
		// call in the setup routine
		void init();
		void init(double tOn_ms, double tOff_ms, double tRepeat_ms, double tInt_ms);
		// return the current debounced state of the pin
		PINSTATE::PINSTATE read();
		PORTS_T *reg;	// MSP specific, maybe move to a inheriting class?
		int bm;	// bitmask == 1<<pin
		// exit LMP when button press received
		bool exitLPMOnPress(bool enable = true);	// TODO: implement
		// call in the timer ISR
		void debounce();
	private:
		void evaluate();			// evaluate the newly read state
		bool polarity;				// active high or active low?
		//	settings that will be counted to to trigger a state change
		unsigned onCnt, offCnt, repCnt;
		unsigned dbncCnts, repCnts;			// number of the same samples in active state
		bool immState;				// non-debounced pin state read in interrupt
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
	public:
		// function to update all input pins from a timer ISR
		static void fromInterrupt();
	private:
		// allocate enough storage
		static InputPin *pins[7];
		static unsigned numInputPins;
	};
};


#endif
