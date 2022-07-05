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

#include <vector>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Rock2dtrTable.hpp>

namespace Opm {

        Rock2dtrTable::Rock2dtrTable()
        {
        }

        Rock2dtrTable  Rock2dtrTable::serializeObject()
        {
            Rock2dtrTable result;

            result.m_transMultValues = {{1.0,2.0},{3.0,4.0}};
            result.m_pressureValues = {1.0, 2.0, 3.0};

            return result;
        }

        void Rock2dtrTable::init(const DeckRecord& record, size_t /* tableIdx */)
        {
            m_pressureValues.push_back(record.getItem("PRESSURE").getSIDoubleData()[0]);
            m_transMultValues.push_back(record.getItem("TRANSMULT").getSIDoubleData());
        }

        size_t Rock2dtrTable::size() const
        {
            return m_pressureValues.size();
        }

        size_t Rock2dtrTable::sizeMultValues() const
        {
            return m_transMultValues[0].size();
        }

        double Rock2dtrTable::getPressureValue(size_t index) const
        {
            return m_pressureValues[index];
        }

        double Rock2dtrTable::getTransMultValue(size_t pressureIndex, size_t saturationIndex) const
        {
            return m_transMultValues[pressureIndex][saturationIndex];
        }

        bool Rock2dtrTable::operator==(const Rock2dtrTable& data) const
        {
              return this->m_transMultValues == data.m_transMultValues &&
                     this->m_pressureValues == data.m_pressureValues;
        }

}

