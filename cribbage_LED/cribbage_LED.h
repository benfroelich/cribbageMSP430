// cribbage LED matrix driver
// Ben Froelich 2016-11-19
#ifndef CRIBBAGE_LED_H
#define CRIBBAGE_LED_H

namespace Cribbage
{
// constants
const int numLEDs = 480;

// user interface class
class DisplayDriver
{
public:
	DisplayDriver();
	void clear();
	void clear(int LED);
	void set(int LED);
private:
	// LED data structures
	typedef struct LEDPin_t
	{
		int bank;	// IO expander that this LED pin is connected to
		int ch;		// IO expander channel that this LED pin is connected to
	} LEDPin_t;
	class LED
	{
	public:
		LED();
		LEDPin_t anode, cathode;
	};
	static const LED LED[numLEDs];
};

};	// namespace Cribbage
#endif
