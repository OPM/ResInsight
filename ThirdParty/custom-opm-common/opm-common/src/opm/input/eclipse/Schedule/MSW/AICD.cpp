/*
  Copyright 2020 Equinor ASA.

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


#include <opm/input/eclipse/Schedule/MSW/AICD.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include "src/opm/input/eclipse/Schedule/MSW/FromWSEG.hpp"


namespace Opm {

AutoICD AutoICD::serializeObject() {
    AutoICD aicd;
    static_cast<SICD&>(aicd) = SICD::serializeObject();
    aicd.m_flow_rate_exponent = 1.0;
    aicd.m_visc_exponent = 2.0;
    aicd.m_oil_density_exponent = 3.0;
    aicd.m_water_density_exponent = 4.0;
    aicd.m_gas_density_exponent = 5.0;
    aicd.m_oil_viscosity_exponent = 6.0;
    aicd.m_water_viscosity_exponent = 7.0;
    aicd.m_gas_viscosity_exponent = 8.0;
    return aicd;
}

using AICD = ParserKeywords::WSEGAICD;
AutoICD::AutoICD(const DeckRecord& record) :
    SICD(record),
    m_flow_rate_exponent(record.getItem<AICD::FLOW_RATE_EXPONENT>().get<double>(0)),
    m_visc_exponent(record.getItem<AICD::VISC_EXPONENT>().get<double>(0)),
    m_oil_density_exponent(record.getItem<AICD::OIL_FLOW_FRACTION>().get<double>(0)),
    m_water_density_exponent(record.getItem<AICD::WATER_FLOW_FRACTION>().get<double>(0)),
    m_gas_density_exponent(record.getItem<AICD::GAS_FLOW_FRACTION>().get<double>(0)),
    m_oil_viscosity_exponent(record.getItem<AICD::OIL_VISC_FRACTION>().get<double>(0)),
    m_water_viscosity_exponent(record.getItem<AICD::WATER_VISC_FRACTION>().get<double>(0)),
    m_gas_viscosity_exponent(record.getItem<AICD::GAS_VISC_FRACTION>().get<double>(0))
{

}


// the function will return a map
// [
//     "WELL1" : [<seg1, aicd1>, <seg2, aicd2> ...]
//     ....
std::map<std::string, std::vector<std::pair<int, AutoICD> > >
AutoICD::fromWSEGAICD(const DeckKeyword& wsegaicd) {
    return fromWSEG<AutoICD>(wsegaicd);
}


bool AutoICD::operator==(const AutoICD& other) const {
    return SICD::operator==(other) &&
        this->m_flow_rate_exponent == other.m_flow_rate_exponent &&
        this->m_visc_exponent == other.m_visc_exponent &&
        this->m_oil_density_exponent == other.m_oil_density_exponent &&
        this->m_water_density_exponent == other.m_water_density_exponent &&
        this->m_gas_density_exponent == other.m_gas_density_exponent &&
        this->m_oil_viscosity_exponent == other.m_oil_viscosity_exponent &&
        this->m_water_viscosity_exponent == other.m_water_viscosity_exponent &&
        this->m_gas_viscosity_exponent == other.m_gas_viscosity_exponent;
}

double AutoICD::flowRateExponent() const {
    return this->m_flow_rate_exponent;
}

double AutoICD::viscExponent() const {
    return this->m_visc_exponent;
}

double AutoICD::oilDensityExponent() const {
    return this->m_oil_density_exponent;
}

double AutoICD::waterDensityExponent() const {
    return this->m_water_density_exponent;
}

double AutoICD::gasDensityExponent() const {
    return this->m_gas_density_exponent;
}

double AutoICD::oilViscExponent() const {
    return this->m_oil_viscosity_exponent;
}

double AutoICD::waterViscExponent() const {
    return this->m_water_viscosity_exponent;
}

double AutoICD::gasViscExponent() const {
    return this->m_gas_viscosity_exponent;
}


}
