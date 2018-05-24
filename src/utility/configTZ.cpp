
#include <time.h>
#include <stdlib.h>
#include <WString.h>

#include "EspGoodies.h"

void configTZ (const __FlashStringHelper* TZ)
{
    setenv("TZ", String(TZ).c_str(), 1/*overwrite*/);
    tzset();
}
