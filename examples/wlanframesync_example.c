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
// wlanframesync_example.c
//
// Test generation/synchronization of wlan frame
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.h"

#include "annex-g-data/G1.c"

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned char * msg_org = annexg_G1;
    struct wlan_txvector_s txvector;
    txvector.LENGTH      = 100;
    txvector.DATARATE    = WLANFRAME_RATE_36;
    txvector.SERVICE     = 0;
    txvector.TXPWR_LEVEL = 0;
    
    // arrays
    float complex buffer[80];   // data buffer

    // create frame generator
    wlanframegen fg = wlanframegen_create();
    wlanframegen_print(fg);

    // create frame synchronizer
    wlanframesync fs = wlanframesync_create(callback, NULL);
    wlanframesync_print(fs);

    // assemble frame
    wlanframegen_assemble(fg, msg_org, txvector);

    // generate/synchronize frame
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = wlanframegen_writesymbol(fg, buffer);

        // run through synchronize
        wlanframesync_execute(fs, buffer, 80);
    }

    // destroy objects
    wlanframegen_destroy(fg);
    wlanframesync_destroy(fs);

    printf("done.\n");
    return 0;
}

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata)
{
    printf("**** callback invoked\n");

    return 0;
}

