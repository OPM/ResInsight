/*
  Copyright (c) 2018 Statoil ASA

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

#include <set>
#include <iostream>

#include <opm/output/eclipse/RestartValue.hpp>

namespace Opm {

    namespace {

        const std::set<std::string> reserved_keys = {"LOGIHEAD", "INTEHEAD" ,"DOUBHEAD", "IWEL", "XWEL","ICON", "XCON" , "OPM_IWEL" , "OPM_XWEL", "ZWEL"};

    }



    RestartValue::RestartValue(data::Solution sol, data::Wells wells_arg) :
        solution(std::move(sol)),
        wells(std::move(wells_arg))
    {
    }

    const std::vector<double>& RestartValue::getExtra(const std::string& key) const {
        const auto iter = std::find_if(this->extra.begin(),
                                       this->extra.end(),
                                       [&](const std::pair<RestartKey, std::vector<double>>& pair)
                                       {
                                         return (pair.first.key == key);
                                       });
        if (iter == this->extra.end())
            throw std::invalid_argument("No such extra key " + key);

        return iter->second;
    }

    bool RestartValue::hasExtra(const std::string& key) const {
        const auto iter = std::find_if(this->extra.begin(),
                                       this->extra.end(),
                                       [&](const std::pair<RestartKey, std::vector<double>>& pair)
                                       {
                                         return (pair.first.key == key);
                                       });
        return  (iter != this->extra.end());
    }

    void RestartValue::addExtra(const std::string& key, UnitSystem::measure dimension, std::vector<double> data) {
        if (key.size() > 8)
            throw std::runtime_error("The keys used for Eclipse output must be maximum 8 characters long.");

        if (this->hasExtra(key))
            throw std::runtime_error("The keys in the extra vector must be unique.");

        if (this->solution.has(key))
            throw std::runtime_error("The key " + key + " is already present in the solution section.");

        if (reserved_keys.find(key) != reserved_keys.end())
            throw std::runtime_error("Can not use reserved key:" + key);

        this->extra.push_back( std::make_pair(RestartKey(key, dimension), std::move(data)));
    }

    void RestartValue::addExtra(const std::string& key, std::vector<double> data) {
        this->addExtra(key, UnitSystem::measure::identity, std::move(data));
    }

    void RestartValue::convertFromSI(const UnitSystem& units) {
        this->solution.convertFromSI(units);
        for (auto & extra_value : this->extra) {
            const auto& restart_key = extra_value.first;
            auto & data = extra_value.second;

            units.from_si(restart_key.dim, data);
        }
    }

    void RestartValue::convertToSI(const UnitSystem& units) {
        this->solution.convertToSI(units);
        for (auto & extra_value : this->extra) {
            const auto& restart_key = extra_value.first;
            auto & data = extra_value.second;

            units.to_si(restart_key.dim, data);
        }
    }


}
