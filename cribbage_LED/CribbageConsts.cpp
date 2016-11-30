#include "cribbage_LED.h"

namespace Cribbage
{
	// bank == I2C address
	const UI::DisplayDriver::DrvMap_t UI::DisplayDriver::anodes[UI::DisplayDriver::numAnodes] =
	{
	//		BANK	CH
			{0,		0},	// anode 0
			{0,		1},	// anode 1
			{0,		2},	// anode 2
			{0,		3},	// anode 3
			{0,		4},	// anode 4
			{0,		5},	// anode 5
			{0,		6},	// anode 6
			{0,		7},	// anode 7

			{4,		0},	// anode 8
			{4,		1},	// anode 9
			{4,		2},	// anode 10
			{4,		3},	// anode 11
			{4,		4},	// anode 12
			{4,		5},	// anode 13
			{4,		6},	// anode 14
			{4,		7},	// anode 15

			{5,		0},	// anode 16
			{5,		1},	// anode 17
			{5,		2},	// anode 18
			{5,		3},	// anode 19
			{5,		4},	// anode 20
			{5,		5},	// anode 21
			{5,		6},	// anode 22
			{5,		7},	// anode 23
	};

	const UI::DisplayDriver::DrvMap_t UI::DisplayDriver::cathodes[UI::DisplayDriver::numCathodes] =
	{
	//		BANK	CH
			{0,		3},	// cathode 0
			{0,		2},	// cathode 1
			{0,		1},	// cathode 2
			{0,		0},	// cathode 3
			{0,		4},	// cathode 4
			{0,		5},	// cathode 5
			{0,		6},	// cathode 6
			{0,		7},	// cathode 7

			{1,		0},	// cathode 8
			{1,		1},	// cathode 9
			{1,		2},	// cathode 10
			{1,		3},	// cathode 11
			{1,		4},	// cathode 12
			{1,		5},	// cathode 13
			{1,		6},	// cathode 14
			{1,		7},	// cathode 15

			{2,		0},	// cathode 16
			{2,		1},	// cathode 17
			{2,		2},	// cathode 18
			{2,		3},	// cathode 19
	};

}
