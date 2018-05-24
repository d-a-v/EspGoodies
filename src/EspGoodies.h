
#ifndef __ESPGOODIES_H
#define __ESPGOODIES_H

#include "utility/TZ.h"

class __FlashStringHelper;

void configTZ (const __FlashStringHelper* TZ); // use only TZ-entries (FPSTR("..."))

void wifi_on();
void wifi_off();
void uart_set_loopback (int uart_nr, bool enable);
int  hard_reset_needed (void);

#endif // __ESPGOODIES
