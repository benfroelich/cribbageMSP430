#include <msp430.h> 
#include <intrinsics.h>
#include "cribbage_LED.h"
// switch to turn off use of features on the launchpad
// E.g. LED's and buttons on the launchpad
#define LAUNCHPAD
#define F_PIN_INTERRUPT 500 // pin interrupt frequency [Hz]
#define F_ACLK 10e3
IO::InputPin
		UP		(1, 1, 0, IO::PULLUP::UP),
		DOWN	(4, 5, 0, IO::PULLUP::UP),
		RIGHT	(1, 2, 0, IO::PULLUP::UP),
		LEFT	(1, 3, 0, IO::PULLUP::UP),
		BACK	(1, 4, 0, IO::PULLUP::UP),
		ENTER	(1, 5, 0, IO::PULLUP::UP);

// local function declarations
// setup timers
//void setUpTimers(const double F_CLK, const double F_PIN_INTERRUPT);
void setUpTimers(const double F_CLK);
//void setUpPins(const double F_PIN_INTERRUPT);
void setUpPins();

int _system_pre_init(void)
{
  WDTCTL = WDTPW | WDTHOLD;
  return 1;
}


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	//const double F_MCLK = 8e6;			// desired MCLK frequency [Hz]


	// set MCLK frequency to 8MHz (DCOFSEL = "110b = If DCORSEL = 0")
	CSCTL1 |= DCOFSEL2_L | DCOFSEL1_L;
	// VLO -> ACLK, DCO -> MCLK,
	CSCTL2 |= SELA__VLOCLK | SELM__DCOCLK;



	// clear lock on port settings
    PM5CTL0 &= ~LOCKLPM5;
    // setup timers for input debouncing
//    setUpTimers(F_ACLK, F_PIN_INTERRUPT);
    setUpTimers(F_ACLK);
    // init inputs
//    setUpPins(F_PIN_INTERRUPT);
    setUpPins();

	__enable_interrupt();
	while(1)
	{
		//_delay_cycles(1000);
		if(UP.read())
		{
			P1OUT ^= BIT0;
		}
		if(DOWN.read())
		{
			P4OUT ^= BIT6;
		}
		_BIS_SR(LPM3_bits + GIE);	// enter LPM3
	}
//	Cribbage::Controller game;
//	game.run();
	return 0;
}

//void setUpTimers(double F_CLK, double F_PIN_INTERRUPT)
void setUpTimers(double F_CLK)
{
	TA0CCR0 = F_CLK/8/F_PIN_INTERRUPT;
	// SMCLK, /8, count to CCR0, enable interrupts
	TA0CTL = TASSEL__ACLK | ID_3 | MC__UP | TAIE;
	// enable interrupt for TA0 CCR0
	TA0CCTL0 = CCIE;
}
//void setUpPins(const double F_PIN_INTERRUPT)
void setUpPins()
{
	// connect the pins declared to the library,
	// this allows the cribbage board to use the
	// input pin pointers that it declares
	Cribbage::UP = &UP;
	Cribbage::DOWN = &DOWN;
	Cribbage::RIGHT = &RIGHT;
	Cribbage::LEFT = &LEFT;
	Cribbage::BACK = &BACK;
	Cribbage::ENTER = &ENTER;
	// initialize the input pins
    UP.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
	DOWN.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
	RIGHT.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
	LEFT.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
	BACK.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
	ENTER.init(25.0, 3.0, 100.0, 1/F_PIN_INTERRUPT*1000);
#ifdef LAUNCHPAD
    // these pins are for debugging on the launchpad only
	P1DIR |= BIT0;
	P4DIR |= BIT6;
#endif
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
		DOWN.debounce();
		RIGHT.debounce();
		LEFT.debounce();
		BACK.debounce();
		ENTER.debounce();
	}
	_BIC_SR(LPM3_EXIT); // wake up from low power mode
}
