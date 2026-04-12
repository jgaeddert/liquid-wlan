// Test interleaver; data obtained from
// Annex G in 1999 specification (Tables ...)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <liquid/liquid.h>

int main(int argc, char*argv[])
{
    // input message
    // 1101 0001
    // 1010 0001
    // 0000 0010
    // 0011 1110
    // 0111 0000
    // 0000 0000
    unsigned char msg_org[6] = {0xd1, 0xa1, 0x02, 0x3e, 0x70, 0x00};

    // interleaved message (test)
    // 1001 0100
    // 1101 0000
    // 0001 0100
    // 1000 0011
    // 0010 0100
    // 1001 0100
    unsigned char msg_test[6] = {0x94, 0xd0, 0x14, 0x83, 0x24, 0x94};

    // options...
    unsigned int ncbps = 48;    // number of coded bits per OFDM symbol
    unsigned int nbpsc = 1;     // number of bits per subcarrier (modulation depth)

    unsigned int k; // original
    unsigned int i;
    unsigned int j;

    unsigned int s = (nbpsc / 2) < 1 ? 1 : nbpsc/2; // max( nbpsc/2, 1 )

    //
    // interleave data
    //

    // internal arrays
    unsigned char msg_enc0[6] = {0,0,0,0,0,0};  // encoder: first permutation
    unsigned char msg_enc1[6] = {0,0,0,0,0,0};  // encoder: second permutation

    printf("first permutation:\n");
    for (k=0; k<ncbps; k++) {
        i = (ncbps/16)*(k % 16) + (k/16);

        printf("%3u > %3u\n", k, i);

        div_t d0 = div(k, 8);
        div_t d1 = div(i, 8);

        unsigned int bit = (msg_org[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_enc0[d1.quot] |= bit << (8-d1.rem-1);
    }

    printf("second permutation:\n");
    for (i=0; i<ncbps; i++) {
        j = s*(i/s) + (i + ncbps - ((16*i)/ncbps) ) % s;

        printf("%3u > %3u\n", i, j);
        
        div_t d0 = div(i, 8);
        div_t d1 = div(j, 8);

        unsigned int bit = (msg_enc0[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_enc1[d1.quot] |= bit << (8-d1.rem-1);
    }

    //
    // de-interleave data
    //

    // internal arrays
    unsigned char msg_dec0[6] = {0,0,0,0,0,0};  // decoder: first permutation
    unsigned char msg_dec1[6] = {0,0,0,0,0,0};  // decoder: second permutation

    for (k=0; k<ncbps; k++) {
        i = (ncbps/16)*(k % 16) + (k/16);

        div_t d0 = div(i, 8);
        div_t d1 = div(k, 8);

        unsigned int bit = (msg_enc1[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_dec0[d1.quot] |= bit << (8-d1.rem-1);
    }

    for (i=0; i<ncbps; i++) {
        j = s*(i/s) + (i + ncbps - ((16*i)/ncbps) ) % s;

        div_t d0 = div(j, 8);
        div_t d1 = div(i, 8);

        unsigned int bit = (msg_dec0[d0.quot] >> (8-d0.rem-1)) & 0x01;
        msg_dec1[d1.quot] |= bit << (8-d1.rem-1);
    }

    // print results
    printf("interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_enc1[i], msg_test[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_enc1, msg_test, 6), 48);
    printf("\n");

    printf("de-interleaved:\n");
    for (i=0; i<6; i++)
        printf("%3u : 0x%.2x (0x%.2x)\n", i, msg_dec1[i], msg_org[i]);
    printf("errors : %3u / %3u\n", count_bit_errors_array(msg_dec1, msg_org, 6), 48);
    printf("\n");

    printf("done.\n");
    return 0;
}

