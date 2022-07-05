/*
  Copyright 2021 Equinor ASA.

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

#include <iomanip>
#include <iostream>
#include <tuple>
#include <getopt.h>
#include <filesystem>
#include <sstream>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ERst.hpp>

using EclEntry = Opm::EclIO::EclFile::EclEntry;

static void printHelp() {

    std::cout << "\nList all arrays found in an EclFile specified on the command line. \n\n"
              << "\nThe program have one option which will only work on unified restart files:\n\n"
              << "-h Print help and exit.\n"
              << "-r List array for a spesific report time step number. Option only valid for a unified restart file. \n\n";
}

void print_array_list(const std::vector<EclEntry>& array_list, const std::vector<int>& element_size)
{

    for (size_t n = 0; n < array_list.size(); n++ ){

        auto array = array_list[n];

        std::string name = std::get<0> ( array );
        Opm::EclIO::eclArrType array_type = std::get<1> ( array );;
        int64_t size = std::get<2> ( array );

        std::string type_str;
        switch ( array_type ) {
        case Opm::EclIO::INTE:
            type_str = "INTE";
            break;
        case Opm::EclIO::REAL:
            type_str = "REAL";
            break;
        case Opm::EclIO::DOUB:
            type_str = "DOUB";
            break;
        case Opm::EclIO::LOGI:
            type_str = "LOGI";
            break;
        case Opm::EclIO::CHAR:
            type_str = "CHAR";
            break;
        case Opm::EclIO::C0NN:
            type_str = "C0NN";
            break;
        case Opm::EclIO::MESS:
            type_str = "MESS";
            break;
        default:
            throw std::runtime_error("unexpected array type" );
        }

        if ((type_str == "C0NN") && (element_size.size() > 0)) {
            std::ostringstream ss;
            ss << "C" << std::setw(3) << std::setfill('0') << element_size[n];
            type_str = ss.str();
        }

        std::cout << std::left << std::setw ( 8 ) << name << "   ";
        std::cout << std::right << std::setw ( 10 ) << size << "   ";
        std::cout << type_str << std::endl;
    }
}


int main(int argc, char **argv) {

    int c                          = 0;
    int reportStepNumber           = -1;
    bool specificReportStepNumber  = false;

    while ((c = getopt(argc, argv, "hr:")) != -1) {
        switch (c) {
        case 'h':
            printHelp();
            return 0;
        case 'r':
            specificReportStepNumber = true;
            reportStepNumber = atoi(optarg);
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    std::filesystem::path filename(argv[argOffset]);

    std::string ext = filename.extension().string();

    std::vector<EclEntry> array_list;
    std::vector<int> element_size;

    if ((specificReportStepNumber) and (ext == ".UNRST")){

        Opm::EclIO::ERst rstfile(filename);

        if (!rstfile.hasReportStepNumber(reportStepNumber)){
            std::string message = "report step number " + std::to_string(reportStepNumber) + " not found ";
            message = message + " in restart file " + filename.string();
            std::cout << message << std::endl;
            exit(1);
        }

        array_list = rstfile.listOfRstArrays(reportStepNumber);

    } else {

        Opm::EclIO::EclFile eclfile(filename);
        array_list = eclfile.getList();

        element_size = eclfile.getElementSizeList();
    }

    print_array_list(array_list, element_size);

    return 0;
}
