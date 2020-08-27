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

#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQInput.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <chrono>
#include <cstddef>
#include <vector>

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace {


std::size_t entriesPerIUDQ()
{
    std::size_t no_entries = 3;
    return no_entries;
}

std::size_t entriesPerIUAD()
{
    std::size_t no_entries = 5;
    return no_entries;
}

std::size_t entriesPerZUDN()
{
    std::size_t no_entries = 2;
    return no_entries;
}

std::size_t entriesPerZUDL()
{
    std::size_t no_entries = 16;
    return no_entries;
}

std::size_t noIGphs(const std::vector<int>& inteHead)
{
    std::size_t no_entries = (inteHead[VI::intehead::NO_GROUP_UDQS] > 0) ? inteHead[20] : 0;
    return no_entries;
}


} // Anonymous

// #####################################################################
// Public Interface (createUdqDims()) Below Separator
// ---------------------------------------------------------------------

std::vector<int>
Opm::RestartIO::Helpers::
createUdqDims(const Schedule&     		sched,
              const std::size_t        	lookup_step,
              const std::vector<int>&   inteHead)
{
    const auto& udqCfg = sched.getUDQConfig(lookup_step);
    std::vector<int> udqDims;
    udqDims.resize(13,0);

    udqDims[ 0] = udqCfg.size();
    udqDims[ 1] = entriesPerIUDQ();
    udqDims[ 2] = inteHead[VI::intehead::NO_IUADS];
    udqDims[ 3] = entriesPerIUAD();
    udqDims[ 4] = entriesPerZUDN();
    udqDims[ 5] = entriesPerZUDL();
    udqDims[ 6] = noIGphs(inteHead);
    udqDims[ 7] = inteHead[VI::intehead::NO_IUAPS];
    udqDims[ 8] = inteHead[VI::intehead::NWMAXZ];
    udqDims[ 9] = inteHead[VI::intehead::NO_WELL_UDQS];
    udqDims[10] = inteHead[VI::intehead::NGMAXZ];
    udqDims[11] = inteHead[VI::intehead::NO_GROUP_UDQS];
    udqDims[12] = inteHead[VI::intehead::NO_FIELD_UDQS];

    return udqDims;
}
