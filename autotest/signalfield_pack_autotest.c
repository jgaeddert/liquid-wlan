/*
 * Copyright (c) 2011 Joseph Gaeddert
 * Copyright (c) 2011 Virginia Polytechnic Institute & State University
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// signalfield_pack_autotest.c
//
// Test SIGNAL field packing/unpacking; data obtained from
// Annex G in 1999 specification (Table G.7, p. 61).
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-wlan.internal.h"

int main(int argc, char*argv[])
{
    // initialize SIGNAL structure
    struct wlan_signal_s signal_org;
    signal_org.rate   = WLAN_SIGNAL_RATE_36;    // rate
    signal_org.R      = 0;                      // reserved
    signal_org.length = 100;                    // data length
    
    // test data
    //  0   1   RATE:R1         8   0   -               16  0   LENGTH (MSB)
    //  1   0   RATE:R1         9   0   -               17  0   PARITY
    //  2   1   RATE:R1         10  1   -               18  0   SIGNAL TAIL
    //  3   1   RATE:R1         11  1   -               19  0   SIGNAL TAIL
    //  4   0   [reserved]      12  0   -               20  0   SIGNAL TAIL
    //  5   0   LENGTH(LSB)     13  0   -               21  0   SIGNAL TAIL
    //  6   0   -               14  0   -               22  0   SIGNAL TAIL
    //  7   1   -               15  0   -               23  0   SIGNAL TAIL
    //      1011 0001               0011 0000               0000 0000
    unsigned char signal_packed_test[3] = {0xb1, 0x30, 0x00};
    unsigned char signal_packed[3];
    struct wlan_signal_s signal_rec;

    unsigned int i;

    // pack signal
    wlan_signal_pack(&signal_org, signal_packed);

    // print packed message
    printf("decoded message:\n");
    for (i=0; i<3; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, signal_packed[i], signal_packed_test[i]);

    // check result
    if (count_bit_errors_array(signal_packed, signal_packed_test, 3) > 0 ) {
        fprintf(stderr,"fail: %s, packing failure\n", __FILE__);
        exit(1);
    }

    // unpack signal field
    wlan_signal_unpack(signal_packed, &signal_rec);

#if 0
    // print decoded message
    printf("decoded message:\n");
    for (i=0; i<3; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_dec[i], msg_org[i]);
#endif

    // check result
    if (signal_rec.rate   != signal_org.rate ||
        signal_rec.R      != signal_org.R    ||
        signal_rec.length != signal_org.length)
    {
        fprintf(stderr,"fail: %s, unpacking failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

