// Test data scrambler

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-wlan.internal.h"

int main(int argc, char*argv[])
{
    // test sequence (shift register intialized with all ones)
    unsigned char sequence_test[127] = {
        0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0,
        1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 
        0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0,
        1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 
        1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1,
        1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 
        1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1,
        1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};

    // create m-sequence generator
    unsigned int m = 7;     // polynomial order
    unsigned int g = 0x91;  // generator polynomial: x^7 + x^4 + 1 = 1001 0001
    unsigned int a = 0x7f;  // initial state: .111 1111

    // create sequence generator
    wlan_lfsr ms = wlan_lfsr_create(m, g, a);

    unsigned int i;
    for (i=0; i<127; i++) {
        unsigned int bit = wlan_lfsr_advance(ms);

        if (bit != sequence_test[i]) {
            fprintf(stderr,"fail: %s, sequence failure\n", __FILE__);
            exit(1);
        }
    }

    // destroy sequence
    wlan_lfsr_destroy(ms);

    printf("done.\n");
    return 0;
}

