#include <msp430.h> 
#include "cribbage_LED.h"

IO::InputPin
		UP		(1, 0, 0, IO::UP, 64, 1024),
		DOWN	(1, 1, 0, IO::UP, 64, 1024),
		RIGHT	(1, 2, 0, IO::UP, 64, 1024),
		LEFT	(1, 3, 0, IO::UP, 64, 1024),
		BACK	(1, 4, 0, IO::UP, 64, 4096),
		ENTER	(1, 5, 0, IO::UP, 64, 1024);


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	// init inputs
    UP.init();
	DOWN.init();
	RIGHT.init();
	LEFT.init();
	BACK.init();
	ENTER.init();

	while(1)
	{

	}
	return 0;
}

// ISR's
