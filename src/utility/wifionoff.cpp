
#include <user_interface.h>

#include "EspGoodies.h"

// https://github.com/esp8266/Arduino/issues/3072#issuecomment-348692479

void wifi_off (void)
{
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);
}

void wifi_on (void)
{
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
}
