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
#ifndef WLISTMANAGER_HPP
#define WLISTMANAGER_HPP

#include <cstddef>
#include <map>
#include <vector>
#include <string>

namespace Opm {

class WList;

class WListManager {
public:
    WListManager() = default;

    static WListManager serializeObject();

    bool hasList(const std::string&) const;
    WList& getList(const std::string& name);
    const WList& getList(const std::string& name) const;
    WList& newList(const std::string& name);
    void delWell(const std::string& well);

    bool operator==(const WListManager& data) const;
    std::vector<std::string> wells(const std::string& wlist_pattern) const;
    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.map(wlists);
    }

private:
    std::map<std::string, WList> wlists;
};

}
#endif
