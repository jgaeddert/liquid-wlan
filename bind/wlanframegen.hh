// wlanframegen bindings
#ifndef __WLANFRAMEGEN_HH__
#define __WLANFRAMEGEN_HH__

#include <complex>
#include <iostream>
#include <string>
#include <liquid/liquid.h>
#include "liquid-wlan.h"
#include "liquid-wlan.python.hh"

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

    std::string repr() const { return std::string("<liquid.wlan.framegen") +
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

    /*! get length of frame in symbols (each 80-samples long) */
    unsigned int length() const
        { return wlanframegen_getframelen(fg); }

  private:
    wlanframegen fg;

#ifdef LIQUID_PYTHONLIB
  public:
    /*! generate frame of a fixed length and data rate */
    py::array_t<std::complex<float>> py_execute(unsigned int _length=200,
                                                unsigned int _datarate=6);
#endif
};

} // namespace wlan
} // namespace liquid

#endif //__framegen_HH__
