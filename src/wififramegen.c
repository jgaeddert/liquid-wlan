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
// wififramegen.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid-802-11.internal.h"

#define DEBUG_WIFIFRAMEGEN            1

struct wififramegen_s {
    // scaling factors
    float g_data;

    // transform object
    FFT_PLAN ifft;          // ifft object
    float complex * X;      // frequency-domain buffer
    float complex * x;      // time-domain buffer

    // PLCP short
    float complex * S0;     // short sequence (frequency)
    float complex * s0;     // short sequence (time)

    // PLCP long
    float complex * S1;     // long sequence (frequency)
    float complex * s1;     // long sequence (time)
};

wififramegen wififramegen_create()
{
    wififramegen q = (wififramegen) malloc(sizeof(struct wififramegen_s));

    // allocate memory for transform objects
    q->X = (float complex*) malloc(64*sizeof(float complex));
    q->x = (float complex*) malloc(64*sizeof(float complex));
    q->ifft = FFT_CREATE_PLAN(64, q->X, q->x, FFT_DIR_BACKWARD, FFT_METHOD);

    // compute scaling factor
    q->g_data = 1.0f / sqrtf(52.0f);

    return q;
}

void wififramegen_destroy(wififramegen _q)
{
    // free transform array memory
    free(_q->X);
    free(_q->x);
    FFT_DESTROY_PLAN(_q->ifft);

    // free main object memory
    free(_q);
}

void wififramegen_print(wififramegen _q)
{
    printf("wififramegen:\n");
}

void wififramegen_reset(wififramegen _q)
{
}

void wififramegen_write_S0(wififramegen _q,
                           float complex * _y)
{
    //memmove(_y, _q->s0, (_q->M)*sizeof(float complex));
}


void wififramegen_write_S1(wififramegen _q,
                           float complex * _y)
{
    //memmove(_y, _q->s1, (_q->M)*sizeof(float complex));
}


// write OFDM symbol
//  _q      :   framging generator object
//  _x      :   input symbols, [size: _M x 1]
//  _y      :   output samples, [size: _M x 1]
void wififramegen_writesymbol(wififramegen _q,
                              float complex * _x,
                              float complex * _y)
{
#if 0
    // move frequency data to internal buffer
    unsigned int i;
    unsigned int k;
    int sctype;
    for (i=0; i<_q->M; i++) {
        // start at mid-point (effective fftshift)
        k = (i + _q->M/2) % _q->M;

        sctype = _q->p[k];
        if (sctype==WIFIFRAME_SCTYPE_NULL) {
            // disabled subcarrier
            _q->X[k] = 0.0f;
        } else if (sctype==WIFIFRAME_SCTYPE_PILOT) {
            // pilot subcarrier
            _q->X[k] = (msequence_advance(_q->ms_pilot) ? 1.0f : -1.0f) * _q->g_data;
        } else {
            // data subcarrier
            _q->X[k] = _x[k] * _q->g_data;
        }

        //printf("X[%3u] = %12.8f + j*%12.8f;\n",i+1,crealf(_q->X[i]),cimagf(_q->X[i]));
    }

    // execute transform
    FFT_EXECUTE(_q->ifft);

    // copy result to output
    memmove( _y, &_q->x[_q->M - _q->cp_len], (_q->cp_len)*sizeof(float complex));
    memmove(&_y[_q->cp_len], _q->x, (_q->M)*sizeof(float complex));
#endif
}


