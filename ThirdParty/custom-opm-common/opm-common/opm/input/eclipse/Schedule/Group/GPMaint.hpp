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

#ifndef GPMAINT_HPP
#define GPMAINT_HPP

#include <optional>
#include <string>

namespace Opm {

class DeckRecord;

class GPMaint {
public:

enum class FlowTarget {
    RESV_PROD = 0,
    RESV_OINJ = 1,
    RESV_WINJ = 2,
    RESV_GINJ = 3,
    SURF_OINJ = 4,
    SURF_WINJ = 5,
    SURF_GINJ = 6,
};

class State {
friend class GPMaint;
    std::optional<std::size_t> report_step;
    double error_integral;
    double initial_rate;
};



    GPMaint() = default;
    GPMaint(std::size_t report_step, const DeckRecord& record);
    static GPMaint serializeObject();

    double pressure_target() const;
    double prop_constant() const;
    double time_constant() const;
    double rate(State& state, double current_rate, double error, double dt) const;
    std::optional<std::pair<std::string, int>> region() const;
    FlowTarget flow_target() const;
    bool operator==(const GPMaint& other) const;
    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_flow_target);
        serializer(m_region_number);
        serializer(m_region_name);
        serializer(m_pressure_target);
        serializer(m_prop_constant);
        serializer(m_time_constant);
        serializer(m_report_step);
    }

private:
    static FlowTarget FlowTargetFromString(const std::string& stringvalue);
    FlowTarget m_flow_target;
    int m_region_number;
    std::string m_region_name;
    double m_pressure_target;
    double m_prop_constant;
    double m_time_constant;
    std::size_t m_report_step;
};
}

#endif
