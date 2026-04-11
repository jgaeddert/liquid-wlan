#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <sys/resource.h>
#include "liquid-wlan.h"

double calculate_execution_time(struct rusage _start, struct rusage _finish)
{
    return _finish.ru_utime.tv_sec - _start.ru_utime.tv_sec
        + 1e-6*(_finish.ru_utime.tv_usec - _start.ru_utime.tv_usec)
        + _finish.ru_stime.tv_sec - _start.ru_stime.tv_sec
        + 1e-6*(_finish.ru_stime.tv_usec - _start.ru_stime.tv_usec);
}

// Helper function to keep code base small
void wlanframegen_benchmark(struct rusage *     _start,
                            struct rusage *     _finish,
                            unsigned long int * _num_iterations,
                            unsigned int        _rate)
{
    unsigned long int i;
    unsigned int dec_msg_len = 100;

    // options
    struct wlan_txvector_s txvector;
    txvector.LENGTH      = dec_msg_len;
    txvector.DATARATE    = _rate;
    txvector.SERVICE     = 0;
    txvector.TXPWR_LEVEL = 0;
    
    // create sample buffer
    float complex buffer[80];
   
    // initialize
    unsigned char msg_org[dec_msg_len];
    for (i=0; i<dec_msg_len; i++)
        msg_org[i] = rand() & 0xff;

    // create frame generator
    wlanframegen fg = wlanframegen_create();

    // sample counter
    unsigned long int n = 0;

    // start trials
    getrusage(RUSAGE_SELF, _start);

    for (i=0; i<(*_num_iterations); i++) {
        // assemble frame
        wlanframegen_assemble(fg, msg_org, txvector);

        // generate frame
        int last_frame = 0;
        while (!last_frame) {
            // write symbol to buffer
            last_frame = wlanframegen_writesymbol(fg, buffer);

            // increase sample counter
            n += 80;
        }
    }
    getrusage(RUSAGE_SELF, _finish);

    // set number of iterations to number of samples generated
    *_num_iterations = n;

    // destroy frame generator
    wlanframegen_destroy(fg);
}

int main() {
    unsigned long int n = 40000;
    struct rusage start, finish;
    unsigned int rate = WLANFRAME_RATE_6;

    // run benchmark(s)
    wlanframegen_benchmark(&start, &finish, &n, rate);

    // compute execution time
    float extime = calculate_execution_time(start, finish);

    // print results
    printf("%-24s : time : %8.5f s, iterations : %8lu (%10.4e samples/s)\n", "wlanframegen", extime, n, (float)n/extime);
    
    return 0;
}

