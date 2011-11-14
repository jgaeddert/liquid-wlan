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
#ifndef __LIQUID_802_11_H__
#define __LIQUID_802_11_H__

/*
 * Make sure the version and version number macros weren't defined by
 * some prevoiusly included header file.
 */
#ifdef LIQUID_802_11_VERSION
#  undef LIQUID_802_11_VERSION
#endif
#ifdef LIQUID_802_11_VERSION_NUMBER
#  undef LIQUID_802_11_VERSION_NUMBER
#endif

/*
 * Compile-time version numbers
 * 
 * LIQUID_802_11_VERSION = "X.Y.Z"
 * LIQUID_802_11_VERSION_NUMBER = (X*1000000 + Y*1000 + Z)
 */
#define LIQUID_802_11_VERSION          "0.0.1"
#define LIQUID_802_11_VERSION_NUMBER   1

/*
 * Run-time library version numbers
 */
extern const char liquid_802_11_version[];
const char * liquid_802_11_libversion(void);
int liquid_802_11_libversion_number(void);

#ifdef __cplusplus
extern "C" {
#   define LIQUID_802_11_USE_COMPLEX_H 0
#else
#   define LIQUID_802_11_USE_COMPLEX_H 1
#endif /* __cplusplus */

#define LIQUID_802_11_CONCAT(prefix, name) prefix ## name
#define LIQUID_802_11_VALIDATE_INPUT

/* 
 * Compile-time complex data type definitions
 *
 * Default: use the C99 complex data type, otherwise
 * define complex type compatible with the C++ complex standard,
 * otherwise resort to defining binary compatible array.
 */
#if LIQUID_802_11_USE_COMPLEX_H==1
#   include <complex.h>
#   define LIQUID_802_11_DEFINE_COMPLEX(R,C) typedef R _Complex C
#elif defined _GLIBCXX_COMPLEX
#   define LIQUID_802_11_DEFINE_COMPLEX(R,C) typedef std::complex<R> C
#else
#   define LIQUID_802_11_DEFINE_COMPLEX(R,C) typedef struct {R real; R imag;} C;
#endif
//#   define LIQUID_802_11_DEFINE_COMPLEX(R,C) typedef R C[2]

LIQUID_802_11_DEFINE_COMPLEX(float,  liquid_float_complex);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __LIQUID_802_11_H__

