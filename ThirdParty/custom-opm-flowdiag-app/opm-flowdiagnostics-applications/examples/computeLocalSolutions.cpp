/*
  Copyright 2017 SINTEF ICT, Applied Mathematics.
  Copyright 2017 Statoil ASA.

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
#endif

#include "exampleSetup.hpp"
#include <opm/flowdiagnostics/CellSet.hpp>

// Syntax (typical):
//   computeToFandTracers case=<ecl_case_prefix> step=<report_number>
int main(int argc, char* argv[])
try {
    example::Setup setup(argc, argv);
    auto& fdTool = setup.toolbox;

    // Create start sets from injector wells.
    using ID = Opm::FlowDiagnostics::CellSetID;
    std::vector<Opm::FlowDiagnostics::CellSet> start;
    for (const auto& well : setup.well_fluxes) {
        if (!well.is_injector_well) {
            continue;
        }
        std::vector<int> completion_cells;
        completion_cells.reserve(well.completions.size());
        for (const auto& completion : well.completions) {
            const auto& gridName = completion.gridName;
            const auto& ijk = completion.ijk;
            const int cell_index = setup.graph.activeCell(ijk, gridName);
            if (cell_index >= 0) {
                completion_cells.push_back(cell_index);
            }
        }
        start.emplace_back(ID(well.name), completion_cells);
    }


    // Solve for injection time of flight and tracers.
    auto sol = fdTool.computeInjectionDiagnostics(start);

    // Choose injector id, default to first injector.
    const std::string id_string = setup.param.getDefault("id", start.front().id().to_string());
    const ID id(id_string);

    // Get local data for injector.
    const bool tracer = setup.param.getDefault("tracer", false);
    const auto& data = tracer ? sol.fd.concentration(id) : sol.fd.timeOfFlight(id);

    // Write it to standard out.
    std::cout.precision(16);
    for (auto item : data) {
        std::cout << item.first << "   " << item.second << '\n';
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << '\n';
}
