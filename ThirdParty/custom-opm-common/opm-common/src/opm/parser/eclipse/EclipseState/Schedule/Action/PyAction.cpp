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
#include <fstream>
#include <memory>

#ifdef EMBEDDED_PYTHON
#include "src/opm/parser/eclipse/Python/PyRunModule.hpp"
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif


#include <opm/common/utility/String.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/PyAction.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>


namespace Opm {
namespace Action {

PyAction::RunCount PyAction::from_string(std::string run_count) {
    run_count = uppercase(run_count);

    if (run_count == "SINGLE")
        return RunCount::single;

    if (run_count == "UNLIMITED")
        return RunCount::unlimited;

    if (run_count == "FIRST_TRUE")
        return RunCount::first_true;

    throw std::invalid_argument("RunCount string: " + run_count + " not recognized ");
}

PyAction PyAction::serializeObject()
{
    PyAction result;

    result.m_name = "name";
    result.m_run_count = RunCount::first_true;
    result.m_active = false;
    result.module_file = "no.such.file.py";

    return result;
}

bool PyAction::active() const {
    return this->m_active;
}


const std::string& PyAction::name() const {
    return this->m_name;
}

void PyAction::update(bool result) const {
    if (this->m_run_count == RunCount::single)
        this->m_active = false;

    if (this->m_run_count == RunCount::first_true && result)
        this->m_active = false;
}

bool PyAction::operator==(const PyAction& other) const {
    return this->m_name == other.m_name &&
        this->m_run_count == other.m_run_count &&
        this->m_active == other.m_active &&
        this->module_file == other.module_file;
}


#ifndef EMBEDDED_PYTHON

bool PyAction::run(EclipseState&, Schedule&, std::size_t, SummaryState&) const
{
    return false;
}

PyAction::PyAction(std::shared_ptr<const Python>, const std::string& name, RunCount run_count, const std::string& fname) :
    m_name(name),
    m_run_count(run_count),
    module_file(fname)
{}

#else

PyAction::PyAction(std::shared_ptr<const Python> python, const std::string& name, RunCount run_count, const std::string& fname) :
    run_module( std::make_shared<Opm::PyRunModule>(python, fname)),
    m_name(name),
    m_run_count(run_count),
    module_file(fname)
{
}


bool PyAction::run(EclipseState& ecl_state, Schedule& schedule, std::size_t report_step, SummaryState& st) const
{
    /*
      For PyAction instances which have been constructed the 'normal' way
      through the four argument constructor the run_module member variable has
      already been correctly initialized, however if this instance lives on a
      rank != 0 process and has been created through deserialization it was
      created without access to a Python handle and we must import the module
      now.
     */
    if (!this->run_module)
        this->run_module = std::make_shared<Opm::PyRunModule>(schedule.python(), this->module_file);

    return this->run_module->run(ecl_state, schedule, report_step, st);
}



#endif

}
}
