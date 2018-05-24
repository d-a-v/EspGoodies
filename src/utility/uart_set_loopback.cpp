
#include <esp8266_peri.h>

#include "EspGoodies.h"

void uart_set_loopback (int uart_nr, bool enable)
{
	if (enable)
		USC0(uart_nr) |= (1 << UCLBE);
	else
		USC0(uart_nr) &= ~(1 << UCLBE);
}
