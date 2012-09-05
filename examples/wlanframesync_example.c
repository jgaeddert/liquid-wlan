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
// wlanframesync_example.c
//
// Test generation/synchronization of wlan frame
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>

#include "liquid-wlan.h"

#define OUTPUT_FILENAME "wlanframesync_example.m"

void usage()
{
    printf("Usage: wlanframesync_example [OPTION]\n");
    printf("  h     : print help\n");
    printf("  d     : enable debugging\n");
    printf("  s     : signal-to-noise ratio [dB], default: 30\n");
    printf("  F     : carrier frequency offset, default: 0.002\n");
    printf("  n     : number of data bytes\n");
    printf("  r     : rate {6,9,12,18,24,36,48,54} M bits/s\n");
}

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata);

int main(int argc, char*argv[])
{
    srand(time(NULL));
    
    // data options
    struct wlan_txvector_s txvector;
    txvector.LENGTH      = 100;
    txvector.DATARATE    = WLANFRAME_RATE_36;
    txvector.SERVICE     = 0;
    txvector.TXPWR_LEVEL = 0;

    // debug options
    int debug_enabled = 0;
    
    // channel options
    float noise_floor = -120.0f;        // noise floor [dB]
    float SNRdB = 20.0f;                // signal-to-noise ratio [dB]
    float phi   = 0.0f;                 // carrier phase offset
    float dphi  = 0.0f;                 // carrier frequency offset

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hds:F:n:r:")) != EOF){
        switch (dopt) {
        case 'h': usage();                          return 0;
        case 'd': debug_enabled = 1;                break;
        case 's': SNRdB = atof(optarg);             break;
        case 'F': dphi  = atof(optarg);             break;
        case 'n': txvector.LENGTH = atoi(optarg);   break;
        case 'r':
            switch ( atoi(optarg) ) {
            case 6:  txvector.DATARATE = WLANFRAME_RATE_6;  break;
            case 9:  txvector.DATARATE = WLANFRAME_RATE_9;  break;
            case 12: txvector.DATARATE = WLANFRAME_RATE_12; break;
            case 18: txvector.DATARATE = WLANFRAME_RATE_18; break;
            case 24: txvector.DATARATE = WLANFRAME_RATE_24; break;
            case 36: txvector.DATARATE = WLANFRAME_RATE_36; break;
            case 48: txvector.DATARATE = WLANFRAME_RATE_48; break;
            case 54: txvector.DATARATE = WLANFRAME_RATE_54; break;
            default:
                fprintf(stderr,"error: %s, invalid rate '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    // validate input
    if (txvector.LENGTH == 0 || txvector.LENGTH > 4095) {
        fprintf(stderr,"error: %s, input vector length must be in [0,4095]\n", argv[0]);
        exit(1);
    }
    
    unsigned int i;

    // initialize original data message
    unsigned char msg_org[txvector.LENGTH];
    for (i=0; i<txvector.LENGTH; i++)
        msg_org[i] = rand() & 0xff;

    float nstd  = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);

    // arrays
    float complex buffer[80];   // data buffer

    // create frame generator
    wlanframegen fg = wlanframegen_create();

    // create frame synchronizer
    wlanframesync fs = wlanframesync_create(callback, (void*)msg_org);
    if (debug_enabled)
        wlanframesync_debug_enable(fs);
    //wlanframesync_print(fs);

    // assemble frame and print
    wlanframegen_assemble(fg, msg_org, txvector);
    wlanframegen_print(fg);

    // open output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"x = [];\n");
    unsigned int n = 0;

    // push noise through synchronizer
    unsigned int d = 32*64 + 2;
    for (i=0; i<d; i++) {
        buffer[0] = nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
        wlanframesync_execute(fs, buffer, 1);
    }

    // generate/synchronize frame
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = wlanframegen_writesymbol(fg, buffer);

        // push through channel (add noise, carrier offset)
        for (i=0; i<80; i++) {
            buffer[i] *= cexpf(_Complex_I*(phi + dphi*n));
            buffer[i] *= gamma;
            buffer[i] += nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;

            // write buffer to file
            fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", n+1, crealf(buffer[i]), cimagf(buffer[i]));

            n++;
        }

        // run through synchronize
        wlanframesync_execute(fs, buffer, 80);
    }

    // plot results
    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(length(x)-1);\n");
    fprintf(fid,"plot(t,real(x), t,imag(x));\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // write debug output if enabled
    if (debug_enabled)
        wlanframesync_debug_print(fs, "wlanframesync_debug.m");

    // destroy objects
    wlanframegen_destroy(fg);
    wlanframesync_destroy(fs);

    printf("done.\n");
    return 0;
}

static int callback(unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata)
{
    printf("**** callback invoked\n");

    // type cast 'userdata' as original data vector
    unsigned char * msg_org = (unsigned char*) _userdata;

    // count errors
    unsigned int num_bit_errors = count_bit_errors_array(_payload, msg_org, _rxvector.LENGTH);
    printf("bit errors : %4u / %4u\n", num_bit_errors, 8*_rxvector.LENGTH);

    return 0;
}

