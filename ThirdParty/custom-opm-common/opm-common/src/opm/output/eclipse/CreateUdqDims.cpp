/*
  Copyright (c) 2018 Equinor ASA
  Copyright (c) 2018 Statoil ASA

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

#include <opm/output/eclipse/UDQDims.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <vector>

std::vector<int>
Opm::RestartIO::Helpers::
createUdqDims(const Schedule&     		sched,
              const std::size_t        	lookup_step,
              const std::vector<int>&   inteHead)
{
    const auto& udqCfg = sched.getUDQConfig(lookup_step);
    Opm::UDQDims dims(udqCfg, inteHead);
    return dims.data();
}
