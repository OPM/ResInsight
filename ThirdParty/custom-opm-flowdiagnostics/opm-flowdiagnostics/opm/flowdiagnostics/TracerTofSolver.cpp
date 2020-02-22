/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016, 2017 Statoil ASA.

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/TracerTofSolver.hpp>
#include <opm/utility/graph/tarjan.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

namespace Opm
{
namespace FlowDiagnostics
{

    namespace
    {
        std::vector<double> expandSparse(const int n, const CellSetValues& v)
        {
            std::vector<double> r(n, 0.0);
            for (const auto& data : v) {
                r[data.first] = data.second;
            }
            return r;
        }
    } // anonymous namespace



    // This computes both in and outflux with a single traversal of the graph.
    struct TracerTofSolver::InOutFluxComputer
    {
        InOutFluxComputer(const AssembledConnections& graph)
        {
            const int num_cells = graph.numRows();
            influx.resize(num_cells, 0.0);
            outflux.resize(num_cells, 0.0);
            for (int cell = 0; cell < num_cells; ++cell) {
                const auto nb = graph.cellNeighbourhood(cell);
                for (const auto& conn : nb) {
                    influx[conn.neighbour] += conn.weight;
                    outflux[cell] += conn.weight;
                }
            }
        }

        std::vector<double> influx;
        std::vector<double> outflux;
    };





    TracerTofSolver::TracerTofSolver(const AssembledConnections& graph,
                                     const AssembledConnections& reverse_graph,
                                     const std::vector<double>& pore_volumes,
                                     const CellSetValues& source_inflow)
        : TracerTofSolver(graph, reverse_graph, pore_volumes, source_inflow, InOutFluxComputer(graph))
    {
    }





    // The InOutFluxComputer is used so that influx_ and outflux_ can be
    // const members of the class.
    TracerTofSolver::TracerTofSolver(const AssembledConnections& graph,
                                     const AssembledConnections& reverse_graph,
                                     const std::vector<double>& pore_volumes,
                                     const CellSetValues& source_inflow,
                                     InOutFluxComputer&& inout)
        : g_(graph)
        , g_reverse_(reverse_graph)
        , pv_(pore_volumes)
        , influx_(std::move(inout.influx))
        , outflux_(std::move(inout.outflux))
        , source_term_(expandSparse(pore_volumes.size(), source_inflow))
        , local_source_term_(pore_volumes.size(), 0.0)
    {
    }





    std::vector<double> TracerTofSolver::solveGlobal()
    {
        // Reset solver variables and set source terms.
        prepareForSolve();
        setupStartArrayFromSource();
        local_source_term_ = source_term_;

        // Compute topological ordering and solve.
        computeOrdering();
        solve();
        std::fill(local_source_term_.begin(), local_source_term_.end(), 0.0);

        // Return computed time-of-flight.
        return tof_;
    }





    TracerTofSolver::LocalSolution TracerTofSolver::solveLocal(const CellSetValues& startset)
    {
        // Reset solver variables and set source terms.
        prepareForSolve();
        setupStartArray(startset);

        // Compute local topological ordering and solve.
        computeLocalOrdering(startset);
        setupLocalSource(startset);
        solve();
        cleanupLocalSource(startset);

        // Return computed time-of-flight.
        CellSetValues local_tof;
        CellSetValues local_tracer;
        const int num_elements = component_starts_.back();
        for (int element = 0; element < num_elements; ++element) {
            const int cell = sequence_[element];
            local_tof[cell] = tof_[cell];
            local_tracer[cell] = tracer_[cell];
            // Verify that tracer values are greater than zero
            if (tracer_[cell] <= 0.0) {
                throw std::logic_error("Tracer is zero in non-isolated cell.");
            }
        }
        return LocalSolution{ std::move(local_tof), std::move(local_tracer) };
    }





    void TracerTofSolver::prepareForSolve()
    {
        // Reset instance variables.
        const int num_cells = pv_.size();
        is_start_.clear();
        is_start_.resize(num_cells, 0);
        tof_.clear();
        tof_.resize(num_cells, 0.0);
        tracer_.clear();
        tracer_.resize(num_cells, 0.0);
        num_multicell_ = 0;
        max_size_multicell_ = 0;
        max_iter_multicell_ = 0;
    }





