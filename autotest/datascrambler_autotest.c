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
// datascrambler_autotest.c
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
    // test sequence (shift register intialized with all ones)
    unsigned char sequence_test[127] = {
        0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0,
        1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 
        0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0,
        1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 
        1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1,
        1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 
        1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1,
        1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};

    // create m-sequence generator
    unsigned int m = 7;     // polynomial order
    unsigned int g = 0x91;  // generator polynomial: x^7 + x^4 + 1 = 1001 0001
    unsigned int a = 0x7f;  // initial state: .111 1111

    // create sequence generator
    wlan_lfsr ms = wlan_lfsr_create(m, g, a);

    unsigned int i;
    for (i=0; i<127; i++) {
        unsigned int bit = wlan_lfsr_advance(ms);

        if (bit != sequence_test[i]) {
            fprintf(stderr,"fail: %s, sequence failure\n", __FILE__);
            exit(1);
        }
    }

    // destroy sequence
    wlan_lfsr_destroy(ms);

    printf("done.\n");
    return 0;
}

