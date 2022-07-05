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

#ifndef WGNAMES_HPP
#define WGNAMES_HPP

#include <string>
#include <unordered_set>

namespace Opm {
namespace Action {

class WGNames {
public:
    WGNames() = default;
    void add_well(const std::string& wname);
    void add_group(const std::string& gname);

    bool has_well(const std::string& wname) const;
    bool has_group(const std::string& gname) const;
    static WGNames serializeObject();
    bool operator==(const WGNames& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->wells);
        serializer(this->groups);
    }

private:
    std::unordered_set<std::string> wells;
    std::unordered_set<std::string> groups;
};

}
}

#endif
