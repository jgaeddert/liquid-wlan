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
// signalfield_symbolgen_test.c
//
// Test SIGNAL field modulation; data obtained from
// Annex G in 1999 specification (Tables G.9-G.12)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"

int main(int argc, char*argv[])
{
    // after encoding, interleaving (Table G.9)
    // 1001 0100
    // 1101 0000
    // 0001 0100
    // 1000 0011
    // 0010 0100
    // 1001 0100
    unsigned char msg_signal[6] = {0x94, 0xd0, 0x14, 0x83, 0x24, 0x94};

    unsigned int i;

    // arrays
    float complex X[64];        // ifft input
    //float complex x[64];        // ifft output
    //float complex symbol[81];   // resulting symbol with cyclic extension, window

    // load data into frequency-domain array
    unsigned int n = 0; // input bit counter
    unsigned char byte = liquid_wlan_reverse_byte[ msg_signal[0] ];
    for (i=0; i<64; i++) {
        if ( i == 0 || (i > 26 && i < 38) ) {
            // NULL subcarrier
            X[i] = 0.0f;
        } else if (i == 11 || i == 25 || i == 39 || i == 53) {
            // PILOT subcarrier
            X[i] = 0.0f;
        } else {
            // DATA subcarrier
            X[i] = (byte & 0x01) ? 1.0f : -1.0f;

            // shift byte
            byte >>= 1;

            // increment counter
            n++;

            // new byte
            if ( (n%8)==0 && n < 48)
                byte = liquid_wlan_reverse_byte[ msg_signal[n/8] ];
        }
    }
    
    // print results
    for (i=0; i<64; i++) {
        int j = (int)i - 32;
        unsigned int k = (i+32)%64;
        printf("X[%3d] = %12.8f + j*%12.8f\n", j, crealf(X[k]), cimagf(X[k]));
    }

    // add pilot subcarriers

    // compute inverse transform

    // add cyclic prefix

    // apply window

    printf("done.\n");
    return 0;
}

