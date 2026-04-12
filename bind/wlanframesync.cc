#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "wlanframesync.hh"

using namespace liquid;

#ifdef LIQUID_PYTHONLIB
void wlan::framesync::py_execute(py::array_t<std::complex<float>> & _buf)
{
    // get buffer info
    py::buffer_info info = _buf.request();

    // verify input size and dimensions
    if (info.itemsize != sizeof(std::complex<float>))
        throw std::runtime_error("invalid input numpy size, use dtype=np.csingle");
    if (info.ndim != 1)
        throw std::runtime_error("invalid number of input dimensions, must be 1-D array");

    // execute on input
    execute((std::complex<float>*) info.ptr, info.shape[0]);
}

int wlan::py_callback_wrapper_framesync(
        int                    _header_valid,
        unsigned char *        _payload,
        struct wlan_rxvector_s _rxvector,
        framesyncstats_s       _stats,
        void *                 _userdata)
{
    // type cast user data as frame synchronizer
    wlan::framesync * fs = (wlan::framesync*) _userdata;

    // payload bytes
    py::array_t<uint8_t> payload({_rxvector.LENGTH,},{1,},(uint8_t*)_payload);

    // frame symbols
    std::complex<float> * s = (std::complex<float>*) _stats.framesyms;
    py::array_t<std::complex<float>> syms({_stats.num_framesyms,},{sizeof(std::complex<float>),},s);

    // wrap C-style callback and invoke python callback
    py::dict stats = py::dict(
        "rxvector"_a = py::dict(
            "length"_a  = _rxvector.LENGTH,
            "rssi"_a    = _rxvector.RSSI,
            "datarate"_a= _rxvector.DATARATE,
            "service"_a = _rxvector.SERVICE
        ),
        "evm"_a           = _stats.evm,
        "rssi"_a          = _stats.rssi,
        "cfo"_a           = _stats.cfo,
        "framesyms"_a     = syms, //py::none(),
        "mod_scheme"_a    = std::string( modulation_types[_stats.mod_scheme].name ),
        "fec"_a           = std::string( fec_scheme_str[_stats.fec0][1] )
        );
    //py::object o =
    fs->py_callback(fs->context,payload,stats);

#if 0
    // interpret return value
    if (py::isinstance<py::bool_>(o)) {
        return bool(py::bool_(o)) ? 0 : 1;
    } else if (py::isinstance<py::int_>(o)) {
        return int(py::int_(o));
    }
#endif
    return 0;
}

void init_framesync(py::module &m)
{
    py::class_<wlan::framesync>(m, "framesync", "WLAN frame synchronizer")
        .def(py::init<py_framesync_callback,py::object>(),
             py::arg("callback") = py_framesync_callback_default,
             py::arg("context") = py::none(),
             "create synchronizer given callback function and context")
        .def("__repr__", &wlan::framesync::repr)
        .def("reset",
             &wlan::framesync::reset,
             "reset frame synchronizer object")
        .def("execute",
             &wlan::framesync::py_execute,
             "execute on a block of samples")
        .def("reset_framedatastats",
             &wlan::framesync::reset_framedatastats,
             "reset frame statistics data")
        .def_property_readonly("framedatastats",
            &wlan::framesync::py_get_framedatastats,
            "get frame data statistics")
        .def_property_readonly("num_frames_detected",
            &wlan::framesync::get_num_frames_detected,
            "get number of frames currently detected")
        .def_property_readonly("num_headers_valid",
            &wlan::framesync::get_num_headers_valid,
            "get number of headers currently valid")
        .def_property_readonly("num_payloads_valid",
            &wlan::framesync::get_num_payloads_valid,
            "get number of payloads currently valid")
        .def_property_readonly("num_bytes_received",
            &wlan::framesync::get_num_bytes_received,
            "get number of bytes currently received")
#if 0
        .def_property("threshold",
            &wlan::framesync::get_threshold,
            &wlan::framesync::set_threshold,
            "get/set detection threshold")
#endif
        ;
}
#endif // LIQUID_PYTHONLIB

