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


#ifndef PYTHON_INTERP
#define PYTHON_INTERP

#include <string>
#include <stdexcept>
#include <memory>

#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Action/PyAction.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>


#ifdef EMBEDDED_PYTHON

#include <pybind11/embed.h>
namespace py = pybind11;
#endif



namespace Opm {

#ifdef EMBEDDED_PYTHON
class Parser;
class Deck;


class __attribute__ ((visibility("hidden"))) PythonInterp {
public:
    explicit PythonInterp(bool enable);
    bool exec(const std::string& python_code);
    bool exec(const std::string& python_code, const Parser& parser, Deck& deck);
    static bool supported() { return true; };
    explicit operator bool() const { return bool(this->guard); }
private:
    void load_module(const std::string& python_file);
    bool exec(const std::string& python_code, py::module& context);

    std::unique_ptr<py::scoped_interpreter> guard;
};


#else

class PythonInterp {
public:
    explicit PythonInterp(bool enable) {
        if (enable)
            this->fail();
    }

    bool exec(const std::string&) {
        return this->fail();
    };

    bool exec(const std::string&, const Parser&, Deck&) {
        return this->fail();
    }

    bool exec(const Action::PyAction&, EclipseState&, Schedule&, std::size_t, SummaryState& ) {
        return this->fail();
    }

    static bool supported() { return false; };
    explicit operator bool() const { return false; }
private:
    bool fail() { throw std::logic_error("The current opm code has been built without Python support;"); }
};

#endif

}

#endif
