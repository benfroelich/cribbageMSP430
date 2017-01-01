/* 	USCII2C.cpp
 *  Created on: Dec 26, 2016
 *      Author: benny
 */
// include msp430 header to get access to USCI registers
#include "msp430.h"
#include "USCII2C.h"
#include <cassert>

IO::USCI_I2C::USCI_I2C()
{
	this->state = IDLE;
}
void IO::USCI_I2C::init(double F_MCLK, double F_I2C, uint8_t defaultAddress,
		volatile unsigned char *SEL_PORT, uint8_t PINS)
{
	this->defAddr = defaultAddress;
	// configure I2C pins, e.g. P1SEL1 |= (BIT6 | BIT7)
	*SEL_PORT |= PINS;
	if(UCB0STAT & UCBUSY)
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
	// put eUSCI_B in reset state while we config it
	UCB0CTLW0 = UCSWRST;
	// use SMCLK as clock source, I2C Mode, send I2C stop
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK;
	assert(F_MCLK/F_I2C > 1);
	UCB0BRW = F_MCLK/F_I2C;	// set I2C frequency
	// TX this many bytes of data
	// this is a typical transmission length for I2C
	// w/ 8-bit registers
	//UCB0TBCNT = 0x01;
	UCB0I2CSA = defaultAddress; 	// client address
	UCB0CTLW0 &= ~UCSWRST; 	// put eUSCI_B in operational state
	UCB0IE |= UCTXIE0 | UCNACKIE;
		UCB0CTLW0 |= UCTXSTP;

	// enable TX interrupt and NACK interrupt
}
void IO::USCI_I2C::transaction(uint16_t *seq, uint16_t seqLen,
		uint8_t *recvData, uint16_t wakeupSRBits)
{
	// we can't start another sequence until the current one is done
	if(UCB0STAT & UCBUSY)
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
	while(UCB0STAT & UCBUSY);
	// load the sequence into the library:
	assert(seq);	// ensure we have a sequence
	this->seq = seq;
	assert(seqLen);	// ensure we have a seqLength
	this->seqLen = seqLen;
	// no assert, could be a null ptr iff only writing
	this->recvData = recvData;
	// no assert, could not be waking up
	this->wakeupSRBits = wakeupSRBits;
	// update status
	seqCtr = 0;
	state = START;
	handleSeq();	// start the sequence transmission
	// exit and handle the transaction from interrupts
}
bool IO::USCI_I2C::checkAddr(uint8_t addr)
{
	uint8_t clientAddrBak, UCB0IEBak;
	bool present;
	UCB0IEBak = UCB0IE;                      // restore old UCB0I2CIE
	clientAddrBak = UCB0I2CSA;                   // store old slave address
	UCB0IE &= ~ UCNACKIE;                    // no NACK interrupt
	UCB0I2CSA = addr;                  // set slave address
	UCB0IE &= ~(UCTXIE0 | UCRXIE0);              // no RX or TX interrupts
	// disable interrupts so we can handle all interrupt flags here
	// and not run any of the ISR code
	__disable_interrupt();
	UCB0CTLW0 |= UCTR | UCTXSTT | UCTXSTP;       // I2C TX, start condition
	while (UCB0CTLW0 & UCTXSTP);                 // wait for STOP condition
	UCB0CTLW0 |= UCTXSTP;
	present = !(UCB0IFG & UCNACKIFG);
	UCB0IFG = 0x00;		// clear the interrupts
	__enable_interrupt();
	UCB0I2CSA = clientAddrBak;                   // restore slave address
	UCB0IE = UCB0IEBak;                   // restore interrupts
	return present;
}
inline void IO::USCI_I2C::handleSeq()
{
	// check for end of sequence
	if (seqCtr==seqLen)
	{
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
		// set status to idle
		this->state = IDLE;
		// TODO: disable interrupts until a new sequence is loaded?
		return;
	}
	// otherwise, check the current sequence
	// 1. check for an address change
	if(seq[seqCtr] & ADDR)
	{
		UCB0I2CSA = (uint8_t)seq[seqCtr++];
	}
	// 2. initiate transaction if it's the first transaction
	if(seqCtr == 0)
	{
		UCB0CTLW0 |= UCTR | UCTXSTT;
	}
	// 3a. check for a data read byte
	if(seq[seqCtr] & READ)
	{
		// load data from bus
		// TODO - implement reads, set controller
		// to a byte from the client selected
		assert(0);
		// try this:
		// 1) UCTR=0 (Receiver) 2) UCTXSTT=1
	}
	// 3b. check for a data write byte
	else if(seq[seqCtr] & WRITE)
	{
		/* "After initialization, master transmitter mode is initiated
		 * by writing the desired slave address to the UCBxI2CSA
		 * register, selecting the size of the slave address with the
		 * UCSLA10 bit, setting UCTR for transmitter mode, and setting
		 * UCTXSTT to generate a START condition." - SLAU367L, 26.3.5.2.1*/

		// load new byte into register
		UCB0CTLW0 |= UCTR | UCTXSTT;                 // I2C TX, start condition
		UCB0TXBUF = (uint8_t)seq[seqCtr++];
	}
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
	case USCI_NONE:          break;     // Vector 0: No interrupts
	case USCI_I2C_UCALIFG:   break;     // Vector 2: ALIFG
	case USCI_I2C_UCNACKIFG:            // Vector 4: NACKIFG
		UCB0CTLW0 |= UCTXSTT;           // resend start and address if NACK
		break;
	case USCI_I2C_UCSTTIFG:  break;     // Vector 6: STTIFG
	case USCI_I2C_UCSTPIFG:  break;     // Vector 8: STPIFG
	case USCI_I2C_UCRXIFG3:  break;     // Vector 10: RXIFG3
	case USCI_I2C_UCTXIFG3:  break;     // Vector 12: TXIFG3
	case USCI_I2C_UCRXIFG2:  break;     // Vector 14: RXIFG2
	case USCI_I2C_UCTXIFG2:  break;     // Vector 16: TXIFG2
	case USCI_I2C_UCRXIFG1:  break;     // Vector 18: RXIFG1
	case USCI_I2C_UCTXIFG1:  break;     // Vector 20: TXIFG1
	case USCI_I2C_UCRXIFG0:  break;     // Vector 22: RXIFG0
	case USCI_I2C_UCTXIFG0:             // Vector 24: TXIFG0
		// we completed a transaction, check for the next cmd
		IO::i2c.handleSeq();
		//			UCB0TXBUF = 0xBE;
		//			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
		//			UCB0IFG &= ~UCTXIFG;			// Clear USCI_B0 TX int flag
		__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
		break;
	default: break;
	}
	// dummy i2c transmission
	// set transmit flag and send a start

	//		UCB0TXBUF = 0xBE; // send a dummy value
	__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
}
