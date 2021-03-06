#include <msp430.h> 
#include <intrinsics.h>
#include "InputHandler.h"
#include "cribbage_LED.h"
// TODO: remove, this is just for debugging I2C
#include "USCII2C.h"
#include "timer.h"
// include to use standard types
#include <stdint.h>
// switch to turn off use of features on the launchpad
// E.g. LED's and buttons on the launchpad
//#define LAUNCHPAD
//#define USING_CTPL

// switch to transmit a dummy transaction for debugging I2C
#define DUMMY_TX

#ifdef LAUNCHPAD
#define DUMMY_TX
// connect inputs to the two buttons on the cribbage board and also some other
// unused pins
IO::InputPin
		UP		(1, 1, 0, IO::PULLUP::UP),
		DOWN	(4, 5, 0, IO::PULLUP::UP),
		RIGHT	(1, 2, 0, IO::PULLUP::UP),
		LEFT	(1, 3, 0, IO::PULLUP::UP),
		BACK	(1, 4, 0, IO::PULLUP::UP),
		ENTER	(1, 5, 0, IO::PULLUP::UP);
#else
// connect inputs to the cribbage controller input buttons
IO::InputPin
		UP		(2, 2, 0, IO::PULLUP::UP),
		DOWN	(2, 3, 0, IO::PULLUP::UP),
		RIGHT	(2, 4, 0, IO::PULLUP::UP),
		LEFT	(2, 5, 0, IO::PULLUP::UP),
		BACK	(2, 6, 0, IO::PULLUP::UP),
		ENTER	(2, 7, 0, IO::PULLUP::UP);
#endif

// local function declarations
void setUpTimers(const uint32_t F_CLK, const uint32_t F_PIN_INTERRUPT);
void setUpPins(const uint32_t F_PIN_INTERRUPT);

Cribbage::Controller game;

int main(void)
{
	const unsigned F_PIN_INTERRUPT = 500; // pin interrupt frequency [Hz]
	const unsigned F_ACLK_KHZ = 10;
	const uint16_t F_MCLK_KHZ = 8e3;			// desired MCLK frequency [Hz]

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	// unlock clock system (CS)
	CSCTL0 = CSKEY;
	// set MCLK frequency to 8MHz (DCOFSEL = "110b = If DCORSEL = 0")
	CSCTL1 |= DCOFSEL2_L | DCOFSEL1_L;
	// VLO -> ACLK, DCO -> MCLK,
	CSCTL2 |= SELA__VLOCLK | SELM__DCOCLK;

    // set up timers for input debouncing
    setUpTimers(F_ACLK_KHZ*1000, F_PIN_INTERRUPT);
    // set up timers for general purpose function calls
    timer_init(F_MCLK_KHZ);
    // init inputs
    setUpPins(F_PIN_INTERRUPT);
	// clear lock on port settings
    PM5CTL0 &= ~LOCKLPM5;

    // setup and check HW:
    game.sysInit(F_MCLK_KHZ);
#ifdef DUMMY_TX
    uint16_t dummyTransaction[] =
    {
			0x40>>1 	| IO::USCI_I2C::ADDR_WR,	// set address
			0x01 		| IO::USCI_I2C::DATA,		// set ptr to output reg
			0xAA 		| IO::USCI_I2C::DATA,		// write to output register
			0x40>>1		| IO::USCI_I2C::ADDR_WR,	// set address
			0x03 		| IO::USCI_I2C::DATA,		// set ptr to cfg reg
			0x00		| IO::USCI_I2C::DATA,		// set all IOs as outputs

			0x46>>1 	| IO::USCI_I2C::ADDR_WR,	// set address
			0x01 		| IO::USCI_I2C::DATA,		// set ptr to output reg
			0xAA 		| IO::USCI_I2C::DATA,		// write to output register
			0x46>>1		| IO::USCI_I2C::ADDR_WR,	// set address
			0x03 		| IO::USCI_I2C::DATA,		// set ptr to cfg reg
			0x00		| IO::USCI_I2C::DATA,		// set all IOs as outputs
    };
#endif
    __enable_interrupt();
	while(1)
	{
#ifdef LAUNCHPAD
		// launchpad debugging demo
		_delay_cycles(1000);
		if(UP.read())
		{
			// invert indicator LED
			P1OUT ^= BIT0;
			// invert matrix LEDs
			dummyTransaction[2] ^= 0xFF;
			IO::i2c.transaction(dummyTransaction, sizeof(dummyTransaction)/sizeof(dummyTransaction[0]), 0, 0);
		}
		if(DOWN.read())
		{
			P4OUT ^= BIT6;
		}
		_BIS_SR(LPM0_bits);	// enter LPM0
#else
#ifdef DUMMY_TX
        IO::i2c.transaction(dummyTransaction, sizeof(dummyTransaction)/sizeof(dummyTransaction[0]), 0, 0);
        _delay_cycles(1000000);
#endif
		//game.run();
#endif	// LAUNCHPAD
	}

	return 0;
}

//////////////////////////////////////////////////////////////
// main support functions and ISRs
//////////////////////////////////////////////////////////////
void setUpTimers(uint32_t F_CLK, uint32_t F_PIN_INTERRUPT)
//void setUpTimers(const double F_CLK)
{
	TA0CCR0 = F_CLK*1000/8/F_PIN_INTERRUPT;
	TA0CCR1 = 0xFFFF;
	TA0CCR2 = 0xFFFF;
	// SMCLK, /8, count to CCR0, enable interrupts
	TA0CTL = TASSEL__ACLK | ID_3 | MC__UP | TAIE;
	// enable interrupt for TA0 CCR0
	TA0CCTL0 = CCIE;
}
void setUpPins(const uint32_t F_PIN_INTERRUPT)
{
	double t_int_ms = 1.0 / (double)F_PIN_INTERRUPT * 1000.0;
	// link the pins defined here to cribbage library,
	// this allows the cribbage board to use the pins
	// for game control
	Cribbage::UP = &UP;
	Cribbage::DOWN = &DOWN;
	Cribbage::RIGHT = &RIGHT;
	Cribbage::LEFT = &LEFT;
	Cribbage::BACK = &BACK;
	Cribbage::ENTER = &ENTER;

	// initialize the input pins
	UP.init(	25.0,	3.0,	100.0,	t_int_ms);
	DOWN.init(	25.0,	3.0,	100.0,	t_int_ms);
	RIGHT.init(	25.0,	3.0,	100.0,	t_int_ms);
	LEFT.init(	25.0,	3.0,	100.0,	t_int_ms);
	BACK.init(	25.0,	3.0,	100.0,	t_int_ms);
	ENTER.init(	25.0,	3.0,	100.0,	t_int_ms);
#ifdef LAUNCHPAD
    // these pins are for debugging on the launchpad only
	P1DIR |= BIT0;
	P4DIR |= BIT6;
#endif
}

/// 2017-01-22: cptl library has it's own _system_pre_init()
#ifndef USING_CTPL
// disable WDT before any initialization takes place to prevent
// WDT reset during initialization of memory
int _system_pre_init(void)
{
  WDTCTL = WDTPW | WDTHOLD;
  return 1;
}
#endif

// ISR's
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
#pragma vector=TIMER0_A1_VECTOR	// why do I need this???
__interrupt void Timer_A (void)
{
	static unsigned i2cCtr = 0;
	// check for debounce interrupt TA0IV;
	if(TA0IV & TA0IV_TAIFG)
	{
		IO::InputPin::fromInterrupt();
	}
	i2cCtr++;
    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
}

