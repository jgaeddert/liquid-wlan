// wlanframegen bindings
#ifndef __WLANFRAMEGEN_HH__
#define __WLANFRAMEGEN_HH__

#include <complex>
#include <iostream>
#include <string>
#include "liquid-wlan.h"

namespace liquid {
namespace wlan {

class framegen
{
  public:
    // default constructor
    framegen() { q = wlanframegen_create();  }

    // destructor
    ~framegen() { wlanframegen_destroy(q); }

    void reset() { }

    // object type
    std::string type() const { return "framegen"; }

    std::string repr() const { return std::string("<liquid.wlen.framegen") +
                    ">"; }

    // overloaded << operator for output stream extraction
    friend std::ostream & operator<<(std::ostream & os, framegen & rhs)
        { os << rhs.repr(); return os; }

    //void execute(unsigned char * _header,
    //             unsigned char * _payload,
    //             std::complex<float> * _frame)
    //{ wlanframegen_execute(q, _header, _payload, _frame); }

  private:
    wlanframegen q;

#ifdef LIQUID_PYTHONLIB
  public:
    py::array_t<std::complex<float>> py_execute(py::object & _header,
                                                py::object & _payload)
    {
        // TODO: assemble header and determine length
        unsigned int frame_len = 100;

        // allocate output buffer
        py::array_t<std::complex<float>> buf(frame_len);

        // TODO: assemble and generate frame

        // pass to top-level execute method
        return buf;
    }
#endif
};

#ifdef LIQUID_PYTHONLIB
static void init_framegen(py::module &m)
{
    py::class_<framegen>(m, "framegen", "Frame generator with 64-byte payload")
        .def(py::init<>())
        .def("__repr__", &framegen::repr)
        .def_property_readonly("header_len",
            &framegen::get_header_length,
            "get length of header (bytes)")
        .def("execute",
            &framegen::py_execute,
            "generate a frame given header and payload",
            py::arg("header")=py::none(),
            py::arg("payload")=py::none())
        ;
}
#endif

} // namespace wlan
} // namespace liquid

#endif //__framegen_HH__
