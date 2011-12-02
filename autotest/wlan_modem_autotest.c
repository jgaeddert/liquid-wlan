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
// wlan_modem_autotest.c
//
// Test modulation/demodulation
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"

// run test with a specific rate
int wlan_modem_runtest(unsigned int _scheme)
{
    unsigned int bps = 0;
    switch (_scheme) {
    case WLAN_MODEM_BPSK:  bps = 1; break;
    case WLAN_MODEM_QPSK:  bps = 2; break;
    case WLAN_MODEM_QAM16: bps = 4; break;
    case WLAN_MODEM_QAM64: bps = 6; break;
    default:
        fprintf(stderr,"error: wlan_modem_runtest(), invalid scheme\n");
        exit(1);
    }

    unsigned int M = 1<<bps;

    float complex x;

    unsigned int i;
    unsigned int s;
    unsigned int num_errors = 0;
    printf("-----\n");
    for (i=0; i<M; i++) {
        x = wlan_modulate(_scheme, i);
        s = wlan_demodulate(_scheme, x);

        // print results
        printf("  %3u > %12.8f + j%12.8f > %3u %s\n", i, crealf(x), cimagf(x), s, i==s ? "" : "*");

        num_errors += i==s ? 0 : 1;
    }

    return num_errors;
}


int main() {
    // run tests
    unsigned int num_errors;

    // BPSK modem
    num_errors = wlan_modem_runtest(WLAN_MODEM_BPSK);
    if (num_errors > 0) {
        fprintf(stderr,"fail: %s, BPSK modem failure\n", __FILE__);
        exit(1);
    }

    // QPSK modem
    num_errors = wlan_modem_runtest(WLAN_MODEM_QPSK);
    if (num_errors > 0) {
        fprintf(stderr,"fail: %s, QPSK modem failure\n", __FILE__);
        exit(1);
    }

    // 16-QAM modem
    num_errors = wlan_modem_runtest(WLAN_MODEM_QAM16);
    if (num_errors > 0) {
        fprintf(stderr,"fail: %s, 16-QAM modem failure\n", __FILE__);
        exit(1);
    }

    // 64-QAM modem
    num_errors = wlan_modem_runtest(WLAN_MODEM_QAM64);
    if (num_errors > 0) {
        fprintf(stderr,"fail: %s, 64-QAM modem failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

