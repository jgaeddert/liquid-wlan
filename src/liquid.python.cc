#include "liquid-wlan.python.hh"

#include "wlanframegen.hh"

namespace liquid {
namespace wlan {

PYBIND11_MODULE(liquidwlan, m) {
    m.doc() = "software-defined radio signal processing library";

    m.def("version", [](){return "0.1.0";}, R"pbdoc(
        Version number for liquid-wlan.

        Some other explanation about the version function.
    )pbdoc");

    // initialize objects
    init_framegen(m);
}

} // namespace wlan
} // namespace liquid