    void TracerTofSolver::setupStartArray(const CellSetValues& startset)
    {
        for (const auto& startpoint : startset) {
            const int cell = startpoint.first;
            is_start_[cell] = 1;
        }
    }





    void TracerTofSolver::setupStartArrayFromSource()
    {
        const int num_cells = pv_.size();
        for (int cell = 0; cell < num_cells; ++cell) {
            if (source_term_[cell] > 0.0) {
                is_start_[cell] = 1;
            }
        }
    }





    void TracerTofSolver::computeOrdering()
    {
        // Compute reverse topological ordering.
        const size_t num_cells = pv_.size();
        assert(g_.startPointers().size() == num_cells + 1);
        struct Deleter { void operator()(TarjanSCCResult* x) { destroy_tarjan_sccresult(x); } };
        std::unique_ptr<TarjanSCCResult, Deleter>
            result(tarjan(num_cells, g_.startPointers().data(), g_.neighbourhood().data()));

        // Must reverse ordering, since Tarjan computes reverse ordering.
        const int ok = tarjan_reverse_sccresult(result.get());
        if (!ok) {
            throw std::runtime_error("Failed to reverse topological ordering in TracerTofSolver::computeOrdering()");
        }

        // Extract data from solution.
        sequence_.resize(num_cells);
        const int num_comp = tarjan_get_numcomponents(result.get());
        component_starts_.resize(num_comp + 1);
        component_starts_[0] = 0;
        for (int comp = 0; comp < num_comp; ++comp) {
            const TarjanComponent tc = tarjan_get_strongcomponent(result.get(), comp);
            std::copy(tc.vertex, tc.vertex + tc.size, sequence_.begin() + component_starts_[comp]);
            component_starts_[comp + 1] = component_starts_[comp] + tc.size;
        }
        assert(component_starts_.back() == int(num_cells));
    }





    void TracerTofSolver::computeLocalOrdering(const CellSetValues& startset)
    {
        // Extract start cells.
        std::vector<int> startcells;
        startcells.reserve(startset.size());
        for (const auto& startpoint : startset) {
            startcells.push_back(startpoint.first);
        }

        // Compute reverse topological ordering.
        const size_t num_cells = pv_.size();
        assert(g_.startPointers().size() == num_cells + 1);
        struct ResultDeleter { void operator()(TarjanSCCResult* x) { destroy_tarjan_sccresult(x); } };
        std::unique_ptr<TarjanSCCResult, ResultDeleter> result;
        {
            struct WorkspaceDeleter { void operator()(TarjanWorkSpace* x) { destroy_tarjan_workspace(x); } };
            std::unique_ptr<TarjanWorkSpace, WorkspaceDeleter> ws(create_tarjan_workspace(num_cells));
            result.reset(tarjan_reachable_sccs(num_cells, g_.startPointers().data(), g_.neighbourhood().data(),
                                               startcells.size(), startcells.data(), ws.get()));
        }

        // Must reverse ordering, since Tarjan computes reverse ordering.
        const int ok = tarjan_reverse_sccresult(result.get());
        if (!ok) {
            throw std::runtime_error("Failed to reverse topological ordering in TracerTofSolver::computeOrdering()");
        }

        // Extract data from solution.
        sequence_.resize(num_cells); // For local solutions this is the upper limit of the size. Will give proper size afterwards.
        const int num_comp = tarjan_get_numcomponents(result.get());
        component_starts_.resize(num_comp + 1);
        component_starts_[0] = 0;
        for (int comp = 0; comp < num_comp; ++comp) {
            const TarjanComponent tc = tarjan_get_strongcomponent(result.get(), comp);
            std::copy(tc.vertex, tc.vertex + tc.size, sequence_.begin() + component_starts_[comp]);
            component_starts_[comp + 1] = component_starts_[comp] + tc.size;
        }
        sequence_.resize(component_starts_.back());
    }





    void TracerTofSolver::setupLocalSource(const CellSetValues& startset)
    {
        for (const auto& startpoint : startset) {
            if (startpoint.second < 0.0) {
                throw std::logic_error("Start set for local solve has negative source value.");
            }
            local_source_term_[startpoint.first] = startpoint.second;
        }
    }





    void TracerTofSolver::cleanupLocalSource(const CellSetValues& startset)
    {
        for (const auto& startpoint : startset) {
            local_source_term_[startpoint.first] = 0.0;
        }
    }





