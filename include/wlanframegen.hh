// wlanframegen bindings
#ifndef __WLANFRAMEGEN_HH__
#define __WLANFRAMEGEN_HH__

#include <complex>
#include <iostream>
#include <string>
#include <liquid/liquid.h>
#include "liquid-wlan.h"

namespace liquid {
namespace wlan {

class framegen
{
  public:
    // default constructor
    framegen() { fg = wlanframegen_create();  }

    // destructor
    ~framegen() { wlanframegen_destroy(fg); }

    void reset() { }

    // object type
    std::string type() const { return "framegen"; }

    std::string repr() const { return std::string("<liquid.wlen.framegen") +
                    ">"; }

    // overloaded << operator for output stream extraction
    friend std::ostream & operator<<(std::ostream & os, framegen & rhs)
        { os << rhs.repr(); return os; }

    // assemble frame (random data)
    void assemble(unsigned int _length,
                  unsigned int _datarate = WLANFRAME_RATE_6,
                  unsigned int _service = 0,
                  unsigned int _txpwr_level = 1);

    /*! @brief write OFDM symbol returning flag if the frame is complete
     *  @param _buf input sample buffer, shape: (80,)
     *  @return boolean flag indicating if frame is complete
     */
    bool writesymbol(std::complex<float> * _buf)
        { return wlanframegen_writesymbol(fg, _buf); }

  private:
    wlanframegen fg;

#ifdef PYTHONLIB
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

#ifdef PYTHONLIB
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
