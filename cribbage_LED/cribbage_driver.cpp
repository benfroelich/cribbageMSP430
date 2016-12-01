#include "cribbage_LED.h"
#include <cassert>

const int Cribbage::UI::playerOffset[MAX_PLAYERS] = {0, 120, 240, 360};

Cribbage::DisplayDriver::DisplayDriver()
{
	// set up variables
	clear();
	disable();
}
void Cribbage::DisplayDriver::clear()
{
	// set gpio expanders to all low or tri-state
	for(unsigned int i=0; i<numLEDs; i++) this->LEDStates[i] = false;
}
void Cribbage::DisplayDriver::clear(unsigned LED)
{
	ctrl(LED, false);
}
void Cribbage::DisplayDriver::set(unsigned LED)
{
	ctrl(LED, true);
}
void Cribbage::DisplayDriver::ctrl(unsigned LED, bool on)
{
	assert(LED<numLEDs);
	LEDStates[LED] = on;
}
void Cribbage::DisplayDriver::enable()
{
	enabled = true;
}
void Cribbage::DisplayDriver::disable()
{
	enabled = false;
}
void Cribbage::Init::showEnabled(unsigned pNum, Controller& ctrlr)
{
	ctrlr.ui.setPlayer(pNum);
	ctrlr.ui.setCurrScore(1);
}
void Cribbage::Init::enter(Controller& ctrlr)
{
	// set minimum number of players
	for(int plr=0; plr<MIN_PLAYERS; plr++) showEnabled(plr, ctrlr);
}
Cribbage::Controller::Controller()
{
	numPlayersChosen = 0;
}
