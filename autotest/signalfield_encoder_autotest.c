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
// signalfield_encoder_autotest.c
//
// Test PLCP Header encoding/decoding (SIGNAL field); data obtained from
// Annex G in 1999 specification (Tables G.7 & G.8, p. 61-62).
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-wlan.internal.h"

#include "annex-g-data/G7.c"
#include "annex-g-data/G8.c"

int main(int argc, char*argv[])
{
    // original data message
    //  0   1   RATE:R1         8   0   -               16  0   LENGTH (MSB)
    //  1   0   RATE:R1         9   0   -               17  0   PARITY
    //  2   1   RATE:R1         10  1   -               18  0   SIGNAL TAIL
    //  3   1   RATE:R1         11  1   -               19  0   SIGNAL TAIL
    //  4   0   [reserved]      12  0   -               20  0   SIGNAL TAIL
    //  5   0   LENGTH(LSB)     13  0   -               21  0   SIGNAL TAIL
    //  6   0   -               14  0   -               22  0   SIGNAL TAIL
    //  7   1   -               15  0   -               23  0   SIGNAL TAIL
    //      1011 0001               0011 0000               0000 0000
    unsigned char * msg_org = annexg_G7;
    unsigned char msg_enc[6];   // encoded message
    unsigned char msg_rec[6];   // received message
    unsigned char msg_dec[3];   // decoded message

    // after encoding (test)
    // 1101 0001
    // 1010 0001
    // 0000 0010
    // 0011 1110
    // 0111 0000
    // 0000 0000
    unsigned char * msg_test = annexg_G8;

    unsigned int i;

    // encode
    wlan_fec_signal_encode(msg_org, msg_enc);

    // print encoded message
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_enc[i], msg_test[i]);

    // test encoder
    if (count_bit_errors_array(msg_enc, msg_test, 6) > 0 ) {
        fprintf(stderr,"fail: %s, encoding failure\n", __FILE__);
        exit(1);
    }

    // channel (add error)
    memmove(msg_rec, msg_enc, 6*sizeof(unsigned char));
    msg_rec[0] ^= 0x40;

    // decode message
    wlan_fec_signal_decode(msg_rec, msg_dec);

    // print decoded message
    printf("decoded message:\n");
    for (i=0; i<3; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_dec[i], msg_org[i]);

    // count errors and print results
    unsigned int num_errors = count_bit_errors_array(msg_dec, msg_org, 3);
    printf("bit errors : %3u / %3u\n", num_errors, 24);

    // test decoder
    if (count_bit_errors_array(msg_dec, msg_org, 3) > 0 ) {
        fprintf(stderr,"fail: %s, decoding failure\n", __FILE__);
        exit(1);
    }

    printf("done.\n");
    return 0;
}

