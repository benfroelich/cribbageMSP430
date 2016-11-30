#include <msp430.h> 
#include "cribbage_LED.h"

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}