    void TracerTofSolver::solve()
    {
        // Solve each component.
        const int num_components = component_starts_.size() - 1;
        for (int comp = 0; comp < num_components; ++comp) {
            const int comp_size = component_starts_[comp + 1] - component_starts_[comp];
            if (comp_size == 1) {
                solveSingleCell(sequence_[component_starts_[comp]]);
            } else {
                solveMultiCell(comp_size, &sequence_[component_starts_[comp]]);
            }
        }

        // Threshold time-of-flight values.
        for (double& t : tof_) {
            t = std::min(t, max_tof_);
        }
    }





    void TracerTofSolver::solveSingleCell(const int cell)
    {
        // Compute influx (divisor of tof expression).
        double source = source_term_[cell];
        if (source == 0.0 && is_start_[cell]) {
            source = std::numeric_limits<double>::infinity(); // Gives 0 tof in start cell.
        }
        const double total_influx = influx_[cell] + source;

        // Compute upwind contribution.
        double upwind_tof_contrib = 0.0;
        double upwind_tracer_contrib = 0.0;
        for (const auto& conn : g_reverse_.cellNeighbourhood(cell)) {
            const int upwind_cell = conn.neighbour;
            const double flux = conn.weight;
            upwind_tof_contrib += tof_[upwind_cell] * tracer_[upwind_cell] * flux;
            upwind_tracer_contrib += tracer_[upwind_cell] * flux;
        }
        if (is_start_[cell]) {
            // For cells tagged as start cells, the tracer value
            // should get a contribution from the local source term
            // (which is then considered to be containing the
            // currently considered tracer).
            upwind_tracer_contrib += local_source_term_[cell];
        }

        // The following should be true if Tarjan was done correctly.
        // Note that global Tarjan will also visit isolated cells,
        // so it is possible to have exactly zero.
        assert(total_influx >= 0.0);
        assert(upwind_tracer_contrib >= 0.0);

        // Compute tracer.
        if (total_influx == 0.0) {
            assert(upwind_tracer_contrib == 0.0);
            // Isolated cell.
            tracer_[cell] = 0.0;
        } else {
            tracer_[cell] = upwind_tracer_contrib / total_influx;
        }

        // Compute time-of-flight.
        if (tracer_[cell] > 0.0) {
            tof_[cell] = (pv_[cell]*tracer_[cell] + upwind_tof_contrib)
                       / (total_influx * tracer_[cell]);
        } else {
            tof_[cell] = max_tof_;
        }

        // Cap time-of-flight if time to fill cell is greater than
        // max_tof_. Note that cells may still have larger than
        // max_tof_ after solveSingleCell() when including upwind
        // contributions, and those in turn can affect cells
        // downstream (so capping in this method will not produce the
        // same result). All tofs will finally be capped in solve() as
        // a post-process. The reason for the somewhat convoluted
        // behaviour is to match existing MRST results.
        if (total_influx < pv_[cell] / max_tof_) {
            tof_[cell] = max_tof_;
        }
    }





    void TracerTofSolver::solveMultiCell(const int num_cells, const int* cells)
    {
        // Record some statistics.
        ++num_multicell_;
        max_size_multicell_ = std::max(max_size_multicell_, num_cells);

        // Using a simple Gauss-Seidel approach.
        double max_tof_delta = 1e100;
        double max_tracer_delta = 1e100;
        int num_iter = 0;
        while (max_tof_delta > gauss_seidel_tof_tol_ || max_tracer_delta > gauss_seidel_tracer_tol_) {
            max_tof_delta = 0.0;
            max_tracer_delta = 0.0;
            ++num_iter;
            for (int ci = 0; ci < num_cells; ++ci) {
                const int cell = cells[ci];
                const double tof_before = tof_[cell];
                const double tracer_before = tracer_[cell];
                solveSingleCell(cell);
                max_tof_delta = std::max(max_tof_delta, std::fabs(tof_[cell] - tof_before));
                const bool zero_change =  ((tracer_before == 0.0) != (tracer_[cell] == 0.0));
                const double tracer_change = zero_change ? 1.0 : std::fabs(tracer_[cell] - tracer_before);
                max_tracer_delta = std::max(max_tracer_delta, tracer_change);
            }
        }
        max_iter_multicell_ = std::max(max_iter_multicell_, num_iter);
    }




} // namespace FlowDiagnostics
} // namespace Opm
