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


#ifndef ACTDIMS_HPP_
#define ACTDIMS_HPP_

#include <cstddef>

namespace Opm {

class Deck;

class Actdims {
public:
    Actdims();
    explicit Actdims(const Deck& deck);

    static Actdims serializeObject();

    std::size_t max_keywords() const;
    std::size_t max_line_count() const;
    std::size_t max_characters() const;
    std::size_t max_conditions() const;
    std::size_t line_size() const;

    bool operator==(const Actdims& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(keywords);
        serializer(line_count);
        serializer(characters);
        serializer(conditions);
    }

private:
    std::size_t keywords;
    std::size_t line_count;
    std::size_t characters;
    std::size_t conditions;
};

}

#endif
