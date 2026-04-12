// Test data scrambler; should generate this sequence (127 bits) with
// the 'all ones' inital state:
//  00001110 11110010 11001001 00000010
//  00100110 00101110 10110110 00001100
//  11010100 11100111 10110100 00101010
//  11111010 01010001 10111000 1111111
// 
// NOTE: the above sequence is the same as the phase of the BPSK pilot
//       subcarriers when replacing the 1s with '-1' and 0s with '1',
//       viz {1,1,1,1, -1,-1,-1,1, -1,-1,-,1-,1...}

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
    // create m-sequence generator
    unsigned int m = 7;     // polynomial order
    unsigned int g = 0x91;  // generator polynomial: x^7 + x^4 + 1 = 1001 0001
    unsigned int a = 0x7f;  // initial state: .111 1111

    // create
    wlan_lfsr ms = wlan_lfsr_create(m, g, a);

    unsigned int i;
    for (i=0; i<127; i++) {
        unsigned int bit = wlan_lfsr_advance(ms);
        printf("%1u", bit);

        if ( ((i+1)%8) == 0 )
            printf("\n");
    }
    printf("\n");

    wlan_lfsr_destroy(ms);


    printf("done.\n");
    return 0;
}

