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
// interleaver_table_test.c
//
// Test generating structured interleaver tables; data obtained from
// Annex G in 1999 specification (Tables ...)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"
    
#include "annex-g-data/G18.c"
#include "annex-g-data/G21.c"

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
    // option(s)
    unsigned int rate = WLANFRAME_RATE_36;   // primitive rate
    
    // 
    unsigned int ncbps  = wlanframe_ratetab[rate].ncbps;   // number of coded bits per OFDM symbol
    unsigned int nbpsc  = wlanframe_ratetab[rate].nbpsc;   // number of bits per subcarrier (modulation depth)
    struct wlan_interleaver_tab_s intlv[ncbps];

    // generate table
    wlan_interleaver_gentab(rate, intlv);

    // print table
    unsigned int i;
    for (i=0; i<ncbps; i++) {
        printf("  %3u : %3u > %3u [mask = 0x%.2x, bit = 0x%.2x]\n",
                i,
                intlv[i].p0,
                intlv[i].p1,
                intlv[i].mask,
                intlv[i].bit);
    }

    // run interleaving test (only if rate is 36)
    if (rate == WLANFRAME_RATE_36) {
        // Table G.18: coded bits of first DATA symbol
        unsigned char * msg_org = annexg_G18;

        // Table G.21: interleaved bits of first DATA symbol
        unsigned char * msg_test = annexg_G21;

        // interleaver output
        unsigned char msg_enc[24];

        // clear encoded message buffer
        memset(msg_enc, 0x00, 24*sizeof(unsigned char));

        // run interleaver
        for (i=0; i<ncbps; i++) {
            msg_enc[ intlv[i].p1 ] |= (msg_org[ intlv[i].p0 ] & intlv[i].mask ) ? intlv[i].bit : 0;
        }

        // print results
        printf("interleaved:\n");
        for (i=0; i<24; i++)
            printf("%3u : 0x%.2x > 0x%.2x (0x%.2x) %s\n", i, msg_org[i], msg_enc[i], msg_test[i], msg_enc[i] == msg_test[i] ? "" : "*");
        printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc, msg_test, 24), ncbps);

    }

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

    // fill table: k > i > j
    for (k=0; k<ncbps; k++) {

        i = (ncbps/16)*(k % 16) + (k/16);
        j = s*(i/s) + (i + ncbps - ((16*i)/ncbps) ) % s;
        
        d0 = div(k, 8);
        d1 = div(j, 8);

        _intlv[k].p0 = d0.quot;
        _intlv[k].p1 = d1.quot;

        _intlv[k].mask = 1 << (8-d0.rem-1);
        _intlv[k].bit  = 1 << (8-d1.rem-1);
    }
}

