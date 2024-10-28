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
void init_framegen(py::module &m)
{
    py::class_<wlan::framegen>(m, "framegen", "Frame generator with 64-byte payload")
        .def(py::init<>())
        .def("__repr__", &wlan::framegen::repr)
        .def_property_readonly("header_len",
            &wlan::framegen::get_header_length,
            "get length of header (bytes)")
        .def("execute",
            &wlan::framegen::py_execute,
            "generate a frame given header and payload",
            py::arg("header")=py::none(),
            py::arg("payload")=py::none())
        ;
}
#endif

