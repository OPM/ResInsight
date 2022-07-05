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

#include <unordered_set>
#include <algorithm>

#include <opm/common/utility/shmatch.hpp>
#include <opm/io/eclipse/rst/state.hpp>
#include <opm/input/eclipse/Schedule/Well/WList.hpp>
#include <opm/input/eclipse/Schedule/Well/WListManager.hpp>

namespace Opm {

    WListManager WListManager::serializeObject()
    {
        WListManager result;
        result.wlists = {{"test1", WList({"test2", "test3"}, "test1")}};

        return result;
    }

    WListManager::WListManager(const RestartIO::RstState& rst_state) {
        for (const auto& [wlist, wells] : rst_state.wlists)
            this->newList(wlist, wells);
    }

    std::size_t WListManager::WListSize() const {
        return (this->wlists.size());
    }

    bool WListManager::hasList(const std::string& name) const {
        return (this->wlists.find(name) != this->wlists.end());
    }

    WList& WListManager::newList(const std::string& name, const std::vector<std::string>& new_well_names) {
        if (this->hasList(name)) {
            auto& wlist = getList(name);
            if (new_well_names.size() > 0) {
                // new well list contains wells
                std::vector<std::string> replace_wellnames;
                for (const auto& wname : wlist.wells()){
                    if (std::count(new_well_names.begin(), new_well_names.end(), wname) == 0) {
                        this->delWListWell(wname, name);
                    } else {
                        replace_wellnames.push_back(wname);
                    }
                }
                for (const auto& rwname : replace_wellnames) {
                    // delete wells to be replaced from well list
                    wlist.del(rwname);
                }
                for (const auto& wname : new_well_names) {
                    // add wells on new wlist
                    this->addWListWell(wname, name);
                }
            } else  {
                // remove all wells from existing well list (empty WLIST NEW)
                for (const auto& wname : wlist.wells()){
                    this->delWListWell(wname, name);
                }
            }
        } else {
            // create a new wlist (new well list name)
            this->wlists.insert( {name, WList({}, name)} );
            for (const auto& wname : new_well_names){
                this->addWListWell(wname, name);
            }
        }
        return this->getList(name);
    }

    WList& WListManager::getList(const std::string& name) {
        return this->wlists.at(name);
    }

    const WList& WListManager::getList(const std::string& name) const {
        return this->wlists.at(name);
    }

    const std::vector<std::string>& WListManager::getWListNames(const std::string& wname) const {
            return this->well_wlist_names.at(wname);
    }

    bool WListManager::hasWList(const std::string& wname) const {
        if (this->well_wlist_names.count(wname) > 0) {
            return true;
        } else {
            return false;
        }
    }

    std::size_t WListManager::getNoWListsWell(std::string wname) const {
        return this->no_wlists_well.at(wname);
    }

    void WListManager::addWListWell(const std::string& wname, const std::string& wlname) {
        //add well to wlist if it is not already in the well list
        auto& wlist = this->getList(wlname);
        wlist.add(wname);
        //add well list to well if not in vector already
        if (this->well_wlist_names.count(wname) > 0) {
            auto& no_wl = this->no_wlists_well.at(wname);
            auto& wlist_vec = this->well_wlist_names.at(wname);
            if (std::count(wlist_vec.begin(), wlist_vec.end(), wlname) == 0) {
                wlist_vec.push_back(wlname);
                no_wl += 1;
            }
        } else {
            //make wlist vector for new well
            std::vector<std::string> new_wlvec;
            std::size_t sz = 1;
            new_wlvec.push_back(wlname);
            this->well_wlist_names.insert({wname, new_wlvec});
            this->no_wlists_well.insert({wname, sz});
        }
    }

    void WListManager::delWell(const std::string& wname) {
        for (auto& pair: this->wlists) {
            auto& wlist = pair.second;
            wlist.del(wname);
            if (this->well_wlist_names.count(wname) > 0) {
                auto& wlist_vec = this->well_wlist_names.at(wname);
                auto& no_wl = this->no_wlists_well.at(wname);
                auto itwl = std::find(wlist_vec.begin(), wlist_vec.end(), wlist.getName());
                if (itwl != wlist_vec.end()) {
                    wlist_vec.erase(itwl);
                    no_wl -= 1;
                    if (no_wl == 0) {
                        wlist_vec.clear();
                    }
                }
            }
        }
    }

    void WListManager::delWListWell(const std::string& wname, const std::string& wlname) {
        //delete well from well list
        auto& wlist = this->getList(wlname);
        wlist.del(wname);

        if (this->well_wlist_names.count(wname) > 0) {
            auto& wlist_vec = this->well_wlist_names.at(wname);
            auto& no_wl = this->no_wlists_well.at(wname);
            // reduce the no of well lists associated with a well, delete whole list if no wlists is zero
            const auto& it = std::find(wlist_vec.begin(), wlist_vec.end(), wlname);
            if (it != wlist_vec.end()) {
                no_wl -= 1;
                if (no_wl == 0) {
                    wlist_vec.clear();
                }

            }
        }
    }

    bool WListManager::operator==(const WListManager& data) const {
        return this->wlists == data.wlists;
    }

    std::vector<std::string> WListManager::wells(const std::string& wlist_pattern) const {
        if (this->hasList(wlist_pattern)) {
            const auto& wlist = this->getList(wlist_pattern);
            return { wlist.wells() };
        } else {
            std::vector<std::string> well_set;
            auto pattern = wlist_pattern.substr(1);
            for (const auto& [name, wlist] : this->wlists) {
                auto wlist_name = name.substr(1);
                if (shmatch(pattern, wlist_name)) {
                    const auto& well_names = wlist.wells();
                    for ( auto it = well_names.begin(); it != well_names.end(); it++ ) {
                       if (std::count(well_set.begin(), well_set.end(), *it) == 0)
                           well_set.push_back(*it);
                    }
                }
            }
            return { well_set };
        }
    }

}
