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
// pilots:
//  -21 -> 43
//   -7 -> 57
//    7 ->  7
//   21 -> 21
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
    float complex x[64];        // ifft output
    float complex symbol[81];   // resulting symbol with cyclic extension, window

    // load data into frequency-domain array
    unsigned int n = 0; // input bit counter
    unsigned char byte = liquid_wlan_reverse_byte[ msg_signal[0] ];
    for (i=0; i<64; i++) {
        unsigned int k = (i+32) % 64;
        if ( k == 0 || (k > 26 && k < 38) ) {
            // NULL subcarrier
            X[k] = 0.0f;
        } else if (k == 7 || k == 21 || k == 43 || k == 57) {
            // PILOT subcarrier
            X[k] = 0.0f;
        } else {
            // DATA subcarrier
            X[k] = (byte & 0x01) ? 1.0f : -1.0f;

            // shift byte
            byte >>= 1;

            // increment counter
            n++;

            // new byte
            if ( (n%8)==0 && n < 48)
                byte = liquid_wlan_reverse_byte[ msg_signal[n/8] ];
        }
    }
    
    // print results (compare to Table G.10)
    printf("frequency-domain SIGNAL symbol (verify with Table G.10):\n");
    for (i=0; i<64; i++) {
        int j = (int)i - 32;
        unsigned int k = (i+32)%64;
        printf("  X[%3d] = %8.4f + j*%8.4f\n", j, crealf(X[k]), cimagf(X[k]));
    }

    // add pilot subcarriers
    unsigned int pilot_phase = 1;
    X[43] = pilot_phase ?  1.0f : -1.0f;
    X[57] = pilot_phase ?  1.0f : -1.0f;
    X[ 7] = pilot_phase ?  1.0f : -1.0f;
    X[21] = pilot_phase ? -1.0f :  1.0f;
    
    printf("frequency-domain SIGNAL symbol with pilots (verify with Table G.11):\n");
    for (i=0; i<64; i++) {
        int j = (int)i - 32;
        unsigned int k = (i+32)%64;
        printf("  X[%3d] = %8.4f + j*%8.4f\n", j, crealf(X[k]), cimagf(X[k]));
    }

    // compute inverse transform
    fft_run(64, X, x, FFT_REVERSE, 0);

    // add cyclic prefix and apply window
#if 1
    memmove(&symbol[ 0], &x[48], 16*sizeof(float complex));
    memmove(&symbol[16], &x[ 0], 64*sizeof(float complex));
    symbol[80] = x[0];

    // apply window
    symbol[ 0] *= 0.5f;
    symbol[80] *= 0.5f;
#else
    for (i=0; i<81; i++) {
        symbol[i] = x[ (i+48) % 64 ];

        if (i==0 || i==80)
            symbol[i] *= 0.5f;
    }
#endif

    // scale by fft size
    for (i=0; i<81; i++)
        symbol[i] /= 64.0f;

    printf("time-domain SIGNAL symbol (verify with Table G.12):\n");
    for (i=0; i<81; i++)
        printf("  s[%3d] = %8.4f + j*%8.4f\n", i, crealf(symbol[i]), cimagf(symbol[i]));

    printf("done.\n");
    return 0;
}

