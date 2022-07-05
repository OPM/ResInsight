/*
  Copyright 2021 NORCE.

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

#include <opm/input/eclipse/EclipseState/MICPpara.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>

Opm::MICPpara::MICPpara() :
    m_density_biofilm( ParserKeywords::MICPPARA::DENSITY_BIOFILM::defaultValue ),
    m_density_calcite( ParserKeywords::MICPPARA::DENSITY_CALCITE::defaultValue ),
    m_detachment_rate( ParserKeywords::MICPPARA::DETACHMENT_RATE::defaultValue ),
    m_critical_porosity(  ParserKeywords::MICPPARA::CRITICAL_POROSITY::defaultValue ),
    m_fitting_factor(  ParserKeywords::MICPPARA::FITTING_FACTOR::defaultValue ),
    m_half_velocity_oxygen(  ParserKeywords::MICPPARA::HALF_VELOCITY_OXYGEN::defaultValue ),
    m_half_velocity_urea(  ParserKeywords::MICPPARA::HALF_VELOCITY_UREA::defaultValue ),
    m_maximum_growth_rate(  ParserKeywords::MICPPARA::MAXIMUM_GROWTH_RATE::defaultValue ),
    m_maximum_oxygen_concentration(  ParserKeywords::MICPPARA::MAXIMUM_OXYGEN_CONCENTRATION::defaultValue ),
    m_maximum_urea_concentration(  ParserKeywords::MICPPARA::MAXIMUM_UREA_CONCENTRATION::defaultValue ),
    m_maximum_urea_utilization(  ParserKeywords::MICPPARA::MAXIMUM_UREA_UTILIZATION::defaultValue ),
    m_microbial_attachment_rate(  ParserKeywords::MICPPARA::MICROBIAL_ATTACHMENT_RATE::defaultValue ),
    m_microbial_death_rate(  ParserKeywords::MICPPARA::MICROBIAL_DEATH_RATE::defaultValue ),
    m_minimum_permeability(  ParserKeywords::MICPPARA::MINIMUM_PERMEABILITY::defaultValue ),
    m_oxygen_consumption_factor(  ParserKeywords::MICPPARA::OXYGEN_CONSUMPTION_FACTOR::defaultValue ),
    m_tolerance_before_clogging(  ParserKeywords::MICPPARA::TOLERANCE_BEFORE_CLOGGING::defaultValue ),
    m_yield_growth_coefficient(  ParserKeywords::MICPPARA::YIELD_GROWTH_COEFFICIENT::defaultValue )
{ }


Opm::MICPpara::MICPpara( const Opm::Deck& deck)
    : MICPpara()
{
    using namespace Opm::ParserKeywords;

    if (!deck.hasKeyword<MICPPARA>())
        return;

    const auto& keyword = deck.get<MICPPARA>().back();
    const auto& record = keyword.getRecord(0);
    this->m_density_biofilm = record.getItem<MICPPARA::DENSITY_BIOFILM>().getSIDouble(0);
    this->m_density_calcite = record.getItem<MICPPARA::DENSITY_CALCITE>().getSIDouble(0);
    this->m_detachment_rate = record.getItem<MICPPARA::DETACHMENT_RATE>().getSIDouble(0);
    this->m_critical_porosity = record.getItem<MICPPARA::CRITICAL_POROSITY>().get< double >(0);
    this->m_fitting_factor = record.getItem<MICPPARA::FITTING_FACTOR>().get< double >(0);
    this->m_half_velocity_oxygen = record.getItem<MICPPARA::HALF_VELOCITY_OXYGEN>().getSIDouble(0);
    this->m_half_velocity_urea = record.getItem<MICPPARA::HALF_VELOCITY_UREA>().getSIDouble(0);
    this->m_maximum_growth_rate = record.getItem<MICPPARA::MAXIMUM_GROWTH_RATE>().getSIDouble(0);
    this->m_maximum_oxygen_concentration = record.getItem<MICPPARA::MAXIMUM_OXYGEN_CONCENTRATION>().getSIDouble(0);
    this->m_maximum_urea_concentration = record.getItem<MICPPARA::MAXIMUM_UREA_CONCENTRATION>().getSIDouble(0);
    this->m_maximum_urea_utilization = record.getItem<MICPPARA::MAXIMUM_UREA_UTILIZATION>().getSIDouble(0);
    this->m_microbial_attachment_rate = record.getItem<MICPPARA::MICROBIAL_ATTACHMENT_RATE>().getSIDouble(0);
    this->m_microbial_death_rate = record.getItem<MICPPARA::MICROBIAL_DEATH_RATE>().getSIDouble(0);
    this->m_minimum_permeability = record.getItem<MICPPARA::MINIMUM_PERMEABILITY>().getSIDouble(0);
    this->m_oxygen_consumption_factor = record.getItem<MICPPARA::OXYGEN_CONSUMPTION_FACTOR>().get< double >(0);
    this->m_tolerance_before_clogging = record.getItem<MICPPARA::TOLERANCE_BEFORE_CLOGGING>().get< double >(0);
    this->m_yield_growth_coefficient = record.getItem<MICPPARA::YIELD_GROWTH_COEFFICIENT>().get< double >(0);
}

