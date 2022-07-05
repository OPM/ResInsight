/*
  Copyright 2021 Equinor ASA.

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

#ifndef SIMULATOR_UPDATE_HPP
#define SIMULATOR_UPDATE_HPP

#include <unordered_set>


namespace Opm {

/*
  This struct is used to communicate back from the Schdule::applyAction() what
  needs to be updated in the simulator when execution is returned to the
  simulator code.
*/


struct SimulatorUpdate {
    // These wells have been affected by the ACTIONX and the simulator needs to
    // reapply rates and state from the newly updated Schedule object.
    std::unordered_set<std::string> affected_wells;

    // If one of the transmissibility multiplier keywords has been invoked as an
    // ACTIONX keyword the simulator needs to recalculate the transmissibility.
    bool tran_update{false};
};

}

#endif
