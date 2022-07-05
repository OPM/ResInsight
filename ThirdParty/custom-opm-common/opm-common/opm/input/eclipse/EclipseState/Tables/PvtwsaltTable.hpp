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
#ifndef OPM_PARSER_PVTWSALT_TABLE_HPP
#define	OPM_PARSER_PVTWSALT_TABLE_HPP

#include <vector>

namespace Opm {

    class DeckKeyword;

    class PvtwsaltTable {
    public:
        PvtwsaltTable();

        static PvtwsaltTable serializeObject();

        void init(const Opm::DeckRecord& record0, const Opm::DeckRecord& record1);
        size_t size() const;
        std::vector<double> getSaltConcentrationColumn() const;
        std::vector<double> getFormationVolumeFactorColumn() const;
        std::vector<double> getCompressibilityColumn() const;
        std::vector<double> getViscosityColumn() const;
        std::vector<double> getViscosibilityColumn() const;
        double getReferencePressureValue() const;
        double getReferenceSaltConcentrationValue() const;
        const std::vector<double>& getTableValues() const;

        bool operator==(const PvtwsaltTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_pRefValues);
            serializer(m_saltConsRefValues);
            serializer(m_tableValues);
        }

    protected:

        double m_pRefValues;
        double m_saltConsRefValues;
        std::vector <double> m_tableValues;

    };

}

#endif
