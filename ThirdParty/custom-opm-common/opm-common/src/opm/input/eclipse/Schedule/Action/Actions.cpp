/*
  Copyright 2018 Equinor ASA.

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
#include <algorithm>

#include <opm/input/eclipse/Schedule/Action/Actions.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>

namespace Opm {
namespace Action {


Actions::Actions(const std::vector<ActionX>& action, const std::vector<PyAction>& pyaction)
    : actions(action),
      pyactions(pyaction)
{}


Actions Actions::serializeObject()
{
    Actions result;
    result.actions = {ActionX::serializeObject()};
    result.pyactions = {PyAction::serializeObject()};

    return result;
}


std::size_t Actions::ecl_size() const {
    return this->actions.size();
}

std::size_t Actions::py_size() const {
    return this->pyactions.size();
}

bool Actions::empty() const {
    return this->actions.empty() && this->pyactions.empty();
}


void Actions::add(const ActionX& action) {
    auto iter = std::find_if( this->actions.begin(), this->actions.end(), [&action](const ActionX& arg) { return arg.name() == action.name(); });
    if (iter == this->actions.end())
        this->actions.push_back(action);
    else {
        auto id = iter->id() + 1;
        *iter = action;
        iter->update_id(id);
    }
}

void Actions::add(const PyAction& pyaction) {
    auto iter = std::find_if( this->pyactions.begin(), this->pyactions.end(), [&pyaction](const PyAction& arg) { return arg.name() == pyaction.name(); });
    if (iter == this->pyactions.end())
        this->pyactions.push_back(pyaction);
    else
        *iter = pyaction;
}


const ActionX& Actions::operator[](const std::string& name) const {
    const auto iter = std::find_if( this->actions.begin(), this->actions.end(), [&name](const ActionX& action) { return action.name() == name; });
    if (iter == this->actions.end())
        throw std::range_error("No such action: " + name);

    return *iter;
}

bool Actions::has(const std::string& name) const {
    const auto iter = std::find_if( this->actions.begin(), this->actions.end(), [&name](const ActionX& action) { return action.name() == name; });
    return (iter != this->actions.end());
}


const ActionX& Actions::operator[](std::size_t index) const {
    return this->actions[index];
}

int Actions::max_input_lines() const {
    std::size_t max_il = 0;
    for (const auto& act : this-> actions) {
        if (act.keyword_strings().size() > max_il) max_il = act.keyword_strings().size() ;
    }
    return static_cast<int>(max_il);
}


bool Actions::ready(const State& state, std::time_t sim_time) const {
    for (const auto& action : this->actions) {
        if (action.ready(state, sim_time))
            return true;
    }
    return false;
}

std::vector<const PyAction *> Actions::pending_python(const State& state) const {
    std::vector<const PyAction *> pyaction_vector;
    for (const auto& pyaction : this->pyactions) {
        if (pyaction.ready(state))
            pyaction_vector.push_back( &pyaction );
    }
    return pyaction_vector;

}


std::vector<const ActionX *> Actions::pending(const State& state, std::time_t sim_time) const {
    std::vector<const ActionX *> action_vector;
    for (const auto& action : this->actions) {
        if (action.ready(state, sim_time))
            action_vector.push_back( &action );
    }
    return action_vector;
}

std::vector<ActionX>::const_iterator Actions::begin() const {
    return this->actions.begin();
}

std::vector<ActionX>::const_iterator Actions::end() const {
    return this->actions.end();
}


bool Actions::operator==(const Actions& data) const {
    return this->actions == data.actions &&
           this->pyactions == data.pyactions;
}

}
}
