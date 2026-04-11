// wlan data scrambler/unscrambler

#include <stdio.h>
#include <stdlib.h>

#include "liquid-wlan.internal.h"

// scramble data
//  _msg_dec    :   original data message [size: _n x 1]
//  _msg_enc    :   scrambled data message [size: _n x 1]
//  _n          :   length of input/output (bytes)
//  _seed       :   linear feedback shift register initial state
void wlan_data_scramble(unsigned char * _msg_dec,
                        unsigned char * _msg_enc,
                        unsigned int _n,
                        unsigned int _seed)
{
    // create m-sequence generator
    unsigned int m = 7;     // polynomial order
    unsigned int g = 0x91;  // generator polynomial: x^7 + x^4 + 1 = 1001 0001
    unsigned int a = _seed; // initial state

    // create
    wlan_lfsr ms = wlan_lfsr_create(m, g, a);

    unsigned int i;
    unsigned char mask;
    for (i=0; i<_n; i++) {
        // generate byte mask (shift 8 bits)
        mask = wlan_lfsr_generate_symbol(ms, 8);

        // apply mask
        _msg_enc[i] = _msg_dec[i] ^ mask;
    }

    // destroy wlan_lfsr object
    wlan_lfsr_destroy(ms);
}

// unscramble data
//  _msg_enc    :   scrambled data message [size: _n x 1]
//  _msg_dec    :   original data message [size: _n x 1]
//  _n          :   length of input/output (bytes)
//  _seed       :   linear feedback shift register initial state
void wlan_data_unscramble(unsigned char * _msg_enc,
                          unsigned char * _msg_dec,
                          unsigned int _n,
                          unsigned int _seed)
{
    // same as scrambler
    wlan_data_scramble(_msg_enc, _msg_dec, _n, _seed);
}

