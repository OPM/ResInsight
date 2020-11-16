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

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQContext.hpp>


namespace Opm {

    UDQContext::UDQContext(const UDQFunctionTable& udqft_arg, const SummaryState& summary_state_arg) :
        udqft(udqft_arg),
        summary_state(summary_state_arg)
    {
        for (const auto& pair : TimeMap::eclipseMonthIndices())
            this->add(pair.first, pair.second);

        /*
          Simulator performance keywords which are expected to be available for
          UDQ keywords; probably better to guarantee that they are present in
          the underlying summary state object.
        */

        this->add("ELAPSED", 0.0);
        this->add("MSUMLINS", 0.0);
        this->add("MSUMNEWT", 0.0);
        this->add("NEWTON", 0.0);
        this->add("TCPU", 0.0);
        this->add("TIME", 0.0);
        this->add("TIMESTEP", 0.0);
    }


    void UDQContext::add(const std::string& key, double value) {
        this->values[key] = value;
    }

    double UDQContext::get(const std::string& key) const {
        const auto& pair_ptr = this->values.find(key);
        if (pair_ptr == this->values.end())
            return this->summary_state.get(key);

        return pair_ptr->second;
    }

    double UDQContext::get_well_var(const std::string& well, const std::string& var) const {
        return this->summary_state.get_well_var(well, var);
    }

    bool UDQContext::has_well_var(const std::string& well, const std::string& var) const {
        return this->summary_state.has_well_var(well, var);
    }

    double UDQContext::get_group_var(const std::string& group, const std::string& var) const {
        return this->summary_state.get_group_var(group, var);
    }

    bool UDQContext::has_group_var(const std::string& group, const std::string& var) const {
        return this->summary_state.has_group_var(group, var);
    }

    std::vector<std::string> UDQContext::wells() const {
        return this->summary_state.wells();
    }

    std::vector<std::string> UDQContext::groups() const {
        return this->summary_state.groups();
    }

    const UDQFunctionTable& UDQContext::function_table() const {
        return this->udqft;
    }
}
