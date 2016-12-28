/*	USCII2C.h
 *  Created on: Dec 26, 2016
 *      Author: benny		 */

#ifndef USCII2C_H_
#define USCII2C_H_

namespace IO
{
	class USCI_I2C
	{
	public:
		USCI_I2C();
		// initialize the I2C hardware and member variables
		// currently just use MCLK as the clock source
		void init(double F_MCLK, double F_I2C, unsigned I2CAddr);
		// Use this to check whether a previously scheduled I2C sequence has been fully processed.
		inline bool done() { return(state == IDLE); };
	private:
		// used by the state machine
		enum {	IDLE = 0,
				START = 2,
				PREPARE_ACKNACK = 4,
				HANDLE_RXTX = 6,
				RECEIVED_DATA = 8,
				PREPARE_STOP = 10,
				STOP = 12
		} state;
		unsigned char I2CAddr;
	};
	// externally defined object required for use in the interrupt
	extern USCI_I2C i2c;
} /* namespace IO */

#endif /* USCII2C_H_ */
