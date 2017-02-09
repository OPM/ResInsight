
#pragma once


#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/utility/ECLFluxCalc.hpp>
#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLWellSolution.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace RigFlowDiagInterfaceTools {

    inline Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph& G, const bool compute_fluxes)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;
        auto flux = ConnVals(ConnVals::NumConnections{ G.numConnections() },
                             ConnVals::NumPhases{ 3 });
    
        auto phas = ConnVals::PhaseID{ 0 };
    
        Opm::ECLFluxCalc calc(G);
    
        const auto phases = { Opm::ECLGraph::PhaseIndex::Aqua   ,
                              Opm::ECLGraph::PhaseIndex::Liquid ,
                              Opm::ECLGraph::PhaseIndex::Vapour };
    
        for ( const auto& p : phases )
        {
            const auto pflux = compute_fluxes ? calc.flux(p) : G.flux(p);
            if ( ! pflux.empty() )
            {
                assert (pflux.size() == flux.numConnections());
                auto conn = ConnVals::ConnID{ 0 };
                for ( const auto& v : pflux )
                {
                    flux(conn, phas) = v;
                    conn.id += 1;
                }
            }
            phas.id += 1;
        }
    
        return flux;
    }

    template <class WellFluxes>
    Opm::FlowDiagnostics::CellSetValues
    extractWellFlows(const Opm::ECLGraph& G,
                     const WellFluxes&    well_fluxes)
    {
        Opm::FlowDiagnostics::CellSetValues inflow;
        for (const auto& well : well_fluxes) {
            for (const auto& completion : well.completions) {
                const int grid_index = completion.grid_index;
                const auto& ijk = completion.ijk;
                const int cell_index = G.activeCell(ijk, grid_index);
                if (cell_index >= 0) {
                    inflow.emplace(cell_index, completion.reservoir_inflow_rate);
                }
            }
        }

        return inflow;
    }


}



