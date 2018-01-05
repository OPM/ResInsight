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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/TracerTofSolver.hpp>

#include <algorithm>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <opm/utility/numeric/RandomVector.hpp>


namespace Opm
{
namespace FlowDiagnostics
{

// ---------------------------------------------------------------------
// Class Toolbox::Impl
// ---------------------------------------------------------------------

class Toolbox::Impl
{
public:
    explicit Impl(ConnectivityGraph g);

    void assignPoreVolume(const std::vector<double>& pvol);
    void assignConnectionFlux(const ConnectionValues& flux);
    void assignInflowFlux(const std::map<CellSetID, CellSetValues>& inflow_flux);

    Forward injDiag (const std::vector<CellSet>& start_sets);
    Reverse prodDiag(const std::vector<CellSet>& start_sets);

private:
    ConnectivityGraph g_;

    std::vector<double> pvol_;
    ConnectionValues    flux_;
    std::map<CellSetID, CellSetValues> inj_flux_by_id_;
    std::map<CellSetID, CellSetValues> prod_flux_by_id_;
    CellSetValues       only_inflow_flux_;
    CellSetValues       only_outflow_flux_;

    AssembledConnections downstream_conn_;
    AssembledConnections upstream_conn_;
    bool conn_built_ = false;

    void buildAssembledConnections();
};

Toolbox::Impl::Impl(ConnectivityGraph g)
    : g_   (std::move(g))
    , pvol_()
    , flux_(ConnectionValues::NumConnections{ 0 },
            ConnectionValues::NumPhases     { 0 })
    , only_inflow_flux_()
    , only_outflow_flux_()
{}

void
Toolbox::Impl::assignPoreVolume(const std::vector<double>& pvol)
{
    if (pvol.size() != g_.numCells()) {
        throw std::logic_error("Inconsistently sized input "
                               "pore-volume field");
    }

    pvol_ = pvol;
}

void
Toolbox::Impl::assignConnectionFlux(const ConnectionValues& flux)
{
    if (flux.numConnections() != g_.numConnections()) {
        throw std::logic_error("Inconsistently sized input "
                               "flux field");
    }

    flux_ = flux;
    conn_built_ = false;
}

void
Toolbox::Impl::assignInflowFlux(const std::map<CellSetID, CellSetValues>& inflow_flux)
{
    only_inflow_flux_.clear();
    only_outflow_flux_.clear();
    inj_flux_by_id_.clear();
    prod_flux_by_id_.clear();
    for (const auto& inflow_set : inflow_flux) {
        const CellSetID& id = inflow_set.first;
        for (const auto& data : inflow_set.second) {
            if (data.second > 0.0) {
                only_inflow_flux_[data.first] += data.second;
                inj_flux_by_id_[id].insert(data);
            } else if (data.second < 0.0) {
                only_outflow_flux_[data.first] += -data.second;
                prod_flux_by_id_[id].insert(std::make_pair(data.first, -data.second));
            }
        }
    }
}

Toolbox::Forward
Toolbox::Impl::injDiag(const std::vector<CellSet>& start_sets)
{
    // Check that we have specified pore volume and fluxes.
    if (pvol_.empty() || flux_.numConnections() == 0) {
        throw std::logic_error("Must set pore volumes and fluxes before calling diagnostics.");
    }

    // Check that start sets are valid.
    for (const auto& start : start_sets) {
        if (inj_flux_by_id_.find(start.id()) == inj_flux_by_id_.end()) {
            throw std::runtime_error("Start set ID not present in data passed to assignInflowFlux().");
        }
        for (const int cell : start) {
            if (only_inflow_flux_.count(cell) != 1) {
                throw std::runtime_error("Start set inconsistent with assignInflowFlux()-given values");
            }
        }
    }

    if (!conn_built_) {
        buildAssembledConnections();
    }

    Solution sol;
    using ToF = Solution::TimeOfFlight;
    using Conc = Solution::TracerConcentration;

    TracerTofSolver solver(downstream_conn_, upstream_conn_, pvol_, only_inflow_flux_);
    sol.assignGlobalToF(solver.solveGlobal());

    for (const auto& start : start_sets) {
        auto solution = solver.solveLocal(inj_flux_by_id_[start.id()]);
        sol.assign(start.id(), ToF{ solution.tof });
        sol.assign(start.id(), Conc{ solution.concentration });
    }

    return Forward{ sol };
}

Toolbox::Reverse
Toolbox::Impl::prodDiag(const std::vector<CellSet>& start_sets)
{
    // Check that we have specified pore volume and fluxes.
    if (pvol_.empty() || flux_.numConnections() == 0) {
        throw std::logic_error("Must set pore volumes and fluxes before calling diagnostics.");
    }

    // Check that start sets are valid.
    for (const auto& start : start_sets) {
        if (prod_flux_by_id_.find(start.id()) == prod_flux_by_id_.end()) {
            throw std::runtime_error("Start set ID not present in data passed to assignInflowFlux().");
        }
        for (const int cell : start) {
            if (only_outflow_flux_.count(cell) != 1) {
                 throw std::runtime_error("Start set inconsistent with assignInflowFlux()-given values");
            }
        }
    }

    if (!conn_built_) {
        buildAssembledConnections();
    }

    Solution sol;
    using ToF = Solution::TimeOfFlight;
    using Conc = Solution::TracerConcentration;

    TracerTofSolver solver(upstream_conn_, downstream_conn_, pvol_, only_outflow_flux_);
    sol.assignGlobalToF(solver.solveGlobal());

    for (const auto& start : start_sets) {
        auto solution = solver.solveLocal(prod_flux_by_id_[start.id()]);
        sol.assign(start.id(), ToF{ solution.tof });
        sol.assign(start.id(), Conc{ solution.concentration });
    }

    return Reverse{ sol };
}

void
Toolbox::Impl::buildAssembledConnections()
{
    // Create the data structures needed by the tracer/tof solver.
    const size_t num_connections = g_.numConnections();
    const size_t num_phases = flux_.numPhases();
    downstream_conn_ = AssembledConnections();
    upstream_conn_ = AssembledConnections();
    for (size_t conn_idx = 0; conn_idx < num_connections; ++conn_idx) {
        auto cells = g_.connection(conn_idx);
        using ConnID = ConnectionValues::ConnID;
        using PhaseID = ConnectionValues::PhaseID;
        // Adding up all phase fluxes. TODO: ensure rigor, allow phase-based calculations.
        double connection_flux = 0.0;
        for (size_t phase = 0; phase < num_phases; ++phase) {
            connection_flux += flux_(ConnID{conn_idx}, PhaseID{phase});
        }
        if (connection_flux > 0.0) {
            downstream_conn_.addConnection(cells.first, cells.second, connection_flux);
            upstream_conn_.addConnection(cells.second, cells.first, connection_flux);
        } else if (connection_flux < 0.0) {
            downstream_conn_.addConnection(cells.second, cells.first, -connection_flux);
            upstream_conn_.addConnection(cells.first, cells.second, -connection_flux);
        }
    }
    const int num_cells = g_.numCells();
    downstream_conn_.compress(num_cells);
    upstream_conn_.compress(num_cells);

    // Mark as built (until flux changed).
    conn_built_ = true;
}

// =====================================================================
// Implementation of public interface below separator
// =====================================================================

// ---------------------------------------------------------------------
// Class Toolbox
// ---------------------------------------------------------------------

Toolbox::
Toolbox(const ConnectivityGraph& conn)
    : pImpl_(new Impl(conn))
{}

Toolbox::~Toolbox()
{}

Toolbox::Toolbox(Toolbox&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{
}

Toolbox&
Toolbox::operator=(Toolbox&& rhs)
{
    pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

void
Toolbox::assignPoreVolume(const std::vector<double>& pv)
{
    pImpl_->assignPoreVolume(pv);
}

void
Toolbox::assignConnectionFlux(const ConnectionValues& flux)
{
    pImpl_->assignConnectionFlux(flux);
}

void
Toolbox::assignInflowFlux(const std::map<CellSetID, CellSetValues>& inflow_flux)
{
    pImpl_->assignInflowFlux(inflow_flux);
}

Toolbox::Forward
Toolbox::
computeInjectionDiagnostics(const std::vector<CellSet>& start_sets)
{
    return pImpl_->injDiag(start_sets);
}

Toolbox::Reverse
Toolbox::
computeProductionDiagnostics(const std::vector<CellSet>& start_sets)
{
    return pImpl_->prodDiag(start_sets);
}


} // namespace FlowDiagnostics
} // namespace Opm
