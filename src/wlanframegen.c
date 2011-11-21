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
// wlanframegen.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid-wlan.internal.h"

#define DEBUG_WLANFRAMEGEN            1

struct wlanframegen_s {
    // options
    unsigned int rate;      // primitive data rate
    unsigned int length;    // original data length (bytes)
    unsigned int seed;      // data scrambler seed

    // scaling factors
    float g_data;

    // transform object
    FFT_PLAN ifft;          // ifft object
    float complex * X;      // frequency-domain buffer
    float complex * x;      // time-domain buffer

#if 0
    // PLCP short
    float complex * S0;     // short sequence (frequency)
    float complex * s0;     // short sequence (time)

    // PLCP long
    float complex * S1;     // long sequence (frequency)
    float complex * s1;     // long sequence (time)
#endif

    // window transition
    unsigned int rampup_len;        // number of samples in overlapping symbols
    float * rampup;                 // ramp up window (ramp down is time-reversed)
    float complex * postfix;        // overlapping symbol buffer

    // lengths
    unsigned int ndbps;             // number of data bits per OFDM symbol
    unsigned int ncbps;             // number of coded bits per OFDM symbol
    unsigned int nbpsc;             // number of bits per subcarrier (modulation depth)
    unsigned int dec_msg_len;       // length of decoded message (bytes)
    unsigned int enc_msg_len;       // length of encoded message (bytes)
    unsigned int nsym;              // number of OFDM symbols in the DATA field
    unsigned int ndata;             // number of bits in the DATA field
    unsigned int npad;              // number of pad bits

    // data arrays
    unsigned char   signal_dec[3];  // decoded message (SIGNAL field)
    unsigned char   signal_enc[6];  // enccoded message (SIGNAL field)
    unsigned char * msg_enc;        // encoded message (DATA field)
    
    // counters/states
    enum {
        WLANFRAMEGEN_STATE_S0A=0,   // write first block of 'short' symbols
        WLANFRAMEGEN_STATE_S0B,     // write second block of 'short' symbols
        WLANFRAMEGEN_STATE_S1A,     // write first block of 'long' symbols
        WLANFRAMEGEN_STATE_S1B,     // write second block of 'long' symbols
        WLANFRAMEGEN_STATE_SIGNAL,  // write SIGNAL symbol
        WLANFRAMEGEN_STATE_PAYLOAD  // write payload symbols
    } state;
    int frame_assembled;            // frame assembled flag
    unsigned int data_symbol_counter;
};

// create WLAN framing generator object
wlanframegen wlanframegen_create()
{
    wlanframegen q = (wlanframegen) malloc(sizeof(struct wlanframegen_s));

    // allocate memory for transform objects
    q->X = (float complex*) malloc(64*sizeof(float complex));
    q->x = (float complex*) malloc(64*sizeof(float complex));
    q->ifft = FFT_CREATE_PLAN(64, q->X, q->x, FFT_DIR_BACKWARD, FFT_METHOD);

    // create transition window/buffer
    // NOTE : ramp length must be less than cyclic prefix length (default: 1)
    // TODO : make ramp length an input parameter
    q->rampup_len = 1;
    q->rampup = (float*) malloc( q->rampup_len * sizeof(float) );
    q->postfix = (float complex*) malloc( q->rampup_len * sizeof(float complex) );

    // initialize ramp/up transition
    unsigned int i;
    for (i=0; i<q->rampup_len; i++) {
        float t = ((float)(i) + 0.5f) / (float)(q->rampup_len);
        float g = sinf(M_PI_2*t);
        q->rampup[i] = g*g;
    }
    
#if DEBUG_WLANFRAMEGEN
    // validate window symmetry
    for (i=0; i<q->rampup_len; i++) {
        printf("    ramp/up[%2u] = %12.8f (%12.8f)\n", i, q->rampup[i],
            q->rampup[i] + q->rampup[q->rampup_len - i - 1]);
    }
#endif

    // set initial properties
    q->rate   = WLANFRAME_RATE_6;
    q->length = 100;
    q->seed   = 0x5d;

    // allocate memory for encoded message
    q->enc_msg_len = wlan_packet_compute_enc_msg_len(q->rate, q->length);
    q->msg_enc = (unsigned char*) malloc(q->enc_msg_len*sizeof(unsigned char));

    // compute scaling factor
    q->g_data = 1.0f / sqrtf(52.0f);

    // reset objects
    wlanframegen_reset(q);

    return q;
}

