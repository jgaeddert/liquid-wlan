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
// signalfield_test.c
//
// Test PLCP Header encoding/decoding (SIGNAL field); data obtained from
// Annex G in 1999 specification (Tables G.7 & G.8, p. 61-62).
//
// Generator polynomials:
//  g0 = 133 (oct) = 1011011 (bin), 1101101 (bin, flipped) = 0x6d
//  g1 = 171 (oct) = 1111001 (bin), 1001111 (bin, flipped) = 0x4f
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
    unsigned char msg_org[3] = {0xb1, 0x30, 0x00};
    unsigned char msg_enc[6];   // after encoding
    //unsigned char msg_dec[3];   // after decoding

    // after encoding (test)
    // 1101 0001
    // 1010 0001
    // 0000 0010
    // 0011 1110
    // 0111 0000
    // 0000 0000
    unsigned char msg_test[6] = {0xd1, 0xa1, 0x02, 0x3e, 0x70, 0x00};

    // initialize encoder
    unsigned int R = 2; // primitive rate, inverted (e.g. R=2 for rate 1/2)
    //unsigned int K = 7; // constraint length
    int poly[2] = {0x6d, 0x4f}; // generator polynomial (same as V27POLYA, V27POLYB in fec.h)

    // decoder objects
    //void * vp;  // decoder object
    
    // 
    // encode message and test against msg_test
    //
    unsigned int i;     // input byte counter
    unsigned int j;     // 
    unsigned int r;
    unsigned int n=0;   // output bit counter
    unsigned int sr=0;  // convolutional shift register
    unsigned char byte_in;
    unsigned char byte_out;
    unsigned char bit;

    for (i=0; i<3; i++) {
        byte_in = msg_org[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with left-most
            bit = (byte_in >> (8-j-1)) & 0x01;
            sr  = (sr << 1) | bit;

            // compute parity bits for each polynomial
            printf("%3u : %1u > [%1u %1u]\n", 8*i+j, bit, parity(sr & poly[0]), parity(sr & poly[1]));
            for (r=0; r<R; r++) {
                byte_out = (byte_out << 1) | parity(sr & poly[r]);
                msg_enc[n/8] = byte_out;
                n++;
            }
        }
    }

    // NOTE: tail bits are already inserted into 'decoded' message

    // print encoded message
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_enc[i], msg_test[i]);


    printf("done.\n");
    return 0;
}

