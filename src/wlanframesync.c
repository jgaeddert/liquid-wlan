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
// wlanframesync.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid-wlan.internal.h"

#define DEBUG_WLANFRAMESYNC             0
#define DEBUG_WLANFRAMESYNC_PRINT       0
#define DEBUG_WLANFRAMESYNC_FILENAME    "wlanframesync_internal_debug.m"
#define DEBUG_WLANFRAMESYNC_BUFFER_LEN  (2048)

#define WLANFRAMESYNC_ENABLE_SQUELCH    0

struct wlanframesync_s {
    // callback
    wlanframesync_callback callback;
    void * userdata;

    // transform object
    FFT_PLAN fft;           // ifft object
    float complex * X;      // frequency-domain buffer
    float complex * x;      // time-domain buffer
    windowcf input_buffer;  // input sequence buffer

#if DEBUG_WLANFRAMESYNC
    agc_crcf agc_rx;        // automatic gain control (rssi)
    windowcf debug_x;
    windowf  debug_rssi;
#endif
};

// create WLAN framing synchronizer object
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data structure
wlanframesync wlanframesync_create(wlanframesync_callback _callback,
                                   void *                 _userdata)
{
    // allocate main object memory
    wlanframesync q = (wlanframesync) malloc(sizeof(struct wlanframesync_s));
    
    // set callback data
    q->callback = _callback;
    q->userdata = _userdata;

    // create transform object
    q->X = (float complex*) malloc(64*sizeof(float complex));
    q->x = (float complex*) malloc(64*sizeof(float complex));
    q->fft = FFT_CREATE_PLAN(64, q->x, q->X, FFT_DIR_FORWARD, FFT_METHOD);
 
    // create input buffer the length of the transform
    q->input_buffer = windowcf_create(80);

#if 0
    // allocate memory for PLCP arrays
    q->S0 = (float complex*) malloc(64*sizeof(float complex));
    q->s0 = (float complex*) malloc(64*sizeof(float complex));
    q->S1 = (float complex*) malloc(64*sizeof(float complex));
    q->s1 = (float complex*) malloc(64*sizeof(float complex));
    wlanframe_init_S0(q->p, q->M, q->S0, q->s0, &q->M_S0);
    wlanframe_init_S1(q->p, q->M, q->S1, q->s1, &q->M_S1);
#endif

    // reset object
    wlanframesync_reset(q);

#if DEBUG_WLANFRAMESYNC
    // agc, rssi
    q->agc_rx = agc_crcf_create();
    agc_crcf_set_bandwidth(q->agc_rx,  1e-2f);
    agc_crcf_set_gain_limits(q->agc_rx, 1e-5f, 1e5f);

    q->debug_x =        windowcf_create(DEBUG_WLANFRAMESYNC_BUFFER_LEN);
    q->debug_rssi =     windowf_create(DEBUG_WLANFRAMESYNC_BUFFER_LEN);
#endif

    // return object
    return q;
}

// destroy WLAN framing synchronizer object
void wlanframesync_destroy(wlanframesync _q)
{
#if DEBUG_WLANFRAMESYNC
    wlanframesync_debug_print(_q, DEBUG_WLANFRAMESYNC_FILENAME);

    agc_crcf_destroy(_q->agc_rx);

    windowcf_destroy(_q->debug_x);
    windowf_destroy(_q->debug_rssi);
#endif

    // free transform object
    windowcf_destroy(_q->input_buffer);
    free(_q->X);
    free(_q->x);
    FFT_DESTROY_PLAN(_q->fft);

    // free main object memory
    free(_q);
}

// print WLAN framing synchronizer object internals
void wlanframesync_print(wlanframesync _q)
{
    printf("wlanframesync:\n");
}

// reset WLAN framing synchronizer object internal state
void wlanframesync_reset(wlanframesync _q)
{
}

// execute framing synchronizer on input buffer
//  _q      :   framing synchronizer object
//  _buffer :   input buffer [size: _n x 1]
//  _n      :   input buffer size
void wlanframesync_execute(wlanframesync          _q,
                           liquid_float_complex * _buffer,
                           unsigned int           _n)
{
}

// get receiver RSSI
float wlanframesync_get_rssi(wlanframesync _q)
{
    return 0.0f;
}

// get receiver carrier frequency offset estimate
float wlanframesync_get_cfo(wlanframesync _q)
{
    return 0.0f;
}


//
// internal methods
//

// frame detection
void wlanframesync_execute_seekplcp(wlanframesync _q)
{
}

// frame detection
void wlanframesync_execute_plcpshort0(wlanframesync _q)
{
}

// frame detection
void wlanframesync_execute_plcpshort1(wlanframesync _q)
{
}

void wlanframesync_execute_plcplong(wlanframesync _q)
{
}

void wlanframesync_execute_rxsymbols(wlanframesync _q)
{
}

// compute S0 metrics
void wlanframesync_S0_metrics(wlanframesync _q,
                              float complex * _G,
                              float complex * _s_hat)
{
}

// estimate short sequence gain
//  _q      :   wlanframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wlanframesync_estimate_gain_S0(wlanframesync _q,
                                    float complex * _x,
                                    float complex * _G)
{
}

// estimate long sequence gain
//  _q      :   wlanframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wlanframesync_estimate_gain_S1(wlanframesync _q,
                                    float complex * _x,
                                    float complex * _G)
{
}

// estimate complex equalizer gain from G0 and G1
//  _q      :   wlanframesync object
//  _ntaps  :   number of time-domain taps for smoothing
void wlanframesync_estimate_eqgain(wlanframesync _q,
                                   unsigned int _ntaps)
{
}

// estimate complex equalizer gain from G0 and G1 using polynomial fit
//  _q      :   wlanframesync object
//  _order  :   polynomial order
void wlanframesync_estimate_eqgain_poly(wlanframesync _q,
                                        unsigned int _order)
{
}

// recover symbol, correcting for gain, pilot phase, etc.
void wlanframesync_rxsymbol(wlanframesync _q)
{
}


void wlanframesync_debug_print(wlanframesync _q,
                               const char * _filename)
{
    FILE * fid = fopen(_filename,"w");
    if (!fid) {
        fprintf(stderr,"error: wlanframe_debug_print(), could not open '%s' for writing\n", _filename);
        return;
    }
    fprintf(fid,"%% %s : auto-generated file\n", DEBUG_WLANFRAMESYNC_FILENAME);
#if DEBUG_WLANFRAMESYNC
    fprintf(fid,"close all;\n");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"n = %u;\n", DEBUG_WLANFRAMESYNC_BUFFER_LEN);
    unsigned int i;
    float complex * rc;
    float * r;

    fprintf(fid,"x = zeros(1,n);\n");
    windowcf_read(_q->debug_x, &rc);
    for (i=0; i<DEBUG_WLANFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(0:(n-1),real(x),0:(n-1),imag(x));\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('received signal, x');\n");

    // write agc_rssi
    fprintf(fid,"\n\n");
    fprintf(fid,"agc_rssi = zeros(1,%u);\n", DEBUG_WLANFRAMESYNC_BUFFER_LEN);
    windowf_read(_q->debug_rssi, &r);
    for (i=0; i<DEBUG_WLANFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"agc_rssi(%4u) = %12.4e;\n", i+1, r[i]);
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(agc_rssi)\n");
    fprintf(fid,"ylabel('RSSI [dB]');\n");
#else
    fprintf(fid,"disp('no debugging info available');\n");
#endif

    fclose(fid);
    printf("wlanframesync/debug: results written to '%s'\n", _filename);
}


