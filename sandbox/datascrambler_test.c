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
// datascrambler_test.c
//
// Test data scrambler; should generate this sequence (127 bits) with
// the 'all ones' inital state:
//  00001110 11110010 11001001 00000010
//  00100110 00101110 10110110 00001100
//  11010100 11100111 10110100 00101010
//  11111010 01010001 10111000 1111111
// 
// NOTE: the above sequence is the same as the phase of the BPSK pilot
//       subcarriers when replacing the 1s with '-1' and 0s with '1',
//       viz {1,1,1,1, -1,-1,-1,1, -1,-1,-,1-,1...}
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-wlan.h"

int main(int argc, char*argv[])
{
    // create m-sequence generator
    unsigned int m = 7;     // polynomial order
    unsigned int g = 0x91;  // generator polynomial: x^7 + x^4 + 1 = 1001 0001
    unsigned int a = 0x7f;  // initial state: .111 1111

    // create
    wlan_lfsr ms = wlan_lfsr_create(m, g, a);

    unsigned int i;
    for (i=0; i<127; i++) {
        unsigned int bit = wlan_lfsr_advance(ms);
        printf("%1u", bit);

        if ( ((i+1)%8) == 0 )
            printf("\n");
    }
    printf("\n");

    wlan_lfsr_destroy(ms);


    printf("done.\n");
    return 0;
}

