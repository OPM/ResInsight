/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_TRACERTOFSOLVER_HEADER_INCLUDED
#define OPM_TRACERTOFSOLVER_HEADER_INCLUDED

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/CellSetValues.hpp>
#include <opm/utility/graph/AssembledConnections.hpp>
#include <vector>

namespace Opm
{
namespace FlowDiagnostics
{


    /// Class for solving the tracer and time-of-flight equations.
    ///
    /// Implements a first-order finite volume solver for
    /// (single-phase) time-of-flight using reordering.
    /// The equation solved is:
    ///     \f[v \cdot \nabla\tau = \phi\f]
    /// in which \f$ v \f$ is the fluid velocity, \f$ \tau \f$ is time-of-flight and
    /// \f$ \phi \f$ is the porosity. This is a boundary value problem, and
    /// \f$ \tau \f$ is considered zero on all inflow boundaries (or well inflows).
    ///
    /// The tracer equation is the same, except for the right hand side which is zero
    /// instead of \f[\phi\f].
    class TracerTofSolver
    {
    public:
        /// Initialize solver with a given flow graph (a weighted,
        /// directed asyclic graph) containing the out-fluxes from
        /// each cell, the reverse graph (with in-fluxes from each
        /// cell), pore volumes and inflow sources (positive).
        TracerTofSolver(const AssembledConnections& graph,
                        const AssembledConnections& reverse_graph,
                        const std::vector<double>& pore_volumes,
                        const CellSetValues& source_inflow);

        /// Compute the global (combining all sources) time-of-flight of each cell.
        ///
        /// TODO: also compute tracer solution.
        std::vector<double> solveGlobal();

        /// Output data struct for solveLocal().
        struct LocalSolution {
            CellSetValues tof;
            CellSetValues concentration;
        };

        /// Compute a local solution tracer and time-of-flight solution.
        ///
        /// Local means that only cells downwind from he startset are considered.
        /// The solution is therefore potentially sparse.
        /// TODO: not implemented!
        LocalSolution solveLocal(const CellSet& startset);

    private:

        // --------------  Private data members --------------

        const AssembledConnections& g_;
        const AssembledConnections& g_reverse_;
        const std::vector<double>& pv_;
        const std::vector<double> influx_;
        const std::vector<double> outflux_;
        std::vector<double> source_term_;
        std::vector<char> is_start_; // char to avoid the nasty vector<bool> specialization
        std::vector<int> sequence_;
        std::vector<int> component_starts_;
        std::vector<double> tof_;
        std::vector<double> tracer_;
        int num_multicell_ = 0;
        int max_size_multicell_ = 0;
        int max_iter_multicell_ = 0;
        const double gauss_seidel_tol_ = 1e-3;
        const double max_tof_ = 200.0 * 365.0 * 24.0 * 60.0 * 60.0; // 200 years.

        // --------------  Private helper class --------------

        struct InOutFluxComputer;

        // --------------  Private methods --------------

        TracerTofSolver(const AssembledConnections& graph,
                        const AssembledConnections& reverse_graph,
                        const std::vector<double>& pore_volumes,
                        const CellSetValues& source_inflow,
                        InOutFluxComputer&& inout);

        void prepareForSolve();

        void setupStartArray(const CellSet& startset);

        void setupStartArrayFromSource();

        void computeOrdering();

        void computeLocalOrdering(const CellSet& startset);

        void solve();

        void solveSingleCell(const int cell);

        void solveMultiCell(const int num_cells, const int* cells);
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_TRACERTOFSOLVER_HEADER_INCLUDED
