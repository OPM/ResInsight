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
#include <fmt/format.h>

#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQContext.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/common/utility/TimeService.hpp>


namespace Opm {

namespace {

bool is_udq(const std::string& key) {
    if (key.size() < 2)
        return false;

    if (key[1] != 'U')
        return false;

    return true;
}

}


    UDQContext::UDQContext(const UDQFunctionTable& udqft_arg, const WellMatcher& wm, SummaryState& summary_state_arg, UDQState& udq_state_arg) :
        udqft(udqft_arg),
        well_matcher(wm),
        summary_state(summary_state_arg),
        udq_state(udq_state_arg)
    {
        for (const auto& pair : TimeService::eclipseMonthIndices())
            this->add(pair.first, pair.second);

        /*
          Simulator performance keywords which are expected to be available for
          UDQ keywords; probably better to guarantee that they are present in
          the underlying summary state object.
        */

        this->add("MSUMLINS", 0.0);
        this->add("MSUMNEWT", 0.0);
        this->add("NEWTON", 0.0);
        this->add("TCPU", 0.0);
    }


    void UDQContext::add(const std::string& key, double value) {
        this->values[key] = value;
    }

    std::optional<double> UDQContext::get(const std::string& key) const {
        if (is_udq(key)) {
            if (this->udq_state.has(key))
                return this->udq_state.get(key);

            return std::nullopt;
        }

        const auto& pair_ptr = this->values.find(key);
        if (pair_ptr != this->values.end())
            return pair_ptr->second;

        return this->summary_state.get(key);
    }

    std::optional<double> UDQContext::get_well_var(const std::string& well, const std::string& var) const {
        if (is_udq(var)) {
            if (this->udq_state.has_well_var(well, var))
                return this->udq_state.get_well_var(well, var);

            return std::nullopt;
        }
        if (this->summary_state.has_well_var(var)) {
            if (this->summary_state.has_well_var(well, var))
                return this->summary_state.get_well_var(well, var);
            else
                return std::nullopt;
        }
        throw std::logic_error(fmt::format("Summary well variable: {} not registered", var));
    }

    std::optional<double> UDQContext::get_group_var(const std::string& group, const std::string& var) const {
        if (is_udq(var)) {
            if (this->udq_state.has_group_var(group, var))
                return this->udq_state.get_group_var(group, var);

            return std::nullopt;
        }

        if (this->summary_state.has_group_var(var)) {
            if (this->summary_state.has_group_var(group, var))
                return this->summary_state.get_group_var(group, var);
            else
                return std::nullopt;
        }
        throw std::logic_error(fmt::format("Summary group variable: {} not registered", var));
    }

    std::vector<std::string> UDQContext::wells() const {
        return this->well_matcher.wells();
    }

    std::vector<std::string> UDQContext::wells(const std::string& pattern) const {
        return this->well_matcher.wells(pattern);
    }

    std::vector<std::string> UDQContext::groups() const {
        return this->summary_state.groups();
    }

    const UDQFunctionTable& UDQContext::function_table() const {
        return this->udqft;
    }

    void UDQContext::update_assign(std::size_t report_step, const std::string& keyword, const UDQSet& udq_result) {
        this->udq_state.add_assign(report_step, keyword, udq_result);
        this->summary_state.update_udq(udq_result, this->udq_state.undefined_value());
    }

    void UDQContext::update_define(std::size_t report_step, const std::string& keyword, const UDQSet& udq_result) {
        this->udq_state.add_define(report_step, keyword, udq_result);
        this->summary_state.update_udq(udq_result, this->udq_state.undefined_value());
    }
}
