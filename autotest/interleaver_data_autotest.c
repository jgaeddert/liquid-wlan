// Test interleaver; data obtained from
// Annex G in 1999 specification (Tables ...)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-wlan.internal.h"

#include "annex-g-data/G18.c"
#include "annex-g-data/G21.c"

int main(int argc, char*argv[])
{
    // Table G.18: coded bits of first DATA symbol
    unsigned char * msg_org = annexg_G18;

    // Table G.21: interleaved bits of first DATA symbol
    unsigned char * msg_test = annexg_G21;

    // options...
    unsigned int ncbps = 192;   // number of coded bits per OFDM symbol
    //unsigned int nbpsc = 4;     // number of bits per subcarrier (modulation depth)
    unsigned int rate = WLANFRAME_RATE_36;

    unsigned int n = ncbps / 8;
    
    unsigned int i;

    // 
    // interleave message
    //
    unsigned char msg_enc[n];
    wlan_interleaver_encode_symbol(rate, msg_org, msg_enc);
    
    // print results
    printf("interleaved:\n");
    for (i=0; i<n; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x) %s\n", i, msg_org[i], msg_enc[i], msg_test[i], msg_enc[i] == msg_test[i] ? "" : "*");
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc, msg_test, n), ncbps);

    if (count_bit_errors_array(msg_enc, msg_test, n) > 0) {
        fprintf(stderr,"fail: %s, encoding failure\n", __FILE__);
        exit(1);
    }

    // 
    // de-interleave message
    //
    unsigned char msg_dec[n];
    wlan_interleaver_decode_symbol(rate, msg_enc, msg_dec);
    
    // print results
    printf("de-interleaved:\n");
    for (i=0; i<n; i++)
        printf("%3u : 0x%.2x > 0x%.2x (0x%.2x) %s\n", i, msg_enc[i], msg_dec[i], msg_org[i], msg_dec[i] == msg_org[i] ? "" : "*");
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_dec, msg_org, n), ncbps);

    if (count_bit_errors_array(msg_dec, msg_org, n) > 0) {
        fprintf(stderr,"fail: %s, decoding failure\n", __FILE__);
        exit(1);
    }

    return 0;
}

