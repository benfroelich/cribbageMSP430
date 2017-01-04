/*	USCII2C.h
 *  Created on: Dec 26, 2016
 *      Author: benny		 */
#ifndef USCII2C_H_
#define USCII2C_H_

// include to use standard types
#include <stdint.h>

namespace IO
{
	class USCI_I2C
	{
	public:
		// used as bitmasks to check addresses
		enum TRANSACTION_TYPE {
			ADDR = 1<<8,
			READ = 1<<9,
			WRITE = 1<<10
		};
		// maybe try this later to replace TRANSACTION_TYPE
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
		void init(double F_MCLK, double F_I2C, uint8_t defaultAddress,
				unsigned busyCnts = 0, volatile unsigned char *SEL_PORT = 0,
				uint8_t PINS = 0);
		//
		void transaction(uint16_t *seq, uint16_t seqLen,
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
		inline bool isAddr(uint16_t seq)	{ return seq & ADDR; };
		inline bool isWrite(uint16_t seq) 	{ return seq & WRITE; };
		inline bool isRead(uint16_t seq) 	{ return seq & READ; };
		// start a write command (Coordinator-sender)
		inline void startWr();
		// start a read command (Coordinator-receiver)
		inline void startRd();
		inline void waitForBusFree();

		// used by the state machine
		// set even values to each state to allow quick processing
		// in ISR using the __even_in_range intrinsic
		// TODO: do I need this, or was this just for USI?
		enum STATE {
			IDLE = 0,
			START = 2,
			PREPARE_ACKNACK = 4,
			HANDLE_RXTX = 6,
			RECEIVED_DATA = 8,
			PREPARE_STOP = 10,
			STOP = 12
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
} /* namespace IO */

#endif /* USCII2C_H_ */
