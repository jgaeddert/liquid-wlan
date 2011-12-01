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

#include "liquid-wlan.internal.h"

// indexable table of above structured auto-generated tables
struct wlan_interleaver_tab_s * wlan_intlv_gentab[8] = {
    wlan_intlv_R6,
    wlan_intlv_R9,
    wlan_intlv_R12,
    wlan_intlv_R18,
    wlan_intlv_R24,
    wlan_intlv_R36,
    wlan_intlv_R48,
    wlan_intlv_R54};


// intereleave one OFDM symbol
//  _rate       :   primitive rate
//  _msg_dec    :   decoded message (de-iterleaved)
//  _msg_enc    :   encoded message (interleaved)
void wlan_interleaver_encode_symbol(unsigned int    _rate,
                                    unsigned char * _msg_dec,
                                    unsigned char * _msg_enc)
{
    // validate input
    if (_rate > WLANFRAME_RATE_54) {
        fprintf(stderr,"error: wlan_interleaver_encode_symbol(), invalid rate\n");
        exit(1);
    }

    // number of coded bits per OFDM symbol
    unsigned int ncbps = wlanframe_ratetab[_rate].ncbps;

    // retrieve structured interleaver table
    struct wlan_interleaver_tab_s * intlv = wlan_intlv_gentab[_rate];

    // run interleaver
    unsigned int i;
    for (i=0; i<ncbps; i++)
        _msg_enc[ intlv[i].p1 ] |= (_msg_dec[ intlv[i].p0 ] & intlv[i].mask0 ) ? intlv[i].mask1 : 0;
}

// de-intereleave one OFDM symbol
//  _rate       :   primitive rate
//  _msg_enc    :   encoded message (interleaved)
//  _msg_dec    :   decoded message (de-iterleaved)
void wlan_interleaver_decode_symbol(unsigned int    _rate,
                                    unsigned char * _msg_enc,
                                    unsigned char * _msg_dec)
{
    // validate input
    if (_rate > WLANFRAME_RATE_54) {
        fprintf(stderr,"error: wlan_interleaver_decode_symbol(), invalid rate\n");
        exit(1);
    }

    // number of coded bits per OFDM symbol
    unsigned int ncbps = wlanframe_ratetab[_rate].ncbps;

    // retrieve structured interleaver table
    struct wlan_interleaver_tab_s * intlv = wlan_intlv_gentab[_rate];

    // run de-interleaver (run interleaver in reverse)
    unsigned int i;
    for (i=0; i<ncbps; i++)
        _msg_dec[ intlv[i].p0 ] |= (_msg_enc[ intlv[i].p1 ] & intlv[i].mask1 ) ? intlv[i].mask0 : 0;
}

