#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "wlanframegen.hh"

using namespace liquid;

void wlan::framegen::assemble(unsigned int _length,
                              unsigned int _datarate,
                              unsigned int _service,
                              unsigned int _txpwr_level)
{
    if (_length < 1 || _length > 1500)
        throw std::invalid_argument("invalid payload size");

    // for now allocate dummy packet to encode
    unsigned char * payload = new unsigned char[_length];

    // assemble internal object
    struct wlan_txvector_s txvector;
    txvector.LENGTH     = _length;
    txvector.DATARATE   = _datarate;
    txvector.SERVICE    = _service;
    txvector.TXPWR_LEVEL= _txpwr_level;
    wlanframegen_assemble(fg, payload, txvector);

    // free temporarily-allocated memory array
    delete [] payload;
}

#ifdef PYTHONLIB
py::array_t<std::complex<float>> wlan::framegen::py_execute(unsigned int _length,
                                                            unsigned int _datarate)
{
    // assemble frame
    assemble(_length, _datarate);

    // determine length of frame in samples
    unsigned int frame_len = length();

    // allocate output buffer
    py::array_t<std::complex<float>> buf(80*frame_len);

    // generate frame
    std::complex<float> * p = (std::complex<float>*) buf.request().ptr;
    for (auto i=0U; i<frame_len; i++)
        wlanframegen_writesymbol(fg, p + i*80);

    // pass to top-level execute method
    return buf;
}
void init_framegen(py::module &m)
{
    py::class_<wlan::framegen>(m, "framegen", "Frame generator with 64-byte payload")
        .def(py::init<>())
        .def("__repr__", &wlan::framegen::repr)
        .def_property_readonly("length",
            &wlan::framegen::length,
            "get length of frame (symbols each 80 samples long)")
        .def("execute",
            &wlan::framegen::py_execute,
            "generate a frame given header and payload",
            py::arg("length")=200,
            py::arg("datarate")=6)
        ;
}
#endif

