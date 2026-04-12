#include "liquid-wlan.python.hh"

#include "wlanframegen.hh"

void init_framegen(py::module &m);
void init_framesync(py::module &m);

PYBIND11_MODULE(liquid_wlan, m)
{
    m.doc() =
    R"pbdoc(
        software-defined radio WLAN library
        -----------------------------------
    )pbdoc";

    // initialize objects
    init_framegen(m);
    init_framesync(m);

    // attributes
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}

