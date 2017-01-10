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

#ifndef OPM_EXAMPLESETUP_HEADER_INCLUDED
#define OPM_EXAMPLESETUP_HEADER_INCLUDED



#include <opm/core/utility/parameters/ParameterGroup.hpp>

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

namespace example {
    inline bool isFile(const boost::filesystem::path& p)
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

    inline boost::filesystem::path
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

    inline Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph& G)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

        using NConn = ConnVals::NumConnections;
        using NPhas = ConnVals::NumPhases;

        const auto nconn = NConn{G.numConnections()};
        const auto nphas = NPhas{3};

        auto flux = ConnVals(nconn, nphas);

        auto phas = ConnVals::PhaseID{0};

        for (const auto& p : { Opm::ECLGraph::PhaseIndex::Aqua   ,
                               Opm::ECLGraph::PhaseIndex::Liquid ,
                               Opm::ECLGraph::PhaseIndex::Vapour })
        {
            const auto pflux = G.flux(p);

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

    namespace Hack {
        inline Opm::FlowDiagnostics::ConnectionValues
        convert_flux_to_SI(Opm::FlowDiagnostics::ConnectionValues&& fl)
        {
            using Co = Opm::FlowDiagnostics::ConnectionValues::ConnID;
            using Ph = Opm::FlowDiagnostics::ConnectionValues::PhaseID;

            const auto nconn = fl.numConnections();
            const auto nphas = fl.numPhases();

            for (auto phas = Ph{0}; phas.id < nphas; ++phas.id) {
                for (auto conn = Co{0}; conn.id < nconn; ++conn.id) {
                    fl(conn, phas) /= 86400;
                }
            }

            return fl;
        }
    }

    inline Opm::ECLGraph
    initGraph(int argc, char* argv[])
    {
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

        if (! graph.selectReportStep(step)) {
            std::ostringstream os;

            os << "Report Step " << step
               << " is Not Available in Result Set '"
               << grid.stem() << '\'';

            throw std::domain_error(os.str());
        }

        return graph;
    }

    inline std::vector<Opm::ECLWellSolution::WellData>
    initWellFluxes(const Opm::ECLGraph& G)
    {
        auto wsol = Opm::ECLWellSolution{};
        return wsol.solution(G.rawResultData(), G.numGrids());
    }

    inline Opm::FlowDiagnostics::Toolbox
    initToolbox(const Opm::ECLGraph& G, const std::vector<Opm::ECLWellSolution::WellData>& well_fluxes)
    {
        const auto connGraph = Opm::FlowDiagnostics::
            ConnectivityGraph{ static_cast<int>(G.numCells()),
                               G.neighbours() };

        // Create the Toolbox.
        auto tool = Opm::FlowDiagnostics::Toolbox{ connGraph };

        tool.assignPoreVolume(G.poreVolume());
        tool.assignConnectionFlux(Hack::convert_flux_to_SI(extractFluxField(G)));

        tool.assignInflowFlux(extractWellFlows(G, well_fluxes));

        return tool;
    }




    struct Setup
    {
        Setup(int argc, char** argv)
            : graph(initGraph(argc, argv))
            , well_fluxes(initWellFluxes(graph))
            , toolbox(initToolbox(graph, well_fluxes))
        {
        }

        Opm::ECLGraph graph;
        std::vector<Opm::ECLWellSolution::WellData> well_fluxes;
        Opm::FlowDiagnostics::Toolbox toolbox;
    };


} // namespace example



#endif // OPM_EXAMPLESETUP_HEADER_INCLUDED
