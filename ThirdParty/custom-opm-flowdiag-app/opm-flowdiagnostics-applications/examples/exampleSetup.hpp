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



#include <opm/common/utility/parameters/ParameterGroup.hpp>

#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/utility/ECLCaseUtilities.hpp>
#include <opm/utility/ECLFluxCalc.hpp>
#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLWellSolution.hpp>

#include <exception>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem/path.hpp>

namespace example {
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
            const auto grav = 0.0;

            Opm::ECLFluxCalc calc(G, init, grav, useEPS);

            return extractFluxField(G, [&calc, &rstrt]
                (const Opm::ECLPhaseIndex p)
            {
                return calc.flux(rstrt, p);
            });
        }

        return extractFluxField(G, [&G, &rstrt]
            (const Opm::ECLPhaseIndex p)
        {
            return G.flux(rstrt, p);
        });
    }

    template <class WellFluxes>
    std::map<Opm::FlowDiagnostics::CellSetID, Opm::FlowDiagnostics::CellSetValues>
    extractWellFlows(const Opm::ECLGraph& G,
                     const WellFluxes&    well_fluxes)
    {
        std::map<Opm::FlowDiagnostics::CellSetID, Opm::FlowDiagnostics::CellSetValues> well_flows;
        for (const auto& well : well_fluxes) {
            Opm::FlowDiagnostics::CellSetValues& inflow = well_flows[Opm::FlowDiagnostics::CellSetID(well.name)];
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

        return well_flows;
    }




    inline Opm::ECLCaseUtilities::ResultSet
    identifyResultSet(const Opm::ParameterGroup& param)
    {
        for (const auto* p : { "case", "grid", "init", "restart" })
        {
            if (param.has(p)) {
                return Opm::ECLCaseUtilities::ResultSet {
                    param.get<std::string>(p)
                };
            }
        }

        throw std::invalid_argument {
            "No Valid Result Set Identified by Input Parameters"
        };
    }




    inline std::unordered_set<int>
    getAvailableSteps(const ::Opm::ECLCaseUtilities::ResultSet& result_set)
    {
        const auto steps = result_set.reportStepIDs();

        return { std::begin(steps), std::end(steps) };
    }





    inline Opm::ParameterGroup
    initParam(int argc, char** argv)
    {
        // Obtain parameters from command line (possibly specifying a parameter file).
        const bool verify_commandline_syntax = true;
        const bool parameter_output = false;
        Opm::ParameterGroup param(argc, argv, verify_commandline_syntax, parameter_output);
        return param;
    }



    inline double simulationTime(const Opm::ECLRestartData& rstrt)
    {
        if (! rstrt.haveKeywordData("DOUBHEAD")) {
            return -1.0;
        }

        const auto& doubhead = rstrt.keywordData<double>("DOUBHEAD");

        // First item (.front()) is simulation time in days
        return doubhead.front();
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
            , result_set     (identifyResultSet(param))
            , init           (result_set.initFile())
            , graph          (::Opm::ECLGraph::load(result_set.gridFile(), init))
            , available_steps(getAvailableSteps(result_set))
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
                   << this->result_set.gridFile().stem();

                throw std::domain_error(os.str());
            }
        }

        bool selectReportStep(const int step)
        {
            if (!this->available_steps.empty() && this->available_steps.count(step) == 0) {
                // Requested report step not amongst those stored in the
                // result set.
                return false;
            }

            if (! (this->result_set.isUnifiedRestart() &&
                   bool(this->restart)))
            {
                // Non-unified (separate) restart files, or first time-step
                // selection in a unified restart file case.
                const auto restart_file =
                    this->result_set.restartFile(step);

                this->openRestartFile(restart_file);
            }

            if (! this->restart->selectReportStep(step)) {
                return false;
            }

            {
                auto wsol = Opm::ECLWellSolution{-1.0, false};
                well_fluxes = wsol.solution(*restart, graph.activeGrids());
            }

            toolbox.assignConnectionFlux(extractFluxField(graph, init, *restart,
                                                          compute_fluxes_, useEPS_));

            toolbox.assignInflowFlux(extractWellFlows(graph, well_fluxes));

            return true;
        }

        Opm::ParameterGroup param;
        Opm::ECLCaseUtilities::ResultSet result_set;
        Opm::ECLInitFileData init;
        Opm::ECLGraph graph;
        std::unordered_set<int> available_steps;
        std::vector<Opm::ECLWellSolution::WellData> well_fluxes;
        Opm::FlowDiagnostics::Toolbox toolbox;
        bool compute_fluxes_ = false;
        bool useEPS_ = false;

        std::shared_ptr<Opm::ECLRestartData> restart;

    private:
        void openRestartFile(const boost::filesystem::path& rstrt)
        {
            this->restart.reset(new Opm::ECLRestartData{ rstrt });
        }
    };


} // namespace example



#endif // OPM_EXAMPLESETUP_HEADER_INCLUDED
