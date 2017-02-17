/*
 * IOHandler.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: benny
 */

#include "IOHandler.h"
#include <msp430.h>
namespace IO {
// set up the ports registry for all the available ports on the target
PORTS_T PORTS[] = {
#ifdef __MSP430_HAS_P1SEL0__	// only declare if the port exists
		{&P1SEL0, &P1REN, &P1IN, &P1DIR, &P1OUT},
#endif
#ifdef __MSP430_HAS_P2SEL0__	// only declare if the port exists
		{&P2SEL0, &P2REN, &P2IN, &P2DIR, &P2OUT},
#endif
#ifdef __MSP430_HAS_P3SEL0__	// only declare if the port exists
		{&P3SEL0, &P3REN, &P3IN, &P3DIR, &P3OUT},
#endif
#ifdef __MSP430_HAS_P4SEL0__	// only declare if the port exists
		{&P4SEL0, &P4REN, &P4IN, &P4DIR, &P4OUT},
#endif
};
// capture the number of ports
const unsigned NUM_PORTS = sizeof(PORTS)/sizeof(PORTS[0]);

} /* namespace IO */
