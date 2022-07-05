/*
  Copyright (C) 2020 Equinor ASA.

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
#ifndef OPM_PARSER_SOLVENTDENSITY_TABLE_HPP
#define OPM_PARSER_SOLVENTDENSITY_TABLE_HPP

namespace Opm {

    class DeckItem;

    class SolventDensityTable {
    public:
        static SolventDensityTable serializeObject();

        void init(const Opm::DeckRecord& record);
        const std::vector<double>& getSolventDensityColumn() const;

        bool operator==(const SolventDensityTable& data) const;

        std::vector<double>::const_iterator begin() const {
            return m_tableValues.begin();
        }

        std::vector<double>::const_iterator end() const {
            return m_tableValues.end();
        }

        std::size_t size() const {
            return this->m_tableValues.size();
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_tableValues);
        }

    private:
        std::vector<double> m_tableValues;
    };
}

#endif
