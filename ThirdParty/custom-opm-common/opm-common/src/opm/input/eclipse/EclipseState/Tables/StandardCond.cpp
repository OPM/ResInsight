/*
  Copyright (C) 2020 by Equinor ASA

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

#include <opm/input/eclipse/EclipseState/Tables/StandardCond.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace Opm {


StandardCond::StandardCond() {
    using ST = ParserKeywords::STCOND;
    double input_temp = ST::TEMPERATURE::defaultValue;
    double input_pressure = ST::PRESSURE::defaultValue;
    UnitSystem units( UnitSystem::UnitType::UNIT_TYPE_METRIC );
    this->temperature = units.to_si(UnitSystem::measure::temperature, input_temp);
    this->pressure = units.to_si(UnitSystem::measure::pressure, input_pressure);
}


StandardCond StandardCond::serializeObject()
{
    StandardCond result;
    result.temperature = 1.0;
    result.pressure = 2.0;

    return result;
}


}
