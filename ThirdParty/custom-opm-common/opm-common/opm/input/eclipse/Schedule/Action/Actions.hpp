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


#ifndef ActionCOnfig_HPP
#define ActionCOnfig_HPP

#include <string>
#include <ctime>
#include <vector>

#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/PyAction.hpp>

namespace Opm {
namespace Action {

class State;

/*
  The Actions class is a container of ACTIONX keywords. The main functionality
  is to provide a list of ACTIONX keywords which are ready to be evaluated.
*/

class Actions {
public:
    Actions() = default;
    Actions(const std::vector<ActionX>& action, const std::vector<PyAction>& pyactions);

    static Actions serializeObject();

    std::size_t py_size() const;
    std::size_t ecl_size() const;
    int max_input_lines() const;
    bool empty() const;
    void add(const ActionX& action);
    void add(const PyAction& pyaction);
    bool ready(const State& state, std::time_t sim_time) const;
    const ActionX& operator[](const std::string& name) const;
    const ActionX& operator[](std::size_t index) const;
    std::vector<const ActionX *> pending(const State& state, std::time_t sim_time) const;
    std::vector<const PyAction *> pending_python(const State& state) const;

    bool has(const std::string& name) const;
    std::vector<ActionX>::const_iterator begin() const;
    std::vector<ActionX>::const_iterator end() const;

    bool operator==(const Actions& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(actions);
        serializer.vector(pyactions);
    }

private:
    std::vector<ActionX> actions;
    std::vector<PyAction> pyactions;
};
}
}
#endif
