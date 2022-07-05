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
#include <opm/input/eclipse/EclipseState/Tables/PvtwsaltTable.hpp>

namespace Opm {

        static const size_t numEntries = 5;
        PvtwsaltTable::PvtwsaltTable()
        {
        }

        void PvtwsaltTable::init(const Opm::DeckRecord& record0, const Opm::DeckRecord& record1)
        {

            m_pRefValues = record0.getItem("P_REF").getSIDoubleData()[0];
            m_saltConsRefValues = record0.getItem("SALT_CONCENTRATION_REF").getSIDoubleData()[0];
            m_tableValues = record1.getItem("DATA").getSIDoubleData();
        }

        size_t PvtwsaltTable::size() const
        {
            return m_tableValues.size()/numEntries;
        }

        double PvtwsaltTable::getReferencePressureValue() const
        {
            return m_pRefValues;
        }

        double PvtwsaltTable::getReferenceSaltConcentrationValue() const
        {
            return m_saltConsRefValues;
        }

        std::vector<double> PvtwsaltTable::getSaltConcentrationColumn() const
        {
            size_t tableindex = 0;
            std::vector<double> saltCons(this->size());
            for(size_t i=0; i<this->size(); ++i){
                saltCons[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return saltCons;

        }

        std::vector<double> PvtwsaltTable::getFormationVolumeFactorColumn() const
        {
            size_t tableindex = 1;
            std::vector<double> formationvolumefactor(this->size());
            for(size_t i=0; i<this->size(); ++i){
                formationvolumefactor[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return formationvolumefactor;

        }

        std::vector<double> PvtwsaltTable::getCompressibilityColumn() const
        {
            size_t tableindex = 2;
            std::vector<double> compresibility(this->size());
            for(size_t i=0; i<this->size(); ++i){
                compresibility[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return compresibility;

        }

        std::vector<double> PvtwsaltTable::getViscosityColumn() const
        {
            size_t tableindex = 3;
            std::vector<double> viscosity(this->size());
            for(size_t i=0; i<this->size(); ++i){
                viscosity[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return viscosity;

        }

        std::vector<double> PvtwsaltTable::getViscosibilityColumn() const
        {
            size_t tableindex = 4;
            std::vector<double> viscosibility(this->size());
            for(size_t i=0; i<this->size(); ++i){
                viscosibility[i] = m_tableValues[tableindex];
                tableindex = tableindex+numEntries;
            }
            return viscosibility;

        }

}

