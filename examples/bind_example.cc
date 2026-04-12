// Test generation of wlan frame
#include <iostream>

#include "wlanframegen.hh"

static int callback(int                    _header_valid,
                    unsigned char *        _payload,
                    struct wlan_rxvector_s _rxvector,
                    void *                 _userdata)
{
    printf("**** callback invoked (header: %s)\n", _header_valid ? "valid" : "INVALID");
    return 0;
}

int main(int argc, char*argv[])
{
    liquid::wlan::framegen fg;
    std::cout << fg << std::endl;

    // assemble payload with random data of a fixed length
    fg.assemble(800);

    // create frame synchronizer
    wlanframesync fs = wlanframesync_create(callback,NULL);

    // write samples to buffer and run through decoder
    // generate/synchronize frame
    std::complex<float> buffer[80];   // tx sample buffer
    int last_frame = 0;
    while (!last_frame) {
        // write symbol
        last_frame = fg.writesymbol(buffer);

#if 0
        // push through channel (add noise, carrier offset)
        for (i=0; i<80; i++) {
            buffer[i] *= cexpf(_Complex_I*(phi + dphi*n));
            buffer[i] *= gamma;
            buffer[i] += nstd*( randnf() + _Complex_I*randnf() )*M_SQRT1_2;

            // write buffer to file
            fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", n+1, crealf(buffer[i]), cimagf(buffer[i]));

            n++;
        }
#endif

        // run through synchronize
        wlanframesync_execute(fs, (liquid_float_complex*)buffer, 80);
    }

    wlanframesync_destroy(fs);
    printf("done.\n");
    return 0;
}

