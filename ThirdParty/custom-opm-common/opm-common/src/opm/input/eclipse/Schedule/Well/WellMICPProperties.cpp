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

#include <opm/input/eclipse/Schedule/Well/WellMICPProperties.hpp>

#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>

Opm::WellMICPProperties Opm::WellMICPProperties::serializeObject()
{
    Opm::WellMICPProperties result;
    result.m_microbialConcentration = 1.0;
    result.m_oxygenConcentration = 2.0;
    result.m_ureaConcentration = 3.0;

    return result;
}

void Opm::WellMICPProperties::handleWMICP(const DeckRecord& rec)
{
    this->m_microbialConcentration = rec.getItem<ParserKeywords::WMICP::MICROBIAL_CONCENTRATION>().getSIDouble(0);
    this->m_oxygenConcentration = rec.getItem<ParserKeywords::WMICP::OXYGEN_CONCENTRATION>().getSIDouble(0);
    this->m_ureaConcentration = rec.getItem<ParserKeywords::WMICP::UREA_CONCENTRATION>().getSIDouble(0);
}

bool Opm::WellMICPProperties::operator==(const WellMICPProperties& other) const
{
    if ((m_microbialConcentration == other.m_microbialConcentration) &&
        (m_oxygenConcentration == other.m_oxygenConcentration) &&
        (m_ureaConcentration == other.m_ureaConcentration))
        return true;
    else
        return false;
}

bool Opm::WellMICPProperties::operator!=(const WellMICPProperties& other) const
{
    return !(*this == other);
}
