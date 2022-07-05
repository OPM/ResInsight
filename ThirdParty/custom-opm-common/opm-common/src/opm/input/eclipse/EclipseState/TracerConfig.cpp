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

#include <fmt/format.h>
#include <algorithm>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/InfoLogger.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/EclipseState/TracerConfig.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TracerVdTable.hpp>

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

const TracerConfig::TracerEntry& TracerConfig::operator[](std::size_t index) const {
    return this->tracers.at(index);
}

const TracerConfig::TracerEntry& TracerConfig::operator[](const std::string& name) const {
    auto iter = std::find_if(this->tracers.begin(), this->tracers.end(), [&name](const TracerEntry& tracer) {
            return tracer.name == name;
        });

    if (iter == this->tracers.end())
        throw std::logic_error(fmt::format("No such tracer: {}", name));

    return *iter;
}


TracerConfig::TracerConfig(const UnitSystem& unit_system, const Deck& deck)
{
    using TR = ParserKeywords::TRACER;
    if (deck.hasKeyword<TR>()) {
        const auto& keyword = deck.get<TR>().back();
        OpmLog::info( keyword.location().format("\nInitializing tracers from {keyword} in {file} line {line}") );
        InfoLogger logger("Tracer tables", 3);
        for (const auto& record : keyword) {
            const auto& name = record.getItem<TR::NAME>().get<std::string>(0);
            Phase phase = phase_from_string(record.getItem<TR::FLUID>().get<std::string>(0));
            double inv_volume = 1.0; // Default scaling (vol/vol).
            std::string unit_string = "";
            if (!record.getItem<TR::UNIT>().defaultApplied(0)) {
                unit_string = record.getItem<TR::UNIT>().get<std::string>(0);
                logger(keyword.location().format("Non-default tracer unit [" + unit_string + "] from {keyword} in {file} line {line}"));
                // Non-default tracer units can be "anything".  For now we just keep it as a "tag", and make no
                // attempts to relate it to physical quantities recognized by the simulator.
                //   We also leave the denominator of the concentration fraction as it is, and do not convert it to
                // simulator-internal volume. Thus concentrations will be _wrong_ in simulator units but _correct_
                // in io-units during computations. Since the passive tracers curently considered have no physical impact,
                // this should be ok. TODO: However, for tracers where correct quantity matters (e.g. chemical reactions)
                // proper scaling also for non-default tracer units will be needed.

                // Correct unit names for non-default tracer units are generated in the method 'get_unit_string' below,
                // and is output by "hijacking" the normal unit-name generating procedure.

                // Towards "proper" scaling:
                //if (phase == Phase::GAS)
                //    inv_volume = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
                //else
                //    inv_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();
                unit_system.getDimension(UnitSystem::measure::liquid_surface_volume); //hush unused-warning ...

                // Convert unit names to upper-case
                std::transform(unit_string.begin(), unit_string.end(), unit_string.begin(),
                    [](unsigned char c){ return std::toupper(c); });
            }

            std::string tracer_field = "TBLKF" + name;
            if (deck.hasKeyword(tracer_field)) {
                const auto& tracer_keyword = deck[tracer_field].back();
                auto free_concentration = tracer_keyword.getRecord(0).getItem(0).getData<double>();
                logger(tracer_keyword.location().format("Loading tracer concentration from {keyword} in {file} line {line}"));

                for (auto& c : free_concentration)
                    c *= inv_volume;

                std::string tracer_field_solution = "TBLKS" + name;
                if (deck.hasKeyword(tracer_field_solution)) {
                    const auto& tracer_keyword_solution = deck[tracer_field_solution].back();
                    auto solution_concentration = tracer_keyword_solution.getRecord(0).getItem(0).getData<double>();
                    logger(tracer_keyword_solution.location().format("Loading tracer concentration from {keyword} in {file} line {line}"));

                    for (auto& c : solution_concentration)
                        c *= inv_volume;

                    this->tracers.emplace_back(name, unit_string, phase, std::move(free_concentration), std::move(solution_concentration)) ;
                    continue;
                }

                this->tracers.emplace_back(name, unit_string, phase, std::move(free_concentration)) ;
                continue;
            }

            std::string tracer_table = "TVDPF" + name;
            if (deck.hasKeyword(tracer_table)) {
                const auto& tracer_keyword = deck[tracer_table].back();
                const auto& deck_item = tracer_keyword.getRecord(0).getItem(0);
                logger(tracer_keyword.location().format("Loading tracer concentration from {keyword} in {file} line {line}"));

                std::string tracer_table_solution = "TVDPS" + name;
                if (deck.hasKeyword(tracer_table_solution)) {
                    const auto& tracer_keyword_solution = deck[tracer_table_solution].back();
                    const auto& deck_item_solution = tracer_keyword_solution.getRecord(0).getItem(0);
                    logger(tracer_keyword_solution.location().format("Loading tracer concentration from {keyword} in {file} line {line}"));

                    this->tracers.emplace_back(name, unit_string, phase,
                                               TracerVdTable(deck_item, inv_volume, this->tracers.size()),
                                               TracerVdTable(deck_item_solution, inv_volume, this->tracers.size())) ;
                    continue;
                }

                this->tracers.emplace_back(name, unit_string, phase, TracerVdTable(deck_item, inv_volume, this->tracers.size()));
                continue;
            }

            this->tracers.emplace_back(name, unit_string, phase);
        }
    }
}

TracerConfig TracerConfig::serializeObject()
{
    TracerConfig result;
    result.tracers = {{"test", "", Phase::OIL, {1.0}}};

    return result;
}

size_t TracerConfig::size() const {
    return this->tracers.size();
}

bool TracerConfig::empty() const {
    return this->tracers.empty();
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

std::string TracerConfig::get_unit_string(const UnitSystem& unit_system, const std::string & tracer_kw) const {
    if (tracer_kw.length() > 4 ) {
        std::string tracer_name = tracer_kw.substr(4);
        for (const auto& tracer : tracers) {
            if (tracer.name == tracer_name) {
                std::string unit_string(tracer.unit_string);
                if (tracer.unit_string != "") {
                    if (tracer_kw[3] == 'R') {
                       std::string rateName = unit_system.name(Opm::UnitSystem::measure::rate);
                       std::size_t found = rateName.find('/');
                       unit_string += rateName.substr(found);
                    }
                    else if (tracer_kw[3] == 'T') {
                    }
                    else if (tracer_kw[3] == 'C') {
                        unit_string += "/";
                        if (tracer.phase == Phase::GAS )
                            unit_string += unit_system.name(Opm::UnitSystem::measure::gas_surface_volume);
                        else /* OIL or WAT */
                            unit_string += unit_system.name(Opm::UnitSystem::measure::liquid_surface_volume);
                    }
                    else {
                        throw std::runtime_error("Tracer summary kw not recognized: " + tracer_kw);
                    }
                }
                return unit_string;
            }
        }
    }
    return std::string("");
}

}
