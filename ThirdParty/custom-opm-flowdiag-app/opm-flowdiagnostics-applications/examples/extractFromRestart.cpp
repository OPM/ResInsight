/*
  Copyright 2017 SINTEF DIGITAL, Mathematics and Cybernetics.
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
#include <opm/utility/ECLResultData.hpp>


// Syntax (typical):
//   extractFromRestart unrst=<ecl_unrst file> step=<report_number> keyword=<keyword to dump> grid_id=<grid number>
int main(int argc, char* argv[]) {
    try {
        Opm::parameter::ParameterGroup param(argc, argv,
                /*verify_commandline_syntax=*/ true,
                /*parameter_output=*/ false);
        const std::string unrst_file = param.get<std::string>("unrst");
        const int report_step = param.getDefault("step", int(0));
        const int grid_id = param.getDefault("grid_id", int(0));
        const std::string keyword = param.get<std::string>("keyword");

        Opm::ECLResultData restart_file(unrst_file);

        if (!restart_file.selectReportStep(report_step)) {
            std::cerr << "Could not find report step " << report_step << "." << std::endl;
            exit(-1);
        }

        if (restart_file.haveKeywordData(keyword, grid_id)) {
            const std::vector<double>& data = restart_file.keywordData<double>(keyword, grid_id);

            std::cout.precision(20);

            // Write out to cout in a matlab-friendly fashion.
            std::cout << "kw_" << keyword << " = {" << std::endl
                    << "'unrst_file=" << unrst_file << "'," << std::endl
                    << "'step=" << report_step << "'," << std::endl
                    << "'grid_id=" << grid_id << "'," << std::endl;
            std::cout << "[";
            std::ostream_iterator<double> out_it(std::cout, ", ");
            std::copy(data.begin(), --data.end(), out_it);
            std::cout << data.back();
            std::cout << "]" << std::endl;
            std::cout << "};" << std::endl;
        }
        else {
            std::cerr << "Could not find the keyword " << keyword
                    << " in report step " << report_step << "." << std::endl;
            exit(-1);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << '\n';
    }
}
