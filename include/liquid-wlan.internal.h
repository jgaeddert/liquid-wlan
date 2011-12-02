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
#ifndef __LIQUID_WLAN_INTERNAL_H__
#define __LIQUID_WLAN_INTERNAL_H__

// Configuration file
#include "config.h"

#include <complex.h>
#include <liquid/liquid.h>

#include "liquid-wlan.h"

//
// utility
//

// reverse byte table
extern const unsigned char liquid_wlan_reverse_byte[256];

// number of ones in a byte modulo 2
extern const unsigned char liquid_wlan_parity[256];


// Use fftw library if installed, otherwise use liquid-dsp (less
// efficient) fft library.
#if HAVE_FFTW3_H
#   include <fftw3.h>
#   define FFT_PLAN             fftwf_plan
#   define FFT_CREATE_PLAN      fftwf_plan_dft_1d
#   define FFT_DESTROY_PLAN     fftwf_destroy_plan
#   define FFT_EXECUTE          fftwf_execute
#   define FFT_DIR_FORWARD      FFTW_FORWARD
#   define FFT_DIR_BACKWARD     FFTW_BACKWARD
#   define FFT_METHOD           FFTW_ESTIMATE
#else
#   define FFT_PLAN             fftplan
#   define FFT_CREATE_PLAN      fft_create_plan
#   define FFT_DESTROY_PLAN     fft_destroy_plan
#   define FFT_EXECUTE          fft_execute
#   define FFT_DIR_FORWARD      FFT_FORWARD
#   define FFT_DIR_BACKWARD     FFT_REVERSE
#   define FFT_METHOD           0
#endif

//
// wi-fi frame (common objects)
//

//
struct wlanframe_param_s {
    unsigned int rate;          // primitive data rate [MBits/s]
    unsigned int mod_scheme;    // modulation scheme (depth is nbpsc field)
    unsigned int fec_scheme;    // forward error-correction coding scheme
    unsigned int nbpsc;         // number of coded bits per subcarrier
    unsigned int ncbps;         // number of coded bits per OFDM symbol
    unsigned int ndbps;         // number of data bits per OFDM symbol
};

// rate-dependent parameters (Table 78)
extern const struct wlanframe_param_s wlanframe_ratetab[8];

//
// signal definition
//

// signal field rate encoding table (see Table 80)
//    WLANFRAME_RATE_6  = 13 : 1101
//    WLANFRAME_RATE_9  = 15 : 1111
//    WLANFRAME_RATE_12 =  5 : 0101
//    WLANFRAME_RATE_18 =  7 : 0111
//    WLANFRAME_RATE_24 =  9 : 1001
//    WLANFRAME_RATE_36 = 11 : 1011
//    WLANFRAME_RATE_48 =  1 : 0001
//    WLANFRAME_RATE_54 =  3 : 0011
extern const unsigned char wlan_signal_R1R4tab[8];

// pack SIGNAL structure into 3-byte array
//  _rate       :   data rate field (e.g. WLANFRAME_RATE_6)
//  _R          :   reserved bit
//  _length     :   length of payload (1-4095)
//  _signal     :   output signal, packed [size: 3 x 1]
void wlan_signal_pack(unsigned int    _rate,
                      unsigned int    _R,
                      unsigned int    _length,
                      unsigned char * _signal);

// unpack SIGNAL structure from 3-byte array
//  _signal     :   input signal, packed [size: 3 x 1]
//  _rate       :   data rate field (e.g. WLANFRAME_RATE_6)
//  _R          :   reserved bit
//  _length     :   length of payload (1-4095)
int wlan_signal_unpack(unsigned char * _signal,
                       unsigned int    * _rate,
                       unsigned int    * _R,
                       unsigned int    * _length);


// 
// encoding/decoding
//

#define LIQUID_WLAN_SOFTBIT_1       (255)
#define LIQUID_WLAN_SOFTBIT_ERASURE (127)
#define LIQUID_WLAN_SOFTBIT_0       (0)

