// Test interleaver; data obtained from Annex G in 1999 specification
// (Tables G.8 & G.9)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"

#include "annex-g-data/G8.c"
#include "annex-g-data/G9.c"

int main(int argc, char*argv[])
{
    // input messages
    unsigned char * msg_org = annexg_G8;

    // interleaved messages (tests)
    unsigned char * msg_test = annexg_G9;

    // options...
    //unsigned int ncbps = 48;    // number of coded bits per OFDM symbol
    //unsigned int nbpsc = 1;     // number of bits per subcarrier (modulation depth)
    unsigned int rate = WLANFRAME_RATE_6;
    
    unsigned int i;

    // 
    // interleave message
    //
    unsigned char msg_enc[6];
    wlan_interleaver_encode_symbol(rate, msg_org, msg_enc);
    
    // print results
    printf("interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x)\n", i, msg_org[i], msg_enc[i], msg_test[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc, msg_test, 6), 48);

    if (count_bit_errors_array(msg_enc, msg_test, 6) > 0) {
        fprintf(stderr,"fail: %s, encoding failure\n", __FILE__);
        exit(1);
    }

    // 
    // de-interleave message
    //
    unsigned char msg_dec[6];
    wlan_interleaver_decode_symbol(rate, msg_enc, msg_dec);
    
    // print results
    printf("de-interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x)\n", i, msg_enc[i], msg_dec[i], msg_org[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_dec, msg_org, 6), 48);

    if (count_bit_errors_array(msg_dec, msg_org, 6) > 0) {
        fprintf(stderr,"fail: %s, decoding failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

