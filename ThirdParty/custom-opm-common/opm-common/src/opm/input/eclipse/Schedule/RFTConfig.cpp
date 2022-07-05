/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,

  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/Schedule/RFTConfig.hpp>

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>

namespace Opm {

std::string RFTConfig::RFT2String(RFT enumValue) {
    switch (enumValue) {
    case RFT::YES:
        return "YES";
    case RFT::REPT:
        return "REPT";
    case RFT::TIMESTEP:
        return "TIMESTEP";
    case RFT::FOPN:
        return "FOPN";
    case RFT::NO:
        return "NO";
    default:
        throw std::invalid_argument("unhandled enum value");
    }
}

RFTConfig::RFT RFTConfig::RFTFromString(const std::string& stringValue) {
    if (stringValue == "YES")
        return RFT::YES;
    else if (stringValue == "REPT")
        return RFT::REPT;
    else if (stringValue == "TIMESTEP")
        return RFT::TIMESTEP;
    else if (stringValue == "FOPN")
        return RFT::FOPN;
    else if (stringValue == "NO")
        return RFT::NO;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue);
}

std::string RFTConfig::PLT2String(PLT enumValue) {
    switch (enumValue) {
    case PLT::YES:
        return "YES";
    case PLT::REPT:
        return "REPT";
    case PLT::TIMESTEP:
        return "TIMESTEP";
    case PLT::NO:
        return "NO";
    default:
        throw std::invalid_argument("unhandled enum value");
    }
}

RFTConfig::PLT RFTConfig::PLTFromString( const std::string& stringValue ){
    if (stringValue == "YES")
        return PLT::YES;
    else if (stringValue == "REPT")
        return PLT::REPT;
    else if (stringValue == "TIMESTEP")
        return PLT::TIMESTEP;
    else if (stringValue == "NO")
        return PLT::NO;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue );
}

bool RFTConfig::operator==(const RFTConfig& data) const {
    return this->first_open_rft == data.first_open_rft &&
           this->rft_state == data.rft_state &&
           this->plt_state == data.plt_state &&
           this->open_wells == data.open_wells;
}


RFTConfig RFTConfig::serializeObject() {
    RFTConfig rft_config;
    rft_config.first_open( true );
    return rft_config;
}


std::optional<RFTConfig> RFTConfig::well_open(const std::string& wname) const {
    auto iter = this->open_wells.find(wname);

    if (iter == this->open_wells.end()) {
        if (this->first_open_rft) {
            auto new_rft = *this;
            new_rft.open_wells[wname] = true;
            new_rft.update(wname, RFT::YES);
            return new_rft;
        } else {
            auto new_rft = *this;
            auto rft_fopn = std::find_if( new_rft.rft_state.begin(), new_rft.rft_state.end(), [&wname](const auto& rft_pair) -> bool { return (rft_pair.second == RFT::FOPN && rft_pair.first == wname); });
            if (rft_fopn != new_rft.rft_state.end())
                rft_fopn->second = RFT::YES;

            new_rft.open_wells[wname] = true;
            return new_rft;
        }
    }

    return {};
}

void RFTConfig::first_open(bool on) {
    this->first_open_rft = on;
}

void RFTConfig::update(const std::string& wname, RFT mode) {
    if (mode == RFT::NO) {
        auto iter = this->rft_state.find(wname);
        if (iter != this->rft_state.end())
            this->rft_state.erase( iter );
        return;
    }
    if (mode == RFT::FOPN) {
        if (this->open_wells.count(wname) > 0) {
            this->open_wells[wname] = true;
            mode = RFT::YES;
        }
    }
    this->rft_state[wname] = mode;
}

bool RFTConfig::rft() const {
    for (const auto& [_, mode] : this->rft_state) {
        (void)_;
        if (mode != RFT::FOPN)
            return true;
    }
    return false;
}

bool RFTConfig::plt() const {
    return (this->plt_state.size() > 0);
}

void RFTConfig::update(const std::string& wname, PLT mode) {
    if (mode == PLT::NO) {
        auto iter = this->plt_state.find(wname);
        if (iter != this->plt_state.end())
            this->plt_state.erase( iter );
        return;
    }

    this->plt_state[wname] = mode;
}

bool RFTConfig::active() const {
    return this->rft() || this->plt();
}

bool RFTConfig::rft(const std::string& wname) const {
    auto well_iter = this->rft_state.find(wname);
    if (well_iter == this->rft_state.end())
        return false;
    auto mode = well_iter->second;
    if (mode == RFT::FOPN)
        return false;
    return true;
}

bool RFTConfig::plt(const std::string& wname) const {
    auto well_iter = this->plt_state.find(wname);
    if (well_iter == this->plt_state.end())
        return false;
    return true;
}

std::optional<RFTConfig> RFTConfig::next() const {
    auto yes_iter_rft = std::find_if( this->rft_state.begin(), this->rft_state.end(), [](const auto& rft_pair) { return rft_pair.second == RFT::YES; });
    auto yes_iter_plt = std::find_if( this->plt_state.begin(), this->plt_state.end(), [](const auto& plt_pair) { return plt_pair.second == PLT::YES; });
    if (yes_iter_rft == this->rft_state.end() && yes_iter_plt == this->plt_state.end())
        return {};

    auto new_rft = *this;
    for (auto iter = new_rft.rft_state.begin(); iter != new_rft.rft_state.end(); ) {
        if (iter->second == RFT::YES)
            iter = new_rft.rft_state.erase(iter);
        else
            iter++;
    }

    for (auto iter = new_rft.plt_state.begin(); iter != new_rft.plt_state.end(); ) {
        if (iter->second == PLT::YES)
            iter = new_rft.plt_state.erase(iter);
        else
            iter++;
    }

    return new_rft;
}

}
