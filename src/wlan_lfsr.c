/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2012 Joseph Gaeddert
 * Copyright (c) 2007, 2008, 2009, 2010, 2012 Virginia Polytechnic
 *                                      Institute & State University
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
// linear feedback shift register
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid-wlan.internal.h"

// create a linear feedback shift register (LFSR) object with
// an internal shift register length of _m bits.
//  _m      :   generator polynomial length, sequence length is (2^m)-1
//  _g      :   generator polynomial, starting with most-significant bit
//  _a      :   initial shift register state, default: 000...001
wlan_lfsr wlan_lfsr_create(unsigned int _m,
                           unsigned int _g,
                           unsigned int _a)
{
    // allocate memory for wlan_lfsr object
    wlan_lfsr ms = (wlan_lfsr) malloc(sizeof(struct wlan_lfsr_s));

    // set internal values
    ms->m = _m;         // generator polynomial length
    ms->g = _g >> 1;    // generator polynomial (clip off most significant bit)

    // initialize state register, reversing order
    // 0001 -> 1000
    unsigned int i;
    ms->a = 0;
    for (i=0; i<ms->m; i++) {
        ms->a <<= 1;
        ms->a |= (_a & 0x01);
        _a >>= 1;
    }

    ms->n = (1<<_m)-1;  // sequence length, (2^m)-1
    ms->v = ms->a;      // shift register
    ms->b = 0;          // return bit

    return ms;
}


// destroy an wlan_lfsr object, freeing all internal memory
void wlan_lfsr_destroy(wlan_lfsr _ms)
{
    free(_ms);
}

// advance wlan_lfsr on shift register, returning output bit
unsigned int wlan_lfsr_advance(wlan_lfsr _ms)
{
    // compute return bit as binary dot product between the
    // internal shift register and the generator polynomial
    _ms->b = liquid_wlan_bdotprod( _ms->v, _ms->g );

    _ms->v <<= 1;       // shift internal register
    _ms->v |= _ms->b;   // push bit onto register
    _ms->v &= _ms->n;   // apply mask to register

    return _ms->b;      // return result
}

// generate pseudo-random symbol from shift register
//  _ms     :   m-sequence object
//  _bps    :   bits per symbol of output
unsigned int wlan_lfsr_generate_symbol(wlan_lfsr _ms,
                                       unsigned int _bps)
{
    unsigned int i;
    unsigned int s = 0;
    for (i=0; i<_bps; i++) {
        s <<= 1;
        s |= wlan_lfsr_advance(_ms);
    }
    return s;
}

// reset wlan_lfsr shift register to original state, typically '1'
void wlan_lfsr_reset(wlan_lfsr _ms)
{
    _ms->v = _ms->a;
}

