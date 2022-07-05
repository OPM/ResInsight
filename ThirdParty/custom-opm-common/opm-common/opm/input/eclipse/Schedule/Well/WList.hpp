/*
  Copyright 2019 Equinor ASA.

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
#ifndef WLIST_HPP
#define WLIST_HPP

#include <cstddef>
#include <unordered_set>
#include <vector>
#include <string>

namespace Opm {

class WList {
public:
    using storage = std::vector<std::string>;

    WList() = default;
    WList(const storage& wlist, std::string wlname);
    std::size_t size() const;
    void add(const std::string& well);
    void del(const std::string& well);
    bool has(const std::string& well) const;
    std::string getName() const;

    std::vector<std::string> wells() const;
    bool operator==(const WList& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(well_list);
        serializer(name);
    }

private:
    storage well_list;
    std::string name;

};

}

#endif