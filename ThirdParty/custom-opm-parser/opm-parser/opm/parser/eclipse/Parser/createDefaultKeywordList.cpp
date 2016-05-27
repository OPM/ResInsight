/*
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
#include <iostream>

#include <opm/parser/eclipse/Generator/KeywordGenerator.hpp>
#include <opm/parser/eclipse/Generator/KeywordLoader.hpp>



int main(int argc , char ** argv) {
    if (argc == 7) {
        const char * config_root = argv[1];
        const char * source_file_name = argv[2];
        const char * header_file_base_path = argv[3];
        const char * header_file_name = argv[4];
        const char * test_file_name = argv[5];
        const char * output_files = argv[6];
        bool verboseLoader = false;
        bool verboseGenerator = true;

        Opm::KeywordLoader loader(verboseLoader);
        Opm::KeywordGenerator generator(verboseGenerator);
        loader.loadMultipleKeywordDirectories( config_root );

        const int num_output_files = std::atoi( output_files );
        generator.updateSource(loader , source_file_name, num_output_files );
        generator.updateHeader(loader, header_file_base_path, header_file_name );
        generator.updateTest(loader , test_file_name );

        exit(0);
    } else {
        std::cerr << "Error calling keyword generator: Expected arguments: <config_root> <source_file_name> <header_build_path> <header_file_name> <test_file_name> <num_output_files>" << std::endl;
        exit(1);
    }
}
