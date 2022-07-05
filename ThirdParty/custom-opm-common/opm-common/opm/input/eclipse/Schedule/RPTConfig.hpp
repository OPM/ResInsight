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
#ifndef RPT_CONFIG_HPP
#define RPT_CONFIG_HPP

#include <string>
#include <unordered_map>

namespace Opm {

class DeckKeyword;

class RPTConfig {
public:
    using Map = std::unordered_map<std::string, unsigned>;
    RPTConfig() = default;
    RPTConfig(const DeckKeyword&);
    bool contains(const std::string& key) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer.template map<Map, false>( m_mnemonics );
    }

    std::unordered_map<std::string, unsigned>::const_iterator begin() const { return this->m_mnemonics.begin(); };
    std::unordered_map<std::string, unsigned>::const_iterator end() const { return this->m_mnemonics.end(); };
    std::size_t size() const { return this->m_mnemonics.size(); };
    unsigned& at(const std::string& key) { return this->m_mnemonics.at(key); };

    static RPTConfig serializeObject();
    bool operator==(const RPTConfig& other) const;

private:
    std::unordered_map<std::string, unsigned> m_mnemonics;
};

}

#endif
