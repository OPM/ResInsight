/*
  Copyright (C) 2020 by Equinor
  Copyright (C) 2020 by TNO

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
#ifndef OPM_PARSER_RWGSALT_TABLE_HPP
#define	OPM_PARSER_RWGSALT_TABLE_HPP

#include <vector>

namespace Opm {

    class DeckKeyword;

    class RwgsaltTable {
    public:
        RwgsaltTable();

        static RwgsaltTable serializeObject();

        void init(const Opm::DeckRecord& record1);
        size_t size() const;
        std::vector<double> getPressureColumn() const;
        std::vector<double> getSaltConcentrationColumn() const;
        std::vector<double> getVaporizedWaterGasRatioColumn() const;
        const std::vector<double>& getTableValues() const;

        bool operator==(const RwgsaltTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_tableValues);
        }

    protected:

        std::vector <double> m_tableValues;

    };

}

#endif
