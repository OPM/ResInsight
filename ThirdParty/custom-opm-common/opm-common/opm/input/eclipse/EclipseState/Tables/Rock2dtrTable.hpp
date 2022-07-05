/*
  Copyright (C) 2019 by Norce

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
#ifndef OPM_PARSER_ROCK2DTR_TABLE_HPP
#define	OPM_PARSER_ROCK2DTR_TABLE_HPP

#include <vector>

namespace Opm {

    class DeckRecord;

    class Rock2dtrTable {
    public:
        Rock2dtrTable();

        static Rock2dtrTable serializeObject();

        void init(const Opm::DeckRecord& record, size_t tableIdx);
        size_t size() const;
        size_t sizeMultValues() const;
        double getPressureValue(size_t index) const;
        double getTransMultValue(size_t pressureIndex, size_t saturationIndex ) const;

        bool operator==(const Rock2dtrTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_transMultValues);
            serializer(m_pressureValues);
        }

    protected:
        std::vector< std::vector <double> > m_transMultValues;
        std::vector< double > m_pressureValues;

    };

}

#endif
