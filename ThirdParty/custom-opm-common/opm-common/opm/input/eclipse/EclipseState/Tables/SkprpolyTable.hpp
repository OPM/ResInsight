/*
  Copyright (C) 2018 Statoil ASA

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

#ifndef OPM_PARSER_SKPRPOLY_TABLE_HPP
#define OPM_PARSER_SKPRPOLY_TABLE_HPP

#include <opm/input/eclipse/EclipseState/Tables/PolyInjTable.hpp>
namespace Opm {

    class DeckKeyword;

    class SkprpolyTable : public PolyInjTable {
    public:
        SkprpolyTable() = default;
        explicit SkprpolyTable(const DeckKeyword& table);

        static SkprpolyTable serializeObject();

        double referenceConcentration() const;

        const std::vector<std::vector<double>>& getSkinPressures() const;

        bool operator==(const SkprpolyTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            this->PolyInjTable::serializeOp(serializer);
            serializer(m_ref_polymer_concentration);
        }

    private:
        double m_ref_polymer_concentration;

    };

}

#endif //OPM_PARSER_SKPRPOLY_TABLE_HPP
