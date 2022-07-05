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

#include <algorithm>
#include <exception>
#include <optional>

#include <fmt/format.h>

#include <opm/output/eclipse/Inplace.hpp>

namespace Opm {

namespace {
static const std::string FIELD_NAME = std::string{"FIELD"};
static const std::size_t FIELD_ID   = 0;
}

void Inplace::add(const std::string& region, Inplace::Phase phase, std::size_t region_id, double value) {
    this->phase_values[region][phase][region_id] = value;
}

void Inplace::add(Inplace::Phase phase, double value) {
    this->add( FIELD_NAME, phase, FIELD_ID, value );
}

double Inplace::get(const std::string& region, Inplace::Phase phase, std::size_t region_id) const {
    auto region_iter = this->phase_values.find(region);
    if (region_iter == this->phase_values.end())
        throw std::logic_error(fmt::format("No such region: {}", region));

    auto phase_iter = region_iter->second.find(phase);
    if (phase_iter == region_iter->second.end())
        throw std::logic_error(fmt::format("No such phase: {}:{}", region, static_cast<int>(phase)));

    auto value_iter = phase_iter->second.find(region_id);
    if (value_iter == phase_iter->second.end())
        throw std::logic_error(fmt::format("No such region id: {}:{}:{}", region, static_cast<int>(phase), region_id));

    return value_iter->second;
}

double Inplace::get(Inplace::Phase phase) const {
    return this->get(FIELD_NAME, phase, FIELD_ID);
}

bool Inplace::has(const std::string& region, Phase phase, std::size_t region_id) const {
    auto region_iter = this->phase_values.find(region);
    if (region_iter == this->phase_values.end())
        return false;

    auto phase_iter = region_iter->second.find(phase);
    if (phase_iter == region_iter->second.end())
        return false;

    auto value_iter = phase_iter->second.find(region_id);
    if (value_iter == phase_iter->second.end())
        return false;

    return true;
}

bool Inplace::has(Phase phase) const {
    return this->has(FIELD_NAME, phase, FIELD_ID);
}

namespace {
std::size_t region_max(const std::unordered_map<std::size_t, double>& region_map) {
    std::size_t max_value = 0;
    for (const auto& [region_id, _] : region_map) {
        (void)_;
        max_value = std::max(max_value, region_id);
    }
    return max_value;
}
}

std::size_t Inplace::max_region() const {
    std::size_t max_value = 0;
    for (const auto& [_, phase_map] : this->phase_values) {
        (void)_;
        for (const auto& [__, region_map] : phase_map) {
            (void)__;
            max_value = std::max(max_value, region_max(region_map));
        }
    }

    return max_value;
}

std::size_t Inplace::max_region(const std::string& region_name) const {
    std::optional<std::size_t> max_value;
    const auto& region_iter = this->phase_values.find(region_name);
    if (region_iter != this->phase_values.end()) {
        max_value = 0;
        for (const auto& [_, region_map] : region_iter->second) {
            (void)_;
            max_value = std::max(*max_value, region_max(region_map));
        }
    }

    if (!max_value.has_value())
        throw std::logic_error(fmt::format("No such region: {}", region_name));

    return max_value.value();
}


// This should probably die - temporarily added for porting of ecloutputblackoilmodule
std::vector<double> Inplace::get_vector(const std::string& region, Phase phase) const {
    std::vector<double> v(this->max_region(region), 0);
    const auto& region_map = this->phase_values.at(region).at(phase);
    for (const auto& [region_id, value] : region_map)
        v[region_id - 1] = value;

    return v;
}


const std::vector<Inplace::Phase>& Inplace::phases() {
    static const std::vector<Phase> phases_ = {
        Inplace::Phase::WATER,
        Inplace::Phase::OIL,
        Inplace::Phase::GAS,
        Inplace::Phase::OilInLiquidPhase,
        Inplace::Phase::OilInGasPhase,
        Inplace::Phase::GasInLiquidPhase,
        Inplace::Phase::GasInGasPhase,
        Inplace::Phase::PoreVolume,
        Inplace::Phase::WaterResVolume,
        Inplace::Phase::OilResVolume,
        Inplace::Phase::GasResVolume,
        Inplace::Phase::SALT,
    };

    return phases_;
}


}
