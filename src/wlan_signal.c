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
// wlan SIGNAL field
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid-wlan.internal.h"

// signal field rate encoding table (see Table 80)
//    WLANFRAME_RATE_6  = 13 : 1101
//    WLANFRAME_RATE_9  = 15 : 1111
//    WLANFRAME_RATE_12 =  5 : 0101
//    WLANFRAME_RATE_18 =  7 : 0111
//    WLANFRAME_RATE_24 =  9 : 1001
//    WLANFRAME_RATE_36 = 11 : 1011
//    WLANFRAME_RATE_48 =  1 : 0001
//    WLANFRAME_RATE_54 =  3 : 0011
const unsigned char wlan_signal_R1_R4[8] = {
    13, 15, 5, 7, 9, 11, 1, 3};

// pack SIGNAL structure into 3-byte array
//  _rate       :   data rate field (e.g. WLANFRAME_RATE_6)
//  _R          :   reserved bit
//  _length     :   length of payload (1-4095)
//  _signal     :   output signal, packed [size: 3 x 1]
void wlan_signal_pack(unsigned int    _rate,
                      unsigned int    _R,
                      unsigned int    _length,
                      unsigned char * _signal)
{
    // validate input
    if (_rate > 7) {
        fprintf(stderr,"error: wlan_signal_pack(), invalid data rate field\n");
        exit(1);
    } else if (_R > 1) {
        fprintf(stderr,"error: wlan_signal_pack(), invalid reserved bit field\n");
        exit(1);
    } else if (_length == 0 || _length > 4095) {
        fprintf(stderr,"error: wlan_signal_pack(), invalid data length field\n");
        exit(1);
    }

    // initialize output array
    _signal[0] = 0x00;
    _signal[1] = 0x00;
    _signal[2] = 0x00;

    // pack 'rate'
    _signal[0] |= ( wlan_signal_R1_R4[_rate] << 4) & 0xf0;

    // pack 'reserved' bit
    _signal[0] |= _R ? 0x08 : 0;

    // pack 'length' (LSB first)
    _signal[0] |= (_length & 0x001) ? 0x04 : 0;
    _signal[0] |= (_length & 0x002) ? 0x02 : 0;
    _signal[0] |= (_length & 0x004) ? 0x01 : 0;
    
    _signal[1] |= (_length & 0x008) ? 0x80 : 0;
    _signal[1] |= (_length & 0x010) ? 0x40 : 0;
    _signal[1] |= (_length & 0x020) ? 0x20 : 0;
    _signal[1] |= (_length & 0x040) ? 0x10 : 0;
    _signal[1] |= (_length & 0x080) ? 0x08 : 0;
    _signal[1] |= (_length & 0x100) ? 0x04 : 0;
    _signal[1] |= (_length & 0x200) ? 0x02 : 0;
    _signal[1] |= (_length & 0x400) ? 0x01 : 0;
    
    _signal[2] |= (_length & 0x800) ? 0x80 : 0;

    // compute parity and add to message
    unsigned int parity = ( liquid_count_ones(_signal[0]) +
                            liquid_count_ones(_signal[1]) +
                            liquid_count_ones(_signal[2]) ) % 2;
    _signal[2] |= parity ? 0x40 : 0;
}

// unpack SIGNAL structure from 3-byte array
//  _signal     :   input signal, packed [size: 3 x 1]
//  _rate       :   data rate field (e.g. WLANFRAME_RATE_6)
//  _R          :   reserved bit
//  _length     :   length of payload (1-4095)
void wlan_signal_unpack(unsigned char * _signal,
                      unsigned int    * _rate,
                      unsigned int    * _R,
                      unsigned int    * _length)
{
    // compute parity (last byte masked with 6 'tail' bits)
    unsigned int parity = ( liquid_count_ones(_signal[0]) +
                            liquid_count_ones(_signal[1]) +
                            liquid_count_ones(_signal[2] & 0x3f) ) % 2;

    // strip parity bit
    unsigned int parity_check = _signal[2] & 0x40 ? 1 : 0;
    if (parity != parity_check) {
        fprintf(stderr,"warning: wlan_signal_unpack(), parity mismatch!\n");
        // TODO : return flag
    }

    // strip data rate field
    // TODO : try to decode even if no match is found
    // TODO : use table for decoding
    unsigned int rate_enc = (_signal[0] >> 4) & 0x0f;
    switch (rate_enc) {
    case 13: *_rate = WLANFRAME_RATE_6;  break;
    case 15: *_rate = WLANFRAME_RATE_9;  break;
    case  5: *_rate = WLANFRAME_RATE_12; break;
    case  7: *_rate = WLANFRAME_RATE_18; break;
    case  9: *_rate = WLANFRAME_RATE_24; break;
    case 11: *_rate = WLANFRAME_RATE_36; break;
    case  1: *_rate = WLANFRAME_RATE_48; break;
    case  3: *_rate = WLANFRAME_RATE_54; break;
    default:
        fprintf(stderr,"warning: wlan_signal_unpack(), invalid rate\n");
        *_rate = WLANFRAME_RATE_6;
        // TODO : return flag
    }

    // unpack 'reserved' bit
    *_R = _signal[0] & 0x08 ? 1 : 0;

    // unpack message length
    unsigned int length = 0;
    length |= (_signal[0] & 0x04) ? 0x001 : 0;
    length |= (_signal[0] & 0x02) ? 0x002 : 0;
    length |= (_signal[0] & 0x01) ? 0x004 : 0;
    
    length |= (_signal[1] & 0x80) ? 0x008 : 0;
    length |= (_signal[1] & 0x40) ? 0x010 : 0;
    length |= (_signal[1] & 0x20) ? 0x020 : 0;
    length |= (_signal[1] & 0x10) ? 0x040 : 0;
    length |= (_signal[1] & 0x08) ? 0x080 : 0;
    length |= (_signal[1] & 0x04) ? 0x100 : 0;
    length |= (_signal[1] & 0x02) ? 0x200 : 0;
    length |= (_signal[1] & 0x01) ? 0x400 : 0;
    
    length |= (_signal[2] & 0x80) ? 0x800 : 0;

    *_length = length;
}

// encode SIGNAL field using half-rate convolutional code
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
void wlan_fec_signal_encode(unsigned char * _msg_dec,
                            unsigned char * _msg_enc)
{
#if 0
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
#else
    // encode using generic encoding method (half-rate encoder)
    wlan_fec_encode(LIQUID_WLAN_FEC_R1_2, 3, _msg_dec, _msg_enc);
#endif
}

// decode SIGNAL field using half-rate convolutional code
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
void wlan_fec_signal_decode(unsigned char * _msg_enc,
                            unsigned char * _msg_dec)
{
#if 0
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
#else
    // decode using generic decoding method (half-rate encoder)
    wlan_fec_decode(LIQUID_WLAN_FEC_R1_2, 3, _msg_enc, _msg_dec);
#endif
}

