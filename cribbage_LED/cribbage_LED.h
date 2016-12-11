// cribbage LED matrix driver
// Ben Froelich 2016-11-19
#ifndef CRIBBAGE_LED_H
#define CRIBBAGE_LED_H
#include "inputHandler.h"

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
		Player* next();
	private:
		unsigned score;	// player's current score
		char pNum;	// unique player ID number
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
		virtual void enter(Controller& ctrlr);
		virtual State* handleInput(Controller& ctrlr);
		virtual void update(Controller& ctrlr);
	};
	// input pins (just placeholders for now)
	extern IO::InputPin UP, DOWN, LEFT, RIGHT, BACK, ENTER;
	class InputHandler // TODO: : public IO
	{
	public:
//		typedef enum Input_t {UP, DOWN, RIGHT, LEFT, BACK, ENTER} Input_t;
	};
	class DisplayDriver
	{
	public:
		DisplayDriver();
		void clear();
		void clear(unsigned LED);
		void set(unsigned LED);
		void ctrl(unsigned LED, bool on);
		void enable();	// enable the matrix
		void disable();	// disable the matrix
	private:
		// constants
		static const int numAnodes = 24;
		static const int numCathodes = 20;
		static const int numLEDs = numAnodes*numCathodes;
		void wrBitsISR();	// only call from ISR
		// LED data structures, static to allow access from ISR
		typedef struct DrvMap_t
		{
			char bank;	// IO expander that this LED pin is connected to
			char ch;	// IO expander channel that this LED pin is connected to
		} DrvMap_t;
		static const DrvMap_t anodes[numAnodes], cathodes[numCathodes];
		char LEDStates[numLEDs];
		bool enabled;
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
		void run();	// run a pass of the state machine
		virtual void enter();
		// figure out what the next state will be
		virtual void handleInput();
		// update the state object and the controller
		virtual void update();
	//private:
		// individual player registry
		Player players[MAX_PLAYERS];
		State *currState, *prevState, *nextState;
		Init init;	// initialization state declaration
		Turns turns; 	// turn state declaration
		UI ui;
		unsigned numPlayersChosen;
	};
};	// namespace Cribbage
#endif
