#include <msp430.h> 
#include "cribbage_LED.h"

IO::InputPin
		UP		(1, 1, 0, IO::PULLUP::UP, 4, 1024),
		DOWN	(4, 5, 0, IO::PULLUP::UP, 4, 1024),
		RIGHT	(1, 2, 0, IO::PULLUP::UP, 4, 1024),
		LEFT	(1, 3, 0, IO::PULLUP::UP, 4, 1024),
		BACK	(1, 4, 0, IO::PULLUP::UP, 4, 4096),
		ENTER	(1, 5, 0, IO::PULLUP::UP, 4, 1024);

int main(void) {
	// link the IO pins we defined above to the library
	Cribbage::UP = &UP;
	Cribbage::DOWN = &DOWN;
	Cribbage::RIGHT = &RIGHT;
	Cribbage::LEFT = &LEFT;
	Cribbage::BACK = &BACK;
	Cribbage::ENTER = &ENTER;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	// clear lock on port settings
    PM5CTL0 &= ~LOCKLPM5;

    // init inputs
    UP.init();
	DOWN.init();
	RIGHT.init();
	LEFT.init();
	BACK.init();
	ENTER.init();
	P1DIR |= BIT0;
	while(1)
	{
		if(P1IN & BIT1) P1OUT |= BIT0;
		else P1OUT &= ~BIT0;
	}
	Cribbage::Controller game;
	game.run();
	return 0;
}

// ISR's
