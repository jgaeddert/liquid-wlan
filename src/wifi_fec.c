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
// wifi physical layer convergence procedure (PLCP) data
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid-802.11.internal.h"

// encode SIGNAL field using half-rate convolutional code
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
void wifi_fec_signal_encode(unsigned char * _msg_dec,
                            unsigned char * _msg_enc)
{
    // initialize encoder
    unsigned int R = 2; // primitive rate, inverted (e.g. R=2 for rate 1/2)
    int poly[2] = {0x6d, 0x4f}; // generator polynomial (same as V27POLYA, V27POLYB in fec.h)

    // encode message and test against msg_test
    unsigned int i;     // input byte counter
    unsigned int j;     // 
    unsigned int r;
    unsigned int n=0;   // output bit counter
    unsigned int sr=0;  // convolutional shift register
    unsigned char byte_in;
    unsigned char byte_out;
    unsigned char bit;

    for (i=0; i<3; i++) {
        byte_in = _msg_dec[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with left-most
            bit = (byte_in >> (8-j-1)) & 0x01;
            sr  = (sr << 1) | bit;

            // compute parity bits for each polynomial
            printf("%3u : %1u > [%1u %1u]\n", 8*i+j, bit, parity(sr & poly[0]), parity(sr & poly[1]));
            for (r=0; r<R; r++) {
                byte_out = (byte_out << 1) | parity(sr & poly[r]);
                _msg_enc[n/8] = byte_out;
                n++;
            }
        }
    }
}

// decode SIGNAL field using half-rate convolutional code
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
void wifi_fec_signal_decode(unsigned char * _msg_enc,
                            unsigned char * _msg_dec)
{
    unsigned int i;

    // unpack encoded bits
    unsigned char bits_enc[48];
    for (i=0; i<6; i++) {
        bits_enc[8*i+0] = (_msg_enc[i] >> 7) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+1] = (_msg_enc[i] >> 6) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+2] = (_msg_enc[i] >> 5) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+3] = (_msg_enc[i] >> 4) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+4] = (_msg_enc[i] >> 3) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+5] = (_msg_enc[i] >> 2) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+6] = (_msg_enc[i] >> 1) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
        bits_enc[8*i+7] = (_msg_enc[i]     ) & 0x01 ? LIQUID_802_11_SOFTBIT_1 : LIQUID_802_11_SOFTBIT_0;
    }
    
    // run decoder
    void * vp = create_viterbi27(48);
    init_viterbi27(vp,0);
    update_viterbi27_blk(vp,bits_enc,48);
    chainback_viterbi27(vp, _msg_dec, 48, 0);
    delete_viterbi27(vp);

}

