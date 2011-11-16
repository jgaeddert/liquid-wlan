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
// signalfield_fec_test.c
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

#define SOFTBIT_1   (255)
#define SOFTBIT_0   (0)

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

    //
    // channel
    //
    for (i=0; i<6; i++)
        msg_rec[i] = msg_enc[i];

    // add error
    msg_rec[0] ^= 0x40;


    //
    // decode message
    //

    // unpack encoded bits
    unsigned char bits_enc[48];
    for (i=0; i<6; i++) {
        bits_enc[8*i+0] = (msg_rec[i] >> 7) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+1] = (msg_rec[i] >> 6) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+2] = (msg_rec[i] >> 5) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+3] = (msg_rec[i] >> 4) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+4] = (msg_rec[i] >> 3) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+5] = (msg_rec[i] >> 2) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+6] = (msg_rec[i] >> 1) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
        bits_enc[8*i+7] = (msg_rec[i]     ) & 0x01 ? SOFTBIT_1 : SOFTBIT_0;
    }
    
    // run decoder
    void * vp = create_viterbi27(48);
    init_viterbi27(vp,0);
    update_viterbi27_blk(vp,bits_enc,48);
    chainback_viterbi27(vp, msg_dec, 48, 0);
    delete_viterbi27(vp);

    // print decoded message
    printf("decoded message:\n");
    for (i=0; i<3; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_dec[i], msg_org[i]);

    // count errors and print results
    unsigned int num_errors = count_bit_errors_array(msg_dec, msg_org, 3);
    printf("bit errors : %3u / %3u\n", num_errors, 24);

    printf("done.\n");
    return 0;
}

