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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <sys/resource.h>
#include <liquid/liquid.h>
#include "liquid-wlan.h"

double calculate_execution_time(struct rusage _start, struct rusage _finish)
{
    return _finish.ru_utime.tv_sec - _start.ru_utime.tv_sec
        + 1e-6*(_finish.ru_utime.tv_usec - _start.ru_utime.tv_usec)
        + _finish.ru_stime.tv_sec - _start.ru_stime.tv_sec
        + 1e-6*(_finish.ru_stime.tv_usec - _start.ru_stime.tv_usec);
}

// Helper function to keep code base small
void wlanframesync_benchmark(struct rusage *     _start,
                             struct rusage *     _finish,
                             unsigned long int * _num_iterations,
                             unsigned int        _rate)
{
    // create buffer (full of noise)
    unsigned int n = 800;
    float complex buffer[n];

    unsigned long int i;
    for (i=0; i<n; i++) {
        buffer[i] = 0.001f*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;
    }

    // create frame synchronizer
    wlanframesync fs = wlanframesync_create(NULL, NULL);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        wlanframesync_execute(fs, buffer, n);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= n;

    // destroy frame sync
    wlanframesync_destroy(fs);
}

int main() {
    unsigned long int n = 10000;
    struct rusage start, finish;
    unsigned int rate = WLANFRAME_RATE_6;

    // run benchmark(s)
    wlanframesync_benchmark(&start, &finish, &n, rate);

    // compute execution time
    float extime = calculate_execution_time(start, finish);

    // print results
    printf("%-24s : time : %8.5f s, iterations : %8lu (%10.4e samples/s)\n", "wlanframesync", extime, n, (float)n/extime);
    
    return 0;
}

