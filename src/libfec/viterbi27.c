/*
 * Copyright Feb 2004, Phil Karn, KA9Q
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

/* 
 * K=7 r=1/2 Viterbi decoder with optional Intel or PowerPC SIMD
 * Original source code released under LGPLv2.1
 * Some modifications made from original for portability.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// include header with forward declarations
#include "liquid-wlan.internal.h"

/* Create a new instance of a Viterbi decoder */
void *wlan_create_viterbi27(int len){
    return wlan_create_viterbi27_port(len);
}

void wlan_set_viterbi27_polynomial(int polys[2]){
    wlan_set_viterbi27_polynomial_port(polys);
}

/* initialize Viterbi decoder for start of new frame */
int wlan_init_viterbi27(void *p,int starting_state){
    return wlan_init_viterbi27_port(p,starting_state);
}

/* Viterbi chainback */
int wlan_chainback_viterbi27(
    void *p,
    unsigned char *data, /* Decoded output data */
    unsigned int nbits, /* Number of data bits */
    unsigned int endstate){ /* Terminal encoder state */

    return wlan_chainback_viterbi27_port(p,data,nbits,endstate);
}

/* Delete instance of a Viterbi decoder */
void wlan_delete_viterbi27(void *p){
    wlan_delete_viterbi27_port(p);
}

/* Update decoder with a block of demodulated symbols
 * Note that nbits is the number of decoded data bits, not the number
 * of symbols!
 */
int wlan_update_viterbi27_blk(void *p,unsigned char syms[],int nbits){
    if(p == NULL)
        return -1;

    wlan_update_viterbi27_blk_port(p,syms,nbits);
    return 0;
}
