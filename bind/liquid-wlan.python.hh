#ifndef __LIQUID_WLAN_PYTHON_HH__
#define __LIQUID_WLAN_PYTHON_HH__

#ifdef LIQUID_PYTHONLIB

#include <complex>
#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
//#include "liquid-wlan.h"
namespace py = pybind11;
using namespace pybind11::literals;

// user context, payload, info
typedef std::function<py::object(py::object,
                                 py::array_t<uint8_t>,
                                 py::dict)> py_framesync_callback;

// default callback function
static py_framesync_callback py_framesync_callback_default =
    [](py::object,
       py::array_t<uint8_t>,
       py::dict)
    { return py::none(); };

#endif // LIQUID_PYTHONLIB

#endif // __LIQUID_WLAN_PYTHON_HH__