// destroy WLAN framing generator object
void wlanframegen_destroy(wlanframegen _q)
{
    // free transform array memory
    free(_q->X);
    free(_q->x);
    FFT_DESTROY_PLAN(_q->ifft);

    // free transition window ramp array and postfix buffer
    free(_q->rampup);
    free(_q->postfix);

    // free memory for encoded message
    free(_q->msg_enc);

    // free main object memory
    free(_q);
}

// print WLAN framing generator object internals
void wlanframegen_print(wlanframegen _q)
{
    printf("wlanframegen:\n");
    if (_q->frame_assembled) {
        printf("    rate        :   %3u Mbits/s\n", wlanframe_ratetab[_q->rate].rate);
        printf("    payload     :   %3u bytes\n", _q->length);
        printf("    signal dec  :   [%.2x %.2x %.2x]\n",
                _q->signal_dec[0],
                _q->signal_dec[1],
                _q->signal_dec[2]);
        printf("    signal enc  :   [%.2x %.2x %.2x %.2x %.2x %.2x]\n",
                _q->signal_enc[0],
                _q->signal_enc[1],
                _q->signal_enc[2],
                _q->signal_enc[3],
                _q->signal_enc[4],
                _q->signal_enc[5]);
    }
}

// reset WLAN framing generator object internal state
void wlanframegen_reset(wlanframegen _q)
{
    // reset state/counters
    _q->frame_assembled = 0;
    _q->state = WLANFRAMEGEN_STATE_S0A;
    _q->data_symbol_counter = 0;

    // clear internal postfix buffer
    unsigned int i;
    for (i=0; i<_q->rampup_len; i++)
        _q->postfix[i] = 0.0f;
}

// assemble frame (see Table 76)
//  _q          :   framing object
//  _payload    :   raw payload data [size: _opts.LENGTH x 1]
//  _txvector   :   framing options
void wlanframegen_assemble(wlanframegen           _q,
                           unsigned char *        _payload,
                           struct wlan_txvector_s _txvector)
{
    // validate input
    if (_txvector.DATARATE > 7) {
        fprintf(stderr,"error: wlanframegen_assemble(), invalid rate\n");
        exit(1);
    } else if (_txvector.LENGTH == 0 || _txvector.LENGTH > 4095) { 
        fprintf(stderr,"error: wlanframegen_assemble(), invalid data length\n");
        exit(1);
    }

    // set internal properties
    _q->rate   = _txvector.DATARATE;
    _q->length = _txvector.LENGTH;
    _q->seed   = 0x5d;  //(_txvector.SERVICE >> 9) & 0x7f;
    // TODO : strip off TXPWR_LEVEL

    // pack SIGNAL field
    unsigned int R = 0; // 'reserved' bit
    wlan_signal_pack(_q->rate, R, _q->length, _q->signal_dec);

    // encode SIGNAL field
    wlan_fec_signal_encode(_q->signal_dec, _q->signal_enc);

    // compute frame parameters
    _q->ndbps  = wlanframe_ratetab[_q->rate].ndbps; // number of data bits per OFDM symbol
    _q->ncbps  = wlanframe_ratetab[_q->rate].ncbps; // number of coded bits per OFDM symbol
    _q->nbpsc  = wlanframe_ratetab[_q->rate].nbpsc; // number of bits per subcarrier (modulation depth)

    // compute number of OFDM symbols
    div_t d = div(16 + 8*_q->length + 6, _q->ndbps);
    _q->nsym = d.quot + (d.rem == 0 ? 0 : 1);

    // compute number of bits in the DATA field
    _q->ndata = _q->nsym * _q->ndbps;

    // compute number of pad bits
    _q->npad = _q->ndata - (16 + 8*_q->length + 6);

    // compute decoded message length (number of data bytes)
    // NOTE : because ndbps is _always_ divisible by 8, so must ndata be
    _q->dec_msg_len = _q->ndata / 8;

    // compute encoded message length (number of data bytes)
    _q->enc_msg_len = (_q->dec_msg_len * _q->ncbps) / _q->ndbps;

    // validate encoded message length
    //assert(_q->enc_msg_len == wlan_packet_compute_enc_msg_len(_q->rate, _q->length));

    // re-allocate buffer for encoded message
    _q->msg_enc = (unsigned char*) realloc(_q->msg_enc, _q->enc_msg_len*sizeof(unsigned char));

    // encode message
    wlan_packet_encode(_q->rate, _q->seed, _q->length, _payload, _q->msg_enc);

