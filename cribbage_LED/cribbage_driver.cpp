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
	this->LEDStates[LED] = on;
}
void Cribbage::DisplayDriver::enable()
{
	this->enabled = true;
}
void Cribbage::DisplayDriver::disable()
{
	this->enabled = false;
}

Cribbage::Controller::Controller()
{

}
void Cribbage::Controller::init()
{
	ui.clear();
	for(unsigned plr = 0; plr < MIN_PLAYERS; plr++)
	{
		// light up the minimum number of players
		ui.setPlayer(plr);
		ui.setCurrScore(1);
	}

}
