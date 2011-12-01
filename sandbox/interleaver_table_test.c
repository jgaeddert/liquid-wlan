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
// interleaver_test.c
//
// Test interleaver; data obtained from
// Annex G in 1999 specification (Tables ...)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

//#include <liquid/liquid.h>

#include "liquid-wlan.internal.h"
    
struct wlan_interleaver_tab_s {
    unsigned char p0;   // input byte index
    unsigned char p1;   // output byte index
    unsigned char mask; // mask
    unsigned char bit;  // output bit flag
};


// generate interleaving table
//  _rate       :   primitive rate
void wlan_interleaver_gentab(unsigned int _rate,
                             struct wlan_interleaver_tab_s * _intlv);

int main(int argc, char*argv[])
{
#if 0
    // options...
    unsigned int ncbps = 48;    // number of coded bits per OFDM symbol
    unsigned int nbpsc = 1;     // number of bits per subcarrier (modulation depth)
#else
    unsigned int rate = WLANFRAME_RATE_6;   // primitive rate
#endif
    
    // 
    unsigned int ncbps  = wlanframe_ratetab[rate].ncbps;   // number of coded bits per OFDM symbol
    unsigned int nbpsc  = wlanframe_ratetab[rate].nbpsc;   // number of bits per subcarrier (modulation depth)
    struct wlan_interleaver_tab_s intlv[ncbps];

    // generate table
    wlan_interleaver_gentab(rate, intlv);

    // print table
    unsigned int i;
    for (i=0; i<ncbps; i++) {
        printf("  %3u > %3u [mask = 0x%.2x, bit = 0x%.2x]\n",
                intlv[i].p0,
                intlv[i].p1,
                intlv[i].mask,
                intlv[i].bit);
    }

#if 0

    memset(msg_enc, 0x00, (ncbps/8)*sizeof(unsigned char));
    for (i=0; i<ncbps; i++) {
        msg_enc[ intlv[i].p0 ] |= (msg_dec[ intlv[i].p1 ] & intlv[i].mask ) ? intlv[i].bit : 0;
    }
#endif

    printf("done.\n");
    return 0;
}

// generate interleaving table
//  _rate       :   primitive rate
void wlan_interleaver_gentab(unsigned int _rate,
                             struct wlan_interleaver_tab_s * _intlv)
{
    // validate input
    if (_rate > WLANFRAME_RATE_54) {
        fprintf(stderr,"error: wlan_interleaver_gentab(), invalid rate\n");
        exit(1);
    }
    
    // strip parameters
    //unsigned int ndbps  = wlanframe_ratetab[_rate].ndbps;   // number of data bits per OFDM symbol
    unsigned int ncbps  = wlanframe_ratetab[_rate].ncbps;   // number of coded bits per OFDM symbol
    unsigned int nbpsc  = wlanframe_ratetab[_rate].nbpsc;   // number of bits per subcarrier (modulation depth)

    unsigned int k; // original
    unsigned int i;
    unsigned int j;

    unsigned int s = (nbpsc / 2) < 1 ? 1 : nbpsc/2; // max( nbpsc/2, 1 )

#if 0
    // internal array
    unsigned char msg_p0[ncbps/8]; // first permutation
#endif

    // clear arrays
    //memset(msg_p0,   0x00, (ncbps/8)*sizeof(unsigned char));
    //memset(_msg_enc, 0x00, (ncbps/8)*sizeof(unsigned char));

    unsigned int msg_p0[ncbps]; // first permutation
    unsigned int msg_p1[ncbps]; // second permutation

    div_t d0;
    div_t d1;
    unsigned char bit;
    
    // outer permutation
    for (k=0; k<ncbps; k++) {
        i = (ncbps/16)*(k % 16) + (k/16);

        d0 = div(k, 8);
        d1 = div(i, 8);

#if 0
        bit = (_msg_dec[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_p0[d1.quot] |= bit << (8-d1.rem-1);
#endif
        
        printf("%3u > %3u\n", k, i);

        msg_p0[k] = i;
    }

    printf("-------\n");

    // inner permutation
    for (i=0; i<ncbps; i++) {
        j = s*(i/s) + (i + ncbps - ((16*i)/ncbps) ) % s;

        d0 = div(i, 8);
        d1 = div(j, 8);

#if 0
        bit = (msg_p0[d0.quot] >> (8-d0.rem-1)) & 0x01;
        _msg_enc[d1.quot] |= bit << (8-d1.rem-1);
#endif
        
        printf("%3u > %3u\n", i, j);
        
        msg_p1[i] = j;
    }

    // fill table
    for (i=0; i<ncbps; i++) {
        // TODO : check these values
        _intlv[i].p0 = i/8;
        _intlv[i].p1 = msg_p0[ msg_p1[i] ] / 8;

        // TODO : use valid mask/bit values
        _intlv[i].mask = 0x01;
        _intlv[i].bit  = 0x01;
    }
}

