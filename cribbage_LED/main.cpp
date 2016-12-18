#include <msp430.h> 
#include "cribbage_LED.h"

IO::InputPin
		UP		(1, 1, 0, IO::PULLUP::UP, 2, 64),
		DOWN	(4, 5, 0, IO::PULLUP::UP, 4, 64),
		RIGHT	(1, 2, 0, IO::PULLUP::UP, 4, 64),
		LEFT	(1, 3, 0, IO::PULLUP::UP, 4, 64),
		BACK	(1, 4, 0, IO::PULLUP::UP, 4, 128),
		ENTER	(1, 5, 0, IO::PULLUP::UP, 4, 64);

// local function declarations
// setup timers
void setUpTimers(const double F_MCLK, const double F_PIN_INTERRUPT);

int main(void) {
	const double F_MCLK = 1e6;	// run master clock at 10MHz
	const double F_PIN_INTERRUPT = 200; // pin interrupt at 100Hz
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
    // setup timers for input debouncing
    setUpTimers(F_MCLK, F_PIN_INTERRUPT);
    // init inputs
    UP.init();
	DOWN.init();
	RIGHT.init();
	LEFT.init();
	BACK.init();
	ENTER.init();
	P1DIR |= BIT0;
	__enable_interrupt();
	while(1)
	{
		//_delay_cycles(10000);
		if(UP.read()) P1OUT ^= BIT0;
	}
	Cribbage::Controller game;
	game.run();
	return 0;
}

void setUpTimers(double F_MCLK, double F_PIN_INTERRUPT)
{
	TA0CCR0 = F_MCLK/8/F_PIN_INTERRUPT;
	// SMCLK, /8, count to CCR0, enable interrupts
	TA0CTL = TASSEL__SMCLK | ID_3 | MC__UP | TAIE;
	// enable interrupt for TA0 CCR0
	TA0CCTL0 = CCIE;
}
// ISR's
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// check for debounce interrupt TA0IV;
	if(TA0IV & TA0IV_TAIFG)
	{
		UP.debounce();
//		DOWN.debounce();
//		RIGHT.debounce();
	}
}
