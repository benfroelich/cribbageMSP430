/* 	USCII2C.cpp
 *  Created on: Dec 26, 2016
 *      Author: benny
 */
// include msp430 header to get access to USCI registers
#include "msp430.h"
#include "USCII2C.h"
#include <cassert>
#include <stdio.h>

IO::USCI_I2C::USCI_I2C()
{
	this->state = IDLE;
}
void IO::USCI_I2C::init(const uint32_t F_MCLKusci, const uint32_t F_I2C, uint8_t defaultAddress,
		unsigned busyCnts, volatile unsigned char *SEL_PORT, uint8_t PINS)
{
	this->busyCnts = busyCnts;
	if(this->busyCnts == 0) this->busyCnts = 1;
	this->defAddr = defaultAddress;
	// configure I2C pins, e.g. P1SEL1 |= (BIT6 | BIT7)
	*SEL_PORT |= PINS;
	// put eUSCI_B in reset state while we config it
	UCB0CTLW0 = UCSWRST;
	// use SMCLK as clock source, I2C Mode, send I2C stop
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK;
	//assert(F_MCLK/F_I2C > 1);
	// TODO: solve stack overflow :(
	// UCB0BRW = (uint16_t)F_MCLKusci/F_I2C;	// set I2C frequency
	UCB0BRW = (uint16_t)(8000000/F_I2C);	// set I2C frequency
	UCB0I2CSA = defaultAddress; 	// client address
	//UCB0CTLW1 |= UCCLTO_3;	// timeout after ~34ms
	UCB0CTLW0 &= ~UCSWRST; 	// put eUSCI_B in operational state
	// enable TX interrupt and NACK interrupt
	UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE0 /*| UCCLTOIE*/;
}

void IO::USCI_I2C::waitForBusFree() {
	unsigned cnt = 0;
	while (UCB0STAT & UCBBUSY)
	{
		// increment counter
		busyCnts++;
		// if counter hits threshold, alert user
		if(cnt == busyCnts)
		{
			printf("oops! I2C bus frozen\n");
			assert(0);
		}
	}
}

bool IO::USCI_I2C::transaction(uint16_t *seq, uint16_t seqLen,
		uint8_t *recvData = NULL, uint16_t wakeupSRBits = 0)
{
	if(!done()) return false;
	// we can't start another sequence until the current one is done
	if(UCB0STAT & UCBBUSY)
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
	waitForBusFree();
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
	state = WORKING;
	// start the sequence transmission, trigger, but don't set data yet
	startSeq();
	// exit and handle the transaction from interrupts
	return true;
}
inline void IO::USCI_I2C::startWr()
{
	UCB0CTLW0 |= UCTR | UCTXSTT;
}
inline void IO::USCI_I2C::startRd()
{
	UCB0CTLW0 &= ~UCTR; UCB0CTLW0 |= UCTXSTT;
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
inline void IO::USCI_I2C::startSeq()
{
	uint16_t curSeq = seq[seqCtr];
	// 1. check for an address byte
	if(isAddr(curSeq))
	{
		UCB0I2CSA = (uint8_t)curSeq;
	}
	// 2a. check for a data read byte
	if(isRdAddr(curSeq)) startRd();
	// 2b. check for a data write byte
	else if(isWrAddr(curSeq)) startWr();
	seqCtr++;
}
inline void IO::USCI_I2C::handleTxRxInt(bool isWrInt)
{
	// TODO: make class private variable?
	uint16_t curCmd;
	uint16_t prevCmd;
	// only set prevCmd if we aren't at the beginning.
	if(seqCtr > 0) prevCmd = seq[seqCtr-1];

	// check for an impending stop or start
	// 1. STOP: end of sequence encountered
	if (seqCtr == seqLen)
	{
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
		// set status to idle so user knows we're ready for a new sequence
		this->state = IDLE;
		return;
	}
	// set current command here so we don't read outside the array
	// if we were at the end of the sequence
	curCmd = seq[seqCtr];
	// 1. check for address:
	if(isAddr(curCmd))
	{
		UCB0I2CSA = (uint8_t)curCmd;
		/* "Setting UCTXSTT generates a repeated START condition. In this case,
		 UCTR may be set or cleared to configure transmitter or receiver,
		 and a different slave address may be written into UCBxI2CSA, if
		 desired." */
		// to accomplish this, send a start write or start read command
		// this will set the UCTR flag appropriately and set the start bit
		if(isWrAddr(curCmd)) 	startWr();
		else 					startRd();
		// addr - continue processing tx
		curCmd = seq[++seqCtr];
	}
	// check for a data byte to TX/RX
	if(isWrInt)
	{
		// write data from the sequence entry to the transmitter buffer
	//	UCB0TXBUF = (uint8_t)curSeq; // causes intermittent data loss :o cmds get truncated to 8 bits!
		UCB0TXBUF = curCmd;
	}
	// this is the read interrupt handler
	else if(!isWrInt)
	{
		// TODO: grab data from register
		unsigned dataRead = UCB0RXBUF;
	}
	// increment sequence counter in preparation for the next interrupt
	seqCtr++;
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
	case USCI_I2C_UCNACKIFG:            // Vector 4: NACKIFG - client NACK'd
		UCB0CTLW0 |= UCTXSTT;           // resend start and address
		break;
	case USCI_I2C_UCCLTOIFG:    		    // Interrupt Vector: I2C Mode: UCCLTOIFG
		UCB0CTLW0 |= UCSWRST;
		UCB0CTLW0 &= ~UCSWRST;
		break;
	case USCI_I2C_UCSTTIFG:  break;     // Vector 6: STTIFG
	case USCI_I2C_UCSTPIFG:  break;     // Vector 8: STPIFG
	case USCI_I2C_UCRXIFG3:  break;     // Vector 10: RXIFG3
	case USCI_I2C_UCTXIFG3:  break;     // Vector 12: TXIFG3
	case USCI_I2C_UCRXIFG2:  break;     // Vector 14: RXIFG2
	case USCI_I2C_UCTXIFG2:  break;     // Vector 16: TXIFG2
	case USCI_I2C_UCRXIFG1:  break;     // Vector 18: RXIFG1
	case USCI_I2C_UCTXIFG1:  break;     // Vector 20: TXIFG1
	case USCI_I2C_UCRXIFG0: 		    // Vector 22: RXIFG0 - received data is ready
		IO::i2c.handleTxRxInt(false);
		break;
	case USCI_I2C_UCTXIFG0:             // Vector 24: TXIFG0
		// we completed a transaction, check for the next cmd
		IO::i2c.handleTxRxInt(true);
		break;
	default: break;
	}

	__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
}
