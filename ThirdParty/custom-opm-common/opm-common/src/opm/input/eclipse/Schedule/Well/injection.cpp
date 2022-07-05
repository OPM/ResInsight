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

#include <stdexcept>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>

#include "injection.hpp"

namespace Opm {
namespace injection {

    double rateToSI(double rawRate, InjectorType wellType, const Opm::UnitSystem &unitSystem) {
        switch (wellType) {
        case InjectorType::MULTI:
            // multi-phase controlled injectors are a really funny
            // construct in Eclipse: the quantity controlled for is
            // not physically meaningful, i.e. Eclipse adds up
            // MCFT/day and STB/day.
            throw std::logic_error("There is no generic way to handle multi-phase injectors at this level!");

        case InjectorType::OIL:
        case InjectorType::WATER:
            return unitSystem.to_si( UnitSystem::measure::liquid_surface_rate, rawRate );

        case InjectorType::GAS:
            return unitSystem.to_si( UnitSystem::measure::gas_surface_rate, rawRate );

        default:
            throw std::logic_error("Unknown injector type");
        }
    }

    double rateToSI(double rawRate, Phase wellPhase, const Opm::UnitSystem& unitSystem) {
        switch (wellPhase) {
        case Phase::OIL:
        case Phase::WATER:
            return unitSystem.to_si( UnitSystem::measure::liquid_surface_rate, rawRate );

        case Phase::GAS:
            return unitSystem.to_si( UnitSystem::measure::gas_surface_rate, rawRate );

        default:
            throw std::logic_error("Unknown injection phase");
        }
    }
}
}

