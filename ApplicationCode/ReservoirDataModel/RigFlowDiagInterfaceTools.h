/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RigFlowDiagResultAddress.h"

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

    std::vector<Opm::ECLPhaseIndex> getPhases(RigFlowDiagResultAddress::PhaseSelection phaseSelection)
    {
        std::vector<Opm::ECLPhaseIndex> phases;

        if (phaseSelection & RigFlowDiagResultAddress::PHASE_GAS)
        {
            phases.push_back(Opm::ECLPhaseIndex::Vapour);
        }
        if (phaseSelection & RigFlowDiagResultAddress::PHASE_OIL)
        {
            phases.push_back(Opm::ECLPhaseIndex::Liquid);
        }
        if (phaseSelection & RigFlowDiagResultAddress::PHASE_WAT)
        {
            phases.push_back(Opm::ECLPhaseIndex::Aqua);
        }

        return phases;
    }

    template <class FluxCalc>
    inline Opm::FlowDiagnostics::ConnectionValues
        extractFluxField(const Opm::ECLGraph& G,
                         FluxCalc&& getFlux,
                         std::vector<Opm::ECLPhaseIndex> actPh)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

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
        extractFluxFieldFromRestartFile(const Opm::ECLGraph& G,
                                        const Opm::ECLRestartData& rstrt,
                                        RigFlowDiagResultAddress::PhaseSelection phaseSelection)
    {
        auto getFlux = [&G, &rstrt]
        (const Opm::ECLPhaseIndex p)
        {
            return G.flux(rstrt, p);
        };

        return extractFluxField(G, getFlux, getPhases(phaseSelection));
    }

    inline Opm::FlowDiagnostics::ConnectionValues
        calculateFluxField(const Opm::ECLGraph& G,
                           const Opm::ECLInitFileData& init,
                           const Opm::ECLRestartData& rstrt,
                           RigFlowDiagResultAddress::PhaseSelection phaseSelection)
    {
        auto satfunc = Opm::ECLSaturationFunc(G, init);

        Opm::ECLFluxCalc calc(G, std::move(satfunc));

        auto getFlux = [&calc, &rstrt]
        (const Opm::ECLPhaseIndex p)
        {
            return calc.flux(rstrt, p);
        };

        return extractFluxField(G, getFlux, getPhases(phaseSelection));
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



