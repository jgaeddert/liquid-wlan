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
// wlanframesync_autotest.c
//
// Test generation/synchronization of wlan frame(s)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>

#include "liquid-wlan.h"

#include "annex-g-data/G1.c"

#define OUTPUT_FILENAME "wlanframesync_example.m"

void usage()
{
    printf("Usage: wlanframesync_example [OPTION]\n");
    printf("  h     : print help\n");
    printf("  s     : signal-to-noise ratio [dB], default: 30\n");
    printf("  F     : carrier frequency offset, default: 0.002\n");
    printf("  r     : rate {6,9,12,18,24,36,48,54} M bits/s\n");
}

// run test with a specific rate
int wlanframesync_runtest(unsigned int _rate);

// callback function
static int callback(int                    _header_valid,
                    unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata);

int main() {
    // run tests
    wlanframesync_runtest(WLANFRAME_RATE_6);
    //wlanframesync_runtest(WLANFRAME_RATE_9);
    wlanframesync_runtest(WLANFRAME_RATE_12);
    wlanframesync_runtest(WLANFRAME_RATE_18);
    wlanframesync_runtest(WLANFRAME_RATE_24);
    wlanframesync_runtest(WLANFRAME_RATE_36);
    wlanframesync_runtest(WLANFRAME_RATE_48);
    wlanframesync_runtest(WLANFRAME_RATE_54);

    return 0;
}

// structure for tracking decoded frames
struct wlanframesync_autotest_s {
    unsigned char * msg_org;
    unsigned int length;
    unsigned int datarate;
    unsigned int num_frames;
    unsigned int valid;
};

int wlanframesync_runtest(unsigned int _rate)
{
    srand(time(NULL));
    
    // data options
    unsigned char * msg_org = annexg_G1;
    struct wlan_txvector_s txvector;
    txvector.LENGTH      = 100;
    txvector.DATARATE    = _rate;
    txvector.SERVICE     = 0;
    txvector.TXPWR_LEVEL = 0;
    
#if 0
    // channel options
    float noise_floor = -120.0f;        // noise floor [dB]
    float SNRdB = 20.0f;                // signal-to-noise ratio [dB]
    float phi   = 0.0f;                 // carrier phase offset
    float dphi  = 0.0f;                 // carrier frequency offset
    
    float nstd  = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);
#endif

    // arrays
    float complex buffer[80];   // data buffer

    // create frame generator
    wlanframegen fg = wlanframegen_create();

    // initialize test data object
    struct wlanframesync_autotest_s testdata;
    testdata.msg_org    = msg_org;
    testdata.length     = txvector.LENGTH;
    testdata.datarate   = txvector.DATARATE;
    testdata.num_frames = 0;
    testdata.valid      = 1;

    // create frame synchronizer
    wlanframesync fs = wlanframesync_create(callback, (void*)&testdata);
    //wlanframesync_print(fs);

    // assemble frame and print
    wlanframegen_assemble(fg, msg_org, txvector);
    wlanframegen_print(fg);

#if 0
    // push noise through synchronizer
    unsigned int d = 32*64 + 2;
    for (i=0; i<d; i++) {
        buffer[0] = nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
        wlanframesync_execute(fs, buffer, 1);
    }
#endif

    // generate/synchronize frame
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = wlanframegen_writesymbol(fg, buffer);

#if 0
        // push through channel (add noise, carrier offset)
        for (i=0; i<80; i++) {
            buffer[i] *= cexpf(_Complex_I*(phi + dphi*n));
            buffer[i] *= gamma;
            buffer[i] += nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
            n++;
        }
#endif

        // run through synchronize
        wlanframesync_execute(fs, buffer, 80);
    }

    // destroy objects
    wlanframegen_destroy(fg);
    wlanframesync_destroy(fs);

    // check results
    if (testdata.num_frames == 0) {
        fprintf(stderr,"wlanframesync_autotest: no frames detected!\n");
        testdata.valid = 0;
    }

    if (!testdata.valid) {
        fprintf(stderr,"fail: %s, synchronization failure (rate = %u)\n", __FILE__, _rate);
        exit(1);
    }
    
    return 0;
}

static int callback(int                    _header_valid,
                    unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata)
{
    printf("**** callback invoked\n");

    struct wlanframesync_autotest_s * testdata = (struct wlanframesync_autotest_s*) _userdata;

    if (!_header_valid) {
        fprintf(stderr,"wlanframesync_autotest: header invalid!\n");
        testdata->valid = 0;
        return 0;
    }

    // count errors
    unsigned int num_bit_errors = count_bit_errors_array(_payload, testdata->msg_org, _rxvector.LENGTH);
    printf("bit errors : %4u / %4u\n", num_bit_errors, 8*_rxvector.LENGTH);

    // increment number of frames decoded
    testdata->num_frames++;

    // check results
    if (num_bit_errors != 0) {
        fprintf(stderr,"wlanframesync_autotest: errors detected!\n");
        testdata->valid = 0;

    } else if (testdata->length != _rxvector.LENGTH) {
        fprintf(stderr,"wlanframesync_autotest: length mismatch\n");
        testdata->valid = 0;

    } else if (testdata->datarate != _rxvector.DATARATE) {
        fprintf(stderr,"wlanframesync_autotest: rate mismatch\n");
        testdata->valid = 0;

    } else if (testdata->num_frames != 1) {
        fprintf(stderr,"wlanframesync_autotest: frame number mismatch\n");
        testdata->valid = 0;
    }

    return 0;
}

