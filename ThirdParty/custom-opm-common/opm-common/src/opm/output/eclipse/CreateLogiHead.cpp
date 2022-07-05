/*
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

#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/LogiHEAD.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <vector>

// #####################################################################
// Public Interface (createLogiHead()) Below Separator
// ---------------------------------------------------------------------

std::vector<bool>
Opm::RestartIO::Helpers::
createLogiHead(const EclipseState& es)
{
    const auto& rspec = es.runspec();
    const auto& tabMgr = es.getTableManager();
    const auto& phases = rspec.phases();
    const auto& wsd   = rspec.wellSegmentDimensions();
    const auto& hystPar = rspec.hysterPar();

    auto pvt = ::Opm::RestartIO::LogiHEAD::PVTModel{};

    pvt.isLiveOil = phases.active(::Opm::Phase::OIL) &&
        !tabMgr.getPvtoTables().empty();

    pvt.isWetGas = phases.active(::Opm::Phase::GAS) &&
        !tabMgr.getPvtgTables().empty();

    pvt.constComprOil = phases.active(::Opm::Phase::OIL) &&
        !(pvt.isLiveOil ||
          tabMgr.hasTables("PVDO") ||
          tabMgr.getPvcdoTable().empty());
	
    const auto lh = LogiHEAD{}
        .variousParam(false, false, wsd.maxSegmentedWells(), hystPar.active())
        .pvtModel(pvt)
        .network(rspec.networkDimensions().maxNONodes())
        ;
	
    return lh.data();
}
