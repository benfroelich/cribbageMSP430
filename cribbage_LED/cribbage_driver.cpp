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
Cribbage::UI::UI()
{
	this->pNum = 0;
	this->clear();
	this->disable();
}
void Cribbage::UI::setPlayer(unsigned int pNum)
{
	this->pNum = pNum;
}
void Cribbage::UI::setCurrScore(unsigned score)
{
	set(playerOffset[pNum] + score);
}
void Cribbage::Init::showEnabled(unsigned pNum, bool on, Controller& ctrlr)
{
	ctrlr.ui.setPlayer(pNum);
	ctrlr.ui.setCurrScore(on);
}
void Cribbage::Init::enter(Controller& ctrlr)
{
	// set minimum number of players
	for(int plr=0; plr<MIN_PLAYERS; plr++) showEnabled(plr, true, ctrlr);
}
Cribbage::State* Cribbage::Init::handleInput(Controller & ctrlr)
{
	State * nextState = &ctrlr.init;
	// increment num players if UP or RIGHT pressed
	if(UP.read() || RIGHT.read())
	{
		if(ctrlr.numPlayersChosen < MAX_PLAYERS)
			ctrlr.numPlayersChosen++;
	}
	// decrement num players if DOWN or LEFT
	else if(DOWN.read() || LEFT.read() || BACK.read())
	{
		if(ctrlr.numPlayersChosen > MIN_PLAYERS)
			ctrlr.numPlayersChosen--;
	}
	// let's start the game!
	else if(ENTER.read())
	{
		// clear display
		for(int plr=0; plr<MAX_PLAYERS; plr++) showEnabled(plr, false, ctrlr);
		nextState = &ctrlr.turns;
	}
	return nextState;
}
void Cribbage::Init::update(Controller& ctrlr)
{
	for(unsigned plr=0; plr<MAX_PLAYERS; plr++)
	{
		showEnabled(plr, plr <= ctrlr.numPlayersChosen, ctrlr);
	}
}
Cribbage::Controller::Controller()
{
	numPlayersChosen = MIN_PLAYERS;
	currState = &init;
	prevState = nextState = 0;
}
void Cribbage::Controller::run()
{
	assert(currState);
	enter();
	handleInput();
	update();
	// update states
	prevState = currState;
	currState = nextState;
	nextState = 0;	// current state must update next state or else!!!
}
void Cribbage::Controller::enter()
{
	// only run the enter code if this is the first time thru
	if(currState != prevState && currState)
		currState->enter(*this);
}
void Cribbage::Controller::handleInput()
{
	nextState = currState->handleInput(*this);
}
void Cribbage::Controller::update()
{
	currState->update(*this);
}
