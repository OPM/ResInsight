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

#include <cstdlib>

#include <iostream>
#include <fstream>
#include <locale>

#include <opm/input/eclipse/Generator/KeywordGenerator.hpp>
#include <opm/input/eclipse/Generator/KeywordLoader.hpp>


int main(int argc, char ** argv) {
    const char * keyword_list_file = argv[1];
    const char * source_file_path = argv[2];
    const char * init_file_name = argv[3];
    const char * header_file_base_path = argv[4];
    const char * header_file_path = argv[5];
    const char * test_file_name = argv[6];

    std::vector<std::string> keyword_list;
    {
        std::string buffer;
        std::ifstream is(keyword_list_file);
        std::getline( is , buffer );
        is.close();

        size_t start = 0;
        while (true) {
            size_t end = buffer.find( ";" , start);
            if (end == std::string::npos) {
                keyword_list.push_back( buffer.substr(start) );
                break;
            }

            keyword_list.push_back( buffer.substr(start, end - start ));
            start = end + 1;
        }
    }
    Opm::KeywordLoader loader( keyword_list, false );
    Opm::KeywordGenerator generator( true );

    generator.updateKeywordSource(loader , source_file_path );
    generator.updateInitSource(loader , init_file_name );
    generator.updateHeader(loader, header_file_base_path, header_file_path );
    generator.updateBuiltInHeader(loader, header_file_base_path, header_file_path );
    generator.updateTest( loader , test_file_name );
    if (argc >= 8)
        generator.updatePybindSource(loader , argv[7]);
}
