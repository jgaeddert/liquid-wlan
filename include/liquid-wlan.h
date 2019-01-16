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
#ifndef __LIQUID_WLAN_H__
#define __LIQUID_WLAN_H__

/*
 * Make sure the version and version number macros weren't defined by
 * some prevoiusly included header file.
 */
#ifdef LIQUID_WLAN_VERSION
#  undef LIQUID_WLAN_VERSION
#endif
#ifdef LIQUID_WLAN_VERSION_NUMBER
#  undef LIQUID_WLAN_VERSION_NUMBER
#endif

/*
 * Compile-time version numbers
 * 
 * LIQUID_WLAN_VERSION = "X.Y.Z"
 * LIQUID_WLAN_VERSION_NUMBER = (X*1000000 + Y*1000 + Z)
 */
#define LIQUID_WLAN_VERSION          "0.1.0"
#define LIQUID_WLAN_VERSION_NUMBER   1000

/*
 * Run-time library version numbers
 */
extern const char liquid_wlan_version[];
const char * liquid_wlan_libversion(void);
int liquid_wlan_libversion_number(void);

#ifdef __cplusplus
extern "C" {
#   define LIQUID_WLAN_USE_COMPLEX_H 0
#else
#   define LIQUID_WLAN_USE_COMPLEX_H 1
#endif /* __cplusplus */

#define LIQUID_WLAN_CONCAT(prefix, name) prefix ## name
#define LIQUID_WLAN_VALIDATE_INPUT

/* 
 * Compile-time complex data type definitions
 *
 * Default: use the C99 complex data type, otherwise
 * define complex type compatible with the C++ complex standard,
 * otherwise resort to defining binary compatible array.
 */
#if LIQUID_WLAN_USE_COMPLEX_H==1
#   include <complex.h>
#   define LIQUID_WLAN_DEFINE_COMPLEX(R,C) typedef R _Complex C
#elif defined _GLIBCXX_COMPLEX
#   define LIQUID_WLAN_DEFINE_COMPLEX(R,C) typedef std::complex<R> C
#else
#   define LIQUID_WLAN_DEFINE_COMPLEX(R,C) typedef struct {R real; R imag;} C;
#endif
//#   define LIQUID_WLAN_DEFINE_COMPLEX(R,C) typedef R C[2]

LIQUID_WLAN_DEFINE_COMPLEX(float,  liquid_float_complex);

// rates
#define WLANFRAME_RATE_6        (0) // BPSK,   r1/2, 1101
#define WLANFRAME_RATE_9        (1) // BPSK,   r3/4, 1111
#define WLANFRAME_RATE_12       (2) // QPSK,   r1/2, 0101
#define WLANFRAME_RATE_18       (3) // QPSK,   r3/4, 0111
#define WLANFRAME_RATE_24       (4) // 16-QAM, r1/2, 1001
#define WLANFRAME_RATE_36       (5) // 16-QAM, r3/4, 1011
#define WLANFRAME_RATE_48       (6) // 64-QAM, r2/3, 0001
#define WLANFRAME_RATE_54       (7) // 64-QAM, r3/4, 0011
#define WLANFRAME_RATE_INVALID (15) // invalid rate

// TXVECTOR parameters structure
struct wlan_txvector_s {
    unsigned int LENGTH;        // length of payload (1-4095)
    unsigned int DATARATE;      // data rate field (e.g. WLANFRAME_RATE_6)
    unsigned int SERVICE;       // NULL: 7 scrambler initialization plus 9 reserved bits
    unsigned int TXPWR_LEVEL;   // transmit power level, (1-8)
};

// RXVECTOR parameters structure
struct wlan_rxvector_s {
    unsigned int LENGTH;        // length of payload (1-4095)
    unsigned int RSSI;          // received signal strength indicator
    unsigned int DATARATE;      // data rate field (e.g. WLANFRAME_RATE_6)
    unsigned int SERVICE;       // NULL: 7 scrambler initialization plus 9 reserved bits
};

// 
// wlan frame generator
//

// forward declaration of WLAN frame generator
typedef struct wlanframegen_s * wlanframegen;

// create WLAN framing generator object
wlanframegen wlanframegen_create();

// destroy WLAN framing generator object
void wlanframegen_destroy(wlanframegen _q);

// print WLAN framing generator object internals
void wlanframegen_print(wlanframegen _q);

// reset WLAN framing generator object internal state
void wlanframegen_reset(wlanframegen _q);

// assemble frame (see Table 76)
//  _q          :   framing object
//  _payload    :   raw payload data [size: _opts.LENGTH x 1]
//  _txvector   :   framing options
void wlanframegen_assemble(wlanframegen           _q,
                           unsigned char *        _payload,
                           struct wlan_txvector_s _txvector);

// write OFDM symbol, returning '1' when frame is complete
//  _q          :   framing generator object
//  _buffer     :   output sample buffer [size: 80 x 1]
int wlanframegen_writesymbol(wlanframegen           _q,
                             liquid_float_complex * _buffer);


// 
// wlan frame synchronizer
//

// forward declaration of WLAN framing synchronizer
typedef struct wlanframesync_s * wlanframesync;

// callback function
//  _header_valid   : flag indicating if header is valid
//  _payload        : received payload (NULL if header isn't valid)
//  _rxvector       : received vector (see Table 77)
//  _userdata       : user-defined data object
typedef int (*wlanframesync_callback)(int                    _header_valid,
                                      unsigned char *        _payload,
                                      struct wlan_rxvector_s _rxvector,
                                      void *                 _userdata);

// create WLAN framing synchronizer object
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data structure
wlanframesync wlanframesync_create(wlanframesync_callback _callback,
                                   void *                 _userdata);

// destroy WLAN framing synchronizer object
void wlanframesync_destroy(wlanframesync _q);

// print WLAN framing synchronizer object internals
void wlanframesync_print(wlanframesync _q);

// reset WLAN framing synchronizer object internal state
void wlanframesync_reset(wlanframesync _q);

// execute framing synchronizer on input buffer
//  _q      :   framing synchronizer object
//  _buffer :   input buffer [size: _n x 1]
//  _n      :   input buffer size
void wlanframesync_execute(wlanframesync          _q,
                           liquid_float_complex * _buffer,
                           unsigned int           _n);

// query methods
float wlanframesync_get_rssi(wlanframesync _q); // received signal strength indication
float wlanframesync_get_cfo(wlanframesync _q);  // carrier offset estimate

// 
// internal/debugging methods
//
void wlanframesync_debug_enable(wlanframesync _q);
void wlanframesync_debug_disable(wlanframesync _q);
void wlanframesync_debug_print(wlanframesync _q, const char * _filename);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __LIQUID_WLAN_H__

