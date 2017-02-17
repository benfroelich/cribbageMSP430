#include "msp430.h"
#include "InputHandler.h"
#include "USCII2C.h"
#include "cribbage_LED.h"
#include "timer.h"
#include <cassert>
//#include <stdio.h>

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
//////////// begin Cribbage::DisplayDriver
Cribbage::DisplayDriver::DisplayDriver() :
		F_I2C_KHZ(100),
		F_POV(60),
		cathodeIt(0),
		newCmdReady(false)
{
	// set up variables
	this->initialized = false;
	clear();
	disable();
}
void Cribbage::DisplayDriver::setupHW(const uint16_t &F_MCLK_KHZ)
{
	// initialize the I2C driver
	IO::i2c.init(F_MCLK_KHZ, F_I2C_KHZ, BASE_I2C_ADDR, 10, &P1SEL1, (BIT6 | BIT7));
	// create a timer to update the POV display
	timer_handle = timer_create(1000/F_POV, 1000/F_POV, updateLEDPOV_CB, this);
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
			//printf("checkHW: error! could not establish "
			//		"communication with driver %d\n", bank);
			stat = false;
		}
		else {}
			//printf("checkHW: established communication with"
			//		" driver %d - it's party time!\n", bank);
	}
	return stat;
}
void Cribbage::DisplayDriver::clear()
{
	// set gpio expanders to all low, all high, or all tri-state
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
void Cribbage::updateLEDPOV_CB(void *arg)
{
	DisplayDriver * that = (DisplayDriver*)arg;
	that->setUpdateFlg();
}
void Cribbage::DisplayDriver::updateLEDMatrix()
{
	updateI2CCmd();
	wrCmdToDrvs();
}
void Cribbage::DisplayDriver::updateI2CCmd()
{
	/**
	 	 - LED's are numbered 0->479 == D1->D480
		 -calculate the LED to light up for a player:
		 	 LEDStates[playeroffset+playerScore] = true;
		 -only 1 cathode is on at a time. Filter the LEDs
		 by requiring that they be in the range
		    [cath*numAnodes, (1+cath)*numAnodes)
		 that's how to translate LED -> anode/cathode.
		 -The DrvMap_t struct contains the bank and bit for each driver.
		 to translate anode and cathode to an I2C register value:
		 	 drvBits[anodeToTurnOn.bank] |= 1 << anodeToTurnOn.ch
		 -now all we have to do is write via I2C to the drivers:
		 	 for each bank
		 	 	 write <drvBits[bank]> to <baseAddr+bank>:
		 -cycle through each cathode and enable the bits for it.
	*/
	// don't update anything if the most recent command has not been sent yet
	if(newCmdReady) return;
	// start
	unsigned LEDOffset = cathodeIt*NUM_ANODES;
	// clear drvBits array
	for(unsigned i=0; i<NUM_LED_DRIVERS; i++) drvBits[i] = 0;
	// write to the data bytes
	for(unsigned anode = 0; anode < NUM_ANODES; anode++)
	{
		if(LEDStates[LEDOffset+anode])
			drvBits[anodes[anode].bank] |= anodes[anode].ch;
		else
			drvBits[anodes[anode].bank] &= ~anodes[anode].ch;
	}
	// enable the cathode
	drvBits[cathodes[cathodeIt].bank] = cathodes[cathodeIt].ch;
	updateDrvCmd();	// translate the drvBits to the drvCmd;
	newCmdReady = true;
	cathodeIt = (cathodeIt + 1) % NUM_CATHODES; // increment the cathode iterator
}
void Cribbage::DisplayDriver::updateDrvCmd()
{
	for(unsigned cmd=0, data=0; cmd<=SEQ_LEN; cmd+=CMDS_PER_DRV, data++)
	{
		// only update the lower byte of the command, since the upper
		// byte stores command type info
		driverCmd[cmd] |= (uint8_t)drvBits[data];
	}
}
void Cribbage::DisplayDriver::wrCmdToDrvs()
{
	// if the i2c bus is ready to take the transaction, update the newCmdReady variable
	newCmdReady = !IO::i2c.transaction(driverCmd, SEQ_LEN, 0, 0);
}
//////////// begin Cribbage::UI
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
void Cribbage::Controller::sysInit(const uint16_t &F_MCLK_KHZ)
{
	// initialize the driver HW resources
	ui.setupHW(F_MCLK_KHZ);
}
void Cribbage::Controller::run()
{
    if(1/*gameFlag*/)
    {
        assert(currState);
        enter();
        handleInput();
        update();
        // update states
        prevState = currState;
        currState = nextState;
        nextState = 0;	// current state must update next state!
    }
    this->ui.updateLEDMatrix();
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
	// start at the first player
	curPlrNum = 0;
	for(unsigned i=0; i<ctrlr.numPlayersChosen; i++)
	{
		ctrlr.players[i]->setPts(0);
	}
}
Cribbage::State *Cribbage::Turns::handleInput(Controller& ctrlr)
{
	// capture current inputs
	if(RIGHT->read())
	{
		curPlrNum = (curPlrNum + 1) % ctrlr.numPlayersChosen;
	}
	if(LEFT->read())
	{
		curPlrNum = (curPlrNum - 1) % ctrlr.numPlayersChosen;
	}
	return &ctrlr.turns;
}
void Cribbage::Turns::update(Controller& ctrlr)
{

}








