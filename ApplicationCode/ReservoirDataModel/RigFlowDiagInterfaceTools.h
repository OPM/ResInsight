
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

    template <class FluxCalc>
    inline Opm::FlowDiagnostics::ConnectionValues
        extractFluxField(const Opm::ECLGraph& G,
            FluxCalc&&           getFlux)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

        const auto actPh = G.activePhases();

        auto flux = ConnVals(ConnVals::NumConnections{ G.numConnections() },
            ConnVals::NumPhases{ actPh.size() });

        auto phas = ConnVals::PhaseID{ 0 };

        for (const auto& p : actPh) {
            const auto pflux = getFlux(p);

            if (!pflux.empty()) {
                assert(pflux.size() == flux.numConnections());

                auto conn = ConnVals::ConnID{ 0 };
                for (const auto& v : pflux) {
                    flux(conn, phas) = v;
                    conn.id += 1;
                }
            }

            phas.id += 1;
        }

        return flux;
    }

    inline Opm::FlowDiagnostics::ConnectionValues
        extractFluxField(const Opm::ECLGraph&       G,
            const Opm::ECLRestartData& rstrt,
            const bool                 compute_fluxes)
    {
        if (compute_fluxes) {
            Opm::ECLFluxCalc calc(G);

            auto getFlux = [&calc, &rstrt]
            (const Opm::ECLGraph::PhaseIndex p)
            {
                return calc.flux(rstrt, p);
            };

            return extractFluxField(G, getFlux);
        }

        auto getFlux = [&G, &rstrt]
        (const Opm::ECLGraph::PhaseIndex p)
        {
            return G.flux(rstrt, p);
        };

        return extractFluxField(G, getFlux);
    }

    template <class WellFluxes>
    Opm::FlowDiagnostics::CellSetValues
        extractWellFlows(const Opm::ECLGraph& G,
            const WellFluxes&    well_fluxes)
    {
        Opm::FlowDiagnostics::CellSetValues inflow;
        for (const auto& well : well_fluxes) {
            for (const auto& completion : well.completions) {
                const auto& gridName = completion.gridName;
                const auto& ijk = completion.ijk;
                const int cell_index = G.activeCell(ijk, gridName);
                if (cell_index >= 0) {
                    // Since inflow is a std::map, if the key was not
                    // already present operator[] will insert a
                    // value-initialized value (as in T() for a type
                    // T), which is zero for built-in numerical types,
                    // including double.
                    inflow[cell_index] += completion.reservoir_inflow_rate;
                }
            }
        }

        return inflow;
    }

}



