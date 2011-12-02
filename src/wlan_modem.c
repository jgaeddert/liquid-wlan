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
// wlan modem (modulator/demodulator)
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "liquid-wlan.internal.h"

//
// modulation
//

float complex wlan_modulate(unsigned int  _scheme,
                            unsigned char _sym)
{
    switch (_scheme) {
    case WLAN_MODEM_BPSK:  return wlan_modulate_bpsk(_sym);
    case WLAN_MODEM_QPSK:  return wlan_modulate_qpsk(_sym);
    case WLAN_MODEM_QAM16: return wlan_modulate_qam16(_sym);
    case WLAN_MODEM_QAM64: return wlan_modulate_qam64(_sym);
    default:
        fprintf(stderr,"error: wlan_modulate(), invalid scheme\n");
        exit(1);
    }
}

float complex wlan_modulate_bpsk(unsigned char _sym)
{
    return _sym ? 1.0f : -1.0f;
}

float complex wlan_modulate_qpsk(unsigned char _sym)
{
    return (_sym & 0x02 ? M_SQRT1_2 : -M_SQRT1_2) +
           (_sym & 0x01 ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
}

float complex wlan_modulate_qam16(unsigned char _sym)
{
    return wlan_modem_qam16[_sym & 0x0f];
}

float complex wlan_modulate_qam64(unsigned char _sym)
{
    return wlan_modem_qam64[_sym & 0x3f];
}

//
// demodulation
//

unsigned char wlan_demodulate(unsigned int  _scheme,
                              float complex _sample)
{
    switch (_scheme) {
    case WLAN_MODEM_BPSK:  return wlan_demodulate_bpsk(_sample);
    case WLAN_MODEM_QPSK:  return wlan_demodulate_qpsk(_sample);
    case WLAN_MODEM_QAM16: return wlan_demodulate_qam16(_sample);
    case WLAN_MODEM_QAM64: return wlan_demodulate_qam64(_sample);
    default:
        fprintf(stderr,"error: wlan_demodulate(), invalid scheme\n");
        exit(1);
    }
}

unsigned char wlan_demodulate_bpsk(float complex _sample)
{
    return crealf(_sample) > 0.0f ? 1 : 0;
}

unsigned char wlan_demodulate_qpsk(float complex _sample)
{
    return ( crealf(_sample) > 0.0f ? 0x01 : 0 ) |
           ( cimagf(_sample) > 0.0f ? 0x02 : 0 );
}

// demodulate 16-QAM symbol
//  2/sqrt(10) ~ 0.632455532033676
unsigned char wlan_demodulate_qam16(float complex _sample)
{
    unsigned char sym = 0x00;

    // demodulate in-phase component
    float v = crealf(_sample);

    if (v > 0.0f) { v -= 0.6324555f; sym |= 0x08; }
    else          { v += 0.6324555f; }

    // 
    if (v > 0.0f) { sym |= 0x04; }
    else          { ; }

    // demodulate quadrature component
    v = cimagf(_sample);

    // 
    if (v > 0.0f) { v -= 0.6324555f; sym |= 0x02; }
    else          { v += 0.6324555f; }

    // 
    if (v > 0.0f) { sym |= 0x01; }
    else          { ; }

    // return symbol
    return sym;
}

// demodulate 16-QAM symbol
//  4/sqrt(42) ~ 0.617213399848368
//  2/sqrt(42) ~ 0.308606699924184
unsigned char wlan_demodulate_qam64(float complex _sample)
{
    unsigned char sym = 0x00;

    // demodulate in-phase component
    float v = crealf(_sample);

    if (v > 0.0f) { v -= 0.6172134f; sym |= 0x20; }
    else          { v += 0.6172134f; }

    // 
    if (v > 0.0f) { v -= 0.3086067f; sym |= 0x10; }
    else          { v += 0.3086067f; }

    // 
    if (v > 0.0f) { sym |= 0x08; }
    else          { ; }

    // demodulate quadrature component
    v = cimagf(_sample);

    if (v > 0.0f) { v -= 0.6172134f; sym |= 0x04; }
    else          { v += 0.6172134f; }

    // 
    if (v > 0.0f) { v -= 0.3086067f; sym |= 0x02; }
    else          { v += 0.3086067f; }

    // 
    if (v > 0.0f) { sym |= 0x01; }
    else          { ; }

    // return symbol
    return sym;
}

// 
// modulation tables
//

// 16-QAM modulation table
const float complex wlan_modem_qam16[16] = {
     -0.94868326 +  -0.94868326*_Complex_I, //   0
     -0.94868326 +  -0.31622776*_Complex_I, //   1
     -0.94868326 +   0.94868326*_Complex_I, //   2
     -0.94868326 +   0.31622776*_Complex_I, //   3
     -0.31622776 +  -0.94868326*_Complex_I, //   4
     -0.31622776 +  -0.31622776*_Complex_I, //   5
     -0.31622776 +   0.94868326*_Complex_I, //   6
     -0.31622776 +   0.31622776*_Complex_I, //   7
      0.94868326 +  -0.94868326*_Complex_I, //   8
      0.94868326 +  -0.31622776*_Complex_I, //   9
      0.94868326 +   0.94868326*_Complex_I, //  10
      0.94868326 +   0.31622776*_Complex_I, //  11
      0.31622776 +  -0.94868326*_Complex_I, //  12
      0.31622776 +  -0.31622776*_Complex_I, //  13
      0.31622776 +   0.94868326*_Complex_I, //  14
      0.31622776 +   0.31622776*_Complex_I};//  15

// 64-QAM modulation table
const float complex wlan_modem_qam64[64] = {
     -1.08012354 +  -1.08012354*_Complex_I, //   0
     -1.08012354 +  -0.77151680*_Complex_I, //   1
     -1.08012354 +  -0.15430336*_Complex_I, //   2
     -1.08012354 +  -0.46291006*_Complex_I, //   3
     -1.08012354 +   1.08012354*_Complex_I, //   4
     -1.08012354 +   0.77151680*_Complex_I, //   5
     -1.08012354 +   0.15430336*_Complex_I, //   6
     -1.08012354 +   0.46291006*_Complex_I, //   7
     -0.77151680 +  -1.08012354*_Complex_I, //   8
     -0.77151680 +  -0.77151680*_Complex_I, //   9
     -0.77151680 +  -0.15430336*_Complex_I, //  10
     -0.77151680 +  -0.46291006*_Complex_I, //  11
     -0.77151680 +   1.08012354*_Complex_I, //  12
     -0.77151680 +   0.77151680*_Complex_I, //  13
     -0.77151680 +   0.15430336*_Complex_I, //  14
     -0.77151680 +   0.46291006*_Complex_I, //  15
     -0.15430336 +  -1.08012354*_Complex_I, //  16
     -0.15430336 +  -0.77151680*_Complex_I, //  17
     -0.15430336 +  -0.15430336*_Complex_I, //  18
     -0.15430336 +  -0.46291006*_Complex_I, //  19
     -0.15430336 +   1.08012354*_Complex_I, //  20
     -0.15430336 +   0.77151680*_Complex_I, //  21
     -0.15430336 +   0.15430336*_Complex_I, //  22
     -0.15430336 +   0.46291006*_Complex_I, //  23
     -0.46291006 +  -1.08012354*_Complex_I, //  24
     -0.46291006 +  -0.77151680*_Complex_I, //  25
     -0.46291006 +  -0.15430336*_Complex_I, //  26
     -0.46291006 +  -0.46291006*_Complex_I, //  27
     -0.46291006 +   1.08012354*_Complex_I, //  28
     -0.46291006 +   0.77151680*_Complex_I, //  29
     -0.46291006 +   0.15430336*_Complex_I, //  30
     -0.46291006 +   0.46291006*_Complex_I, //  31
      1.08012354 +  -1.08012354*_Complex_I, //  32
      1.08012354 +  -0.77151680*_Complex_I, //  33
      1.08012354 +  -0.15430336*_Complex_I, //  34
      1.08012354 +  -0.46291006*_Complex_I, //  35
      1.08012354 +   1.08012354*_Complex_I, //  36
      1.08012354 +   0.77151680*_Complex_I, //  37
      1.08012354 +   0.15430336*_Complex_I, //  38
      1.08012354 +   0.46291006*_Complex_I, //  39
      0.77151680 +  -1.08012354*_Complex_I, //  40
      0.77151680 +  -0.77151680*_Complex_I, //  41
      0.77151680 +  -0.15430336*_Complex_I, //  42
      0.77151680 +  -0.46291006*_Complex_I, //  43
      0.77151680 +   1.08012354*_Complex_I, //  44
      0.77151680 +   0.77151680*_Complex_I, //  45
      0.77151680 +   0.15430336*_Complex_I, //  46
      0.77151680 +   0.46291006*_Complex_I, //  47
      0.15430336 +  -1.08012354*_Complex_I, //  48
      0.15430336 +  -0.77151680*_Complex_I, //  49
      0.15430336 +  -0.15430336*_Complex_I, //  50
      0.15430336 +  -0.46291006*_Complex_I, //  51
      0.15430336 +   1.08012354*_Complex_I, //  52
      0.15430336 +   0.77151680*_Complex_I, //  53
      0.15430336 +   0.15430336*_Complex_I, //  54
      0.15430336 +   0.46291006*_Complex_I, //  55
      0.46291006 +  -1.08012354*_Complex_I, //  56
      0.46291006 +  -0.77151680*_Complex_I, //  57
      0.46291006 +  -0.15430336*_Complex_I, //  58
      0.46291006 +  -0.46291006*_Complex_I, //  59
      0.46291006 +   1.08012354*_Complex_I, //  60
      0.46291006 +   0.77151680*_Complex_I, //  61
      0.46291006 +   0.15430336*_Complex_I, //  62
      0.46291006 +   0.46291006*_Complex_I};//  63

