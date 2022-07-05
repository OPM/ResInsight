/*
  Copyright 2021 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/Action/WGNames.hpp>


namespace Opm {
namespace Action {

void WGNames::add_well(const std::string& wname) {
    this->wells.insert(wname);
}

void WGNames::add_group(const std::string& gname) {
    this->groups.insert(gname);
}

bool WGNames::has_well(const std::string& wname) const {
    return (this->wells.count(wname) == 1);
}

bool WGNames::has_group(const std::string& gname) const {
    return (this->groups.count(gname) == 1);
}

WGNames WGNames::serializeObject() {
    WGNames wgn;
    wgn.add_well("W1");
    wgn.add_group("G1");
    return wgn;
}

bool WGNames::operator==(const WGNames& other) const {
    return this->wells == other.wells && this->groups == other.groups;
}

}
}
