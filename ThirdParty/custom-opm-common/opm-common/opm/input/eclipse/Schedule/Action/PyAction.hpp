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


#ifndef PYACTION_HPP_
#define PYACTION_HPP_


#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Opm {

class Python;
class EclipseState;
class Schedule;
class SummaryState;
class PyRunModule;

namespace Action {
class State;

class PyAction {
public:
   enum class RunCount {
       single,
       unlimited,
       first_true
    };


    static RunCount from_string(std::string run_count);
    static PyAction serializeObject();
    PyAction() = default;
    PyAction(std::shared_ptr<const Python> python, const std::string& name, RunCount run_count, const std::string& module_file);
    bool run(EclipseState& ecl_state, Schedule& schedule, std::size_t report_step, SummaryState& st,
             const std::function<void(const std::string&, const std::vector<std::string>&)>& actionx_callback) const;
    const std::string& name() const;
    bool ready(const State& state) const;
    bool operator==(const PyAction& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_name);
        serializer(m_run_count);
        serializer(module_file);
        serializer(m_active);
    }

private:
    void update(bool result) const;

    mutable std::shared_ptr< PyRunModule > run_module;
    std::string m_name;
    RunCount m_run_count;
    std::string module_file;
    mutable bool m_active = true;
};
}

}

#endif
