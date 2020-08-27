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

#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellTestConfig.hpp>

namespace Opm {


WellTestConfig::WellTestConfig() {

}


WellTestConfig WellTestConfig::serializeObject()
{
    WellTestConfig result;
    result.wells = {{"test", ECONOMIC, 1.0, 2, 3.0, 4}};

    return result;
}


void WellTestConfig::add_well(const std::string& well, Reason shut_reason, double test_interval,
                              int num_retries, double startup_time, const int current_step) {

    WTESTWell* well_ptr = getWell(well, shut_reason);

    if (well_ptr) {
        *well_ptr = WTESTWell{well, shut_reason, test_interval, num_retries, startup_time, current_step};
    } else {
        wells.push_back({well, shut_reason, test_interval, num_retries, startup_time, current_step});
    }
}


void WellTestConfig::add_well(const std::string& well, const std::string& reasons, double test_interval,
                              int num_retries, double startup_time, const int current_step) {
    if (reasons.empty())
        throw std::invalid_argument("Can not pass empty string to stop testing to add_well() method.");

    for (auto c : reasons) {
        switch(c) {
        case 'P' :
            add_well(well, Reason::PHYSICAL, test_interval, num_retries, startup_time, current_step);
            break;
         case 'E' :
            add_well(well, Reason::ECONOMIC, test_interval, num_retries, startup_time, current_step);
            break;
        case 'G':
            add_well(well, Reason::GROUP, test_interval, num_retries, startup_time, current_step);
            break;
        case 'D':
            add_well(well, Reason::THP_DESIGN, test_interval, num_retries, startup_time, current_step);
            break;
        case 'C':
            add_well(well, Reason::COMPLETION, test_interval, num_retries, startup_time, current_step);
            break;
        default:
            throw std::invalid_argument("Invalid character in WTEST configuration");
        }
    }
}


void WellTestConfig::drop_well(const std::string& well) {
    wells.erase(std::remove_if(wells.begin(),
                               wells.end(),
                               [&well](const WTESTWell& wtest_well) { return (wtest_well.name == well); }),
                wells.end());
}

bool WellTestConfig::has(const std::string& well) const {
    const auto well_iter = std::find_if(wells.begin(),
                                        wells.end(),
                                        [&well](const WTESTWell& wtest_well) { return (wtest_well.name == well); });
    return (well_iter != wells.end());
}


bool WellTestConfig::has(const std::string& well, Reason reason) const {
    const auto well_iter = std::find_if(wells.begin(),
                                        wells.end(),
                                        [&well, &reason](const WTESTWell& wtest_well)
                                        {
                                            return (reason == wtest_well.shut_reason && wtest_well.name == well);
                                        });
    return (well_iter != wells.end());
}


const WellTestConfig::WTESTWell& WellTestConfig::get(const std::string& well, Reason reason) const {
    const auto well_iter = std::find_if(wells.begin(),
                                        wells.end(),
                                        [&well, &reason](const WTESTWell& wtest_well)
                                        {
                                            return (reason == wtest_well.shut_reason && wtest_well.name == well);
                                        });
    if (well_iter == wells.end())
        throw std::invalid_argument("No such WTEST object");

    return *well_iter;
}



std::string WellTestConfig::reasonToString(const Reason reason) {
    switch(reason) {
    case PHYSICAL:
        return std::string("PHYSICAL");
    case ECONOMIC:
        return std::string("ECONOMIC");
    case GROUP:
        return std::string("GROUP");
    case THP_DESIGN:
        return std::string("THP_DESIGN");
    case COMPLETION:
        return std::string("COMPLETION");
    default:
        throw std::runtime_error("unknown closure reason");
    }
}



WellTestConfig::WTESTWell*  WellTestConfig::getWell(const std::string& well_name, const Reason reason) {
    const auto well_iter = std::find_if(wells.begin(), wells.end(), [&well_name, &reason](const WTESTWell& well) {
        return (reason == well.shut_reason && well.name == well_name);
    });

    return (well_iter == wells.end() ? nullptr : std::addressof(*well_iter) );
}



size_t WellTestConfig::size() const {
    return wells.size();
}


bool WellTestConfig::operator==(const WellTestConfig& data) const {
    return this->wells == data.wells;
}



}


