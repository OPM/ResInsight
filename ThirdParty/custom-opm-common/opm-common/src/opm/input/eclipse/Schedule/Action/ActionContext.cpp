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

#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/common/utility/TimeService.hpp>

namespace Opm {
namespace Action {

    void Context::add(const std::string& func, const std::string& arg, double value) {
        this->values[func + ":" + arg] = value;
    }

    Context::Context(const SummaryState& summary_state_arg, const WListManager& wlm_) :
        summary_state(summary_state_arg),
        wlm(wlm_)
    {
        for (const auto& pair : TimeService::eclipseMonthIndices())
            this->add(pair.first, pair.second);
    }

    void Context::add(const std::string& func, double value) {
        this->values[func] = value;
    }


    double Context::get(const std::string& func, const std::string& arg) const {
        return this->get(func + ":" + arg);
    }

    double Context::get(const std::string& key) const {
        const auto& iter = this->values.find(key);
        if (iter != this->values.end())
            return iter->second;

        return this->summary_state.get(key);
    }


    std::vector<std::string> Context::wells(const std::string& key) const {
        return this->summary_state.wells(key);
    }


    const WListManager& Context::wlist_manager() const {
        return this->wlm;
    }
}
}
