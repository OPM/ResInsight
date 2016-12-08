#include "gtest/gtest.h"

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/utility/graph/AssembledConnections.hpp>
#include <opm/utility/ECLGraph.hpp>
#include "opm/utility/ECLWellSolution.hpp"
#include "opm/flowdiagnostics/ConnectivityGraph.hpp"
#include "opm/flowdiagnostics/ConnectionValues.hpp"
#include "opm/flowdiagnostics/CellSetValues.hpp"
#include "opm/flowdiagnostics/Toolbox.hpp"
const std::string casePath = "\\\\csfiles\\Store\\ProjectData\\StatoilReservoir\\ReferenceCases\\simple_FlowDiag_Model\\";

Opm::FlowDiagnostics::ConnectionValues
extractFluxField(const Opm::ECLGraph& G, const int step)
{
    using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

    using NConn = ConnVals::NumConnections;
    using NPhas = ConnVals::NumPhases;

    const auto nconn = NConn{ G.numConnections() };
    const auto nphas = NPhas{ 3 };

    auto flux = ConnVals(nconn, nphas);

    auto phas = ConnVals::PhaseID{ 0 };

    for(const auto& p :{ Opm::BlackoilPhases::Aqua   ,
         Opm::BlackoilPhases::Liquid ,
         Opm::BlackoilPhases::Vapour })
    {
        const auto pflux = G.flux(p, step);

        if(! pflux.empty())
        {
            assert (pflux.size() == nconn.total);

            auto conn = ConnVals::ConnID{ 0 };

            for(const auto& v : pflux)
            {
                flux(conn, phas) = v;

                conn.id += 1;
            }
        }

        phas.id += 1;
    }

    return flux;
}

Opm::FlowDiagnostics::Toolbox
initialiseFlowDiagnostics(const Opm::ECLGraph& G,
                          const std::vector<Opm::ECLWellSolution::WellData>& well_fluxes,
                          const int step)
{
    const auto connGraph = Opm::FlowDiagnostics::
        ConnectivityGraph{ static_cast<int>(G.numCells()),
        G.neighbours() };

    using FDT = Opm::FlowDiagnostics::Toolbox;

    auto fl = extractFluxField(G, step);
    const size_t num_conn = fl.numConnections();
    const size_t num_phases = fl.numPhases();
    for(size_t conn = 0; conn < num_conn; ++conn)
    {
        using Co = Opm::FlowDiagnostics::ConnectionValues::ConnID;
        using Ph = Opm::FlowDiagnostics::ConnectionValues::PhaseID;
        for(size_t phase = 0; phase < num_phases; ++phase)
        {
            fl(Co{ conn }, Ph{ phase }) /= 86400; // HACK! converting to SI.
        }
    }

    Opm::FlowDiagnostics::CellSetValues inflow;
    for(const auto& well : well_fluxes)
    {
        for(const auto& completion : well.completions)
        {
            const int grid_index = completion.grid_index;
            const auto& ijk = completion.ijk;
            const int cell_index = G.activeCell(ijk, grid_index);
            inflow.addCellValue(cell_index, completion.reservoir_inflow_rate);
        }
    }

    // Create the Toolbox.
    auto tool = FDT{ connGraph };
    tool.assignPoreVolume(G.poreVolume());
    tool.assignConnectionFlux(fl);
    tool.assignInflowFlux(inflow);

    return tool;
}

TEST(opm_flowdiagnostics_test, basic_construction)
{
    auto g = Opm::AssembledConnections{};
    auto s = Opm::FlowDiagnostics::CellSet{};

    try 
    {

        Opm::ECLGraph eclGraph = Opm::ECLGraph::load(casePath + "SIMPLE.EGRID", 
                                                     casePath + "SIMPLE.INIT");
        eclGraph.assignFluxDataSource(casePath + "SIMPLE.UNRST");

        Opm::ECLWellSolution wsol(casePath + "SIMPLE.UNRST");
        auto well_fluxes = wsol.solution(2, eclGraph.numGrids());


       // Opm::FlowDiagnostics::ConnectivityGraph connGraph( static_cast<int>(eclGraph.numCells()),
       //                                                   eclGraph.neighbours() );


        auto fdTool = initialiseFlowDiagnostics(eclGraph, well_fluxes, 2);

        // Solve for time of flight.
        using FDT = Opm::FlowDiagnostics::Toolbox;
        std::vector<Opm::FlowDiagnostics::CellSet> start;
        auto sol = fdTool.computeInjectionDiagnostics(start);
        std::vector<double> globalTimeOfFlight = sol.fd.timeOfFlight();

    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << '\n';
    }
}
