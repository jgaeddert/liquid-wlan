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
// wifi forward error-correction encoder/decoder
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid-802-11.internal.h"

// r1/2 base generator polynomials (same as V27POLYA, V27POLYB in fec.h)
const unsigned int wificonv_genpoly[2] = {0x6d, 0x4f};

// 2/3-rate K=7 puncturing matrix
const unsigned char wificonv_v27p23_pmatrix[12] = {
    1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 1, 0};

// 3/4-rate K=7 puncturing matrix
const unsigned char wificonv_v27p34_pmatrix[18] = {
    1, 1, 0, 1, 1, 0, 1, 1, 0,
    1, 0, 1, 1, 0, 1, 1, 0, 1};

// table of available convolutional codecs
const struct wificonv_s wificonv_fectab[3] = {
    //  genpoly           R  K  punctured? pmatrix                  P
    {   wificonv_genpoly, 2, 7, 0,         NULL,                    0},
    {   wificonv_genpoly, 2, 7, 1,         wificonv_v27p23_pmatrix, 6},
    {   wificonv_genpoly, 2, 7, 1,         wificonv_v27p34_pmatrix, 9}};

// encode data using convolutional code
//  _fec_scheme :   error-correction scheme
//  _dec_msg_len:   length of decoded message
//  _msg_dec    :   decoded message (with tail bits inserted)
//  _msg_enc    :   encoded message
void wifi_fec_encode(unsigned int    _fec_scheme,
                     unsigned int    _dec_msg_len,
                     unsigned char * _msg_dec,
                     unsigned char * _msg_enc)
{
    // validate input
    if (_fec_scheme != LIQUID_WIFI_FEC_R1_2 &&
        _fec_scheme != LIQUID_WIFI_FEC_R2_3 &&
        _fec_scheme != LIQUID_WIFI_FEC_R3_4)
    {
        fprintf(stderr,"error: wifi_fec_encode(), invalid scheme\n");
        exit(1);
    } else if (_dec_msg_len == 0) {
        fprintf(stderr,"error: wifi_fec_encode(), input message length must be greater than zero\n");
        exit(1);
    }

    // initialize encoder options
    unsigned int R                = wificonv_fectab[_fec_scheme].R;
    //unsigned int K              = wificonv_fectab[_fec_scheme].K;
    const unsigned int * genpoly  = wificonv_fectab[_fec_scheme].genpoly;
    
    // puncturing options
    int punctured                 = wificonv_fectab[_fec_scheme].punctured;
    unsigned int P                = wificonv_fectab[_fec_scheme].P;
    const unsigned char * pmatrix = wificonv_fectab[_fec_scheme].pmatrix;

    // bookkeeping
    unsigned int i;     // input byte index
    unsigned int j;     // bit index
    unsigned int r;     // output convolutional encoder branch
    unsigned int sr=0;  // convolutional shift register
    unsigned int n=0;   // output bit counter
    unsigned int p=0;   // puncturing matrix column index

    unsigned char bit;          // input bit
    unsigned char byte_in;      // intput byte
    unsigned char byte_out=0;   // output byte

    for (i=0; i<_dec_msg_len; i++) {
        byte_in = _msg_dec[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with most significant
            bit = (byte_in >> (7-j)) & 0x01;
            sr = (sr << 1) | bit;

            // compute parity bits for each polynomial
            for (r=0; r<R; r++) {
                if (punctured) {
                    // enable output determined by puncturing matrix
                    if (pmatrix[r*P + p]) {
                        byte_out = (byte_out<<1) | parity(sr & genpoly[r]);
                        _msg_enc[n/8] = byte_out;
                        n++;
                    }
                } else {
                    // enable all outputs
                    byte_out = (byte_out << 1) | parity(sr & genpoly[r]);
                    _msg_enc[n/8] = byte_out;
                    n++;
                }
            }

            // update puncturing matrix column index
            if (punctured)
                p = (p+1) % P;
        }
    }

    // NOTE: tail bits are already inserted into 'decoded' message

}

