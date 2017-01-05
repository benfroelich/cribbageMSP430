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
void IO::USCI_I2C::init(double F_MCLK, double F_I2C, uint8_t defaultAddress,
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
	assert(F_MCLK/F_I2C > 1);
	UCB0BRW = F_MCLK/F_I2C;	// set I2C frequency
	UCB0I2CSA = defaultAddress; 	// client address
	UCB0CTLW0 &= ~UCSWRST; 	// put eUSCI_B in operational state
	// enable TX interrupt and NACK interrupt
	UCB0IE |= UCTXIE0 | UCNACKIE;
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

void IO::USCI_I2C::transaction(uint16_t *seq, uint16_t seqLen,
		uint8_t *recvData, uint16_t wakeupSRBits)
{
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
	state = START;
	// start the sequence transmission, trigger, but don't set data yet
	startSeq();
	// exit and handle the transaction from interrupts
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
		curSeq = seq[++seqCtr];	// increment and process the next sequence entry
	}
	// 2a. check for a data read byte
	if(isRead(curSeq)) startRd();
	// 2b. check for a data write byte
	else if(isWrite(curSeq)) startWr();
}
inline void IO::USCI_I2C::handleTxRxInt(bool isWrInt)
{
	// TODO: make class private variable?
	uint16_t curCmd = seq[seqCtr];
	// use this to prepare for the next command.
	// Don't set yet because we could be at the end of the sequence.
	uint16_t nextCmd;
	//////////////////////
	// process current command:
	//////////////////////
	// check for a data write byte
	if(isWrite(curCmd))
	{
		// write data from the sequence entry to the transmitter buffer
	//	UCB0TXBUF = (uint8_t)curSeq; // causes intermittent data loss :o cmds get truncated to 8 bits!
		UCB0TXBUF = curCmd;
	}
	// check for a data read byte
	else if(isRead(curCmd))
	{
		// TODO: grab data from register
		unsigned dataRead = UCB0RXBUF;
	}

	//////////////////////
	// prepare for next command
	//////////////////////
	// check for an impending stop or start
	// 1. STOP: end of sequence encountered - check for end of sequence
	if (seqCtr == seqLen)
	{
		// send a stop
		UCB0CTLW0 |= UCTXSTP;
		// set status to idle so user knows we're ready for a new sequence
		this->state = IDLE;
		return;
	}
	// no stop yet, load the next command in the sequence
	seqCtr++;
	nextCmd = seq[seqCtr];
	// 2. RESTART w/ current address:
	// 2.a. read->write
	if(isWrite(nextCmd) & !isWrInt)
	{
		startWr();
	}
	// 2.b. write->read
	else if(isRead(nextCmd) & isWrInt)
	{
		startRd();
	}
	// 3. RESTART w/ new address
	else if(isAddr(nextCmd))
	{
		UCB0I2CSA = (uint8_t)nextCmd;
		/* "Setting UCTXSTT generates a repeated START condition. In this case,
		 UCTR may be set or cleared to configure transmitter or receiver,
		 and a different slave address may be written into UCBxI2CSA, if
		 desired." */
		// to accomplish this, send a start write or start read command
		// this will set the UCTR flag appropriately and set the start bit

		// increment counter past address cmd to get to next rd/wr command
		// (we need to peek to see if it's a read or a write).
		_delay_cycles(2000);
		nextCmd = seq[++seqCtr];
		if(isWrite(nextCmd)) startWr();
		else if(isRead(nextCmd)) startRd();
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
	case USCI_I2C_UCNACKIFG:            // Vector 4: NACKIFG - client NACK'd
		UCB0CTLW0 |= UCTXSTT;           // resend start and address
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
