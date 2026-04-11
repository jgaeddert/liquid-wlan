// Run-time library version numbers

#include "liquid-wlan.h"

const char liquid_wlan_version[] = "1.0.0";

const char * liquid_wlan_libversion(void)
{
    return liquid_wlan_version;
}

int liquid_wlan_libversion_number(void)
{
    return 0x010000;
}

