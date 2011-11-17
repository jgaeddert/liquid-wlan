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
// wififramesync_example.c
//
// Test generation/synchronization of wifi frame
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid-802-11.h"

static int callback(float complex * _X,
                    void * _userdata);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // create frame generator
    wififramegen fg = wififramegen_create();
    wififramegen_print(fg);

    // create frame synchronizer
    wififramesync fs = wififramesync_create(callback, NULL);
    wififramesync_print(fs);

    // destroy objects
    wififramegen_destroy(fg);
    wififramesync_destroy(fs);


    printf("done.\n");
    return 0;
}

static int callback(float complex * _X,
                    void * _userdata)
{
    printf("**** callback invoked\n");

    return 0;
}

