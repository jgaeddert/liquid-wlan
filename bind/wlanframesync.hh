// wlanframesync bindings
#ifndef __WLANFRAMESYNC_HH__
#define __WLANFRAMESYNC_HH__

#include <complex>
#include <iostream>
#include <string>
#include <liquid/liquid.h>
#include "liquid-wlan.h"
#include "liquid-wlan.python.hh"

namespace liquid {
namespace wlan {

#ifdef LIQUID_PYTHONLIB
// forward declaration of callback wrapper
int py_callback_wrapper_framesync(
        int                    _header_valid,
        unsigned char *        _payload,
        struct wlan_rxvector_s _rxvector,
        framesyncstats_s       _stats,
        void *                 _userdata);
#endif

class framesync
{
  public:
    // default constructor
    framesync(wlanframesync_callback _callback=NULL, void * _userdata=NULL)
        { fs = wlanframesync_create(_callback, _userdata); }

    // destructor
    ~framesync() { wlanframesync_destroy(fs); }

    // reset internal state
    void reset() { wlanframesync_reset(fs); }

    // object type
    std::string type() const { return "framesync"; }

    // string representation of object
    std::string repr() const { return std::string("<liquid.wlan.framesync") +
                    ">"; }

    // overloaded << operator for output stream extraction
    friend std::ostream & operator<<(std::ostream & os, framesync & rhs)
        { os << rhs.repr(); return os; }

    void execute(std::complex<float> * _buf, unsigned int _buf_len)
        { wlanframesync_execute(fs, _buf, _buf_len); }

#if 0
    void set_callback(framesync_callback _callback=NULL)
        { wlanframesync_set_callback(fs, _callback); }

    void set_userdata(void * _userdata=NULL)
        { wlanframesync_set_userdata(fs, _userdata); }

    // get/set detection threshold
    void  set_threshold(float _threshold) { wlanframesync_set_threshold(fs,_threshold); }
    float get_threshold() const           { return wlanframesync_get_threshold(fs);     }
#endif

    // specific frame data statistics
    unsigned int get_num_frames_detected() const
        { return get_framedatastats().num_frames_detected; }
    unsigned int get_num_headers_valid() const
        { return get_framedatastats().num_headers_valid; }
    unsigned int get_num_payloads_valid() const
        { return get_framedatastats().num_payloads_valid; }
    unsigned int get_num_bytes_received() const
        { return get_framedatastats().num_bytes_received; }

    void reset_framedatastats() { wlanframesync_reset_framedatastats(fs); }

    framedatastats_s get_framedatastats() const
        { return wlanframesync_get_framedatastats(fs); }

  private:
    wlanframesync fs;

#ifdef LIQUID_PYTHONLIB
  //private:
  public:
    py_framesync_callback py_callback;
    py::object context;
    friend int py_callback_wrapper_framesync(
        int                    _header_valid,
        unsigned char *        _payload,
        struct wlan_rxvector_s _rxvector,
        framesyncstats_s       _stats,
        void *                 _userdata);

  public:
    // python-specific constructor with keyword arguments
    framesync(py_framesync_callback _callback,
              py::object            _context)
    {
        fs = wlanframesync_create(py_callback_wrapper_framesync, this);
        py_callback = _callback;
        context     = _context;
    }

    void py_execute(py::array_t<std::complex<float>> & _buf);

    py::dict py_get_framedatastats() const
    {
        framedatastats_s v = wlanframesync_get_framedatastats(fs);
        return py::dict(
            "num_frames_detected"_a = v.num_frames_detected,
            "num_headers_valid"_a   = v.num_headers_valid,
            "num_payloads_valid"_a  = v.num_payloads_valid,
            "num_bytes_received"_a  = v.num_bytes_received
            );
    }
#endif
};

} // namespace wlan
} // namespace liquid

#endif //__WLANFRAMESYNC_HH__
