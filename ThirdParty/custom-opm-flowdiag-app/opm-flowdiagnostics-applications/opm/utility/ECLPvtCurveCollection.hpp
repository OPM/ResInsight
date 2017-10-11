/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#ifndef OPM_ECLPVTCURVECOLLECTION_HEADER_INCLUDED
#define OPM_ECLPVTCURVECOLLECTION_HEADER_INCLUDED

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLPvtGas.hpp>
#include <opm/utility/ECLPvtOil.hpp>

#include <memory>
#include <vector>

/// \file
///
/// Facility for evaluating pressure-dependent fluid properties (formation
/// volume factor, viscosities &c) for oil or gas based on tabulated
/// descriptions as represented in an ECL result set (INIT file 'TAB'
/// vector).

namespace Opm {
    class ECLInitFileData;
} // Opm

namespace Opm { namespace ECLPVT {

    class ECLPvtCurveCollection
    {
    public:
        ECLPvtCurveCollection(const ECLGraph&        G,
                              const ECLInitFileData& init);

        FlowDiagnostics::Graph
        getPvtCurve(const RawCurve      curve,
                    const ECLPhaseIndex phase,
                    const int           activeCell) const;

    private:
        /// Forward map: Cell -> PVT Region ID
        std::vector<int> pvtnum_;

        /// Gas PVT property evaluator.
        std::shared_ptr<Gas> gas_; // shared => default special member funcs.

        /// Oil PVT property evaluator.
        std::shared_ptr<Oil> oil_;
    };

}} // Opm::ECLPVT

#endif // OPM_ECLPVTCURVECOLLECTION_HEADER_INCLUDED
