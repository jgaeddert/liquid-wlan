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
// wlan_interleaver_gentab.c
//
// generate strucutred interleaver table
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void usage()
{
    printf("Usage: wlan_interleaver_gentab [OPTION]\n");
    printf("  h     : print help\n");
    printf("  r     : rate {6,9,12,18,24,36,48,54} M bits/s\n");
}

// structured interleaver element
struct wlan_interleaver_tab_s {
    unsigned char p0;       // input (de-interleaved) byte index
    unsigned char p1;       // output (interleaved) byte index
    unsigned char mask0;    // input (de-interleaved) bit mask
    unsigned char mask1;    // output (interleaved) bit mask
};

int main(int argc, char*argv[])
{
    // option(s)
    unsigned int rate  = 6;     // primitive rate
    unsigned int ncbps = 48;    // number of coded bits per OFDM symbol
    unsigned int nbpsc = 1;     // number of bits per subcarrier (modulation depth)
    
    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hr:")) != EOF){
        switch (dopt) {
        case 'h':
            usage();
            return 0;
        case 'r':
            switch ( atoi(optarg) ) {
            case 6:  rate = 6;  ncbps = 48;  nbpsc = 1; break;
            case 9:  rate = 9;  ncbps = 48;  nbpsc = 1; break;
            case 12: rate = 12; ncbps = 96;  nbpsc = 2; break;
            case 18: rate = 18; ncbps = 96;  nbpsc = 2; break;
            case 24: rate = 24; ncbps = 192; nbpsc = 4; break;
            case 36: rate = 36; ncbps = 192; nbpsc = 4; break;
            case 48: rate = 48; ncbps = 288; nbpsc = 6; break;
            case 54: rate = 54; ncbps = 288; nbpsc = 6; break;
            default:
                fprintf(stderr,"error: %s, invalid rate '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }
    
    // create structured interleaver table
    struct wlan_interleaver_tab_s intlv[ncbps];

    // generate table
    unsigned int k; // original
    unsigned int i;
    unsigned int j;

    unsigned int s = (nbpsc / 2) < 1 ? 1 : nbpsc/2; // max( nbpsc/2, 1 )

    div_t d0;
    div_t d1;

    // fill table: k > i > j
    for (k=0; k<ncbps; k++) {

        i = (ncbps/16)*(k % 16) + (k/16);
        j = s*(i/s) + (i + ncbps - ((16*i)/ncbps) ) % s;
        
        d0 = div(k, 8);
        d1 = div(j, 8);

        intlv[k].p0 = d0.quot;
        intlv[k].p1 = d1.quot;

        intlv[k].mask0 = 1 << (8-d0.rem-1);
        intlv[k].mask1 = 1 << (8-d1.rem-1);
    }

    // print table
    printf("// auto-generated file (do not edit)\n");
    printf("\n");
    printf("// structured interleaver table for rate %u M bits/s\n", rate);
    printf("struct interleaver_gentab_s wlan_intlv_R%u[%u] = {\n", rate, ncbps);
    for (i=0; i<ncbps; i++) {
        printf("    {%3u, %3u, 0x%.2x, 0x%.2x},\n",
            intlv[i].p0,
            intlv[i].p1,
            intlv[i].mask0,
            intlv[i].mask1);
    }
    printf("};\n");

    return 0;
}

