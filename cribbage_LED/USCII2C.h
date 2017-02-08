/*	USCII2C.h
 *  Created on: Dec 26, 2016
 *      Author: benny		 */
#ifndef USCII2C_H_
#define USCII2C_H_

// include to use standard types
#include <stdint.h>
#include "msp430.h"

namespace IO
{
	class USCI_I2C
	{
	public:
		// used as bitmasks to define what type of command is being executed
		enum CMD_TYPE {
			ADDR_RD = 	1<<8,
			ADDR_WR = 	1<<9,
			DATA = 		1<<10,
			START = 	1<<11,
			STOP = 		1<<12
		};
		// maybe try this later to replace CMD_TYPE
//		union I2C_TRANSACTION
//		{
//			uint16_t packet;
//			uint8_t data;
//			uint8_t isAddr:	1;
//			uint8_t isRd: 	1;
//			uint8_t isWr: 	1;
//		};
		USCI_I2C();
		// initialize the I2C hardware and member variables
		// currently just use MCLK as the clock source
		// busy counts will prevent an infinite loop if the bus becomes permanently busy
		// set to -1 to disable.
		void init(const uint32_t &F_MCLK, const uint32_t &F_I2C, const uint8_t &defaultAddress,
				const unsigned &busyCnts = 0, volatile unsigned char *SEL_PORT = 0,
				uint8_t PINS = 0);
		// initiate a transaction.
		// return true if the transaction successfully started.
		// return false if the bus is busy or an error occurred
		bool transaction(uint16_t *seq, uint16_t seqLen,
				uint8_t *recvData, uint16_t wakeupSRBits);
		// Use this to check whether a previously scheduled I2C sequence has been
		// fully processed.
		inline bool done() { return(state == IDLE); };
		// returns true if an acknowledge was received for the given address
		// ported from TI_USCI_I2C_slave_present
		// 	- made to work for eUSCI
		//	- replaced master-slave terminology w/ coordinator-client
		bool checkAddr(uint8_t addr);
		// used by ISR
		inline void handleTxRxInt(bool isWrInt);
		inline void startSeq();
	private:
		// check the flag set in the data packet
		inline bool isAddr(uint16_t seq)	{ return seq & (ADDR_WR | ADDR_RD); };
		inline bool isWrAddr(uint16_t seq)  { return seq & ADDR_WR; };
		inline bool isRdAddr(uint16_t seq)  { return seq & ADDR_RD; };
		inline bool isData(uint16_t seq) 	{ return seq & DATA; };
		// transmitter mode?
		inline bool isTx()	{ return UCB0CTLW0 & UCTR; };
		// start a write command (Coordinator-sender)
		inline void startWr();
		// start a read command (Coordinator-receiver)
		inline void startRd();
		inline void waitForBusFree();
		// allow user to set values if the bus is idle
		enum STATE {
			IDLE = 0,
			WORKING = 1
		} state;
		uint8_t defAddr;	// default I2C address
		uint16_t *seq;
		uint16_t seqLen, seqCtr;
		uint8_t *recvData;
		uint16_t wakeupSRBits;
		unsigned busyCnts;
	};
	// externally defined object required for use in the interrupt
	extern USCI_I2C i2c;
	/**
	 * \brief 	initialize the I2C channel
	 * \detail 	inline function to prevent stack overflow and make operation faster
	 * 			should be called infrequently anyway
	 * 	 */
	inline void IO::USCI_I2C::init(
			const uint32_t &F_MCLK,
			const uint32_t &F_I2C,
			const uint8_t &defaultAddress,
			const unsigned &busyCnts,
			volatile unsigned char *SEL_PORT,
			uint8_t PINS)
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
		UCB0BRW = (uint16_t)(F_MCLK/F_I2C);	// set I2C frequency
		UCB0I2CSA = defaultAddress; 	// client address
		//UCB0CTLW1 |= UCCLTO_3;	// timeout after ~34ms
		UCB0CTLW0 &= ~UCSWRST; 	// put eUSCI_B in operational state
		// enable TX interrupt and NACK interrupt
		UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE0 /*| UCCLTOIE*/;
	}
} /* namespace IO */

#endif /* USCII2C_H_ */
