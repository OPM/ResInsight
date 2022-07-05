/*
  Copyright (C) 2014 by Andreas Lauser

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
#ifndef OPM_PARSER_PVTO_TABLE_HPP
#define OPM_PARSER_PVTO_TABLE_HPP

#include <opm/input/eclipse/EclipseState/Tables/PvtxTable.hpp>

#include <array>
#include <cstddef>
#include <vector>

namespace Opm {

    class DeckKeyword;

    class PvtoTable : public PvtxTable {
    public:
        struct FlippedFVF {
            std::size_t i;
            std::array<double, std::size_t{2}> Rs;
            std::array<double, std::size_t{2}> Bo;
        };

        PvtoTable() = default;
        PvtoTable(const DeckKeyword& keyword, size_t tableIdx);

        static PvtoTable serializeObject();

        bool operator==(const PvtoTable& data) const;

        std::vector<FlippedFVF> nonMonotonicSaturatedFVF() const;
    };
}

#endif
