/*
This Code is a copy paste of part of the contents of pybind11/embed.h
It allows for slightly changing the python embedding without changing the pybind11 sourcecode.
*/

#ifndef OPM_EMBED_MODULE
#define OPM_EMBED_MODULE

#ifdef EMBEDDED_PYTHON
#include <pybind11/embed.h>

#define OPM_EMBEDDED_MODULE(name, variable)                              \
    static void PYBIND11_CONCAT(pybind11_init_, name)(pybind11::module &);    \
    static PyObject PYBIND11_CONCAT(*pybind11_init_wrapper_, name)() {        \
        auto m = pybind11::module(PYBIND11_TOSTRING(name));                   \
        try {                                                                 \
            PYBIND11_CONCAT(pybind11_init_, name)(m);                         \
            return m.ptr();                                                   \
        } catch (pybind11::error_already_set &e) {                            \
            PyErr_SetString(PyExc_ImportError, e.what());                     \
            return nullptr;                                                   \
        } catch (const std::exception &e) {                                   \
            PyErr_SetString(PyExc_ImportError, e.what());                     \
            return nullptr;                                                   \
        }                                                                     \
    }                                                                         \
    PYBIND11_EMBEDDED_MODULE_IMPL(name)                                       \
    Opm::embed::python_module name(PYBIND11_TOSTRING(name),           \
                               PYBIND11_CONCAT(pybind11_init_impl_, name));   \
    void PYBIND11_CONCAT(pybind11_init_, name)(pybind11::module &variable)

namespace Opm {
namespace embed {

/// Python 2.7/3.x compatible version of `PyImport_AppendInittab` and error checks.
struct python_module {
#if PY_MAJOR_VERSION >= 3
    using init_t = PyObject *(*)();
#else
    using init_t = void (*)();
#endif
    python_module(const char *name, init_t init) {

        auto result = PyImport_AppendInittab(name, init);
        if (result == -1)
            pybind11::pybind11_fail("Insufficient memory to add a new module");
    }
};

}
}




#endif

#endif
