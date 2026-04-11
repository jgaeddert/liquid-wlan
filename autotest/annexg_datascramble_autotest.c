// Test data scrambler
// data obtained from Annex G in 1999 specification (Tables G.13 & G.16)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>
#include "liquid-wlan.internal.h"

// data structures from Annex G
#include "annex-g-data/G13.c"
#include "annex-g-data/G16.c"

int main(int argc, char*argv[])
{
    //
    unsigned int n=18;          // data size (bytes)
    unsigned int seed = 0x5d;   // 1011101

    // first 144 data bits (Table G.13)
    unsigned char * msg_org = annexg_G13;

    // first 144 data bits, scrambled (Table G.16)
    unsigned char * msg_scrambled_test = annexg_G16;

    // arrays
    unsigned char msg_scrambled[18];
    unsigned char msg_unscrambled[18];

    // scramble data
    wlan_data_scramble(msg_org, msg_scrambled, n, seed);

    // check result
    if (count_bit_errors_array(msg_scrambled, msg_scrambled_test, n) > 0 ) {
        fprintf(stderr,"fail: %s, scrambling failure\n", __FILE__);
        exit(1);
    }

    // unscramble data
    wlan_data_unscramble(msg_scrambled, msg_unscrambled, n, seed);

    // check result
    if (count_bit_errors_array(msg_unscrambled, msg_org, n) > 0 ) {
        fprintf(stderr,"fail: %s, unscrambling failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

