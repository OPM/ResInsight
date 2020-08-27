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

#ifndef ACTION_STATE_HPP
#define ACTION_STATE_HPP

#include <ctime>
#include <map>

namespace Opm {
namespace Action {

class ActionX;
class State {

struct RunState {
    RunState(std::time_t sim_time) :
        run_count(1),
        last_run(sim_time)
    {}

    void add_run(std::time_t sim_time) {
        this->last_run = sim_time;
        this->run_count += 1;
    }


    std::size_t run_count;
    std::time_t last_run;
};

public:
    void add_run(const ActionX& action, std::time_t sim_time);
    std::size_t run_count(const ActionX& action) const;
    std::time_t run_time(const ActionX& action) const;
private:
    std::map<std::pair<std::string, std::size_t>, RunState> run_state;
};


}
}
#endif
