#include <msp430.h>
#include <cassert>
#include "IOHandler.h"
#include "InputHandler.h"

// define the input pins pointer registry
IO::InputPin *IO::InputPin::pins[7] = {0};
// initialize static number of input pins
unsigned IO::InputPin::numInputPins = 0;

IO::InputPin::InputPin(unsigned port, unsigned pin, bool polarity, PULLUP::PULLUP pull,
		double tOn_ms, double tOff_ms, double tRepeat_ms, double tInt) :
	port(port),
	pin(pin),
	polarity(polarity),
	dbncCnts(0),		// init samp cnt
	state(PINSTATE::OFF),		// invalid state
	pu(pull),
	dbncState(IO::PINSTATE::OFF),
	immState(false)
{
	// calculate dbncCnts required to meet timing for tOn, tOff, and tRepeat
	onCnt = tOn_ms/tInt;
	offCnt = tOff_ms/tInt;
	repCnt = tRepeat_ms/tInt;
	// setup port pointers
	assert(port<=NUM_PORTS && port>0);
	reg = &PORTS[port-1];
	// setup pin bitmask
	bm = 1<<pin;
	// add input pin to the registry
	pins[numInputPins++] = this;
};
IO::InputPin::InputPin(unsigned port, unsigned pin, bool polarity, PULLUP::PULLUP pull) :
	port(port),
	pin(pin),
	polarity(polarity),
	dbncCnts(100),		// init samp cnt
	state(PINSTATE::OFF),		// invalid state
	pu(pull),
	dbncState(IO::PINSTATE::OFF),
	immState(false)
{
	// setup port pointers
	assert(port<=NUM_PORTS && port>0);
	reg = &PORTS[port-1];
	// setup pin bitmask
	bm = 1<<pin;
	// add input pin to the registry
	pins[numInputPins++] = this;
};
void IO::InputPin::init()
{
	// disable any special function on the pin
	*(reg->sel0) &= ~bm;
	// set direction to input
	*(reg->dir) &= ~bm;
	// set pullup
	switch(pu)
	{
	case PULLUP::UP:
		*(reg->ren) |= bm;
		*(reg->out) |= bm;
		break;
	case PULLUP::DOWN:
		*reg->ren |= bm;
		*reg->out &= ~bm;
		break;
	case PULLUP::OFF:
		*reg->ren &= ~bm;
		break;
	}
	// initialize state to off/inactive
	dbncState = state = PINSTATE::OFF;
}
void IO::InputPin::init(double tOn_ms, double tOff_ms, double tRepeat_ms, double tInt_ms)
{
	// calculate dbncCnts required to meet timing for tOn, tOff, and tRepeat
	onCnt = tOn_ms/tInt_ms;
	offCnt = tOff_ms/tInt_ms;
	repCnt = tRepeat_ms/tInt_ms;
	// check all the counts to make sure they are >= 1;
	// this indicates that the waits are too short and or
	// that the interrupt period is too long.
	assert(onCnt);
	assert(offCnt);
	assert(repCnt);
	init();
}
// implementation of inline functions
inline void IO::InputPin::debounce()
{
	// read the current pin level (1/0) and convert to a state (ON/OFF)
	immState = *reg->in & bm ? polarity : !polarity;
	// store the level read
	evaluate();
}
inline void IO::InputPin::evaluate()
{
	// evaluate the new state depending upon our current state
	switch(state)
	{
	case PINSTATE::OFF:
		// state is different, increment dbncCnts
		if(immState) dbncCnts--;
		else dbncCnts = offCnt;
		// if we've counted all the way down, enter the ON state
		if(dbncCnts==0)
		{
			state = dbncState = PINSTATE::ON;
			dbncCnts = onCnt;
			// reset repeat counter
			// set the first delay to be 4x longer than the next ones
			repCnts = repCnt<<2;
		}
		break;
	case PINSTATE::ON:
	case PINSTATE::REPEAT:
		// if the state doesn't match, count down on the debounce counter
		if(!immState)
		{
			// count down the debounce counter
			dbncCnts--;
			// reset repeat counter
			// set the first delay to be 4x longer than the next ones
			if(state == PINSTATE::ON)
			{
				repCnts = repCnt<<2;
			}
			else
			{
				repCnts = repCnt;
			}
		}
		else	// pin is in same state
		{
			// reset dbncCnts
			dbncCnts = onCnt;
			// count down the repeat counter
			repCnts--;
		}
		// switch state when the pin has become stable for dbncCnts
		if(dbncCnts==0)
		{
			state = dbncState = PINSTATE::OFF;
			dbncCnts = offCnt;
		}
		// trigger a repeat when the repeat counts have reached 0
		else if(repCnts == 0)
		{
			// if we've reached a repeat, retrigger the repeat state
			state = dbncState = PINSTATE::REPEAT;
			// and reset the repeat count so we can run again
			repCnts = repCnt;
		}
		break;
	default:
		assert(0);	// bad state
	}
}
void IO::InputPin::fromInterrupt()
{
	// debounce all input pins
	for (unsigned i = 0; i< numInputPins; i++)
	{
		pins[i]->debounce();
	}
}
IO::PINSTATE::PINSTATE IO::InputPin::read()
{
	// store the current state of the pin
	static PINSTATE::PINSTATE retState;
	retState = dbncState;
	// reset the debounced state so that there is only 1 read per button press/repeated press hold cycle
	dbncState = PINSTATE::OFF;
	return retState;
}
