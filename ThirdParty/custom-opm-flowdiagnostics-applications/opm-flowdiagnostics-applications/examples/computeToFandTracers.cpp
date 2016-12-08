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
#endif

#include <opm/core/utility/parameters/ParameterGroup.hpp>
#include <opm/core/props/BlackoilPhases.hpp>

#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLWellSolution.hpp>

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

namespace {
    bool isFile(const boost::filesystem::path& p)
    {
        namespace fs = boost::filesystem;

        auto is_regular_file = [](const fs::path& pth)
        {
            return fs::exists(pth) && fs::is_regular_file(pth);
        };

        return is_regular_file(p)
            || (fs::is_symlink(p) &&
                is_regular_file(fs::read_symlink(p)));
    }

    boost::filesystem::path
    deriveFileName(boost::filesystem::path         file,
                   const std::vector<std::string>& extensions)
    {
        for (const auto& ext : extensions) {
            file.replace_extension(ext);

            if (isFile(file)) {
                return file;
            }
        }

        const auto prefix = file.parent_path() / file.stem();

        std::ostringstream os;

        os << "Unable to derive valid filename from model prefix "
           << prefix.generic_string();

        throw std::invalid_argument(os.str());
    }

    Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph& G, const int step)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

        using NConn = ConnVals::NumConnections;
        using NPhas = ConnVals::NumPhases;

        const auto nconn = NConn{G.numConnections()};
        const auto nphas = NPhas{3};

        auto flux = ConnVals(nconn, nphas);

        auto phas = ConnVals::PhaseID{0};

        for (const auto& p : { Opm::BlackoilPhases::Aqua   ,
                               Opm::BlackoilPhases::Liquid ,
                               Opm::BlackoilPhases::Vapour })
        {
            const auto pflux = G.flux(p, step);

            if (! pflux.empty()) {
                assert (pflux.size() == nconn.total);

                auto conn = ConnVals::ConnID{0};

                for (const auto& v : pflux) {
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
        for (size_t conn = 0; conn < num_conn; ++conn) {
            using Co = Opm::FlowDiagnostics::ConnectionValues::ConnID;
            using Ph = Opm::FlowDiagnostics::ConnectionValues::PhaseID;
            for (size_t phase = 0; phase < num_phases; ++phase) {
                fl(Co{conn}, Ph{phase}) /= 86400; // HACK! converting to SI.
            }
        }

        Opm::FlowDiagnostics::CellSetValues inflow;
        for (const auto& well : well_fluxes) {
            for (const auto& completion : well.completions) {
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
} // Anonymous namespace

// Syntax (typical):
//   computeToFandTracers case=<ecl_case_prefix> step=<report_number>

int main(int argc, char* argv[])
try {
    // Obtain parameters from command line (possibly specifying a parameter file).
    const bool verify_commandline_syntax = true;
    const bool parameter_output = false;
    Opm::parameter::ParameterGroup param(argc, argv, verify_commandline_syntax, parameter_output);

    // Obtain filenames for grid, init and restart files, as well as step number.
    using boost::filesystem::path;
    using std::string;
    const string casename = param.getDefault<string>("case", "DEFAULT_CASE_NAME");
    const path grid = param.has("grid") ? param.get<string>("grid")
        : deriveFileName(casename, { ".EGRID", ".FEGRID", ".GRID", ".FGRID" });
    const path init = param.has("init") ? param.get<string>("init")
        : deriveFileName(casename, { ".INIT", ".FINIT" });
    const path restart = param.has("restart") ? param.get<string>("restart")
        : deriveFileName(casename, { ".UNRST", ".FUNRST" });
    const int step = param.getDefault("step", 0);

    // Read graph and fluxes, initialise the toolbox.
    auto graph = Opm::ECLGraph::load(grid, init);
    graph.assignFluxDataSource(restart);
    Opm::ECLWellSolution wsol(restart);
    auto well_fluxes = wsol.solution(step, graph.numGrids());
    auto fdTool = initialiseFlowDiagnostics(graph, well_fluxes, step);

    // Solve for time of flight.
    using FDT = Opm::FlowDiagnostics::Toolbox;
    std::vector<Opm::FlowDiagnostics::CellSet> start;
    auto sol = fdTool.computeInjectionDiagnostics(start);
    const auto& tof = sol.fd.timeOfFlight();

    // Write it to standard out.
    std::cout.precision(16);
    for (double t : tof) {
        std::cout << t << '\n';
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << '\n';
}
