
liquid-wlan
===========
Software reference implementation of IEEE 802.11a physical layer

liquid-wlan is a free and open-source wireless LAN (local area network)
implementation of the 802.11 a/g physical layer specification.

Installation and Dependencies
-----------------------------

### Library Dependencies ###

The following libraries must be installed on your machine before
building liquid-wlan:

  * [liquid-dsp](http://github.com/jgaeddert/liquid-dsp), `231a3088d3`
    revision or later
  * `libc/libm` (standard C and math libraries)

You may _optionally_ have FFTW installed

  * [fftw3](www.fftw.org/)

### Getting the source code ###

Clone the entire Git [repository](http://github.com/jgaeddert/liquid-wlan)
        
    $ git clone git://github.com/jgaeddert/liquid-dsp.git

### Installation ###

Once you have obtained a copy of the source code, you can now build the
WLAN library:

    $ ./bootstrap.sh
    $ ./configure
    $ make
    $ sudo make install

If you are installing on Linux for the first time, you will also need
to rebind your dynamic libraries with `sudo ldconfig` to make the
shared object available.

If you decide that you want to remove the installed library, simply
run

    $ sudo make uninstall

### Run all test scripts ###

Source code validation is a critical step in any software library,
particulary for verifying the portability of code to different
processors and platforms. Packaged with liquid-dsp are a number of
automatic test scripts to validate the correctness of the source code.
The test scripts are located in the `autotest/` subdirectory and
take the form of a C source file. Each test program is compiled and
linked against the local library. You can compile and run all the test
programs by invoking

    $ make check

### Examples ###

All examples are built as stand-alone programs and can be compiled with
the make target `examples`:

    $ make examples

### Benchmarking tool ###

Packaged with liquid-wlan are benchmarks to determine the speed each
signal processing element can run on your machine. These are stand-
alone programs located in the `benchmark` subdirectory. You can build
and run all the benchmark programs with the following command:

    $ make bench

