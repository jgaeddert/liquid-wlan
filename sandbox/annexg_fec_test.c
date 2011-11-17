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
// annexg_fec_test.c
//
// Test data encoding/decoding;
// data obtained from Annex G in 1999 specification
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

#include "liquid-802-11.internal.h"

// use internal wlan codec?
#define USE_INTERNAL_CODEC 1

void print_byte_array(unsigned char * _data,
                      unsigned int    _n);

int main(int argc, char*argv[])
{
    // options/parameters
    unsigned int length = 100;  // original data length (bytes)
    unsigned int ndbps  = 144;  // number of data bits per OFDM symbol
    unsigned int ncbps  = 192;  // number of coded bits per OFDM symbol
    unsigned int nbpsc  =   4;  // number of bits per subcarrier (modulation depth)
    unsigned int seed   = 0x5d; // data scrambler seed

    // original data message (Table G.1)
    unsigned char msg_data[100] = {
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

    // compute number of OFDM symbols
    div_t d = div(16 + 8*length + 6, ndbps);
    unsigned int nsym = d.quot + (d.rem == 0 ? 0 : 1);

    // compute number of bits in the DATA field
    unsigned int ndata = nsym * ndbps;

    // compute number of pad bits
    unsigned int npad = ndata - (16 + 8*length + 6);

    // compute decoded message length (number of data bytes)
    // NOTE : because ndbps is _always_ divisible by 8, so must ndata be
    unsigned int dec_msg_len = ndata / 8;

    // compute encoded message length (number of data bytes)
    unsigned int enc_msg_len = (dec_msg_len * ncbps) / ndbps;

    // print status
    printf("    nsym        :   %3u symbols\n", nsym);
    printf("    ndata       :   %3u bits\n", ndata);
    printf("    npad        :   %3u bits\n", npad);
    printf("    dec msg len :   %3u bytes\n", dec_msg_len);
    printf("    enc msg len :   %3u bytes\n", enc_msg_len);

    unsigned char msg_org[dec_msg_len];         // original message
    unsigned char msg_scrambled[dec_msg_len];   // scrambled message
    unsigned char msg_enc[enc_msg_len];         // encoded message
    unsigned char msg_int[enc_msg_len];         // interleaved message

    unsigned char msg_rec[enc_msg_len];         // received message
    
    unsigned char msg_deint[enc_msg_len];       // de-interleaved message
    unsigned char msg_dec[dec_msg_len];         // decoded message
    unsigned char msg_unscrambled[dec_msg_len]; // unscrambled message
    unsigned char msg_rx[length];               // recovered message (compare to msg_data)
    
    unsigned int i;

    // 
    // assemble raw data message (prepend SERVICE bits, reverse bytes,
    // add padding)
    //
    msg_org[0] = 0x00;
    msg_org[1] = 0x00;
    for (i=0; i<length; i++)
        msg_org[i+2] = liquid_802_11_reverse_byte[msg_data[i]];
    for (i=length+2; i<dec_msg_len; i++)
        msg_org[i] = 0x00;

    // print original message
    printf("original data (verify with Table G.13/G.14):\n");
    print_byte_array(msg_org, dec_msg_len);

    // 
    // scramble data
    //
    wlan_data_scramble(msg_org, msg_scrambled, dec_msg_len, seed);

    // zero tail bits (basically just revert scrambling these bits). For the
    // example given in Annex G, this amounts to the 6 bits after the SERVICE
    // and data bits (indices 816..821).
    msg_scrambled[length+2] &= 0x03;

    // print scrambled message
    printf("scrambled data (verify with Table G.16/G.17):\n");
    print_byte_array(msg_scrambled, dec_msg_len);

    // 
    // encode data
    //
#if USE_INTERNAL_CODEC
    wlan_fec_encode(LIQUID_WLAN_FEC_R3_4, dec_msg_len, msg_scrambled, msg_enc);
#else
    // initialize encoder
    unsigned int R = 2; // primitive rate, inverted (e.g. R=2 for rate 1/2)
    //unsigned int K = 7; // constraint length
    int poly[2] = {0x6d, 0x4f}; // generator polynomial (same as V27POLYA, V27POLYB in fec.h)
    
    // 3/4-rate K=7 puncturing matrix
    const char pmatrix[18] = {
        1, 1, 0, 1, 1, 0, 1, 1, 0,
        1, 0, 1, 1, 0, 1, 1, 0, 1};

    unsigned int P = 9; // columns of puncturing matrix

    unsigned int j;     // 
    unsigned int r;     // 
    unsigned int sr=0;  // convolutional shift register
    unsigned int n=0;   // output bit counter
    unsigned int p=0;   // puncturing matrix column index

    unsigned char bit;
    unsigned char byte_in;
    unsigned char byte_out=0;

    for (i=0; i<dec_msg_len; i++) {
        byte_in = msg_scrambled[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with most significant
            bit = (byte_in >> (7-j)) & 0x01;
            sr = (sr << 1) | bit;

            // compute parity bits for each polynomial
            for (r=0; r<R; r++) {
                // enable output determined by puncturing matrix
                if (pmatrix[r*P + p]) {
                    byte_out = (byte_out<<1) | parity(sr & poly[r]);
                    msg_enc[n/8] = byte_out;
                    n++;
                } else {
                }
            }

            // update puncturing matrix column index
            p = (p+1) % P;
        }
    }

    // NOTE: tail bits are already inserted into 'decoded' message
#endif

    // print encoded message
    printf("encoded data (verify with Table G.18):\n");
    print_byte_array(msg_enc, enc_msg_len);

    // 
    // interleave symbols
    //
    
    for (i=0; i<nsym; i++)
        wlan_interleaver_encode_symbol(ncbps, nbpsc, &msg_enc[(i*ncbps)/8], &msg_int[(i*ncbps)/8]);

    // print interleaved message
    printf("interleaved data (verify with Table G.21):\n");
    print_byte_array(msg_int, enc_msg_len);

    //
    // TODO : modulate/demodulate
    //

    // for now, just copy data
    memmove(msg_rec, msg_int, enc_msg_len*sizeof(unsigned char));

    //
    // de-interleave symbols
    //

    for (i=0; i<nsym; i++)
        wlan_interleaver_decode_symbol(ncbps, nbpsc, &msg_rec[(i*ncbps)/8], &msg_deint[(i*ncbps)/8]);

    // print de-interleaved message
    printf("de-interleaved data (verify with Table G.18):\n");
    printf(" bit errors: %3u / %3u\n", count_bit_errors_array(msg_deint, msg_enc, enc_msg_len), 8*enc_msg_len);
    print_byte_array(msg_deint, enc_msg_len);

    //
    // decode message
    //

#if USE_INTERNAL_CODEC
    wlan_fec_decode(LIQUID_WLAN_FEC_R3_4, dec_msg_len, msg_deint, msg_dec);
#else
    // unpack bytes, adding erasures at punctured indices
    // compute number of encoded bits with erasure insertions, removing
    // the additional padding to fill last OFDM symbol
    unsigned int num_enc_bits = dec_msg_len * 8 * R - npad;
    unsigned char enc_bits[num_enc_bits];

    n=0;   // input byte index
    unsigned int k=0;   // intput bit index (0<=k<8)
    p=0;   // puncturing matrix column index
    //unsigned char bit;
    byte_in = msg_deint[n];
    for (i=0; i<num_enc_bits; i+=R) {
        //
        for (r=0; r<R; r++) {
            if (pmatrix[r*P + p]) {
                // push bit from input
                bit = (byte_in >> (7-k)) & 0x01;
                enc_bits[i+r] = bit ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
                k++;
                if (k==8) {
                    k = 0;
                    n++;
                    byte_in = msg_deint[n];
                }
            } else {
                // push erasure
                enc_bits[i+r] = LIQUID_802_11_SOFTBIT_ERASURE;
            }
        }
        p = (p+1) % P;
    }

    // run Viterbi decoder
    void * vp = create_viterbi27(num_enc_bits);
    init_viterbi27(vp,0);
    update_viterbi27_blk(vp,enc_bits,num_enc_bits);
    chainback_viterbi27(vp, msg_dec, num_enc_bits, 0);
    delete_viterbi27(vp);
#endif

    // print decoded message
    // NOTE : clip padding and tail bits
    printf("decoded data (verify with Table G.16/G.17):\n");
    printf(" bit errors: %3u / %3u\n", count_bit_errors_array(msg_dec, msg_scrambled, length+2), 8*(length+2));
    //print_byte_array(msg_dec, dec_msg_len);
    print_byte_array(msg_dec, length+2);


    //
    // unscramble data
    //

    // TODO : strip scrambling seed from header?
    wlan_data_scramble(msg_dec, msg_unscrambled, dec_msg_len, seed);

    // print unscrambled message
    // NOTE : clip padding bits
    printf("unscrambled data (verify with Table G.13/G.14):\n");
    printf(" bit errors: %3u / %3u\n", count_bit_errors_array(msg_unscrambled, msg_org, length+2), 8*(length+2));
    //print_byte_array(msg_unscrambled, dec_msg_len);
    print_byte_array(msg_unscrambled, length+2);

    //
    // recover original data sequence
    //

    // strip SERVICE bits/padding, and reverse bytes
    for (i=0; i<length; i++)
        msg_rx[i] = liquid_802_11_reverse_byte[ msg_unscrambled[i+2] ];

    // print recovered message
    printf("recovered data (verify with Table G.1):\n");
    printf(" bit errors: %3u / %3u\n", count_bit_errors_array(msg_rx, msg_data, length), 8*length);
    print_byte_array(msg_rx, length);

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

