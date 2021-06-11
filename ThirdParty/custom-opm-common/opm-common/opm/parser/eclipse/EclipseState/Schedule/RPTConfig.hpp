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

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

class RPTConfig: public std::unordered_map<std::string,unsigned> {
#if __cplusplus <= 201703L
public:
    bool contains(const std::string&) const;
#endif

public:
    using std::unordered_map<std::string,unsigned>::unordered_map;

    RPTConfig(const DeckKeyword&);
};

}

#endif
