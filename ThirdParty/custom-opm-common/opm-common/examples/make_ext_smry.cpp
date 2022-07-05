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


#include <filesystem>
#include <iostream>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

#include "config.h"

#if _OPENMP
#include <omp.h>
#endif

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/EclUtil.hpp>


static void printHelp() {

    std::cout << "\nThis program create one or more lodsmry files, designed for effective load on the demand.   \n"
              << "These files are created with input from the smspec and unsmry file. \n"
              << "\nIn addition, the program takes these options (which must be given before the arguments):\n\n"
              << "-f if ESMRY file exist, this will be replaced. Default behaviour is that existing file is kept.\n"
              << "-n Maximum number of threads to be used if mulitple files should be created.\n"
              << "-h Print help and exit.\n\n";
}


int main(int argc, char **argv) {

    int c                          = 0;
#ifdef _OPENMP
    int max_threads = -1;
#endif
    bool force                     = false;

    while ((c = getopt(argc, argv, "fn:h")) != -1) {
        switch (c) {
        case 'f':
            force = true;
            break;
        case 'h':
            printHelp();
            return 0;
        case 'n':
#ifdef _OPENMP
            max_threads = atoi(optarg);
#else
            std::cerr << "OpenMP is disabled - using single thread only\n";
#endif
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

#ifdef _OPENMP
    int available_threads = omp_get_max_threads();

    if (max_threads < 0)
        max_threads = available_threads-2;
    else if (max_threads > (available_threads - 1))
        max_threads = available_threads-1;

    if (max_threads > (argc-argOffset))
        max_threads = argc-argOffset;

    omp_set_num_threads(max_threads);
#endif

    auto lap0 = std::chrono::system_clock::now();

    #pragma omp parallel for
    for (int f = argOffset; f < argc; f ++){
        std::filesystem::path inputFileName = argv[f];

        std::filesystem::path esmryFileName = inputFileName.parent_path() / inputFileName.stem();
        esmryFileName = esmryFileName += ".ESMRY";

        if (Opm::EclIO::fileExists(esmryFileName) && (force))
            remove (esmryFileName);

        Opm::EclIO::ESmry smryFile(argv[f]);
        if (!smryFile.make_esmry_file()){
            std::cout << "\n! Warning, smspec already have one lod file, existing kept use option -f to replace this" << std::endl;
        }
    }

    auto lap1 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds1 = lap1-lap0;
    std::cout << "\nruntime for creating " << (argc-argOffset) << " ESMRY files: " << elapsed_seconds1.count() << " seconds\n" << std::endl;

    return 0;
}
