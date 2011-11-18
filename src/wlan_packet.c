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
// wlan_packet.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"

void liquid_print_byte_array(unsigned char * _data,
                             unsigned int    _n)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        printf(" %.2x", _data[i]);

        if ( ((i+1)%16)==0 || i==(_n-1) )
            printf("\n");
    }
}

// assemble data (prepend SERVICE bits, etc.), scramble, encode, interleave
void wlan_packet_encode(unsigned int    _rate,
                        unsigned int    _seed,
                        unsigned int    _length,
                        unsigned char * _msg_dec,
                        unsigned char * _msg_enc)
{
    // validate input
    if (_rate > 7) {
        fprintf(stderr,"error: wlan_packet_encode(), invalid rate\n");
        exit(1);
    }

#if 0
    // options/parameters
    unsigned int length = 100;  // original data length (bytes)
    unsigned int ndbps  = 144;  // number of data bits per OFDM symbol
    unsigned int ncbps  = 192;  // number of coded bits per OFDM symbol
    unsigned int nbpsc  =   4;  // number of bits per subcarrier (modulation depth)
    unsigned int seed   = 0x5d; // data scrambler seed
#else
    // strip parameters
    unsigned int length = _length;                          // original data length (bytes)
    unsigned int ndbps  = wlanframe_ratetab[_rate].ndbps;   // number of data bits per OFDM symbol
    unsigned int ncbps  = wlanframe_ratetab[_rate].ncbps;   // number of coded bits per OFDM symbol
    unsigned int nbpsc  = wlanframe_ratetab[_rate].nbpsc;   // number of bits per subcarrier (modulation depth)
    unsigned int seed   = _seed;                            // 0x5d; // data scrambler seed

    // forward error-correction scheme
    unsigned int fec_scheme = wlanframe_ratetab[_rate].fec_scheme;
#endif

#if 0
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
#endif

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

    unsigned int i;

    // 
    // assemble raw data message (prepend SERVICE bits, reverse bytes,
    // add padding)
    //
    msg_org[0] = 0x00;
    msg_org[1] = 0x00;
    for (i=0; i<length; i++)
        msg_org[i+2] = liquid_wlan_reverse_byte[_msg_dec[i]];
    for (i=length+2; i<dec_msg_len; i++)
        msg_org[i] = 0x00;

    // print original message
    printf("original data (verify with Table G.13/G.14):\n");
    liquid_print_byte_array(msg_org, dec_msg_len);

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
    liquid_print_byte_array(msg_scrambled, dec_msg_len);

    // 
    // encode data
    //
    wlan_fec_encode(fec_scheme, dec_msg_len, msg_scrambled, msg_enc);
    // NOTE: tail bits are already inserted into 'decoded' message

    // print encoded message
    printf("encoded data (verify with Table G.18):\n");
    liquid_print_byte_array(msg_enc, enc_msg_len);

    // 
    // interleave symbols
    //
    
    for (i=0; i<nsym; i++)
        wlan_interleaver_encode_symbol(ncbps, nbpsc, &msg_enc[(i*ncbps)/8], &msg_int[(i*ncbps)/8]);

    // print interleaved message
    printf("interleaved data (verify with Table G.21):\n");
    liquid_print_byte_array(msg_int, enc_msg_len);
    
    // copy result to output
    memmove(_msg_enc, msg_int, enc_msg_len*sizeof(unsigned char));

    return;
}

// de-interleave, decode, de-scramble, extract data (SERVICE bits, etc.)
// TODO : return seed...
void wlan_packet_decode(unsigned int    _rate,
                        unsigned int    _seed,
                        unsigned int    _length,
                        unsigned char * _msg_enc,
                        unsigned char * _msg_dec)
{
    // validate input
    if (_rate > 7) {
        fprintf(stderr,"error: wlan_packet_decode(), invalid rate\n");
        exit(1);
    }

#if 0
    // options/parameters
    unsigned int length = 100;  // original data length (bytes)
    unsigned int ndbps  = 144;  // number of data bits per OFDM symbol
    unsigned int ncbps  = 192;  // number of coded bits per OFDM symbol
    unsigned int nbpsc  =   4;  // number of bits per subcarrier (modulation depth)
    unsigned int seed   = 0x5d; // data scrambler seed
#else
    // strip parameters
    unsigned int length = _length;                          // original data length (bytes)
    unsigned int ndbps  = wlanframe_ratetab[_rate].ndbps;   // number of data bits per OFDM symbol
    unsigned int ncbps  = wlanframe_ratetab[_rate].ncbps;   // number of coded bits per OFDM symbol
    unsigned int nbpsc  = wlanframe_ratetab[_rate].nbpsc;   // number of bits per subcarrier (modulation depth)
    unsigned int seed   = _seed;                            // 0x5d; // data scrambler seed

    // forward error-correction scheme
    unsigned int fec_scheme = wlanframe_ratetab[_rate].fec_scheme;
#endif

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

    unsigned char msg_deint[enc_msg_len];       // de-interleaved message
    unsigned char msg_dec[dec_msg_len];         // decoded message
    unsigned char msg_unscrambled[dec_msg_len]; // unscrambled message
    unsigned char msg_rx[length];               // recovered message (compare to msg_data)
    
    unsigned int i;


    //
    // de-interleave symbols
    //

    for (i=0; i<nsym; i++)
        wlan_interleaver_decode_symbol(ncbps, nbpsc, &_msg_enc[(i*ncbps)/8], &msg_deint[(i*ncbps)/8]);

    // print de-interleaved message
    printf("de-interleaved data (verify with Table G.18):\n");
    liquid_print_byte_array(msg_deint, enc_msg_len);

    //
    // decode message
    //

    wlan_fec_decode(fec_scheme, dec_msg_len, msg_deint, msg_dec);

    // print decoded message
    // NOTE : clip padding and tail bits
    printf("decoded data (verify with Table G.16/G.17):\n");
    //liquid_print_byte_array(msg_dec, dec_msg_len);
    liquid_print_byte_array(msg_dec, length+2);


    //
    // unscramble data
    //

    // TODO : strip scrambling seed from header?
    wlan_data_scramble(msg_dec, msg_unscrambled, dec_msg_len, seed);

    // print unscrambled message
    // NOTE : clip padding bits
    printf("unscrambled data (verify with Table G.13/G.14):\n");
    //liquid_print_byte_array(msg_unscrambled, dec_msg_len);
    liquid_print_byte_array(msg_unscrambled, length+2);

    //
    // recover original data sequence
    //

    // strip SERVICE bits/padding, and reverse bytes
    for (i=0; i<length; i++)
        msg_rx[i] = liquid_wlan_reverse_byte[ msg_unscrambled[i+2] ];

    // print recovered message
    printf("recovered data (verify with Table G.1):\n");
    liquid_print_byte_array(msg_rx, length);

    // copy to output
    memmove(_msg_dec, msg_rx, length*sizeof(unsigned char));

    return;
}
