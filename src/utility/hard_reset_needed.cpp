
#include <esp8266_peri.h>

#include "EspGoodies.h"

int hard_reset_needed (void)
{
	// taken from core's Updater.cpp
	// (Esp.reset() will not reset after serial flashing)
	return ((GPI >> 16) & 0xf) != 1;
}
