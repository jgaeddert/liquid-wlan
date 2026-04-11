// WLAN interleaver

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

    // clear output array
    memset(_msg_enc, 0x00, (ncbps/8)*sizeof(unsigned char));

    // run interleaver, loop unwrapped
    unsigned int i;
    for (i=0; i<ncbps; i+=8) {
        _msg_enc[ intlv[i  ].p1 ] |= (_msg_dec[ intlv[i  ].p0 ] & intlv[i  ].mask0 ) ? intlv[i  ].mask1 : 0;
        _msg_enc[ intlv[i+1].p1 ] |= (_msg_dec[ intlv[i+1].p0 ] & intlv[i+1].mask0 ) ? intlv[i+1].mask1 : 0;
        _msg_enc[ intlv[i+2].p1 ] |= (_msg_dec[ intlv[i+2].p0 ] & intlv[i+2].mask0 ) ? intlv[i+2].mask1 : 0;
        _msg_enc[ intlv[i+3].p1 ] |= (_msg_dec[ intlv[i+3].p0 ] & intlv[i+3].mask0 ) ? intlv[i+3].mask1 : 0;
        _msg_enc[ intlv[i+4].p1 ] |= (_msg_dec[ intlv[i+4].p0 ] & intlv[i+4].mask0 ) ? intlv[i+4].mask1 : 0;
        _msg_enc[ intlv[i+5].p1 ] |= (_msg_dec[ intlv[i+5].p0 ] & intlv[i+5].mask0 ) ? intlv[i+5].mask1 : 0;
        _msg_enc[ intlv[i+6].p1 ] |= (_msg_dec[ intlv[i+6].p0 ] & intlv[i+6].mask0 ) ? intlv[i+6].mask1 : 0;
        _msg_enc[ intlv[i+7].p1 ] |= (_msg_dec[ intlv[i+7].p0 ] & intlv[i+7].mask0 ) ? intlv[i+7].mask1 : 0;
    }
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

    // clear output array
    memset(_msg_dec, 0x00, (ncbps/8)*sizeof(unsigned char));

    // run de-interleaver (run interleaver in reverse), loop unwrapped
    unsigned int i;
    for (i=0; i<ncbps; i+=8) {
        _msg_dec[ intlv[i  ].p0 ] |= (_msg_enc[ intlv[i  ].p1 ] & intlv[i  ].mask1 ) ? intlv[i  ].mask0 : 0;
        _msg_dec[ intlv[i+1].p0 ] |= (_msg_enc[ intlv[i+1].p1 ] & intlv[i+1].mask1 ) ? intlv[i+1].mask0 : 0;
        _msg_dec[ intlv[i+2].p0 ] |= (_msg_enc[ intlv[i+2].p1 ] & intlv[i+2].mask1 ) ? intlv[i+2].mask0 : 0;
        _msg_dec[ intlv[i+3].p0 ] |= (_msg_enc[ intlv[i+3].p1 ] & intlv[i+3].mask1 ) ? intlv[i+3].mask0 : 0;
        _msg_dec[ intlv[i+4].p0 ] |= (_msg_enc[ intlv[i+4].p1 ] & intlv[i+4].mask1 ) ? intlv[i+4].mask0 : 0;
        _msg_dec[ intlv[i+5].p0 ] |= (_msg_enc[ intlv[i+5].p1 ] & intlv[i+5].mask1 ) ? intlv[i+5].mask0 : 0;
        _msg_dec[ intlv[i+6].p0 ] |= (_msg_enc[ intlv[i+6].p1 ] & intlv[i+6].mask1 ) ? intlv[i+6].mask0 : 0;
        _msg_dec[ intlv[i+7].p0 ] |= (_msg_enc[ intlv[i+7].p1 ] & intlv[i+7].mask1 ) ? intlv[i+7].mask0 : 0;
    }
}

