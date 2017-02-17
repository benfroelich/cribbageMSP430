/*
 * IOHandler.h
 *
 *  Created on: Jan 25, 2017
 *      Author: benny
 */

#ifndef IOHANDLER_H_
#define IOHANDLER_H_
namespace IO {

	const extern unsigned NUM_PORTS;
	// define a type for a pointer to an IO control register
	typedef volatile unsigned char *const IO_REG;

	struct PORTS_T
	{
		IO_REG sel0, sel1, ren, in, dir, out;
	};
	extern PORTS_T PORTS[];

} /* namespace IO */

#endif /* IOHANDLER_H_ */
