/*
  Copyright 2020 Equinor ASA.

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

#ifndef OPM_ROCK_CONFIG_HPP
#define OPM_ROCK_CONFIG_HPP

#include <string>

namespace Opm {

class Deck;
class FieldPropsManager;

class RockConfig {
public:

enum class Hysteresis {
    REVERS = 1,
    IRREVERS = 2,
    HYSTER = 3,
    BOBERG = 4,
    REVLIMIT = 5,
    PALM_MAN = 6,
    NONE = 7
};


struct RockComp {
    double pref;
    double compressibility;

    RockComp() = default;
    RockComp(double pref_arg, double comp_arg);
    bool operator==(const RockComp& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(pref);
        serializer(compressibility);
    }
};


    RockConfig();
    RockConfig(const Deck& deck, const FieldPropsManager& fp);

    static RockConfig serializeObject();

    bool active() const;
    const std::vector<RockConfig::RockComp>& comp() const;
    const std::string& rocknum_property() const;
    std::size_t num_rock_tables() const;
    Hysteresis hysteresis_mode() const;
    bool water_compaction() const;

    bool operator==(const RockConfig& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_active);
        serializer.vector(m_comp);
        serializer(num_property);
        serializer(num_tables);
        serializer(m_water_compaction);
        serializer(hyst_mode);
    }

private:
    bool m_active = false;
    std::vector<RockComp> m_comp;
    std::string num_property;
    std::size_t num_tables = 0;
    bool m_water_compaction = false;
    Hysteresis hyst_mode = Hysteresis::REVERS;
};

} //namespace Opm

#endif
