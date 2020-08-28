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
#ifdef _WIN32
#include "cross-platform/windows/Substitutes.hpp"
#else
#include <fnmatch.h>
#endif

#include <opm/parser/eclipse/EclipseState/Schedule/Well/WList.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WListManager.hpp>

namespace Opm {

    WListManager WListManager::serializeObject()
    {
        WListManager result;
        result.wlists = {{"test1", WList({"test2", "test3"})}};

        return result;
    }

    bool WListManager::hasList(const std::string& name) const {
        return (this->wlists.find(name) != this->wlists.end());
    }


    WList& WListManager::newList(const std::string& name) {
        this->wlists.erase(name);
        this->wlists.insert( {name, WList() });
        return this->getList(name);
    }


    WList& WListManager::getList(const std::string& name) {
        return this->wlists.at(name);
    }

    const WList& WListManager::getList(const std::string& name) const {
        return this->wlists.at(name);
    }

    void WListManager::delWell(const std::string& well) {
        for (auto& pair: this->wlists) {
            auto& wlist = pair.second;
            wlist.del(well);
        }
    }

    bool WListManager::operator==(const WListManager& data) const {
        return this->wlists == data.wlists;
    }

}
