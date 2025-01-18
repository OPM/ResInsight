#include <pybind11/pybind11.h>

#include "Riaapplication.h"

int add(int a, int b) {

    auto app = RiaApplication::instance();
    app->closeProject();


    return a + b;
}



// Create a Python module named `example` and bind the `add` function.
PYBIND11_MODULE(example, m) {
    m.doc() = "Example module created using Pybind11";  // Module docstring
    m.def("add", &add, "A function that adds two numbers");

}
