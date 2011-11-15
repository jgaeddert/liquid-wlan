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
// 802.11a/g interleaver
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid-802.11.internal.h"

// intereleave one OFDM symbol
//  _ncbps      :   number of coded bits per OFDM symbol
//  _nbpsc      :   number of bits per subcarrier (modulation depth)
//  _msg_dec    :   decoded message (de-iterleaved)
//  _msg_enc    :   encoded message (interleaved)
void wifi_interleaver_encode_symbol(unsigned int _ncbps,
                                    unsigned int _nbpsc,
                                    unsigned char * _msg_dec,
                                    unsigned char * _msg_enc)
{
    // validate input
    if (_ncbps % 8) {
        fprintf(stderr,"error: wifi_interleaver_encode_symbol(), invalid ncbps\n");
        exit(1);
    }

    unsigned int k; // original
    unsigned int i;
    unsigned int j;

    unsigned int s = (_nbpsc / 2) < 1 ? 1 : _nbpsc/2; // max( _nbpsc/2, 1 )

    // internal array
    unsigned char msg_p0[_ncbps/8]; // first permutation

    // clear arrays
    memset(msg_p0,   0x00, (_ncbps/8)*sizeof(unsigned char));
    memset(_msg_enc, 0x00, (_ncbps/8)*sizeof(unsigned char));

    div_t d0;
    div_t d1;
    unsigned char bit;
    
    // outer permutation
    for (k=0; k<_ncbps; k++) {
        i = (_ncbps/16)*(k % 16) + (k/16);

        d0 = div(k, 8);
        d1 = div(i, 8);

        bit = (_msg_dec[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_p0[d1.quot] |= bit << (8-d1.rem-1);
        
        //printf("%3u > %3u (%1u)\n", k, i, bit);
    }

    //printf("-------\n");

    // inner permutation
    for (i=0; i<_ncbps; i++) {
        j = s*(i/s) + (i + _ncbps - ((16*i)/_ncbps) ) % s;

        d0 = div(i, 8);
        d1 = div(j, 8);

        bit = (msg_p0[d0.quot] >> (8-d0.rem-1)) & 0x01;
        _msg_enc[d1.quot] |= bit << (8-d1.rem-1);
        
        //printf("%3u > %3u (%1u)\n", i, j, bit);
    }
}

// intereleave message
//  _ncbps      :   number of coded bits per OFDM symbol
//  _nbpsc      :   number of bits per subcarrier (modulation depth)
//  _n          :   input messge length (bytes)
//  _msg_dec    :   decoded message (de-iterleaved)
//  _msg_enc    :   encoded message (interleaved)
void wifi_interleaver_encode(unsigned int _ncbps,
                             unsigned int _nbpsc,
                             unsigned int _n,
                             unsigned char * _msg_dec,
                             unsigned char * _msg_enc)
{
}

// de-intereleave one OFDM symbol
//  _ncbps      :   number of coded bits per OFDM symbol
//  _nbpsc      :   number of bits per subcarrier (modulation depth)
//  _msg_enc    :   encoded message (interleaved)
//  _msg_dec    :   decoded message (de-iterleaved)
void wifi_interleaver_decode_symbol(unsigned int _ncbps,
                                    unsigned int _nbpsc,
                                    unsigned char * _msg_enc,
                                    unsigned char * _msg_dec)
{
    // validate input
    if (_ncbps % 8) {
        fprintf(stderr,"error: wifi_interleaver_encode_symbol(), invalid ncbps\n");
        exit(1);
    }

    unsigned int k; // original
    unsigned int i;
    unsigned int j;

    unsigned int s = (_nbpsc / 2) < 1 ? 1 : _nbpsc/2; // max( _nbpsc/2, 1 )

    // internal array
    unsigned char msg_p0[_ncbps/8]; // first permutation
    
    // clear arrays
    memset(msg_p0,   0x00, (_ncbps/8)*sizeof(unsigned char));
    memset(_msg_dec, 0x00, (_ncbps/8)*sizeof(unsigned char));


    div_t d0;
    div_t d1;
    unsigned char bit;
    
    // inner permutation
    for (i=0; i<_ncbps; i++) {
        j = s*(i/s) + (i + _ncbps - ((16*i)/_ncbps) ) % s;

        d0 = div(j, 8);
        d1 = div(i, 8);

        bit = (_msg_enc[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_p0[d1.quot] |= bit << (8-d1.rem-1);
    }

    // outer permutation
    for (k=0; k<_ncbps; k++) {
        i = (_ncbps/16)*(k % 16) + (k/16);

        d0 = div(i, 8);
        d1 = div(k, 8);

        bit = (msg_p0[d0.quot] >> (8-d0.rem-1)) & 0x01;
        _msg_dec[d1.quot] |= bit << (8-d1.rem-1);
    }
}

// de-intereleave message
//  _ncbps      :   number of coded bits per OFDM symbol
//  _nbpsc      :   number of bits per subcarrier (modulation depth)
//  _n          :   input messge length (bytes)
//  _msg_enc    :   encoded message (interleaved)
//  _msg_dec    :   decoded message (de-iterleaved)
void wifi_interleaver_decode(unsigned int _ncbps,
                             unsigned int _nbpsc,
                             unsigned int _n,
                             unsigned char * _msg_enc,
                             unsigned char * _msg_dec)
{
}

