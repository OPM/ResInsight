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

#include <opm/parser/eclipse/EclipseState/Schedule/RFTConfig.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

namespace Opm {

RFTConfig::RFTConfig()
    : tm{}
    , first_rft_event(tm.size())
    , well_open_rft_time{false, 0}
{
}

RFTConfig::RFTConfig(const TimeMap& time_map) :
    tm(time_map),
    first_rft_event(tm.size()),
    well_open_rft_time{false, 0}
{
}

RFTConfig RFTConfig::serializeObject()
{
    RFTConfig result;
    result.tm = TimeMap::serializeObject();
    result.first_rft_event = 1;
    result.well_open_rft_time = {true, 2};
    result.well_open_rft_name = {{"test1", 3}};
    result.well_open = {{"test2", 4}};
    result.rft_config = {{"test3", {{{RFT::TIMESTEP, 5}}, 6}}};
    result.plt_config = {{"test3", {{{PLT::REPT, 7}}, 8}}};

    return result;
}

bool RFTConfig::rft(const std::string& well_name, std::size_t report_step) const {
    if (report_step >= this->tm.size())
        throw std::invalid_argument("Invalid report step " + std::to_string(report_step));

    if (this->outputRftAtWellopen(this->well_open.find(well_name), report_step))
        return true;

    auto cfg = this->rft_config.find(well_name);
    if (cfg == this->rft_config.end())
        return false;

    const auto& rft_pair = cfg->second[report_step];
    if (rft_pair.first == RFT::YES)
        return (rft_pair.second == report_step);

    if (rft_pair.first == RFT::NO)
        return false;

    if (rft_pair.first == RFT::REPT)
        return true;

    if (rft_pair.first == RFT::TIMESTEP)
        return true;

    return false;
}

bool RFTConfig::plt(const std::string& well_name, std::size_t report_step) const {
    if (report_step >= this->tm.size())
        throw std::invalid_argument("Invalid ");

    auto cfg = this->plt_config.find(well_name);
    if (cfg == this->plt_config.end())
        return false;

    const auto& plt_pair = cfg->second[report_step];
    if (plt_pair.first == PLT::YES)
        return (plt_pair.second == report_step);

    if (plt_pair.first == PLT::NO)
        return false;

    if (plt_pair.first == PLT::REPT)
        return true;

    if (plt_pair.first == PLT::TIMESTEP)
        return true;

    return false;
}

template <typename Value>
void RFTConfig::updateConfig(const std::string& well_name,
                             const std::size_t  report_step,
                             const Value        value,
                             ConfigMap<Value>&  cfgmap)
{
    auto cfg = cfgmap.find(well_name);
    if (cfg == cfgmap.end()) {
        auto state = DynamicState<std::pair<Value, std::size_t>> {
            this->tm, std::make_pair(Value::NO, 0)
        };

        auto stat = cfgmap.emplace(well_name, std::move(state));
        if (! stat.second)
            return;

        cfg = stat.first;
    }

    cfg->second.update(report_step, std::make_pair(value, report_step));
}

void RFTConfig::updateRFT(const std::string& well_name, std::size_t report_step, RFT value) {
    if (value == RFT::FOPN) {
        this->setWellOpenRFT(well_name, report_step);

        auto wo = this->well_open.find(well_name);
        if (wo != this->well_open.end())
            // New candidate first RFT event time is 'report_step' if
            // well is already open, well's open event time otherwise.
            this->updateFirst(std::max(report_step, wo->second));
    } else {
        this->updateConfig(well_name, report_step, value, this->rft_config);

        if (value != RFT::NO)
            // YES, REPT, or TIMESTEP.
            this->updateFirstIfNotShut(well_name, report_step);
    }
}

void RFTConfig::updatePLT(const std::string& well_name, std::size_t report_step, PLT value) {
    this->updateConfig(well_name, report_step, value, this->plt_config);

    if (value != PLT::NO)
        // YES, REPT, or TIMESTEP.
        this->updateFirstIfNotShut(well_name, report_step);
}


bool RFTConfig::getWellOpenRFT(const std::string& well_name, std::size_t report_step) const {
    if (this->well_open_rft_name.count(well_name) > 0)
        return true;

    return (this->well_open_rft_time.first && this->well_open_rft_time.second <= report_step);
}


void RFTConfig::setWellOpenRFT(std::size_t report_step) {
    this->well_open_rft_time.second = this->well_open_rft_time.first
        ? std::min(this->well_open_rft_time.second, report_step)
        : report_step;

    this->well_open_rft_time.first = true;

    this->updateFirst(this->firstWellopenStepNotBefore(report_step));
}

void RFTConfig::setWellOpenRFT(const std::string& well_name) {
    this->setWellOpenRFT(well_name, std::size_t(0));
}


void RFTConfig::addWellOpen(const std::string& well_name, std::size_t report_step) {
    // Implicitly handles 'well_name' already being in the map.
    this->well_open.emplace(well_name, report_step);
}

bool RFTConfig::active(std::size_t report_step) const {
    for (auto well = this->well_open.begin(), end = this->well_open.end(); well != end; ++well) {
        if (this->outputRftAtWellopen(well, report_step))
            return true;
    }

    for (const auto& rft_pair : this->rft_config) {
        if (this->rft(rft_pair.first, report_step))
            return true;
    }

    for (const auto& plt_pair : this->plt_config) {
        if (this->rft(plt_pair.first, report_step))
            return true;
    }

    return false;
}

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

const TimeMap& RFTConfig::timeMap() const {
    return tm;
}

bool RFTConfig::operator==(const RFTConfig& data) const {
    return this->tm == data.tm &&
           this->first_rft_event == data.first_rft_event &&
           this->well_open_rft_time == data.well_open_rft_time &&
           this->well_open_rft_name == data.well_open_rft_name &&
           this->well_open == data.well_open &&
           this->rft_config == data.rft_config &&
           this->plt_config == data.plt_config;
}

bool RFTConfig::outputRftAtWellopen(WellOpenTimeMap::const_iterator well_iter, const std::size_t report_step) const {
    assert ((report_step < this->tm.size()) && "Internal logic error for report step");

    if (well_iter == this->well_open.end())
        return false;

    if (report_step < this->first_rft_event)
        return false;

    if (this->well_open_rft_time.first && this->well_open_rft_time.second <= report_step) {
        // A general "Output RFT when the well is opened" has been
        // configured with WRFT.  Output RFT data if the well opens
        // at this report step.
        if (well_iter->second == report_step)
            return true;
    }

    auto rft_open = this->well_open_rft_name.find(well_iter->first);
    if (rft_open == this->well_open_rft_name.end())
        // No FOPN event configured for this well (i.e., well_iter->first).
        return false;

    // An FOPN setting has been configured with the WRFTPLT keyword.
    // Output RFT data if we're at the FOPN event time and the well is
    // already open or if we're after FOPN event time and the well
    // opens at this step.
    return ((report_step == rft_open->second) && (well_iter->second <= report_step))
        || ((report_step >  rft_open->second) && (well_iter->second == report_step));
}

std::size_t RFTConfig::firstWellopenStepNotBefore(const std::size_t report_step) const {
    if (well_open.empty())
        // No well-open events at all (unexpected).
        return this->tm.size();

    using VT = WellOpenTimeMap::value_type;

    auto event = std::min_element(this->well_open.begin(), this->well_open.end(),
        [report_step](const VT& elem, const VT& low) -> bool
    {
        if (elem.second < report_step)
            return false;

        return elem.second < low.second;
    });

    if (event->second < report_step)
        // All well-open events happen *before* 'report_step'.
        return this->tm.size();

    return event->second;
}

void RFTConfig::updateFirstIfNotShut(const std::string& well_name, const std::size_t report_step) {
    auto wo = this->well_open.find(well_name);

    if ((wo != this->well_open.end()) && (wo->second <= report_step))
        // Well opens no later than 'report_step'.  New candidate
        // first RFT output event is 'report_step'.
        this->updateFirst(report_step);
}

void RFTConfig::updateFirst(const std::size_t report_step) {
    this->first_rft_event = std::min(this->first_rft_event, report_step);
}

void RFTConfig::setWellOpenRFT(const std::string& well_name, const std::size_t report_step)
{
    auto pos = this->well_open_rft_name.find(well_name);
    if (pos == this->well_open_rft_name.end()) {
        auto stat = this->well_open_rft_name.emplace(well_name, this->tm.size());
        if (! stat.second)
            return;

        pos = stat.first;
    }

    pos->second = std::min(pos->second, report_step);
}
}
