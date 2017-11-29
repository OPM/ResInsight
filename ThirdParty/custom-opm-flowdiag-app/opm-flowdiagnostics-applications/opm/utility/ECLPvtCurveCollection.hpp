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
#include <opm/utility/ECLPropertyUnitConversion.hpp>
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

        /// Define a collection of units of measure for output purposes.
        ///
        /// PVT property curves will be reported in the appropriate units of
        /// this system.  If this function is never called (or called with
        /// null pointer), then the output units are implicitly set to the
        /// flow-diagnostics module's internal units of measurement (meaning
        /// all properties and curves will be reported in strict SI units).
        ///
        /// \param[in] usys Collection of units of measure for output
        ///    purposes.  Typically the return value from one of the \code
        ///    *UnitConvention() \endcode functions of the \code ECLUnits
        ///    \endcode namespace.
        void setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys);

        /// Retrieve 2D graph representation of Phase PVT property function
        /// in a specific active cell.
        ///
        /// \param[in] curve PVT property curve descriptor
        ///
        /// \param[in] phase Phase for which to compute extract graph
        ///    representation of PVT property function.
        ///
        /// \param[in] activeCell Index of particular active cell in model.
        ///
        /// \return Collection of 2D graphs for PVT property curve
        ///    identified by requests represented by \p curve, \p phase and
        ///    \p activeCell.  One curve (vector element) for each tabulated
        ///    node of the primary look-up key.  Single curve (i.e., a
        ///    single element vector) in the case of dry gas (no vaporised
        ///    oil) or dead oil (no dissolved gas).  Return values provided
        ///    in the system of units specified by setOutputUnits().  Strict
        ///    SI if no output units have been defined.
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

        /// Compute a single dynamic property in a single active cell for a
        /// collection of cell states.
        ///
        /// Note: Phase property inputs (phase pressure and mixing ratios)
        ///    must be in strict SI units of measure (Pascal and sm^3/sm^3,
        ///    respectively).
        ///
        /// \param[in] property Named property.  Must be one of \code
        ///    RawCurve::FVF \endcode or \code RawCurve::Viscosity \endcode.
        ///    All other values return an empty result.
        ///
        /// \param[in] phase Phase for which to compute the property value.
        ///    Must be \code ECLPhaseIndex::Vapour \endcode or \code
        ///    ECLPhaseIndex::Liquid \endcode.  All other values return an
        ///    empty result.
        ///
        /// \param[in] activeCell Index of particular active cell in model.
        ///
        /// \param[in] phasePress Sequence of phase pressure values
        ///    pertaining to \p activeCell.  Could, for instance, be the
        ///    entire time-series of oil pressure values in that cell.
        ///
        /// \param[in] mixRatio Sequence of phase mixing ratio values
        ///    pertaining to \p activeCell.  Could, for instance, be the
        ///    entire time-series of dissolved gas/oil ratio values in that
        ///    cell.  Must be empty or match the size of \p phasePress.  If
        ///    empty, treated as \code
        ///    std::vector<double>(phasePress.size(), 0.0) \endcode which is
        ///    typically appropriate only for dry gas or dead oil cases.
        ///
        /// \return Sequence of dynamic property values corresponding to the
        ///    requested property name of the identified phase in the
        ///    particular active cell.  Empty for invalid requests, number
        ///    of elements equal to \code phasePress.size() \endcode
        ///    otherwise.  Return values are in strict SI units of
        ///    measure--i.e., rm^3/sm^3 for the formation volume factors and
        ///    Pascal seconds for the viscosities.
        std::vector<double>
        getDynamicPropertySI(const RawCurve             property,
                             const ECLPhaseIndex        phase,
                             const int                  activeCell,
                             const std::vector<double>& phasePress,
                             const std::vector<double>& mixRatio
                                 = std::vector<double>()) const;

        /// Compute a single dynamic property in a single active cell for a
        /// collection of cell states.
        ///
        /// This interface is intended for direct calculation based on raw
        /// (unconverted) data vectors from an ECL result set.
        /// Consequently, phase property inputs (phase pressure and mixing
        /// ratios) must be in the result set's native/serialised collection
        /// of units of measure (e.g., pressures in Atmospheres and mixing
        /// ratios in scm^3/scm^3 for the "LAB" system of units).
        ///
        /// \param[in] property Named property.  Must be one of \code
        ///    RawCurve::FVF \endcode or \code RawCurve::Viscosity \endcode.
        ///    All other values return an empty result.
        ///
        /// \param[in] phase Phase for which to compute the property value.
        ///    Must be \code ECLPhaseIndex::Vapour \endcode or \code
        ///    ECLPhaseIndex::Liquid \endcode.  All other values return an
        ///    empty result.
        ///
        /// \param[in] activeCell Index of particular active cell in model.
        ///
        /// \param[in] phasePress Sequence of phase pressure values
        ///    pertaining to \p activeCell.  Could, for instance, be the
        ///    entire time-series of oil pressure values in that cell.  Must
        ///    be in the serialised system of units (i.e., Bars for METRIC,
        ///    Psi for FIELD, and Atm for LAB and PVT-M).
        ///
        /// \param[in] mixRatio Sequence of phase mixing ratio values
        ///    pertaining to \p activeCell.  Could, for instance, be the
        ///    entire time-series of dissolved gas/oil ratio values in that
        ///    cell.  Must be empty or match the size of \p phasePress.  If
        ///    empty, treated as \code
        ///    std::vector<double>(phasePress.size(), 0.0) \endcode which is
        ///    typically appropriate only for dry gas or dead oil cases.
        ///
        ///    Must be in the serialised system of units.  In other words
        ///    when the \p mixRatio represents the dissolved gas/oil ratio
        ///    (Rs), then the input must be given in sm^3/sm^3 for METRIC,
        ///    Mscf/stb for FIELD, scm^3/scm^3 for LAB and sm^3/sm^3 for
        ///    PVT-M.  Similarly, when the \p mixRatio represents the
        ///    vapourised oil/gas ratio (Rv), then the input must be given
        ///    in sm^3/sm^3 for METRIC, stb/Mscf for FIELD, scm^3/scm^3 for
        ///    LAB and sm^3/sm^3 for PVT-M).
        ///
        /// \return Sequence of dynamic property values corresponding to the
        ///    requested property name of the identified phase in the
        ///    particular active cell.  Empty for invalid requests, number
        ///    of elements equal to \code phasePress.size() \endcode
        ///    otherwise.  Return values provided in the system of units
        ///    specified by setOutputUnits().  Strict SI if no output units
        ///    have been defined.
        std::vector<double>
        getDynamicPropertyNative(const RawCurve      property,
                                 const ECLPhaseIndex phase,
                                 const int           activeCell,
                                 std::vector<double> phasePress, // Mutable copy
                                 std::vector<double> mixRatio    // Mutable copy
                                 = std::vector<double>()) const;

    private:
        /// Forward map: Cell -> PVT Region ID
        std::vector<int> pvtnum_;

        /// Gas PVT property evaluator.
        std::shared_ptr<Gas> gas_; // shared => default special member funcs.

        /// Oil PVT property evaluator.
        std::shared_ptr<Oil> oil_;

        /// Native unit system of INIT file.  Used in the implementation of
        /// member function getDynamicPropertyNative().
        std::shared_ptr<const ECLUnits::UnitSystem> usys_native_;

        /// Explicit representation of internal system of units.  Strict SI.
        /// Used in getDynamicPropertyNative().
        std::shared_ptr<const ECLUnits::UnitSystem> usys_internal_;

        /// User-specified system of units for output of properties.  Used
        /// by getPvtCurve() and getDynamicPropertyNative().
        std::shared_ptr<const ECLUnits::UnitSystem> usys_output_{nullptr};

        /// Determine if a particular request can be met by the internal
        /// implementation.
        ///
        /// \param[in] phase Phase for which to compute a property.
        ///
        /// \param[in] activeCell Index of particular active cell in model.
        ///
        /// \return True if \p phase is supported and \p activeCell is
        ///    within range of the currently defined model.  False otherwise.
        bool isValidRequest(const ECLPhaseIndex phase,
                            const int           activeCell) const;

        /// Convert a sequence of 2D graphs to user-defined system of units.
        ///
        /// \param[in] graph Sequence of 2D graphs corresponding to a
        ///    particular curve request.
        ///
        /// \param[in] property Named property.
        ///
        /// \param[in] phase Phase for which to compute the property curve.
        ///    Must be \code ECLPhaseIndex::Vapour \endcode or \code
        ///    ECLPhaseIndex::Liquid \endcode.
        std::vector<FlowDiagnostics::Graph>
        convertToOutputUnits(std::vector<FlowDiagnostics::Graph>&& graph,
                             const RawCurve                        curve,
                             const ECLPhaseIndex                   phase) const;
    };

}} // Opm::ECLPVT

#endif // OPM_ECLPVTCURVECOLLECTION_HEADER_INCLUDED
