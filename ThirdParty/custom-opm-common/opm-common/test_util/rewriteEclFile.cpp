/*
  Copyright 2019 Equinor ASA.

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

#include <cmath>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <getopt.h>
#include <sstream>
#include <stdexcept>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclOutput.hpp>


static void printHelp() {

    std::cout << "\rewriteEclFile needs a minimum of one arguments which is the input file name.  \n"
              << "\nIn addition, the program takes these options (which must be given before the arguments):\n\n"
              << "-h Print help and exit.\n\n";
}

int main(int argc, char **argv) {

    int c                          = 0;

    while ((c = getopt(argc, argv, "h")) != -1) {
        switch (c) {
        case 'h':
            printHelp();
            return 0;
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    Opm::EclIO::EclFile reffile(argv[argOffset]);
    auto arrayList = reffile.getList();

    std::string outputFile=std::string(argv[argOffset]);

    int p1 = outputFile.find_last_of(".");
    std::string ext = outputFile.substr(p1+1);

    outputFile = outputFile.substr(0,p1) + "_REWRITE." + ext;
    Opm::EclIO::EclOutput outFile(outputFile, reffile.formattedInput());

    if (reffile.is_ix())
        outFile.set_ix();

    reffile.loadData();

    std::vector<int> elementSizeList = reffile.getElementSizeList();

    for (size_t n = 0; n < arrayList.size(); n++){

        std::string name = std::get<0>(arrayList[n]);
        auto arrType = std::get<1>(arrayList[n]);

        if (arrType == Opm::EclIO::INTE) {
            auto data = reffile.get<int>(n);
            outFile.write(name, data);
        } else if (arrType == Opm::EclIO::CHAR) {
            auto data = reffile.get<std::string>(n);
            outFile.write(name, data);
        } else if (arrType == Opm::EclIO::C0NN) {
            auto data = reffile.get<std::string>(n);
            outFile.write(name, data, elementSizeList[n]);
        } else if (arrType == Opm::EclIO::REAL) {
            auto data = reffile.get<float>(n);
            outFile.write(name, data);
        } else if (arrType == Opm::EclIO::DOUB) {
            auto data = reffile.get<double>(n);
            outFile.write(name, data);
        } else if (arrType == Opm::EclIO::LOGI) {
            auto data = reffile.get<bool>(n);
            outFile.write(name, data);
        } else if (arrType == Opm::EclIO::MESS) {
            outFile.message(name);
        }
    }

    return 0;
}
