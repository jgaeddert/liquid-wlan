=========================================================================

  liquid-wlan : 802.11 a/g SDR reference implementation

=========================================================================

liquid-wlan is a free and open-source wireless LAN (local area network)
implementation of the 802.11 a/g physical layer specification.

=========================================================================
 Installation and Dependencies
=========================================================================

Library Dependencies:
  * liquid-dsp (http://github.com/jgaeddert/liquid-dsp) revision 231a3088d3
  * fftw3      (www.fftw.org/)
  * libc/libm  (standard C and math libraries)

BUILD:
    $ ./reconf
    $ ./configure
    $ make

INSTALL:
    # make install

UNINSTALL:
    # make uninstall

TEST:
    $ make check

