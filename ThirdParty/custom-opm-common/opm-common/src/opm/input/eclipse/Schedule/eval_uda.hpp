/*
  Copyright 2019 Equinor ASA.

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

#ifndef WELL_UDA_HPP
#define WELL_UDA_HPP

#include <string>

#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/ScheduleTypes.hpp>

namespace Opm {
class UDAvalue;
class SummaryState;
class UnitSystem;

namespace UDA {

    double eval_well_uda(const UDAValue& value, const std::string& name, const SummaryState& st, double udq_undefined);
    double eval_well_uda_rate(const UDAValue& value, const std::string& name, const SummaryState& st, double udq_undefined, InjectorType wellType, const UnitSystem& unitSystem);

    double eval_group_uda(const UDAValue& value, const std::string& name, const SummaryState& st, double udq_undefined);
    double eval_group_uda_rate(const UDAValue& value, const std::string& name, const SummaryState& st, double udq_undefined, Phase phase, const UnitSystem& unitSystem);
}

}


#endif
