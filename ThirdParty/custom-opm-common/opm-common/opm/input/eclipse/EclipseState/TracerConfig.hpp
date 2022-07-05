/*
  Copyright (C) 2020 Equinor

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

#ifndef OPM_TRACER_CONFIG_HPP
#define OPM_TRACER_CONFIG_HPP

#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TracerVdTable.hpp>

namespace Opm {

class Deck;
class UnitSystem;

class TracerConfig {
public:
    struct TracerEntry {
        std::string name;
        std::string unit_string;
        Phase phase = Phase::OIL;
        std::optional<std::vector<double>> free_concentration;
        std::optional<std::vector<double>> solution_concentration;
        std::optional<TracerVdTable> free_tvdp;
        std::optional<TracerVdTable> solution_tvdp;
        std::string fname() const {
            return this->name + "F";
        }


        TracerEntry() = default;
        TracerEntry(const std::string& name_, const std::string& unit_string_,
                    Phase phase_, std::vector<double> free_concentration_)
            : name(name_)
            , unit_string(unit_string_)
            , phase(phase_)
            , free_concentration(std::move(free_concentration_))
        {}

        TracerEntry(const std::string& name_, const std::string& unit_string_,
                    Phase phase_, std::vector<double> free_concentration_, std::vector<double> solution_concentration_)
            : name(name_)
            , unit_string(unit_string_)
            , phase(phase_)
            , free_concentration(std::move(free_concentration_))
            , solution_concentration(std::move(solution_concentration_))
        {}

        TracerEntry(const std::string& name_, const std::string& unit_string_,
                    Phase phase_, TracerVdTable free_tvdp_)
            : name(name_)
            , unit_string(unit_string_)
            , phase(phase_)
            , free_tvdp(std::move(free_tvdp_))
        {}

        TracerEntry(const std::string& name_, const std::string& unit_string_,
                    Phase phase_, TracerVdTable free_tvdp_, TracerVdTable solution_tvdp_)
            : name(name_)
            , unit_string(unit_string_)
            , phase(phase_)
            , free_tvdp(std::move(free_tvdp_))
            , solution_tvdp(std::move(solution_tvdp_))
        {}

        TracerEntry(const std::string& name_, const std::string& unit_string_, Phase phase_)
            : name(name_)
            , unit_string(unit_string_)
            , phase(phase_)
        {}

        bool operator==(const TracerEntry& data) const {
            return this->name == data.name &&
                   this->unit_string == data.unit_string &&
                   this->phase == data.phase &&
                   this->free_concentration == data.free_concentration &&
                   this->solution_concentration == data.solution_concentration &&
                   this->free_tvdp == data.free_tvdp &&
                   this->solution_tvdp == data.solution_tvdp;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(name);
            serializer(unit_string);
            serializer(phase);
            serializer(free_concentration);
            serializer(solution_concentration);
            serializer(this->free_tvdp);
            serializer(this->solution_tvdp);
        }
    };

    TracerConfig() = default;
    TracerConfig(const UnitSystem& unit_system, const Deck& deck);

    static TracerConfig serializeObject();

    size_t size() const;
    bool empty() const;

    const std::vector<TracerEntry>::const_iterator begin() const;
    const std::vector<TracerEntry>::const_iterator end() const;
    const TracerEntry& operator[](const std::string& name) const;
    const TracerEntry& operator[](std::size_t index) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(tracers);
    }

    bool operator==(const TracerConfig& data) const;

    std::string get_unit_string(const UnitSystem& unit_system, const std::string & tracer_kw) const;

private:
    std::vector<TracerEntry> tracers;
};

}

#endif
