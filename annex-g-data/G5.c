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

// Table G.5: Frequency domain of long sequence
// NOTE : data are fft-shifted from table listing. Here the data are listed
//        as the IFFT input such that the first element corresponds to the
//        DC subcarrier.
float complex annexg_G5[64] = {
      0.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      0.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
     -1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I,
      1.0000 +  0.0000*_Complex_I};

