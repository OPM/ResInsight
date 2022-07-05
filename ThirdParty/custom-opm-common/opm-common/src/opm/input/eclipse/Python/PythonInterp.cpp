/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifdef EMBEDDED_PYTHON
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/pytypes.h>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/common/utility/FileSystem.hpp>

#include "python/cxx/export.hpp"
#include "PythonInterp.hpp"
#include "EmbedModule.hpp"

namespace py = pybind11;
namespace Opm {


/*
  OPM_EMBEDDED_MODULE create a Python of all the Python/C++ classes which are
  generated in the python::common::export_all() function in the wrapping code.
*/

OPM_EMBEDDED_MODULE(opm_embedded, module) {
    python::common::export_all(module);
}


bool PythonInterp::exec(const std::string& python_code, py::module& context) {
    py::bool_ def_result = false;
    context.attr("result") = &def_result;
    py::exec(python_code, py::globals() , py::dict(py::arg("context") = context));
    const auto& result = static_cast<py::bool_>(context.attr("result"));
    return result;
}



bool PythonInterp::exec(const std::string& python_code, const Parser& parser, Deck& deck) {
    if (!this->guard)
        throw std::logic_error("Python interpreter not enabled");

    auto context = py::module::import("opm_embedded");
    context.attr("deck") = &deck;
    context.attr("parser") = &parser;
    return this->exec(python_code, context);
}



bool PythonInterp::exec(const std::string& python_code) {
    if (!this->guard)
        throw std::logic_error("Python interpreter not enabled");

    auto context = py::module::import("opm_embedded");
    return this->exec(python_code, context);
}

PythonInterp::PythonInterp(bool enable) {
    if (enable) {
        if (Py_IsInitialized())
            throw std::logic_error("An instance of the Python interpreter is already running");

        this->guard = std::make_unique<py::scoped_interpreter>();
    }
}

}
#endif
