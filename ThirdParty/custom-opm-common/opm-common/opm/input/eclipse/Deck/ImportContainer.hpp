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

#ifndef IMPORT_CONTAINER_HPP
#define IMPORT_CONTAINER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>


namespace Opm {

class Parser;
class UnitSystem;

class ImportContainer {
public:
    ImportContainer(const Parser& parser, const UnitSystem& unit_system, const std::string& fname, bool formatted, std::size_t deck_size);

    std::vector<DeckKeyword>::iterator begin() { return this->keywords.begin(); }
    std::vector<DeckKeyword>::iterator end() { return this->keywords.end(); }
private:
    std::vector<DeckKeyword> keywords;
};


}

#endif // IMPORT_CONTAINER_HPP
