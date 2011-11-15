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
// plcpheader_test.c
//
// Test PLCP Header encoding/decoding (SIGNAL field); data obtained from
// Annex G in 1999 specification.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-802.11.h"

int main(int argc, char*argv[])
{
    // original data message
    //  0   1   RATE:R1         8   0   -               16  0   LENGTH (MSB)
    //  1   0   RATE:R1         9   0   -               17  0   PARITY
    //  2   1   RATE:R1         10  0   -               18  0   SIGNAL TAIL
    //  3   1   RATE:R1         11  1   -               19  0   SIGNAL TAIL
    //  4   0   [reserved]      12  1   -               20  0   SIGNAL TAIL
    //  5   0   LENGTH(LSB)     13  0   -               21  0   SIGNAL TAIL
    //  6   0   -               14  0   -               22  0   SIGNAL TAIL
    //  7   1   -               15  0   -               23  0   SIGNAL TAIL
    //      1011 0001               0001 1000               0000 0000
    unsigned char msg_org[3] = {0xb1, 0x18, 0x00};

    // after encoding (test)
    // 1101 0001
    // 1010 0001
    // 0000 0010
    // 0011 1110
    // 0111 0000
    // 0000 0000
    unsigned char msg_test[6] = {0xb1, 0xa1, 0x02, 0x3e, 0x70, 0x00};

    // TODO : encode message and test against msg_test
    

    printf("done.\n");
    return 0;
}

