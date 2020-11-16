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

#include <opm/parser/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/parser/eclipse/EclipseState/TracerConfig.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TracerVdTable.hpp>

namespace Opm {

namespace {

Phase phase_from_string(const std::string& phase_string) {
    if (phase_string == "WAT")
        return Phase::WATER;

    if (phase_string == "OIL")
        return Phase::OIL;

    if (phase_string == "GAS")
        return Phase::GAS;

    throw std::invalid_argument("Tracer: invalid fluid name " + phase_string);
}

}

TracerConfig::TracerConfig(const UnitSystem& unit_system, const Deck& deck)
{
    using TR = ParserKeywords::TRACER;
    if (deck.hasKeyword<TR>()) {
        const auto& keyword = deck.getKeyword<TR>();
        for (const auto& record : keyword) {
            const auto& name = record.getItem<TR::NAME>().get<std::string>(0);
            Phase phase = phase_from_string(record.getItem<TR::FLUID>().get<std::string>(0));
            double inv_volume;

            if (phase == Phase::GAS)
                inv_volume = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
            else
                inv_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();

            std::string tracer_field = "TBLKF" + name;
            if (deck.hasKeyword(tracer_field)) {
                auto concentration = deck.getKeyword(tracer_field).getRecord(0).getItem(0).getData<double>();
                for (auto& c : concentration)
                    c *= inv_volume;

                this->tracers.emplace_back(name, phase, std::move(concentration)) ;
                continue;
            }

            std::string tracer_table = "TVDPF" + name;
            if (deck.hasKeyword(tracer_table)) {
                const auto& deck_item = deck.getKeyword(tracer_table).getRecord(0).getItem(0);
                this->tracers.emplace_back(name, phase, TracerVdTable(deck_item, inv_volume));
                continue;
            }

            throw std::runtime_error("Uninitialized tracer concentration for tracer " + name);
        }
    }
}

TracerConfig TracerConfig::serializeObject()
{
    TracerConfig result;
    result.tracers = {{"test", Phase::OIL, {1.0}}};

    return result;
}

size_t TracerConfig::size() const {
    return this->tracers.size();
}

const std::vector<TracerConfig::TracerEntry>::const_iterator TracerConfig::begin() const {
    return this->tracers.begin();
}

const std::vector<TracerConfig::TracerEntry>::const_iterator TracerConfig::end() const {
    return this->tracers.end();
}

bool TracerConfig::operator==(const TracerConfig& other) const {
    return this->tracers == other.tracers;
}

}
