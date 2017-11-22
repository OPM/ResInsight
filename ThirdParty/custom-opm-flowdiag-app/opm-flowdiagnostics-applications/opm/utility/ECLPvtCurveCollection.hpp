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
#include <opm/utility/ECLUnitHandling.hpp>

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
        /// Constructor
        ///
        /// \param[in] G Connected topology of current model's active cells.
        ///    Needed to linearise region mapping (e.g., SATNUM) that is
        ///    distributed on local grids to all of the model's active cells
        ///    (\code member function G.rawLinearisedCellData() \endcode).
        ///
        /// \param[in] init Container of tabulated PVT functions for all PVT
        ///    regions in the model \p G.
        ECLPvtCurveCollection(const ECLGraph&        G,
                              const ECLInitFileData& init);

        /// Retrieve 2D graph representation of Phase PVT property function
        /// in a specific active cell.
        ///
        /// \param[in] curve PVT property curve descriptor
        ///
        /// \param[in] phase Phase for which to compute extract graph
        ///    representation of PVT property function.
        ///
        /// \param[in] activeCell Index of particular active cell in model..
        ///
        /// \return Collection of 2D graphs for PVT property curve
        ///    identified by requests represented by \p curve, \p phase and
        ///    \p activeCell.  One curve (vector element) for each tabulated
        ///    node of the primary look-up key.  Single curve (i.e., a
        ///    single element vector) in the case of dry gas (no vaporised
        ///    oil) or dead oil (no dissolved gas).
        ///
        ///    No curves for water or dead oil with constant compressibility
        ///    (i.e., keyword 'PVCDO' in the input deck).
        ///
        /// Example: Retrieve collection of gas viscosity curves pertaining
        ///    to model's active cell 31415.
        ///
        ///    \code
        ///       const auto curves =
        ///           pvtCC.getPvtCurve(ECLPVT::RawCurve::Viscosity,
        ///                             ECLPhaseIndex::Vapour, 31415);
        ///    \endcode
        std::vector<FlowDiagnostics::Graph>
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

        /// Unit handling (SI -> result-set convention)
        std::shared_ptr<const ECLUnits::UnitSystem> usys_;
    };

}} // Opm::ECLPVT

#endif // OPM_ECLPVTCURVECOLLECTION_HEADER_INCLUDED
