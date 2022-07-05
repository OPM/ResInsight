/*
  Copyright 2020 Equinor ASA.

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

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/Actions.hpp>
#include <opm/io/eclipse/rst/state.hpp>

namespace Opm {
namespace Action {

State::action_id State::make_id(const ActionX& action) {
    return std::make_pair(action.name(), action.id());
}


std::size_t State::run_count(const ActionX& action) const {
    auto count_iter = this->run_state.find(this->make_id(action));
    if (count_iter == this->run_state.end())
        return 0;

    return count_iter->second.run_count;
}

std::time_t State::run_time(const ActionX& action) const {
    auto state = this->run_state.at(this->make_id(action));
    return state.last_run;
}


void State::add_run(const ActionX& action, std::time_t run_time, Result result) {
    const auto& id  = this->make_id(action);
    auto count_iter = this->run_state.find(id);
    if (count_iter == this->run_state.end())
        this->run_state.insert( std::make_pair(id, run_time) );
    else
        count_iter->second.add_run(run_time);

    this->last_result.insert_or_assign(action.name(), std::move(result));
}

void State::add_run(const PyAction& action, bool result) {
    this->m_python_result.insert_or_assign( action.name(), result );
}


std::optional<Result> State::result(const std::string& action) const {
    auto iter = this->last_result.find(action);
    if (iter == this->last_result.end())
        return std::nullopt;

    return iter->second;
}


std::optional<bool> State::python_result(const std::string& action) const {
    auto iter = this->m_python_result.find(action);
    if (iter == this->m_python_result.end())
        return std::nullopt;

    return iter->second;
}



/*
  When restoring from restart file we initialize the number of times it has run
  and the last run time. From the evaluation only the 'true' evaluation is
  restored, not the well/group set.
*/
void State::load_rst(const Actions& action_config, const RestartIO::RstState& rst_state) {
    for (const auto& rst_action : rst_state.actions) {
        if (rst_action.run_count > 0) {
            const auto& action = action_config[rst_action.name];
            this->add_run(action, rst_action.last_run.value(), Action::Result{ true });
        }
    }
}


bool State::operator==(const State& other) const {
    return this->run_state == other.run_state &&
           this->last_result == other.last_result &&
           this->m_python_result == other.m_python_result;
}


State State::serializeObject() {
    State st;
    st.run_state.insert(std::make_pair( std::make_pair("ACTION", 100), RunState::serializeObject()));
    st.last_result.insert( std::make_pair("ACTION", Result::serializeObject()));
    st.m_python_result.insert( std::make_pair("PYACTION", false) );
    return st;
}

}
}
