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
// interleaver_data_autotest.c
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

#include "liquid-wlan.internal.h"

int main(int argc, char*argv[])
{
    // input messages
    unsigned char msg_org[24] = {
        0x2b, 0x08, 0xa1, 0xf0,
        0x9d, 0xb5, 0x9a, 0x1d,
        0x4a, 0xfb, 0xe8, 0xc2,
        0x8f, 0xc0, 0xc8, 0x73,
        0xc0, 0x43, 0xe0, 0x19,
        0xe0, 0xd3, 0xeb, 0xb2};

    // interleaved message (test)
    unsigned char msg_test[24] = {
        0x77, 0xf0, 0xef, 0xc4,
        0x73, 0x00, 0xbf, 0x11,
        0x10, 0x9a, 0x1d, 0x12,
        0x6e, 0x38, 0xf5, 0x69,
        0x1b, 0x6b, 0x98, 0x43,
        0x00, 0x0d, 0xb3, 0x6d};

    // options...
    unsigned int ncbps = 192;   // number of coded bits per OFDM symbol
    unsigned int nbpsc = 4;     // number of bits per subcarrier (modulation depth)

    unsigned int n = ncbps / 8;
    
    unsigned int i;

    // 
    // interleave message
    //
    unsigned char msg_enc[n];
    wlan_interleaver_encode_symbol(ncbps, nbpsc, msg_org, msg_enc);
    
    // print results
    printf("interleaved:\n");
    for (i=0; i<n; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x) %s\n", i, msg_org[i], msg_enc[i], msg_test[i], msg_enc[i] == msg_test[i] ? "" : "*");
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc, msg_test, n), ncbps);

    if (count_bit_errors_array(msg_enc, msg_test, n) > 0) {
        fprintf(stderr,"fail: %s, encoding failure\n", __FILE__);
        exit(1);
    }

    // 
    // de-interleave message
    //
    unsigned char msg_dec[n];
    wlan_interleaver_decode_symbol(ncbps, nbpsc, msg_enc, msg_dec);
    
    // print results
    printf("de-interleaved:\n");
    for (i=0; i<n; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x) %s\n", i, msg_enc[i], msg_dec[i], msg_org[i], msg_dec[i] == msg_org[i] ? "" : "*");
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_dec, msg_org, n), ncbps);

    if (count_bit_errors_array(msg_dec, msg_org, n) > 0) {
        fprintf(stderr,"fail: %s, decoding failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

