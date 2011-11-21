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
// annexg_framegen_autotest.c
//
// Test generation of WLAN frame (see Annex G, Table G.24)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.h"

#include "annex-g-data/G1.c"
#include "annex-g-data/G24.c"

#define OUTPUT_FILENAME "annexg_framegen_autotest.m"

int main(int argc, char*argv[])
{
    // error tolerance (needs to be high due to low precision of table data)
    float tol = 5e-2f;

    // options
    unsigned char * msg_org = annexg_G1;
    struct wlan_txvector_s txvector;
    txvector.LENGTH      = 100;
    txvector.DATARATE    = WLANFRAME_RATE_36;
    txvector.SERVICE     = 0;
    txvector.TXPWR_LEVEL = 0;
    
    // arrays
    float complex buffer[80];   // data buffer
    float complex frame[881];   // output frame
    float complex * frame_test = annexg_G24;

    // create frame generator
    wlanframegen fg = wlanframegen_create();

    // assemble frame and print
    wlanframegen_assemble(fg, msg_org, txvector);

    unsigned int i;
    unsigned int n = 0; // output sample counter

    // generate/synchronize frame
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = wlanframegen_writesymbol(fg, buffer);

        // save frame samples
        for (i=0; i<80; i++) {
            if (n < 881) frame[n++] = buffer[i] / 8; // apply scaling factor
        }
    }

    // destroy objects
    wlanframegen_destroy(fg);
    

    // run test
    unsigned int num_errors=0;
    for (i=0; i<881; i++) {
        float e = fabsf( frame[i] - frame_test[i] );
        printf("  x[%3u] = %8.4f + j%8.4f (%8.4f + j%8.4f), e = %12.4e %s\n",
                i,
                crealf(frame[i]),      cimagf(frame[i]),
                crealf(frame_test[i]), cimagf(frame_test[i]),
                e,
                e < tol ? "" : "*");

        // check error tolerance
        if (e > tol)
            num_errors++;
    }
    
    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"x = zeros(1,881);\n");
    fprintf(fid,"y = zeros(1,881);\n");

    for (i=0; i<881; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(frame[i]),      cimagf(frame[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(frame_test[i]), cimagf(frame_test[i]));
    }
    
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // check results
    printf("num sample errors : %3u / 881\n", num_errors);
    if (num_errors > 0) {
        fprintf(stderr,"fail: %s, failure\n", __FILE__);
        exit(1);
    }

    printf("done.\n");
    return 0;
}

