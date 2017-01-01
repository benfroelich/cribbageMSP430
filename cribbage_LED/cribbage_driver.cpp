#include "msp430.h"
#include "InputHandler.h"
#include "USCII2C.h"
#include "cribbage_LED.h"
#include <cassert>
#include <stdio.h>

// define the externally declared I2C object here to claim it
IO::USCI_I2C IO::i2c;

char Cribbage::Player::numPlayers = 0;
// todo: what was this for?
Cribbage::Player players_g[Cribbage::MAX_PLAYERS];
Cribbage::Player::Player()
{
	this->score = 0;
	// set player ID to static player count and increment player count
	this->pNum = numPlayers++;
}
Cribbage::DisplayDriver::DisplayDriver() : F_I2C(100e3)
{
	// set up variables
	this->initialized = false;
	clear();
	disable();
}
void Cribbage::DisplayDriver::setupHW(double F_MCLK)
{
	// initialize the I2C driver to 100kHz
	IO::i2c.init(F_MCLK, 100e3, BASE_I2C_ADDR, &P1SEL1, (BIT6 | BIT7));
	this->initialized = true;
	checkHW();
}
bool Cribbage::DisplayDriver::checkHW()
{
	bool LEDDriverCommEstablished[NUM_LED_DRIVERS];
	bool stat = true;
	if(!initialized) setupHW();
	for(unsigned bank = 0; bank < NUM_LED_DRIVERS; bank++)
	{
		LEDDriverCommEstablished[bank] = true;
		if(!IO::i2c.checkAddr(BASE_I2C_ADDR + bank))
		{
			LEDDriverCommEstablished[bank] = false;
			printf("checkHW: error! could not establish "
					"communication with driver %d\n", bank);
			stat = false;
		}
		else
			printf("checkHW: established "
					"communication with driver %d\n", bank);
	}
	return stat;


}
void Cribbage::DisplayDriver::clear()
{
	// set gpio expanders to all low or tri-state
	for(unsigned int i=0; i<NUM_LEDS; i++) this->LEDStates[i] = false;
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
	assert(LED<NUM_LEDS);
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
	if(UP->read() || RIGHT->read())
	{
		if(ctrlr.numPlayersChosen < MAX_PLAYERS)
			ctrlr.numPlayersChosen++;
	}
	// decrement num players if DOWN or LEFT
	else if(DOWN->read() || LEFT->read() || BACK->read())
	{
		if(ctrlr.numPlayersChosen > MIN_PLAYERS)
			ctrlr.numPlayersChosen--;
	}
	// let's start the game!
	else if(ENTER->read())
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
	// assign the class's player pointers to the global player array
	for(int plr=0; plr < MAX_PLAYERS; plr++)
	{
		this->players[plr] = &players_g[plr];
	}
}
void Cribbage::Controller::sysInit(double F_MCLK)
{
	// initialize the driver HW resources
	ui.setupHW(F_MCLK);
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
void Cribbage::Turns::enter(Controller& ctrlr)
{
	// errrrm do nothing fer now
}
Cribbage::State *Cribbage::Turns::handleInput(Controller& ctrlr)
{
	return &ctrlr.turns;
}
void Cribbage::Turns::update(Controller& ctrlr)
{

}








