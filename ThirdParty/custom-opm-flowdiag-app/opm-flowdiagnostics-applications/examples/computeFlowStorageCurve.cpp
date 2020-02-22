/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016, 2017 Statoil ASA.

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

#include "exampleSetup.hpp"

#include <opm/flowdiagnostics/DerivedQuantities.hpp>
#include <numeric>

// Syntax (typical):
//   computeFlowStorageCurve case=<ecl_case_prefix> step=<report_number>
int main(int argc, char* argv[])
try {
    example::Setup setup(argc, argv);
    auto& fdTool = setup.toolbox;

    // Solve for forward and reverse time of flight.
    std::vector<Opm::FlowDiagnostics::CellSet> start;
    auto fwd = fdTool.computeInjectionDiagnostics(start);
    auto rev = fdTool.computeProductionDiagnostics(start);
    auto pv = setup.graph.poreVolume();

    const bool ignore_disconnected = setup.param.getDefault("ignore_disconnected", true);
    if (ignore_disconnected) {
        // Give disconnected cells zero pore volume.
        std::vector<int> nbcount(setup.graph.numCells(), 0);
        for (int nb : setup.graph.neighbours()) {
            if (nb >= 0) {
                ++nbcount[nb];
            }
        }
        for (size_t i = 0; i < pv.size(); ++i) {
            if (nbcount[i] == 0) {
                pv[i] = 0.0;
            }
        }
    }

    // Compute graph.
    const double max_pv_fraction = setup.param.getDefault("max_pv_fraction", 0.1);
    auto fphi = Opm::FlowDiagnostics::flowCapacityStorageCapacityCurve(fwd, rev, pv, max_pv_fraction);

    // Write it to standard out.
    std::cout.precision(16);
    const int sz = fphi.first.size();
    for (int i = 0; i < sz; ++i) {
        std::cout << fphi.first[i] << "    " << fphi.second[i] << '\n';
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << '\n';
}