/* r=1/2 k=7 convolutional encoder polynomials
 * The NASA-DSN convention is to use V27POLYA inverted, then V27POLYB
 * The CCSDS/NASA-GSFC convention is to use V27POLYB, then V27POLYA inverted
 */
#define	V27POLYA	0x6d
#define	V27POLYB	0x4f

// generic interface
void * wlan_create_viterbi27(int len);
void wlan_set_viterbi27_polynomial(int polys[2]);
int wlan_init_viterbi27(void *vp,int starting_state);
int wlan_update_viterbi27_blk(void *vp,unsigned char sym[],int npairs);
int wlan_chainback_viterbi27(void *vp, unsigned char *data,unsigned int nbits,unsigned int endstate);
void wlan_delete_viterbi27(void *vp);

// portable C interface
void * wlan_create_viterbi27_port(int len);
void wlan_set_viterbi27_polynomial_port(int polys[2]);
int wlan_init_viterbi27_port(void *p,int starting_state);
int wlan_chainback_viterbi27_port(void *p,unsigned char *data,unsigned int nbits,unsigned int endstate);
void wlan_delete_viterbi27_port(void *p);
int wlan_update_viterbi27_blk_port(void *p,unsigned char *syms,int nbits);

static inline int parity(int x){
  /* Fold down to one byte */
  x ^= (x >> 16);
  x ^= (x >> 8);
  //return parityb(x);
  return liquid_wlan_parity[x];
}



// wlan convolutional encoder/decoder properties
struct wlanconv_s {
    // base convolutional encoder/decoder properties (fixed for 802.11a/g)
    const unsigned int * genpoly;   // generator polynomials [fixed: 0x6d, 0x4f]
    unsigned int   R;               // primitive rate        [fixed: 2]
    unsigned int   K;               // constraint length     [fixed: 7]
    
    // puncturing options
    int punctured;                  // punctured?
    const unsigned char * pmatrix;  // puncturing matrix [size: R x P]
    unsigned int P;                 // columns of puncturing matrix
};

// convolutional encoder/decoder constants
extern const unsigned  int wlanconv_genpoly[2];         // r1/2 base generator polynomials
extern const unsigned char wlanconv_v27p23_pmatrix[12]; // r2/3 puncturing matrix
extern const unsigned char wlanconv_v27p34_pmatrix[18]; // r3/4 puncturing matrix

#define LIQUID_WLAN_FEC_R1_2    (0) // r1/2
#define LIQUID_WLAN_FEC_R2_3    (1) // r2/3
#define LIQUID_WLAN_FEC_R3_4    (2) // r3/4
extern const struct wlanconv_s wlanconv_fectab[3];      // available codecs

// encode SIGNAL field using half-rate convolutional code
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
void wlan_fec_signal_encode(unsigned char * _msg_dec,
                            unsigned char * _msg_enc);

// decode SIGNAL field using half-rate convolutional code
//  _msg_enc    :   48-bit signal field [size: 6 x 1]
//  _msg_dec    :   24-bit signal field [size: 3 x 1]
void wlan_fec_signal_decode(unsigned char * _msg_enc,
                            unsigned char * _msg_dec);

// encode data using convolutional code
//  _fec_scheme :   error-correction scheme
//  _dec_msg_len:   length of decoded message
//  _msg_dec    :   decoded message (with tail bits inserted)
//  _msg_enc    :   encoded message
void wlan_fec_encode(unsigned int    _fec_scheme,
                     unsigned int    _dec_msg_len,
                     unsigned char * _msg_dec,
                     unsigned char * _msg_enc);

// decode data using convolutional code
//  _fec_scheme :   error-correction scheme
//  _dec_msg_len:   length of decoded message
//  _msg_enc    :   encoded message
//  _msg_dec    :   decoded message (with tail bits inserted)
void wlan_fec_decode(unsigned int    _fec_scheme,
                     unsigned int    _dec_msg_len,
                     unsigned char * _msg_enc,
                     unsigned char * _msg_dec);


