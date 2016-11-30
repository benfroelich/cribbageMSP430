// cribbage LED matrix driver
// Ben Froelich 2016-11-19
#ifndef CRIBBAGE_LED_H
#define CRIBBAGE_LED_H
#include "gameframe.h"

namespace Cribbage
{
	class InputHandler // TODO: : public IO
	{
	public:
		typedef enum Input_t {UP, DOWN, RIGHT, LEFT, BACK, ENTER} Input_t;
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

	class Controller: private Game::Frame
	{
	public:
		Controller();
		void init();
	private:
		UI ui;
	};
};	// namespace Cribbage
#endif
