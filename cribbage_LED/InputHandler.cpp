#include <msp430.h>
#include <cassert>
#include "InputHandler.h"

IO::PORTS_T IO::PORTS[4] = {
		{&P1SEL0, &P1REN, &P1IN, &P1DIR, &P1OUT},
		{&P2SEL0, &P2REN, &P2IN, &P2DIR, &P2OUT},
		{&P3SEL0, &P3REN, &P3IN, &P3DIR, &P3OUT},
		{&P4SEL0, &P4REN, &P4IN, &P4DIR, &P4OUT} };

IO::InputPin::InputPin(unsigned port, unsigned pin, bool polarity,
	PULLUP::PULLUP pull, unsigned activeSamps, unsigned holdRepeat) :
	port(port),
	pin(pin),
	polarity(polarity),
	holdRepeat(holdRepeat),
	counts(0),		// init samp cnt
	state(PINSTATE::OFF),		// invalid state
	pu(pull)
{
	assert(port<=NUM_PORTS && port>0);
	reg = &PORTS[port-1];
	bm = 1<<pin;
	// fill every bit of the debounce mask with
	// a pattern to match the history of the pin with
	// e.g. activeSamps == 3 -> dbncPtrn = 0b00000111
	for(unsigned i=0; i<sizeof(dbncPtrn)*8; i++)
	{
		bool lvl = polarity && (i > activeSamps);
		// stuff the bit into the debouncePattern
		dbncPtrn |= lvl;
		// and shift everything over once
		dbncPtrn = dbncPtrn << 1;
	}
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
void IO::InputPin::debounce()
{
	// read the current value on the pin and convert from a
	// level (1/0) to a state (on/off or active/inactive)
	bool newState = *reg->in & bm ? polarity : !polarity;
	// store the state read
	pinHist = newState | (pinHist << 1);
	if(!checkForTransition()) checkForRepeat();
}
bool IO::InputPin::checkForTransition()
{
	// check for a matching debounce pattern
	if(pinHist == dbncPtrn)
	{
		state = dbncState = PINSTATE::ON;
		return true;
	}
	return false;
}
bool IO::InputPin::checkForRepeat()
{
	// if the state is still active (ON/REPEAT), increment the counter
	if(state) counts++;
	else return false;
	// if the count is high enough, set the debouncedState to true again
	// this allows checking for held presses
	if(counts == holdRepeat)
	{
		state = dbncState = PINSTATE::REPEAT;
		holdRepeat = 0;
		return true;
	}
	// no repeat detected?
	return false;
}
IO::PINSTATE::PINSTATE IO::InputPin::read()
{
	// store the current state of the pin
	PINSTATE::PINSTATE retState = dbncState;
	// reset the debounced state so that there is only 1 read per button press/repeated press hold cycle
	dbncState = PINSTATE::OFF;
	return retState;
}
