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
// packet_codec_test.c
//
// Test data encoding/decoding using internal packet encoder/decoder
// methods; data obtained from Annex G in 1999 specification
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include <fec.h>

#include "liquid-wlan.internal.h"

void print_byte_array(unsigned char * _data,
                      unsigned int    _n);

int main(int argc, char*argv[])
{
    // options/parameters
    unsigned int rate   = WLANFRAME_RATE_36;    // primitive data rate
    unsigned int length = 100;                  // original data length (bytes)
    unsigned int seed   = 0x5d;                 // data scrambler seed

    // original data message (Table G.1)
    unsigned char msg_org[100] = {
        0x04, 0x02, 0x00, 0x2e, 0x00, 
        0x60, 0x08, 0xcd, 0x37, 0xa6, 
        0x00, 0x20, 0xd6, 0x01, 0x3c, 
        0xf1, 0x00, 0x60, 0x08, 0xad, 
        0x3b, 0xaf, 0x00, 0x00, 0x4a, 
        0x6f, 0x79, 0x2c, 0x20, 0x62, 
        0x72, 0x69, 0x67, 0x68, 0x74, 
        0x20, 0x73, 0x70, 0x61, 0x72, 
        0x6b, 0x20, 0x6f, 0x66, 0x20, 
        0x64, 0x69, 0x76, 0x69, 0x6e, 
        0x69, 0x74, 0x79, 0x2c, 0x0a, 
        0x44, 0x61, 0x75, 0x67, 0x68, 
        0x74, 0x65, 0x72, 0x20, 0x6f, 
        0x66, 0x20, 0x45, 0x6c, 0x79, 
        0x73, 0x69, 0x75, 0x6d, 0x2c, 
        0x0a, 0x46, 0x69, 0x72, 0x65, 
        0x2d, 0x69, 0x6e, 0x73, 0x69, 
        0x72, 0x65, 0x64, 0x20, 0x77, 
        0x65, 0x20, 0x74, 0x72, 0x65, 
        0x61, 0xda, 0x57, 0x99, 0xed};

    // encoded data length
    unsigned int enc_msg_len = wlan_packet_compute_enc_msg_len(rate, length);
    printf("dec msg len : %4u bytes\n", length);
    printf("enc msg len : %4u bytes\n", enc_msg_len);

    // arrays
    unsigned char msg_enc[enc_msg_len]; // encoded data
    unsigned char msg_dec[length];      // decoded data

    // encode message
    wlan_packet_encode(rate, seed, length, msg_org, msg_enc);

    printf("encoded message:\n");
    print_byte_array(msg_enc, enc_msg_len);
    
    // decode message
    wlan_packet_decode(rate, seed, length, msg_enc, msg_dec);
    
    printf("decoded message:\n");
    print_byte_array(msg_dec, length);

    // compute errors
    printf("bit errors : %4u / %4u\n", count_bit_errors_array(msg_dec, msg_org, length), 8*length);

    printf("done.\n");
    return 0;
}

void print_byte_array(unsigned char * _data,
                      unsigned int    _n)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        printf(" %.2x", _data[i]);

        if ( ((i+1)%16)==0 || i==(_n-1) )
            printf("\n");
    }
}

