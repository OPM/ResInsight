/*
  Copyright 2018 Statoil ASA.

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
#include <stdexcept>
#include <algorithm>
#include <opm/io/eclipse/rst/state.hpp>

#include <opm/input/eclipse/Schedule/Well/WellTestConfig.hpp>

namespace Opm {

WellTestConfig::WTESTWell::WTESTWell(const std::string& name_arg, int shut_reason_arg, double test_interval_arg, int num_test_arg, double startup_time_arg, int begin_report_step_arg) :
    name(name_arg),
    reasons(shut_reason_arg),
    test_interval(test_interval_arg),
    num_test(num_test_arg),
    startup_time(startup_time_arg),
    begin_report_step(begin_report_step_arg)
{}

WellTestConfig::WTESTWell WellTestConfig::WTESTWell::serializeObject() {
    return WellTestConfig::WTESTWell("name", static_cast<int>(Reason::PHYSICAL), 100, 1, 674, 56);
}

bool WellTestConfig::WTESTWell::test_well(int num_attempt, double elapsed) const {
    if (num_attempt >= this->num_test && this->num_test != 0)
        return false;

    if (elapsed < this->test_interval)
        return false;

    return true;
}


int WellTestConfig::WTESTWell::ecl_reasons() const
{
    int ecl_value = 1;

    if (this->reasons & static_cast<int>(Reason::PHYSICAL))
        ecl_value *= WTest::EclConfigReason::PHYSICAL;

    if (this->reasons & static_cast<int>(Reason::ECONOMIC))
        ecl_value *= WTest::EclConfigReason::ECONOMIC;

    if (this->reasons & static_cast<int>(Reason::GROUP))
        ecl_value *= WTest::EclConfigReason::GCON;

    if (this->reasons & static_cast<int>(Reason::THP_DESIGN))
        ecl_value *= WTest::EclConfigReason::THPLimit;

    if (this->reasons & static_cast<int>(Reason::COMPLETION))
        ecl_value *= WTest::EclConfigReason::CONNECTION;

    return ecl_value;
}

namespace {

void update(int& opm_reasons, const WellTestConfig::Reason opm_reason, const int ecl_reasons, const int ecl_reason) {
    if (ecl_reasons % ecl_reason == 0)
        opm_reasons += static_cast<int>(opm_reason);
}

}

int WellTestConfig::WTESTWell::inverse_ecl_reasons(int ecl_reasons) {
    int reasons{0};

    update(reasons, Reason::PHYSICAL  , ecl_reasons, WTest::EclConfigReason::PHYSICAL);
    update(reasons, Reason::ECONOMIC  , ecl_reasons, WTest::EclConfigReason::ECONOMIC);
    update(reasons, Reason::GROUP     , ecl_reasons, WTest::EclConfigReason::GCON);
    update(reasons, Reason::THP_DESIGN, ecl_reasons, WTest::EclConfigReason::THPLimit);
    update(reasons, Reason::COMPLETION, ecl_reasons, WTest::EclConfigReason::CONNECTION);

    return reasons;
}



WellTestConfig WellTestConfig::serializeObject()
{
    WellTestConfig result;
    result.wells.emplace(  "W1", WellTestConfig::WTESTWell::serializeObject() );
    return result;
}


void WellTestConfig::add_well(const std::string& well, int reasons, double test_interval,
                              int num_retries, double startup_time, const int current_step) {
    this->wells.insert_or_assign(well, WTESTWell(well, reasons, test_interval, num_retries, startup_time, current_step));
}

void WellTestConfig::add_well(const std::string& well, const std::string& reasons_string, double test_interval,
                              int num_retries, double startup_time, const int current_step) {
    if (reasons_string.empty())
        throw std::invalid_argument("Can not pass empty string to stop testing to add_well() method.");

    int reasons{0};

    for (auto c : reasons_string) {
        switch(c) {
        case 'P' :
            reasons += static_cast<int>(Reason::PHYSICAL);
            break;
        case 'E' :
            reasons += static_cast<int>(Reason::ECONOMIC);
            break;
        case 'G':
            reasons += static_cast<int>(Reason::GROUP);
            break;
        case 'D':
            reasons += static_cast<int>(Reason::THP_DESIGN);
            break;
        case 'C':
            reasons += static_cast<int>(Reason::COMPLETION);
            break;
        default:
            throw std::invalid_argument("Invalid character in WTEST configuration");
        }
    }
    this->add_well(well, reasons, test_interval, num_retries, startup_time, current_step);
}


void WellTestConfig::drop_well(const std::string& well) {
    this->wells.erase(well);
}

bool WellTestConfig::has(const std::string& well) const {
    const auto well_iter = this->wells.find(well);
    return (well_iter != wells.end());
}


bool WellTestConfig::has(const std::string& well, Reason reason) const {
    const auto well_iter = this->wells.find(well);
    if (well_iter == wells.end())
        return false;

    return ((well_iter->second.reasons & static_cast<int>(reason)) != 0);
}


const WellTestConfig::WTESTWell& WellTestConfig::get(const std::string& well) const {
    return this->wells.at(well);
}



std::string WellTestConfig::reasonToString(const Reason reason) {
    switch(reason) {
    case Reason::PHYSICAL:
        return std::string("PHYSICAL");
    case Reason::ECONOMIC:
        return std::string("ECONOMIC");
    case Reason::GROUP:
        return std::string("GROUP");
    case Reason::THP_DESIGN:
        return std::string("THP_DESIGN");
    case Reason::COMPLETION:
        return std::string("COMPLETION");
    default:
        throw std::runtime_error("unknown closure reason");
    }
}


bool WellTestConfig::empty() const {
    return this->wells.empty();
}


bool WellTestConfig::operator==(const WellTestConfig& data) const {
    return this->wells == data.wells;
}


WellTestConfig::WellTestConfig(const RestartIO::RstState& rst_state, int report_step) {
    for (const auto& well : rst_state.wells) {
        if (well.wtest_config_reasons != 0) {
            this->add_well(well.name,
                           WellTestConfig::WTESTWell::inverse_ecl_reasons(well.wtest_config_reasons),
                           well.wtest_interval,
                           well.wtest_remaining,
                           well.wtest_startup,
                           report_step);
        }
    }
}

}

