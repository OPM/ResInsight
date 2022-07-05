/*
  Copyright 2019 SINTEF Digital, Mathematics and Cybernetics.

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

#include <opm/input/eclipse/Schedule/Well/WellFoamProperties.hpp>

#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/UDAValue.hpp>

Opm::WellFoamProperties Opm::WellFoamProperties::serializeObject()
{
    Opm::WellFoamProperties result;
    result.m_foamConcentration = 1.0;

    return result;
}

void Opm::WellFoamProperties::handleWFOAM(const DeckRecord& rec)
{
    this->m_foamConcentration = rec.getItem("FOAM_CONCENTRATION").get<UDAValue>(0).getSI();
}

bool Opm::WellFoamProperties::operator==(const WellFoamProperties& other) const
{
    return this->m_foamConcentration == other.m_foamConcentration;
}

bool Opm::WellFoamProperties::operator!=(const WellFoamProperties& other) const
{
    return this->m_foamConcentration != other.m_foamConcentration;
}
