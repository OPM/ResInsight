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

#include <opm/utility/ECLFluxCalc.hpp>
#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLWellSolution.hpp>

#include <exception>
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


    template <class FluxCalc>
    inline Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph& G,
                     FluxCalc&&           getFlux)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

        const auto actPh = G.activePhases();

        auto flux = ConnVals(ConnVals::NumConnections{G.numConnections()},
                             ConnVals::NumPhases{actPh.size()});

        auto phas = ConnVals::PhaseID{0};

        for (const auto& p : actPh) {
            const auto pflux = getFlux(p);

            if (! pflux.empty()) {
                assert (pflux.size() == flux.numConnections());

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

    inline Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph&        G,
                     const Opm::ECLInitFileData& init,
                     const Opm::ECLRestartData&  rstrt,
                     const bool                  compute_fluxes,
                     const bool                  useEPS)
    {
        if (compute_fluxes) {
            auto satfunc = ::Opm::ECLSaturationFunc(G, init, useEPS);

            Opm::ECLFluxCalc calc(G, std::move(satfunc));

            auto getFlux = [&calc, &rstrt]
                (const Opm::ECLPhaseIndex         p)
            {
                return calc.flux(rstrt, p);
            };

            return extractFluxField(G, getFlux);
        }

        auto getFlux = [&G, &rstrt]
            (const Opm::ECLPhaseIndex         p)
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




    struct FilePaths
    {
        FilePaths(const Opm::ParameterGroup& param)
        {
            const string casename = param.getDefault<string>("case", "DEFAULT_CASE_NAME");
            grid = param.has("grid") ? param.get<string>("grid")
                : deriveFileName(casename, { ".EGRID", ".FEGRID", ".GRID", ".FGRID" });
            init = param.has("init") ? param.get<string>("init")
                : deriveFileName(casename, { ".INIT", ".FINIT" });
            restart = param.has("restart") ? param.get<string>("restart")
                : deriveFileName(casename, { ".UNRST", ".FUNRST" });
        }

        using path = boost::filesystem::path;
        using string = std::string;

        path grid;
        path init;
        path restart;
    };




    inline Opm::ParameterGroup
    initParam(int argc, char** argv)
    {
        // Obtain parameters from command line (possibly specifying a parameter file).
        const bool verify_commandline_syntax = true;
        const bool parameter_output = false;
        Opm::ParameterGroup param(argc, argv, verify_commandline_syntax, parameter_output);
        return param;
    }




    inline Opm::FlowDiagnostics::Toolbox
    initToolbox(const Opm::ECLGraph& G)
    {
        const auto connGraph = Opm::FlowDiagnostics::
            ConnectivityGraph{ static_cast<int>(G.numCells()),
                               G.neighbours() };

        // Create the Toolbox.
        auto tool = Opm::FlowDiagnostics::Toolbox{ connGraph };
        tool.assignPoreVolume(G.poreVolume());

        return tool;
    }




    struct Setup
    {
        Setup(int argc, char** argv)
            : param          (initParam(argc, argv))
            , file_paths     (param)
            , init           (file_paths.init)
            , rstrt          (file_paths.restart)
            , graph          (::Opm::ECLGraph::load(file_paths.grid, init))
            , well_fluxes    ()
            , toolbox        (initToolbox(graph))
            , compute_fluxes_(param.getDefault("compute_fluxes", false))
            , useEPS_        (param.getDefault("use_ep_scaling", false))
        {
            const int step = param.getDefault("step", 0);

            if (! this->selectReportStep(step)) {
                std::ostringstream os;

                os << "Report Step " << step
                   << " is Not Available in Result Set "
                   << file_paths.grid.stem();

                throw std::domain_error(os.str());
            }
        }

        bool selectReportStep(const int step)
        {
            if (! rstrt.selectReportStep(step)) {
                return false;
            }

            {
                auto wsol = Opm::ECLWellSolution{};
                well_fluxes = wsol.solution(rstrt, graph.activeGrids());
            }

            toolbox.assignConnectionFlux(extractFluxField(graph, init, rstrt,
                                                          compute_fluxes_, useEPS_));

            toolbox.assignInflowFlux(extractWellFlows(graph, well_fluxes));

            return true;
        }

        Opm::ParameterGroup param;
        FilePaths file_paths;
        Opm::ECLInitFileData init;
        Opm::ECLRestartData rstrt;
        Opm::ECLGraph graph;
        std::vector<Opm::ECLWellSolution::WellData> well_fluxes;
        Opm::FlowDiagnostics::Toolbox toolbox;
        bool compute_fluxes_ = false;
        bool useEPS_ = false;
    };


} // namespace example



#endif // OPM_EXAMPLESETUP_HEADER_INCLUDED
