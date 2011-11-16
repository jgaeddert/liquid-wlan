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
// wififramesync.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid-802-11.internal.h"

#define DEBUG_WIFIFRAMESYNC             0
#define DEBUG_WIFIFRAMESYNC_PRINT       0
#define DEBUG_WIFIFRAMESYNC_FILENAME    "wififramesync_internal_debug.m"
#define DEBUG_WIFIFRAMESYNC_BUFFER_LEN  (2048)

#define WIFIFRAMESYNC_ENABLE_SQUELCH    0

struct wififramesync_s {
    // callback
    wififramesync_callback callback;
    void * userdata;

    // transform object
    FFT_PLAN fft;           // ifft object
    float complex * X;      // frequency-domain buffer
    float complex * x;      // time-domain buffer
    windowcf input_buffer;  // input sequence buffer

#if DEBUG_WIFIFRAMESYNC
    agc_crcf agc_rx;        // automatic gain control (rssi)
    windowcf debug_x;
    windowf  debug_rssi;
#endif
};

wififramesync wififramesync_create(wififramesync_callback _callback,
                                   void * _userdata)
{
    // allocate main object memory
    wififramesync q = (wififramesync) malloc(sizeof(struct wififramesync_s));
    
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
    wififrame_init_S0(q->p, q->M, q->S0, q->s0, &q->M_S0);
    wififrame_init_S1(q->p, q->M, q->S1, q->s1, &q->M_S1);
#endif

    // reset object
    wififramesync_reset(q);

#if DEBUG_WIFIFRAMESYNC
    // agc, rssi
    q->agc_rx = agc_crcf_create();
    agc_crcf_set_bandwidth(q->agc_rx,  1e-2f);
    agc_crcf_set_gain_limits(q->agc_rx, 1e-5f, 1e5f);

    q->debug_x =        windowcf_create(DEBUG_WIFIFRAMESYNC_BUFFER_LEN);
    q->debug_rssi =     windowf_create(DEBUG_WIFIFRAMESYNC_BUFFER_LEN);
#endif

    // return object
    return q;
}

void wififramesync_destroy(wififramesync _q)
{
#if DEBUG_WIFIFRAMESYNC
    wififramesync_debug_print(_q, DEBUG_WIFIFRAMESYNC_FILENAME);

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

void wififramesync_print(wififramesync _q)
{
    printf("wififramesync:\n");
}

void wififramesync_reset(wififramesync _q)
{
}

void wififramesync_execute(wififramesync _q,
                           float complex * _x,
                           unsigned int _n)
{
}

// get receiver RSSI
float wififramesync_get_rssi(wififramesync _q)
{
    return 0.0f;
}

// get receiver carrier frequency offset estimate
float wififramesync_get_cfo(wififramesync _q)
{
    return 0.0f;
}


//
// internal methods
//

// frame detection
void wififramesync_execute_seekplcp(wififramesync _q)
{
}

// frame detection
void wififramesync_execute_plcpshort0(wififramesync _q)
{
}

// frame detection
void wififramesync_execute_plcpshort1(wififramesync _q)
{
}

void wififramesync_execute_plcplong(wififramesync _q)
{
}

void wififramesync_execute_rxsymbols(wififramesync _q)
{
}

// compute S0 metrics
void wififramesync_S0_metrics(wififramesync _q,
                              float complex * _G,
                              float complex * _s_hat)
{
}

// estimate short sequence gain
//  _q      :   wififramesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wififramesync_estimate_gain_S0(wififramesync _q,
                                    float complex * _x,
                                    float complex * _G)
{
}

// estimate long sequence gain
//  _q      :   wififramesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wififramesync_estimate_gain_S1(wififramesync _q,
                                    float complex * _x,
                                    float complex * _G)
{
}

// estimate complex equalizer gain from G0 and G1
//  _q      :   wififramesync object
//  _ntaps  :   number of time-domain taps for smoothing
void wififramesync_estimate_eqgain(wififramesync _q,
                                   unsigned int _ntaps)
{
}

// estimate complex equalizer gain from G0 and G1 using polynomial fit
//  _q      :   wififramesync object
//  _order  :   polynomial order
void wififramesync_estimate_eqgain_poly(wififramesync _q,
                                        unsigned int _order)
{
}

// recover symbol, correcting for gain, pilot phase, etc.
void wififramesync_rxsymbol(wififramesync _q)
{
}


void wififramesync_debug_print(wififramesync _q,
                               const char * _filename)
{
    FILE * fid = fopen(_filename,"w");
    if (!fid) {
        fprintf(stderr,"error: wififrame_debug_print(), could not open '%s' for writing\n", _filename);
        return;
    }
    fprintf(fid,"%% %s : auto-generated file\n", DEBUG_WIFIFRAMESYNC_FILENAME);
#if DEBUG_WIFIFRAMESYNC
    fprintf(fid,"close all;\n");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"n = %u;\n", DEBUG_WIFIFRAMESYNC_BUFFER_LEN);
    unsigned int i;
    float complex * rc;
    float * r;

    fprintf(fid,"x = zeros(1,n);\n");
    windowcf_read(_q->debug_x, &rc);
    for (i=0; i<DEBUG_WIFIFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(0:(n-1),real(x),0:(n-1),imag(x));\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('received signal, x');\n");

    // write agc_rssi
    fprintf(fid,"\n\n");
    fprintf(fid,"agc_rssi = zeros(1,%u);\n", DEBUG_WIFIFRAMESYNC_BUFFER_LEN);
    windowf_read(_q->debug_rssi, &r);
    for (i=0; i<DEBUG_WIFIFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"agc_rssi(%4u) = %12.4e;\n", i+1, r[i]);
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(agc_rssi)\n");
    fprintf(fid,"ylabel('RSSI [dB]');\n");
#else
    fprintf(fid,"disp('no debugging info available');\n");
#endif

    fclose(fid);
    printf("wififramesync/debug: results written to '%s'\n", _filename);
}


