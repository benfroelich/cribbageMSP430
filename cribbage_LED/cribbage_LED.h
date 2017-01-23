// cribbage LED matrix driver
// Ben Froelich 2016-11-19
#ifndef CRIBBAGE_LED_H
#define CRIBBAGE_LED_H

// include to use standard types
#include <stdint.h>

// declare the input pin class, the cribbage library uses it
namespace IO
{
	class InputPin;
}

namespace Cribbage
{
	const unsigned MAX_PLAYERS = 4;
	const unsigned MIN_PLAYERS = 2;
	const unsigned MAX_TURNS = 120;
	class State;
	class Controller;
	class Player
	{
	public:
		Player();
		//Player* next();
	private:
		unsigned score;	// player's current score
		char pNum;	// unique player ID number
		static char numPlayers;
	};

	class Turn
	{
	public:
		Turn();
		Turn(Player* plr, unsigned score); // I don't know if this will be useful...
		Player* plr;
		unsigned score;
	};
	class Queue
	{
		Queue();
		void addTurn(Player *plr, unsigned score);
		void undo();
		void redo();
	private:
		unsigned currTurn;
		Turn turns[MAX_TURNS*MAX_PLAYERS]; // allocate space for a command history
	};
	class State
	{
	public:
		virtual void enter(Controller& ctrlr)=0;
		virtual State* handleInput(Controller& ctrlr)=0;
		virtual void update(Controller& ctrlr)=0;
	};
	// input pin access, defined in user main
	IO::InputPin * UP;
	IO::InputPin * DOWN;
	IO::InputPin * LEFT;
	IO::InputPin * RIGHT;
	IO::InputPin * BACK;
	IO::InputPin * ENTER;
	class InputHandler // TODO: : public IO
	{
	public:
	};
	// this display driver uses the I2C library to control the driver matrix
	class DisplayDriver
	{
	public:
		DisplayDriver();
		void setupHW(double F_MCLK = 8e6);	// setup I2C
		bool checkHW();
		void clear();
		void clear(unsigned LED);
		void set(unsigned LED);
		void ctrl(unsigned LED, bool on);
		void enable();	// enable the matrix
		void disable();	// disable the matrix
	private:
		// constants
		const double F_I2C;
		// 24 anode x 20 cathode matrix driven by IO expanders
		static const int NUM_ANODES = 24;
		static const int NUM_CATHODES = 20;
		static const int NUM_LEDS = NUM_ANODES*NUM_CATHODES;
		static const int NUM_LED_DRIVERS = 6;	// 6 LED drivers
		// the base I2C address for the LED drivers
		static const int BASE_I2C_ADDR = 0x40>>1;
		// the command sequence to updat the LED's will require
		// three commands per driver: ADDR, PTR, DATA
		static const int SEQ_LEN = 3*NUM_LED_DRIVERS;
		// TODO: is numLEDsPerDriver needed? implied by uint8_t?
		// 8 LED's per driver IC
		static const int NUM_LEDS_PER_DRIVER = 8;
		// update the I2C command that will be sent to the drivers periodically
		// for the persistence of vision LED matrix
		void updateI2CCmd();
		// only call from ISR to initiate the next I2C command
		void wrBitsToDrvsInISR();
		// LED data structures, static to allow access from ISR
		typedef struct DrvMap_t
		{
			char bank;	// IO expander that this LED pin is connected to
			char ch;	// IO expander channel that this LED pin is connected to
		} DrvMap_t;
		static const DrvMap_t anodes[NUM_ANODES], cathodes[NUM_CATHODES];
		uint16_t driverCmd[SEQ_LEN];
		char LEDStates[NUM_LEDS];
		uint8_t drvBits[NUM_LED_DRIVERS];
		bool enabled;
		bool initialized;
		// iterator across POV cathodes
		unsigned cathodeIt;
	};
	// user interface class that controls the display
	class UI: public DisplayDriver, public InputHandler
	{
	public:
		UI();
		void setPlayer(unsigned pNum);
		void setCurrScore(unsigned score);		// solid, even when not curr player
		void setTargetScore(unsigned score);	// flash this LED
	private:
		static const int playerOffset[MAX_PLAYERS];
		unsigned pNum;	// current player to set score for
	};
	class Init: public State
	{
	public:
		virtual void enter(Controller& ctrlr);
		virtual State* handleInput(Controller& ctrlr);
		virtual void update(Controller& ctrlr);
	private:
		void showEnabled(unsigned pNum, bool on, Controller& ctrlr);
	};
	class Turns: public State
	{
	public:
		// called only upon initial entry into state from another state
		virtual void enter(Controller& ctrlr);
		virtual State* handleInput(Controller& ctrlr);
		virtual void update(Controller& ctrlr);
	};
	class Controller
	{
	public:
		Controller();
		// init driver, game status, and hardware
		void sysInit(double F_MCLK);
		void run();	// run a pass of the state machine
		virtual void enter();
		// figure out what the next state will be
		virtual void handleInput();
		// update the state object and the controller
		virtual void update();
	//private:
		// individual player registry
		Player * players[MAX_PLAYERS];
		State *currState, *prevState, *nextState;
		// single states for pointers
		Init init;	// initialization state declaration
		Turns turns; 	// turn state declaration
		UI ui;
		unsigned numPlayersChosen;
	};
};	// namespace Cribbage
#endif