//
// data scrambler/de-scrambler
//

// scramble data
//  _msg_dec    :   original data message [size: _n x 1]
//  _msg_enc    :   scrambled data message [size: _n x 1]
//  _n          :   length of input/output (bytes)
//  _seed       :   linear feedback shift register initial state
void wlan_data_scramble(unsigned char * _msg_dec,
                        unsigned char * _msg_enc,
                        unsigned int _n,
                        unsigned int _seed);

// unscramble data
//  _msg_enc    :   scrambled data message [size: _n x 1]
//  _msg_dec    :   original data message [size: _n x 1]
//  _n          :   length of input/output (bytes)
//  _seed       :   linear feedback shift register initial state
void wlan_data_unscramble(unsigned char * _msg_enc,
                          unsigned char * _msg_dec,
                          unsigned int _n,
                          unsigned int _seed);

//
// interleaver
//

// structured interleaver element
struct wlan_interleaver_tab_s {
    unsigned char p0;       // input (de-interleaved) byte index
    unsigned char p1;       // output (interleaved) byte index
    unsigned char mask0;    // input (de-interleaved) bit mask
    unsigned char mask1;    // output (interleaved) bit mask
};

// external auto-generated structured interleaver tables (see liquid-wlan/src/gentab)
extern struct wlan_interleaver_tab_s wlan_intlv_R6[48];
extern struct wlan_interleaver_tab_s wlan_intlv_R9[48];
extern struct wlan_interleaver_tab_s wlan_intlv_R12[96];
extern struct wlan_interleaver_tab_s wlan_intlv_R18[96];
extern struct wlan_interleaver_tab_s wlan_intlv_R24[192];
extern struct wlan_interleaver_tab_s wlan_intlv_R36[192];
extern struct wlan_interleaver_tab_s wlan_intlv_R48[288];
extern struct wlan_interleaver_tab_s wlan_intlv_R54[288];

// indexable table of above structured auto-generated tables
extern struct wlan_interleaver_tab_s * wlan_intlv_gentab[8];

// intereleave one OFDM symbol
//  _rate       :   primitive rate
//  _msg_dec    :   decoded message (de-iterleaved)
//  _msg_enc    :   encoded message (interleaved)
void wlan_interleaver_encode_symbol(unsigned int    _rate,
                                    unsigned char * _msg_dec,
                                    unsigned char * _msg_enc);

// de-intereleave one OFDM symbol
//  _rate       :   primitive rate
//  _msg_enc    :   encoded message (interleaved)
//  _msg_dec    :   decoded message (de-iterleaved)
void wlan_interleaver_decode_symbol(unsigned int    _rate,
                                    unsigned char * _msg_dec,
                                    unsigned char * _msg_enc);


//
// high-level packet encoder/decoder
//

// compute encoded message length
unsigned int wlan_packet_compute_enc_msg_len(unsigned int _rate,
                                             unsigned int _length);

// assemble data (prepend SERVICE bits, etc.), scramble, encode, interleave
void wlan_packet_encode(unsigned int    _rate,
                        unsigned int    _seed,
                        unsigned int    _length,
                        unsigned char * _msg_dec,
                        unsigned char * _msg_enc);

// de-interleave, decode, de-scramble, extract data (SERVICE bits, etc.)
void wlan_packet_decode(unsigned int    _rate,
                        unsigned int    _seed,
                        unsigned int    _length,
                        unsigned char * _msg_enc,
                        unsigned char * _msg_dec);

// 
// modem (modulation/demodulation)
//

#define WLAN_MODEM_BPSK     (0)
#define WLAN_MODEM_QPSK     (1)
#define WLAN_MODEM_QAM16    (2)
#define WLAN_MODEM_QAM64    (3)

