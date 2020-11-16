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

#ifndef OPM_PYTHON_HPP
#define OPM_PYTHON_HPP

#include <memory>
#include <string>

namespace Opm {
class PythonInterp;
class Parser;
class Deck;
class SummaryState;
class Schedule;
class EclipseState;

namespace Action {
    class PyAction;
}
/*
  This class is a thin wrapper around the PythonInterp class. The Python class
  can always be safely instantiated, but the actual PythonInterp implementation
  depends on whether Python support was enabled when this library instance was
  compiled.

  If one the methods actually invoking the Python interpreter is invoked without
  proper Python support a dummy PythinInterp instance will be used; and that
  will just throw std::logic_error. The operator bool can be used to check if
  this Python manager indeed has a valid Python runtime:


     auto python = std::make_shared<Python>();

     if (python)
         python.exec("print('Hello world')")
     else
         OpmLog::Error("This version of opmcommon has been built with support for embedded Python");


  The default constructor will enable the Python interpreter if the current
  version of opm-common has been built support for embedded Python, by using the
  alternative Python(Enable enable) constructor you can explicitly say if you
  want Python support or not; if that request can not be satisfied you will get
  std::logic_error().

  Observe that the real underlying Python interpreter is essentially a singleton
  - i.e. only a one interpreter can be active at any time. If a Python
  interpreter has already been instantiated you can still create an additional
  Opm::Python instance, but that will be empty and not capable of actually
  running Python code - so although it is technically possible to have more than
  simultaneous Opm::Python instance it is highly recommended to create only one.

  The details of the interaction between build configuration, constructor arg
  and multiple instances is summarized in the table below. The columns should be
  interpreted as follows:

    Build: This is whether opm has been built with support for embedding Python,
       i.e. whether the flag OPM_ENABLE_EMBEDDED_PYTHON was set to True at
       configure time.

    Constructor arg: This the enum argument passed to the constructor. The
       default value is Enable::TRY which means that we will try to instantiate
       a Python interpreter. If that fails - either because a Python interpreter
       is already running or because opm-common has been built without Python
       support - you will get a empty but valid Opm::Python object back.

    Existing instance: Is there already Python interpreter running? The value *
       implies that the end result will be the same irrespective of whether we
       have a Python instance running.

    Result: What kind of Opm::Python instance will we get - here { } implies an
       empty Opm::Python instance. This does *not* hold on to an actual
       interpreter and can not be used to run code - for this type of
       Opm::Python instance the enabled() method will return false. { Python }
       means that we will get a Opm::Python instance which manages a true Python
       interpreter.

       std::logic_error means that you have asked for something which can not be
       satisfied and std::logic_error exception will be raised.


  Build:   |  Constructor arg   |  Existing instance  | Result
  ---------|--------------------|---------------------|-------
  True     |  OFF               |  *                  | { }
  True     |  ON                |  True               | std::logic_error
  True     |  ON                |  False              | { Python }
  True     |  TRY               |  True               | { }
  True     |  TRY               |  False              | { Python }
  False    |  OFF               |  *                  | { }
  False    |  ON                |  *                  | std::logic_error
  False    |  TRY               |  *                  | { }
  ---------|--------------------|---------------------|-------


*/


class Python {
public:

    enum class Enable {
        ON,    /* Enable the Python extensions - throw std::logic_error() if it fails. */
        TRY,   /* Try to enable Python extensions*/
        OFF    /* Do not enable Python */
    };

    explicit Python(Enable enable = Enable::TRY);
    bool exec(const std::string& python_code) const;
    bool exec(const std::string& python_code, const Parser& parser, Deck& deck) const;
    bool exec(const Action::PyAction& py_action, EclipseState& ecl_state, Schedule& schedule, std::size_t report_step, SummaryState& st) const;

    /*
      The enabled function returns true if this particular Python instance
      manages a true Python interpreter.
    */
    bool enabled() const;

    /*
      The supported function return true if this instance of opm-common has been
      compiled with support for Python.
    */
    static bool supported();
    bool run_module(const std::string& path);
private:
    std::shared_ptr<PythonInterp> interp;
};

}



#endif

