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
// interleaver_autotest.c
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

#include <liquid/liquid.h>
#include <fec.h>

int main(int argc, char*argv[])
{
    // input messages
    unsigned char msg_org[6] = {0xd1, 0xa1, 0x02, 0x3e, 0x70, 0x00};

    // interleaved messages (tests)
    unsigned char msg_test[6] = {0x94, 0xd0, 0x14, 0x83, 0x24, 0x94};

    // options...
    unsigned int ncbps = 48;    // number of coded bits per OFDM symbol
    unsigned int nbpsc = 1;     // number of bits per subcarrier (modulation depth)
    
    unsigned int i;

    // 
    // interleave message
    //
    unsigned char msg_enc[6];
    wifi_interleaver_encode_symbol(ncbps, nbpsc, msg_org, msg_enc);
    
    // print results
    printf("interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x)\n", i, msg_org[i], msg_enc[i], msg_test[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc, msg_test, 6), 48);

    if (count_bit_errors_array(msg_enc, msg_test, 6) > 0) {
        fprintf(stderr,"fail: %s, encoding failure\n", __FILE__);
        exit(1);
    }

    // 
    // de-interleave message
    //
    unsigned char msg_dec[6];
    wifi_interleaver_decode_symbol(ncbps, nbpsc, msg_enc, msg_dec);
    
    // print results
    printf("de-interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x)\n", i, msg_enc[i], msg_dec[i], msg_org[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_dec, msg_org, 6), 48);

    if (count_bit_errors_array(msg_dec, msg_org, 6) > 0) {
        fprintf(stderr,"fail: %s, decoding failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

