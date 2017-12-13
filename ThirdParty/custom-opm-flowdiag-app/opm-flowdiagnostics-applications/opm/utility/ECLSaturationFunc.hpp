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

#ifndef OPM_ECLSATURATIONFUNC_HEADER_INCLUDED
#define OPM_ECLSATURATIONFUNC_HEADER_INCLUDED

#include <opm/flowdiagnostics/DerivedQuantities.hpp>
#include <opm/utility/ECLEndPointScaling.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <memory>
#include <vector>

/// \file
///
/// Public interface to relative permeability evaluation machinery.  The
/// back-end is aware of ECLIPSE's standard three-phase model for relative
/// permeability of oil and the two- and three-point saturation end-point
/// scaling options.  Vertical scaling of relative permeability is not
/// supported at present.

namespace Opm {
    class ECLGraph;
    class ECLRestartData;
    class ECLInitFileData;

    /// Gateway to engine for computing relative permeability values based
    /// on tabulated saturation functions in ECL output.
    class ECLSaturationFunc
    {
    public:
        /// Protocol for describing a particular saturation function
        /// request.
        struct RawCurve
        {
            /// Which saturation function does this request reference.
            enum class Function {
                /// Relative permeability functions
                RelPerm,

                /// Capillary pressure
                CapPress,
            };

            /// Which one-dimensional sub-system does this request reference.
            enum class SubSystem {
                /// Oil-Gas subsystem
                OilGas,

                /// Oil-Water subsystem
                OilWater,
            };

            /// Particular saturation function of this request.
            Function curve;

            /// Particular sub-system of this request.
            SubSystem subsys;

            /// Phase/component for which to form the effective saturation
            /// function curve.
            ECLPhaseIndex thisPh;
        };

        using InvalidEPBehaviour = ::Opm::SatFunc::
            EPSEvalInterface::InvalidEndpointBehaviour;

        /// Constructor
        ///
        /// \param[in] G Connected topology of current model's active cells.
        ///    Needed to linearise region mapping (e.g., SATNUM) that is
        ///    distributed on local grids to all of the model's active cells
        ///    (\code member function G.rawLinearisedCellData() \endcode).
        ///
        /// \param[in] init Container of tabulated saturation functions and
        ///    saturation table end points, if applicable, for all active
        ///    cells in the model \p G.
        ///
        /// \param[in] useEPS Whether or not to include effects of
        ///    saturation end-point scaling.  No effect if the INIT result
        ///    set does not actually include saturation end-point scaling
        ///    data.  Otherwise, enables turning EPS off even if associate
        ///    data is present in the INIT result set.
        ///
        ///    Default value (\c true) means that effects of EPS are
        ///    included if requisite data is present in the INIT result.
        ///
        /// \param[in] handle_invalid Run-time policy for how to handle
        ///    scaling requests relative to invalid scaled saturations
        ///    (e.g., SWL = -1.0E+20).
        ///
        ///    Default value (\c UseUnscaled) means that invalid scalings
        ///    are treated as unscaled.
        ECLSaturationFunc(const ECLGraph&          G,
                          const ECLInitFileData&   init,
                          const bool               useEPS = true,
                          const InvalidEPBehaviour handle_invalid
                          = InvalidEPBehaviour::UseUnscaled);

        /// Destructor.
        ~ECLSaturationFunc();

        /// Move constructor.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing engine for saturation function
        ///    evaluation.  Does not have a valid implementation when the
        ///    constructor completes.
        ECLSaturationFunc(ECLSaturationFunc&& rhs);

        /// Copy constructor.
        ///
        /// \param[in] rhs Existing engine for saturation function
        ///    evaluation.
        ECLSaturationFunc(const ECLSaturationFunc& rhs);

        /// Move assignment operator.
        ///
        /// Subsumes the implementation of an existing object.
        ///
        /// \param[in] rhs Existing engine for saturation function
        ///    evaluation.  Does not have a valid implementation when the
        ///    constructor completes.
        ///
        /// \return \code *this \endcode.
        ECLSaturationFunc& operator=(ECLSaturationFunc&& rhs);

        /// Assignment operator.
        ///
        /// \param[in] rhs Existing engine for saturation function
        ///    evaluation.
        ///
        /// \return \code *this \endcode.
        ECLSaturationFunc& operator=(const ECLSaturationFunc& rhs);

