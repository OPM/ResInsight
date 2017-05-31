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

#include "exampleSetup.hpp"


// Syntax (typical):
//   computeToFandTracers case=<ecl_case_prefix> step=<report_number>
int main(int argc, char* argv[])
try {
    example::Setup setup(argc, argv);
    auto& fdTool = setup.toolbox;

    // Solve for time of flight.
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
