/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_ECLGRAPH_HEADER_INCLUDED
#define OPM_ECLGRAPH_HEADER_INCLUDED

#include <opm/utility/ECLResultData.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include <boost/filesystem.hpp>

/// \file
///
/// Facility for extracting active cells and neighbourship relations from
/// on-disk ECLIPSE output, featuring on-demand property loading from
/// backing object (e.g., restart vectors at various time points).

namespace Opm {

    /// Package an ECLIPSE result set (represented as GRID, INIT, and
    /// restart files) in form usable with the computational engine
    /// represented by class \code Opm::FlowDiagnostics::ToolBox \endcode.
    class ECLGraph
    {
    public:
        using Path = boost::filesystem::path;

        /// Disabled default constructor.
        ECLGraph() = delete;

        /// Destructor.
        ~ECLGraph();

        /// Move constructor.
        ///
        /// \param[in] rhs Graph from which to appropriate resources.
        /// Invalid upon return.
        ECLGraph(ECLGraph&& rhs);

        /// Disabled copy constructor
        ECLGraph(const ECLGraph& rhs) = delete;

        /// Move assignment operator.
        ///
        /// \param[in] rhs Graph from which to appropriate resources.
        ///
        /// \return Reference to \code *this \endcode.
        ECLGraph& operator=(ECLGraph&& rhs);

        /// Disabled assignment operator.
        ECLGraph& operator=(const ECLGraph& rhs) = delete;

        /// Named constructor.
        ///
        /// \param[in] grid Name or prefix of ECL grid (i.e., .GRID or
        ///                 .EGRID) file.
        ///
        /// \param[in] init Name of ECL INIT file corresponding to \p grid
        ///                 input.  Assumed to provide at least a complete
        ///                 set of pore-volume values (i.e., for all global
        ///                 cells defined in the \p grid).
        ///
        ///                 If available in the INIT file, the constructor
        ///                 will also leverage the transmissibility data
        ///                 when constructing the active cell neighbourship
        ///                 table.
        ///
        /// \return Fully formed ECLIPSE connection graph with property
        /// associations.
        static ECLGraph
        load(const Path& grid, const Path& init);

        /// Assign source object for phase flux calculation.
        ///
        /// \param[in] src Name of ECL restart file, possibly unified, from
        ///                which next set of phase fluxes should be retrieved.
        void assignFluxDataSource(const Path& src);

        /// Retrieve number of grids.
        ///
        /// \return   The number of LGR grids plus one (the main grid).
        int numGrids() const;

        /// Retrieve active cell ID from (I,J,K) tuple in particular grid.
        ///
        /// \param[in] ijk Cartesian index tuple of particular cell.
        ///
        /// \param[in] gridID Identity of specific grid to which to relate
        ///     the (I,J,K) tuple.  Use zero (default) for main grid and
        ///     positive indices for any LGRs.  The (I,J,K) indices must be
        ///     within the ranges implied by the specific grid.
        ///
        /// \return Active ID (relative to linear, global numbering) of cell
        ///     (I,J,K) from specified grid.  Negative one (-1) if (I,J,K)
        ///     outside valid range or if the specific cell identified by \p
        ///     ijk and \p gridID is not actually active.
        int activeCell(const std::array<int,3>& ijk,
                       const int                gridID = 0) const;

        /// Retrieve number of active cells in graph.
        std::size_t numCells() const;

        /// Retrieve number of connections in graph.
        std::size_t numConnections() const;

        /// Retrieve neighbourship relations between active cells.
        ///
        /// The \c i-th connection is between active cells \code
        /// neighbours()[2*i + 0] \endcode and \code neighbours()[2*i + 1]
        /// \endcode.
        std::vector<int> neighbours() const;

        /// Retrieve static pore-volume values on active cells only.
        ///
        /// Corresponds to the \c PORV vector in the INIT file, possibly
        /// restricted to those active cells for which the pore-volume is
        /// strictly positive.
        std::vector<double> poreVolume() const;

        /// Restrict dynamic result set data to single report step.
        ///
        /// This method must be called before calling either flux() or
        /// rawResultData().
        ///
        /// \param[in] rptstep Selected temporal vector.  Report-step ID.
        ///
        /// \return Whether or not dynamic data for the requested report
        ///    step exists in the underlying result set identified in method
        ///    assignFluxDataSource().
        bool selectReportStep(const int rptstep) const;

        /// Access underlying result set.
        ///
        /// The result set dynamic data corresponds to the most recent call
        /// to method selectReportStep().
        const ::Opm::ECLResultData& rawResultData() const;

        /// Enum for indicating requested phase from the flux() method.
        enum class PhaseIndex { Aqua = 0, Liquid = 1, Vapour = 2 };

        /// Retrive phase flux on all connections defined by \code
        /// neighbours() \endcode.
        ///
        /// \param[in] phase Canonical phase for which to retrive flux.
        ///
        /// \return Flux values corresponding to selected phase.  Empty if
        ///    unavailable in the result set (e.g., when querying the gas
        ///    flux in an oil/water system or if no flux values at all were
        ///    output to the restart file).
        std::vector<double>
        flux(const PhaseIndex phase) const;

    private:
        /// Implementation class.
        class Impl;

        using ImplPtr = std::unique_ptr<Impl>;

        /// Constructor.  Intentially not \c explicit.
        ECLGraph(ImplPtr pImpl);

        /// Pointer to implementation.
        ImplPtr pImpl_;
    };

} // namespace Opm

#endif // OPM_ECLGRAPH_HEADER_INCLUDED
