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
    unsigned long int n = 20000;
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

