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
}

// encode with punctured code
void wifi_fec_encode_punctured(unsigned char * _msg_dec,
                               unsigned char * _msg_enc,
                               unsigned int    _dec_msg_len,
                               unsigned char * _pmatrix,
                               unsigned char   _P)
{
    unsigned int i,j,r; // bookkeeping
    unsigned int sr=0;  // convolutional shift register
    unsigned int n=0;   // output bit counter
    unsigned int p=0;   // puncturing matrix column index
    
    unsigned int R = 2; // primitive rate, inverted (e.g. R=2 for rate 1/2)
    int poly[2] = {0x6d, 0x4f}; // generator polynomial (same as V27POLYA, V27POLYB in fec.h)

    unsigned char bit;
    unsigned char byte_in;
    unsigned char byte_out=0;

    for (i=0; i<_dec_msg_len; i++) {
        byte_in = _msg_dec[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with most significant
            bit = (byte_in >> (7-j)) & 0x01;
            sr = (sr << 1) | bit;

            // compute parity bits for each polynomial
            for (r=0; r<R; r++) {
                // enable output determined by puncturing matrix
                if (_pmatrix[r*_P+p]) {
                    byte_out = (byte_out<<1) | parity(sr & poly[r]);
                    _msg_enc[n/8] = byte_out;
                    n++;
                } else {
                }
            }

            // update puncturing matrix column index
            p = (p+1) % _P;
        }
    }
}