    // flag frame as being assembled
    _q->frame_assembled = 1;
}


// write OFDM symbol, returning '1' when frame is complete
//  _q          :   framing generator object
//  _buffer     :   output sample buffer [size: 80 x 1]
int wlanframegen_writesymbol(wlanframegen    _q,
                             float complex * _buffer)
{
    // validate input
    if (!_q->frame_assembled) {
        fprintf(stderr,"error: wlanframegen_writesymbol(), frame not assembled\n");
        exit(1);
    }

    //
    switch (_q->state) {
    case WLANFRAMEGEN_STATE_S0A:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating first short sequence\n");
#endif
        wlanframegen_writesymbol_S0a(_q, _buffer);
        _q->state = WLANFRAMEGEN_STATE_S0B;
        return 0;
    case WLANFRAMEGEN_STATE_S0B:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating second short sequence\n");
#endif
        wlanframegen_writesymbol_S0b(_q, _buffer);
        _q->state = WLANFRAMEGEN_STATE_S1A;
        return 0;
    case WLANFRAMEGEN_STATE_S1A:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating first long sequence\n");
#endif
        wlanframegen_writesymbol_S1a(_q, _buffer);
        _q->state = WLANFRAMEGEN_STATE_S1B;
        return 0;
    case WLANFRAMEGEN_STATE_S1B:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating second long sequence\n");
#endif
        wlanframegen_writesymbol_S1b(_q, _buffer);
        _q->state = WLANFRAMEGEN_STATE_SIGNAL;
        return 0;
    case WLANFRAMEGEN_STATE_SIGNAL:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating SIGNAL symbol\n");
#endif
        wlanframegen_writesymbol_signal(_q, _buffer);
        _q->state = WLANFRAMEGEN_STATE_PAYLOAD;
        return 0;
    case WLANFRAMEGEN_STATE_PAYLOAD:
#if DEBUG_WLANFRAMEGEN
        printf("wlanframegen_writesymbol(), generating data symbol [%3u]\n", _q->data_symbol_counter);
#endif
        wlanframegen_writesymbol_data(_q, _buffer);
        _q->data_symbol_counter++;

        if (_q->data_symbol_counter < _q->nsym)
            return 0;
        else
            break;
    default:
        // should never get to this point
        fprintf(stderr,"error: wlanframegen_writesymbol(), invalid state\n");
        exit(1);
    }

#if 0
    // move frequency data to internal buffer
    unsigned int i;
    unsigned int k;
    int sctype;
    for (i=0; i<_q->M; i++) {
        // start at mid-point (effective fftshift)
        k = (i + _q->M/2) % _q->M;

        sctype = _q->p[k];
        if (sctype==WLANFRAME_SCTYPE_NULL) {
            // disabled subcarrier
            _q->X[k] = 0.0f;
        } else if (sctype==WLANFRAME_SCTYPE_PILOT) {
            // pilot subcarrier
            _q->X[k] = (msequence_advance(_q->ms_pilot) ? 1.0f : -1.0f) * _q->g_data;
        } else {
            // data subcarrier
            _q->X[k] = _x[k] * _q->g_data;
        }

        //printf("X[%3u] = %12.8f + j*%12.8f;\n",i+1,crealf(_q->X[i]),cimagf(_q->X[i]));
    }

    // execute transform
    FFT_EXECUTE(_q->ifft);

    // copy result to output
    memmove( _y, &_q->x[_q->M - _q->cp_len], (_q->cp_len)*sizeof(float complex));
    memmove(&_y[_q->cp_len], _q->x, (_q->M)*sizeof(float complex));
#endif

    // reset and return
#if DEBUG_WLANFRAMEGEN
    printf("wlanframegen_writesymbol(), resetting frame generator\n");
#endif
    wlanframegen_reset(_q);
    return 1;
}

// 
// internal methods
//

