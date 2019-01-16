/*
 * Copyright (c) 2019 Joseph Gaeddert
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
// wlanframesync_performance_example.c
//
// Test performance of frame detection, header decoding, and payload
// decoding in additive white Gauss noise (AWGN) channels.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <liquid/liquid.h>
#include "liquid-wlan.h"

void usage()
{
    printf("Usage: wlanframesync_example [OPTION]\n");
    printf(" -h         : print help\n");
    printf(" -s <snr>   : starting SNR (dB),  default: -3\n");
    printf(" -d <step>  : SNR step size (dB), default:  1\n");
    printf(" -x <snr>   : stopping SNR (dB),  default:  10\n");
    printf(" -n <num>   : number of trials,   default: 1000\n");
    printf(" -r <seed>  : random seed,        default: time(NULL)\n");
}

int frame_detected;
int header_decoded;
int payload_decoded;

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata);

void run_trial(wlanframegen  _fg,
               wlanframesync _fs,
               float         _snr);

int main(int argc, char*argv[])
{
    // options
    float           SNRdB_min   = -3.0f;    // SNR (dB), min
    float           SNRdB_step  =  1.0f;    // SNR (dB), step
    float           SNRdB_max   = 10.0f;    // SNR (dB), max
    unsigned int    num_trials  = 1000;     // number of trials to run
    unsigned int    seed        =    0;     // random seed

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hs:d:x:n:r:")) != EOF){
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 's': SNRdB_min  = atof(optarg);    break;
        case 'd': SNRdB_step = atof(optarg);    break;
        case 'x': SNRdB_max  = atof(optarg);    break;
        case 'n': num_trials = atoi(optarg);    break;
        case 'r': seed       = atoi(optarg);    break;
        default:
            fprintf(stderr,"error: %s, invalid rate '%s'\n", argv[0], optarg);
            exit(1);
        }
    }
    // validate input
    if (SNRdB_min >= SNRdB_max) {
        fprintf(stderr,"error: invalid SNR range\n");
        exit(1);
    } else if (SNRdB_step <= 0) {
        fprintf(stderr,"error: invalid SNR step size\n");
        exit(1);
    }

    // set random seed
    srand(seed==0 ? time(NULL) : seed);

    // create frame generator and synchronizer objects
    wlanframegen fg  = wlanframegen_create();
    wlanframesync fs = wlanframesync_create(callback, NULL);

    // start trials
    float SNRdB;
    for (SNRdB = SNRdB_min; SNRdB <= SNRdB_max; SNRdB += SNRdB_step) {
        unsigned int n;
        unsigned int num_frames_detected = 0;
        unsigned int num_headers_decoded = 0;
        unsigned int num_payloads_decoded= 0;
        for (n=0; n<num_trials; n++) {
            // run trial and update counters
            run_trial(fg, fs, SNRdB);
            num_frames_detected  += frame_detected;
            num_headers_decoded  += header_decoded;
            num_payloads_decoded += payload_decoded;
        }
        // print results to screen
        printf("  %8.3f %6u %6u %6u\n", SNRdB, 
            num_frames_detected,
            num_headers_decoded,
            num_payloads_decoded);
    }

    // destroy objects and return
    wlanframegen_destroy(fg);
    wlanframesync_destroy(fs);
    return 0;
}

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata)
{
    frame_detected  = 1;
    header_decoded  = 1;
    payload_decoded = 1;
    return 0;
}

void run_trial(wlanframegen  _fg,
               wlanframesync _fs,
               float         _snr)
{
    struct wlan_txvector_s txvector = {
        .LENGTH      = 16,
        .DATARATE    = WLANFRAME_RATE_36,
        .SERVICE     = 0,
        .TXPWR_LEVEL = 0};

    float noise_floor = -120.0f;        // noise floor [dB]
    float nstd  = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (_snr + noise_floor)/20.0f);

    // reset objects and global flags
    wlanframegen_reset(_fg);
    wlanframesync_reset(_fs);
    frame_detected  = 0;
    header_decoded  = 0;
    payload_decoded = 0;

    //
    float complex buffer[80];
    unsigned char msg_org[txvector.LENGTH];

    // assemble frame and print
    unsigned int i;
    for (i=0; i<txvector.LENGTH; i++)
        msg_org[i] = rand() & 0xff;
    wlanframegen_assemble(_fg, msg_org, txvector);

    // push noise through synchronizer
    unsigned int d = rand() & 0xff;
    for (i=0; i<d; i++) {
        buffer[0] = nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
        wlanframesync_execute(_fs, buffer, 1);
    }

    // generate/synchronize frame
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = wlanframegen_writesymbol(_fg, buffer);

        // push through channel (add noise, carrier offset)
        for (i=0; i<80; i++)
            buffer[i] = buffer[i]*gamma + nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;

        // run through synchronizer
        wlanframesync_execute(_fs, buffer, 80);
    }

    // push more noise through synchronizer
    for (i=0; i<100; i++) {
        buffer[0] = nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
        wlanframesync_execute(_fs, buffer, 1);
    }
}