extern const float complex wlan_modem_qam16[16];
extern const float complex wlan_modem_qam64[64];

float complex wlan_modulate(unsigned int  _scheme,
                            unsigned char _sym);

float complex wlan_modulate_bpsk(unsigned char _sym);
float complex wlan_modulate_qpsk(unsigned char _sym);
float complex wlan_modulate_qam16(unsigned char _sym);
float complex wlan_modulate_qam64(unsigned char _sym);

unsigned char wlan_demodulate(unsigned int  _scheme,
                              float complex _sample);

unsigned char wlan_demodulate_bpsk(float complex _sample);
unsigned char wlan_demodulate_qpsk(float complex _sample);
unsigned char wlan_demodulate_qam16(float complex _sample);
unsigned char wlan_demodulate_qam64(float complex _sample);


// 
// 802.11a/g framing
//

// PLCP short sequence
extern const float complex wlanframe_S0[64]; // freq
extern const float complex wlanframe_s0[64]; // time

// PLCP long sequence
extern const float complex wlanframe_S1[64]; // freq
extern const float complex wlanframe_s1[64]; // time

#define WLANFRAME_SCTYPE_NULL   0
#define WLANFRAME_SCTYPE_PILOT  1
#define WLANFRAME_SCTYPE_DATA   2

//
// wi-fi frame generator (internal methods)
//

// compute symbol: add/update pilots, add nulls and compute transform
//  * input stored in 'X' (internal ifft input)
//  * output stored in 'x' (internal ifft output)
void wlanframegen_compute_symbol(wlanframegen _q);

// generate symbol (add cyclic prefix/postfix, overlap)
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
                            float complex * _symbol);

void wlanframegen_writesymbol_S0a(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_S0b(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_S1a(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_S1b(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_signal(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_data(wlanframegen _q, float complex * _buffer);
void wlanframegen_writesymbol_null(wlanframegen _q, float complex * _buffer);

//
// wi-fi frame synchronizer (internal methods)
//

void wlanframesync_execute_seekplcp(wlanframesync _q);
void wlanframesync_execute_rxshort0(wlanframesync _q);
void wlanframesync_execute_rxshort1(wlanframesync _q);
void wlanframesync_execute_rxlong0(wlanframesync _q);
void wlanframesync_execute_rxlong1(wlanframesync _q);
void wlanframesync_execute_rxsignal(wlanframesync _q);
void wlanframesync_execute_rxdata(wlanframesync _q);

// estimate short sequence gain
//  _q      :   wlanframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wlanframesync_estimate_gain_S0(wlanframesync _q,
                                    float complex * _x,
                                    float complex * _G);

// compute S0 metrics
void wlanframesync_S0_metrics(wlanframesync _q,
                              float complex * _G,
                              float complex * _s_hat);

// estimate carrier frequency offset from S0 gains
float wlanframesync_estimate_cfo_S0(float complex * _G0a,
                                    float complex * _G0b);

// estimate long sequence gain
//  _q      :   wlanframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
void wlanframesync_estimate_gain_S1(wlanframesync _q,
                                    float complex * _x,
                                    float complex * _G);

// compute S1 metrics
void wlanframesync_S1_metrics(wlanframesync _q,
                              float complex * _G,
                              float complex * _s_hat);

// estimate carrier frequency offset from S1 gains
float wlanframesync_estimate_cfo_S1(float complex * _G1a,
                                    float complex * _G1b);

// estimate equalizer gain from internal S1 gains using polynomial
void wlanframesync_estimate_eqgain_poly(wlanframesync _q);

// recover symbol, correcting for gain, pilot phase, etc.
void wlanframesync_rxsymbol(wlanframesync _q);

// decode SIGNAL field
void wlanframesync_decode_signal(wlanframesync _q);

#endif // __LIQUID_WLAN_INTERNAL_H__

