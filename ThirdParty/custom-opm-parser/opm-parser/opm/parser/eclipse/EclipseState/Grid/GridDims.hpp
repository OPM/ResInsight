/*
  Copyright 2016  Statoil ASA.

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

#ifndef OPM_PARSER_GRIDDIMS_HPP
#define OPM_PARSER_GRIDDIMS_HPP

#include <array>
#include <stdexcept>
#include <vector>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {
    class GridDims
    {
    public:

        GridDims(std::array<int, 3> xyz);

        GridDims(size_t nx, size_t ny, size_t nz);

        GridDims(const Deck& deck);

        size_t getNX() const;

        size_t getNY() const;
        size_t getNZ() const;

        const std::array<int, 3> getNXYZ() const;

        size_t getGlobalIndex(size_t i, size_t j, size_t k) const;

        const std::array<int, 3> getIJK(size_t globalIndex) const;

        size_t getCartesianSize() const;

        void assertGlobalIndex(size_t globalIndex) const;

        void assertIJK(size_t i, size_t j, size_t k) const;

    protected:
        GridDims();

        size_t m_nx;
        size_t m_ny;
        size_t m_nz;

    private:
        void init(const DeckKeyword& keyword);
    };
}

#endif /* OPM_PARSER_GRIDDIMS_HPP */
