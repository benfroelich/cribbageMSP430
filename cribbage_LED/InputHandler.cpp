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
	pu(pull),
	dbncState(IO::PINSTATE::OFF),
	pinHist(0x0000)
{
	assert(port<=NUM_PORTS && port>0);
	reg = &PORTS[port-1];
	bm = 1<<pin;
	// fill every bit of the debounce mask with
	// a pattern to match the history of the pin with
	// e.g. activeSamps == 3 -> dbncPtrn == 0b0000000000000111
	bool lvl;
	for(int i = sizeof(pinHist_t)*8; i>=0; i--)
	{
		// level is XOR of polarity and active samples
		lvl = polarity != (i >= activeSamps);
		// shift everything over once
		dbncPtrn = dbncPtrn << 1;
		// and stuff the bit into the debouncePattern
		dbncPtrn |= lvl;
		// also avoid false triggers by setting the pinHist to all off
		pinHist |= pinHist << 1 | !polarity;
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
	// read the current pin level (1/0)
	bool newLvl = *reg->in & bm ? true : false;
	// store the state read
	pinHist = newLvl | (pinHist << 1);
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
	// check for inverse of pattern (button released)
	else if(pinHist == (pinHist_t)~dbncPtrn)
	{
		state = dbncState = PINSTATE::OFF;
		return true;
	}
	return false;
}
bool IO::InputPin::checkForRepeat()
{
	// if the state is still active (ON/REPEAT), increment the counter
	if(state) counts++;
	else
	{
		if(counts > 1)
			counts--;
		return false;
	}
	// if the count is high enough, set the debouncedState to true again
	// this allows checking for held presses
	// for the first press, wait until holdRepeat == counts
	if(counts == holdRepeat)
	{
		state = dbncState = PINSTATE::REPEAT;
		counts = 0;
		return true;
	}
	if(counts == holdRepeat>>2)
	{
		// if we've already entered the REPEAT state, flag repeat at 8x the repeatCount speed
		if(state == PINSTATE::REPEAT)
		{
			state = dbncState = PINSTATE::REPEAT;
			counts = 0;
			return true;
		}
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