        /// Define a collection of units of measure for output purposes.
        ///
        /// Capillary pressure curves produced by getSatFuncCurve() will be
        /// reported in the pressure units of this system.  If this function
        /// is never called (or called with null pointer), then the output
        /// units are implicitly set to the flow-diagnostics module's
        /// internal units of measurement (meaning all properties and curves
        /// will be reported in strict SI units).
        ///
        /// \param[in] usys Collection of units of measure for output
        ///    purposes.  Typically the return value from one of the \code
        ///    *UnitConvention() \endcode functions of the \code ECLUnits
        ///    \endcode namespace.
        void setOutputUnits(std::unique_ptr<const ECLUnits::UnitSystem> usys);

        /// Compute relative permeability values in all active cells for a
        /// single phase.
        ///
        /// \param[in] G Connected topology of current model's active cells.
        ///    Needed to linearise phase saturations (e.g., SOIL) that are
        ///    distributed on local grids to all of the model's active cells
        ///    (\code member function G.rawLinearisedCellData() \endcode).
        ///
        /// \param[in] rstrt ECLIPSE restart vectors.  Result set view
        ///    assumed to be positioned at a particular report step of
        ///    interest.
        ///
        /// \param[in] p Phase for which to compute relative permeability
        ///    values.
        ///
        /// \return Derived relative permeability values of active phase \p
        ///    p for all active cells in model \p G.  Empty if phase \p p is
        ///    not actually active in the current result set.
        std::vector<double>
        relperm(const ECLGraph&       G,
                const ECLRestartData& rstrt,
                const ECLPhaseIndex   p) const;

        /// Retrieve 2D graph representations of sequence of effective
        /// saturation functions in a single cell.
        ///
        /// \param[in] func Sequence of saturation function descriptions.
        ///
        /// \param[in] activeCell Index of active cell from which to derive
        ///    the effective saturation function.  Use member function \code
        ///    ECLGraph::activeCell() \endcode to translate a global cell
        ///    (I,J,K) tuple--relative to a model grid--to a linear active
        ///    cell ID.
        ///
        /// \param[in] useEPS Whether or not to include effects of
        ///    saturation end-point scaling.  No effect if the INIT result
        ///    set from which the object was constructed does not actually
        ///    include saturation end-point scaling data.  Otherwise,
        ///    enables turning EPS off even if associate data is present in
        ///    the INIT result set.
        ///
        ///    Default value (\c true) means that effects of EPS are
        ///    included if requisite data is present in the INIT result.
        ///
        /// \return Sequence of 2D graphs for all saturation function
        ///    requests represented by \p func.  In particular, the \c i-th
        ///    element of the result corresponds to input request \code
        ///    func[i] \endcode.  Abscissas are stored in \code
        ///    graph[i].first \endcode and ordinates are stored in \code
        ///    graph[i].second \endcode.  If a particular request is
        ///    semantically invalid, such as when requesting the water
        ///    relative permeability in the oil-gas system, then the
        ///    corresponding graph in the result is empty.
        ///
        /// Example: Retrieve relative permeability curves for oil in active
        ///    cell 2718 in both the oil-gas and oil-water sub-systems while
        ///    excluding effects of end-point scaling.  This effectively
        ///    retrieves the "raw" tabulated saturation functions in the
        ///    INIT result set.
        ///
        ///    \code
        ///       using RC = ECLSaturationFunc::RawCurve;
        ///       auto func = std::vector<RC>{};
        ///       func.reserve(2);
        ///
        ///       // Request krog (oil rel-perm in oil-gas system)
        ///       func.push_back(RC{
        ///           RC::Function::RelPerm,
        ///           RC::SubSystem::OilGas,
        ///           ECLPhaseIndex::Liquid
        ///       });
        ///
        ///       // Request krow (oil rel-perm in oil-water system)
        ///       func.push_back(RC{
        ///           RC::Function::RelPerm,
        ///           RC::SubSystem::OilWater,
        ///           ECLPhaseIndex::Liquid
        ///       });
        ///
        ///       const auto graph =
        ///           sfunc.getSatFuncCurve(func, 2718, false);
        ///    \endcode
        std::vector<FlowDiagnostics::Graph>
        getSatFuncCurve(const std::vector<RawCurve>& func,
                        const int                    activeCell,
                        const bool                   useEPS = true) const;

    private:
        /// Implementation backend.
        class Impl;

        /// Pointer to actual backend/engine object.
        std::unique_ptr<Impl> pImpl_;
    };

} // namespace Opm

#endif // OPM_ECLSATURATIONFUNC_HEADER_INCLUDED
