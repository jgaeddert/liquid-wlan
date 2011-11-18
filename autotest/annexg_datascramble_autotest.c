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
// annexg_datascramble_autotest.c
//
// Test data scrambler
// data obtained from Annex G in 1999 specification (Tables G.13 & G.16)
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
    //
    unsigned int n=18;          // data size (bytes)
    unsigned int seed = 0x5d;   // 1011101

    // first 144 data bits (Table G.13)
    unsigned char msg_org[18] = {    
        0x00, 0x00, 0x20, 0x40, 0x00,
        0x74, 0x00, 0x06, 0x10, 0xb3,
        0xec, 0x65, 0x00, 0x04, 0x6b,
        0x80, 0x3c, 0x8f};

    // first 144 data bits, scrambled (Table G.16)
    unsigned char msg_scrambled_test[18] = {    
        0x6c, 0x19, 0x89, 0x8f, 0x68,
        0x21, 0xf4, 0xa5, 0x61, 0x4f,
        0xd7, 0xae, 0x24, 0x0c, 0xf3,
        0x3a, 0xe4, 0xbc};

    // arrays
    unsigned char msg_scrambled[18];
    unsigned char msg_unscrambled[18];

    // scramble data
    wlan_data_scramble(msg_org, msg_scrambled, n, seed);

    // check result
    if (count_bit_errors_array(msg_scrambled, msg_scrambled_test, n) > 0 ) {
        fprintf(stderr,"fail: %s, scrambling failure\n", __FILE__);
        exit(1);
    }

    // unscramble data
    wlan_data_unscramble(msg_scrambled, msg_unscrambled, n, seed);

    // check result
    if (count_bit_errors_array(msg_unscrambled, msg_org, n) > 0 ) {
        fprintf(stderr,"fail: %s, unscrambling failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

