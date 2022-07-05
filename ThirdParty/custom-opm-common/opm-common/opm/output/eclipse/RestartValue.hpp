/*
  Copyright (c) 2017 Statoil ASA
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
#ifndef RESTART_VALUE_HPP
#define RESTART_VALUE_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/output/data/Aquifer.hpp>
#include <opm/output/data/Solution.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>

namespace Opm {

    class RestartKey {
    public:

        std::string key;
        UnitSystem::measure dim;
        bool required = false;

        RestartKey() = default;

        RestartKey( const std::string& _key, UnitSystem::measure _dim)
            : key(_key),
              dim(_dim),
              required(true)
        {}


        RestartKey( const std::string& _key, UnitSystem::measure _dim, bool _required)
            : key(_key),
              dim(_dim),
              required(_required)
        {}

        bool operator==(const RestartKey& key2) const
        {
            return key == key2.key &&
                   dim == key2.dim &&
                   required == key2.required;
        }
    };

    /*
      A simple class used to communicate values between the simulator and
      the RestartIO functions.
    */
    class RestartValue {
    public:
        using ExtraVector = std::vector<std::pair<RestartKey, std::vector<double>>>;

        data::Solution solution{};
        data::Wells wells{};
        data::GroupAndNetworkValues grp_nwrk{};
        data::Aquifers aquifer{};
        ExtraVector extra{};

        RestartValue(data::Solution sol,
                     data::Wells wells_arg,
                     data::GroupAndNetworkValues grpn_nwrk_arg,
                     data::Aquifers aquifer_arg);

        RestartValue() = default;

        bool hasExtra(const std::string& key) const;
        void addExtra(const std::string& key, UnitSystem::measure dimension, std::vector<double> data);
        void addExtra(const std::string& key, std::vector<double> data);
        const std::vector<double>& getExtra(const std::string& key) const;

        void convertFromSI(const UnitSystem& units);
        void convertToSI(const UnitSystem& units);

        bool operator==(const RestartValue& val2) const
        {
            return (this->solution == val2.solution)
                && (this->wells == val2.wells)
                && (this->grp_nwrk == val2.grp_nwrk)
                && (this->aquifer == val2.aquifer)
                && (this->extra == val2.extra);
        }
    };

}

#endif // RESTART_VALUE_HPP
