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

#include <vector>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RwgsaltTable.hpp>

namespace Opm {

        static const size_t numEntries = 3;
        RwgsaltTable::RwgsaltTable()
        {
        }

        RwgsaltTable RwgsaltTable::serializeObject()
        {
            RwgsaltTable result;
            result.m_tableValues = {1.0, 2.0, 3.0};

            return result;
        }

        void RwgsaltTable::init(const Opm::DeckRecord& record1)
        {
            m_tableValues = record1.getItem("DATA").getSIDoubleData();
        }

        size_t RwgsaltTable::size() const
        {
            return m_tableValues.size()/numEntries;
        }

        const std::vector<double>& RwgsaltTable::getTableValues() const
        {
            return m_tableValues;
        }

        std::vector<double> RwgsaltTable::getPressureColumn() const
        {
            size_t tableindex = 0;
            std::vector<double> pressure(this->size());
            for(size_t i=0; i<this->size(); ++i){
                pressure[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return pressure;

        }

        std::vector<double> RwgsaltTable::getSaltConcentrationColumn() const
        {
            size_t tableindex = 1;
            std::vector<double> saltConc(this->size());
            for(size_t i=0; i<this->size(); ++i){
                saltConc[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return saltConc;

        }

        std::vector<double> RwgsaltTable::getVaporizedWaterGasRatioColumn() const
        {
            size_t tableindex = 2;
            std::vector<double> vaporizedwatergasratio(this->size());
            for(size_t i=0; i<this->size(); ++i){
                vaporizedwatergasratio[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return vaporizedwatergasratio;

        }       

        bool RwgsaltTable::operator==(const RwgsaltTable& data) const
        {
            return m_tableValues == data.m_tableValues;
        }

}