// generate symbol
//  _x          :   input time-domain symbol [size: 64 x 1]
//  _x_prime    :   post-fix from previous symbol [size: _p x 1], output
//                  post-fix from this new symbol
//  _rampup     :   ramp up window; ramp down is time-reversed [size: _p x 1]
//  _p          :   post-fix size
//  _symbol     :   output symbol [size: 80 x 1]
void wlanframegen_gensymbol(float complex * _x,
                            float complex * _x_prime,
                            float         * _rampup,
                            unsigned int    _p,
                            float complex * _symbol)
{
    // validate input
    if (_p >= 16) {
        fprintf(stderr,"error: wlanframegen_gensymbol(), transition length cannot exceed cyclic prefix\n");
        exit(1);
    }

    // copy input symbol with cyclic prefix to output symbol
    memmove(&_symbol[ 0], &_x[48], 16*sizeof(float complex));
    memmove(&_symbol[16], &_x[ 0], 64*sizeof(float complex));

    // apply window to over-lapping regions
    unsigned int i;
    for (i=0; i<_p; i++) {
        _symbol[i] *= _rampup[i];
        _symbol[i] += _x_prime[i] * _rampup[_p-i-1];
    }

    // copy post-fix to output (first _p samples of input symbol)
    memmove(_x_prime, _x, _p*sizeof(float complex));
}

// write first PLCP short sequence 'symbol' to buffer; this is the first
// five 'short' symbols
//
//  0         32        64        96       128       160
//  +----+----+----+----+----+----+----+----+----+----+
//  | s0 | s0 | s0 | s0 | s0 | s0 | s0 | s0 | s0 | s0 |
//  +----+----+----+----+----+----+----+----+----+----+-----> time
//       |                   |    |                   |
//       |<-     s0[a]     ->|    |<-     s0[b]     ->|
//
void wlanframegen_writesymbol_S0a(wlanframegen _q,
                                  float complex * _buffer)
{
    // generate first 'short sequence' symbol
    wlanframegen_gensymbol((float complex*) wlanframe_s0,
                           _q->postfix,
                           _q->rampup,
                           _q->rampup_len,
                           _buffer);
}

// write second PLCP short sequence 'symbol' to buffer
void wlanframegen_writesymbol_S0b(wlanframegen _q,
                                  float complex * _buffer)
{
    // same as s0[a]
    wlanframegen_writesymbol_S0a(_q, _buffer);
}

// write first PLCP long sequence 'symbol' to buffer
//
//  0         32        64        96       128       160
//  +----+----+----+----+----+----+----+----+----+----+
//  |/////////|       s1[0]       |       s1[1]       | ...
//  +----+----+----+----+----+----+----+----+----+----+-----> time
//       |                   |    |                   |
//       |<-     s1[a]     ->|    |<-     s1[b]     ->|
//
void wlanframegen_writesymbol_S1a(wlanframegen _q,
                                  float complex * _buffer)
{
    // NOTE : the 'long' sequence is like a 128-sample symbol with
    //        a 32-sample cyclic prefix; need to split appropriately
    //        (see diagram above)
    memmove(&_q->x[ 0], &wlanframe_s1[48], 16*sizeof(float complex));
    memmove(&_q->x[16], &wlanframe_s1[ 0], 48*sizeof(float complex));
    
    // generate first 'long sequence' symbol
    wlanframegen_gensymbol(_q->x,
                           _q->postfix,
                           _q->rampup,
                           _q->rampup_len,
                           _buffer);
}

// write second PLCP long sequence 'symbol' to buffer
void wlanframegen_writesymbol_S1b(wlanframegen _q,
                                  float complex * _buffer)
{
    // NOTE : the 'long' sequence is like a 128-sample symbol with
    //        a 32-sample cyclic prefix; need to split appropriately
    //        (see diagram above)
    memmove(_q->x, wlanframe_s1, 64*sizeof(float complex));
    
    // generate first 'long sequence' symbol
    wlanframegen_gensymbol(_q->x,
                           _q->postfix,
                           _q->rampup,
                           _q->rampup_len,
                           _buffer);
}

// write SIGNAL symbol
void wlanframegen_writesymbol_signal(wlanframegen _q,
                                     float complex * _buffer)
{
    wlanframegen_writesymbol_null(_q, _buffer);
}

// write data symbol(s)
void wlanframegen_writesymbol_data(wlanframegen _q,
                                   float complex * _buffer)
{
    wlanframegen_writesymbol_null(_q, _buffer);
}

// write null symbol(s)
void wlanframegen_writesymbol_null(wlanframegen _q,
                                   float complex * _buffer)
{
#if 0
    memset(_q->x, 0x00, 64*sizeof(float complex));
#else
    unsigned int i;
    for (i=0; i<64; i++)
        _q->x[i] = 0.0f;
#endif
    
    // generate symbol
    wlanframegen_gensymbol(_q->x,
                           _q->postfix,
                           _q->rampup,
                           _q->rampup_len,
                           _buffer);
}

