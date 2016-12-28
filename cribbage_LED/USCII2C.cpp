/* 	USCII2C.cpp
 *  Created on: Dec 26, 2016
 *      Author: benny
 */
// include msp430 header to get access to USCI registers
#include "msp430.h"
#include "USCII2C.h"
#include <cassert>

namespace IO
{
	USCI_I2C::USCI_I2C()
	{
		this->state = IDLE;
	}
	void USCI_I2C::init(double F_MCLK, double F_I2C, unsigned I2CAddr)
	{
		this->I2CAddr = I2CAddr;
		// put eUSCI_B in reset state while we config it
		UCB0CTLW0 |= UCSWRST;
		// use SMCLK as clock source, I2C Mode, send I2C stop
		UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK;
		UCB0BRW = F_MCLK/F_I2C;	// set I2C frequency
		//UCB0CTLW1 |= UCASTP_2; 	// automatic STOP assertion
		UCB0TBCNT = 0x01; 		// TX this many bytes of data
		UCB0I2CSA = I2CAddr; 	// client address
		P1SEL1 |= BIT6 | BIT7; 	// configure I2C pins
		UCB0CTL1 &= ~UCSWRST; 	// put eUSCI_B in operational state
		// enable TX interrupt and NACK interrupt
		UCB0IE |= UCTXIE0 | UCNACKIE;
	}
	// I2C ISR
	// address I2C interrupts here, including updating the
	// transmission buffer register with the next byte
	// to send
	#pragma vector=USCI_B0_VECTOR
	__interrupt void EUSCI_B0(void)
	{
		switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
		{
		case USCI_NONE:          break;         // Vector 0: No interrupts
		case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
		case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
			UCB0CTLW0 |= UCTXSTT;                 // resend start if NACK
			break;
		case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
		case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
		case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
		case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
		case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
		case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
		case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
		case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
		case USCI_I2C_UCRXIFG0:  break;         // Vector 22: RXIFG0
		case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
			UCB0CTLW0 |= UCTXSTP;               // I2C stop condition
			UCB0IFG &= ~UCTXIFG;                // Clear USCI_B0 TX int flag
			__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
			break;
		default: break;
		}
		// dummy i2c transmission
		// set transmit flag and send a start

		UCB0TXBUF = 0xBE; // send a dummy value
        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
	}

} /* namespace IO */
