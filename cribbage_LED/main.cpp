#include <msp430.h> 
#include "cribbage_LED.h"

InputPin
		UP		(P1IN, BIT0, 64, 1024),
		DOWN		(P1IN, BIT1, 64, 1024),
		RIGHT	(P1IN, BIT2, 64, 1024),
		LEFT		(P1IN, BIT3, 64, 1024),
		BACK		(P1IN, BIT4, 64, 4096),
		ENTER	(P1IN, BIT5, 64, 1024);


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}
