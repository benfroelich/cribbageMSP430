#include <msp430.h>
#include <cassert>
#include "InputHandler.h"

IO::PORTS_T IO::PORTS[4] = {
		{&P1SEL0, &P1REN, &P1IN, &P1DIR, &P1OUT},
		{&P2SEL0, &P2REN, &P2IN, &P2DIR, &P2OUT},
		{&P3SEL0, &P3REN, &P3IN, &P3DIR, &P3OUT},
		{&P4SEL0, &P4REN, &P4IN, &P4DIR, &P4OUT} };

IO::InputPin::InputPin(unsigned port, unsigned pin, bool polarity,
	PULLUP pull, unsigned debounceSamps, unsigned holdRepeat) :
	port(port),
	pin(pin),
	polarity(polarity),
	holdRepeat(holdRepeat),
	debounceSamps(debounceSamps),
	counts(0),		// init samp cnt
	state(2),		// invalid state
	pu(pull)
{
	assert(port<=NUM_PORTS && port>0);
	reg = &PORTS[port-1];
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
	case UP:
		*(reg->ren) |= bm;
		*(reg->out) |= bm;
		break;
	case DOWN:
		*reg->ren |= bm;
		*reg->out &= ~bm;
		break;
	case OFF:
		*reg->ren &= ~bm;
		break;
	}
	// initialize state to off/inactive
	debouncedState = state = false;
}
void IO::InputPin::debounce()
{
	// read the current value on the pin and convert from a
	// level (1/0) to a state (on/off or active/inactive)
	bool newState = *reg->in & bm ? polarity : !polarity;

	// if the pin is not on, set status to off and return
	if(!newState)
	{
		state = debouncedState = false;
		counts = 0;
		return;
		// TODO: add provision for rejecting on->off->on glitches
	}
	// pin is active
	else
	{
		counts++;
		if(counts == debounceSamps)
		{
			debouncedState = true;
		}
		// if counts is debounceSamps+holdRepeat, set state to ON again
		else if(counts == (debounceSamps + holdRepeat))
		{
			debouncedState = true;
			counts -= holdRepeat;
		}
	}
	// store non-debounced state
	state = newState;
}
bool IO::InputPin::read()
{
	// store the current state of the pin
	bool retState = debouncedState;
	// reset the debounced state so that there is only 1 read per button press/repeated press hold cycle
	debouncedState = false;
	return retState;
}
