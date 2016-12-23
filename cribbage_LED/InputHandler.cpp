#include <msp430.h>
#include <cassert>
#include "InputHandler.h"

IO::PORTS_T IO::PORTS[4] = {
		{&P1SEL0, &P1REN, &P1IN, &P1DIR, &P1OUT},
		{&P2SEL0, &P2REN, &P2IN, &P2DIR, &P2OUT},
		{&P3SEL0, &P3REN, &P3IN, &P3DIR, &P3OUT},
		{&P4SEL0, &P4REN, &P4IN, &P4DIR, &P4OUT} };

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
};
void IO::InputPin::init()
{
	// disable any special function on the pin
	*(reg->sel) &= ~bm;
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
void IO::InputPin::debounce()
{
	// read the current pin level (1/0) and convert to a state (ON/OFF)
	immState = *reg->in & bm ? polarity : !polarity;
	// store the level read
	evaluate();
}
void IO::InputPin::evaluate()
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

IO::PINSTATE::PINSTATE IO::InputPin::read()
{
	// store the current state of the pin
	static PINSTATE::PINSTATE retState;
	retState = dbncState;
	// reset the debounced state so that there is only 1 read per button press/repeated press hold cycle
	dbncState = PINSTATE::OFF;
	return retState;
}
