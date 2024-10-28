#include "liquid-wlan.python.hh"

#include "wlanframegen.hh"

void init_framegen(py::module &m);

PYBIND11_MODULE(liquidwlan, m) {
    m.doc() =
    R"pbdoc(
        software-defined radio WLAN library
        -----------------------------------
    )pbdoc";

    // initialize objects
    init_framegen(m);

    // attributes
    m.attr("__version__") = "0.1.0";
}

