/*
   Copyright 2019 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include "EclRegressionTest.hpp"

#include <opm/common/ErrorMacros.hpp>

#include <iostream>
#include <string>
#include <getopt.h>
#include <fstream>

static void printHelp() {
    std::cout << "\ncompareECL compares ECLIPSE files (restart (.RST), unified restart (.UNRST), initial (.INIT), summary (.SMRY), unified summary (.UNSMRY) or .RFT) and gridsizes (from .EGRID or .GRID file) from two simulations.\n"
              << "The program takes four arguments:\n\n"
              << "1. Case number 1, reference case (full path without extension)\n"
              << "2. Case number 2, test case (full path without extension)\n"
              << "3. Absolute tolerance\n"
              << "4. Relative tolerance (between 0 and 1)\n\n"
              << "In addition, the program takes these options (which must be given before the arguments):\n\n"
              << "-a Run a full analysis of errors.\n"
              << "-h Print help and exit.\n"
              << "-d Use report steps only when comparing results from summary files.\n"
              << "-i Execute integration test (regression test is default).\n"
              << "   The integration test compares SGAS, SWAT and PRESSURE in unified restart files, and WOPR, WGPR, WWPR and WBHP (all wells) in summary file. \n"
              << "-k Specify specific keyword to compare (capitalized), for examples -k PRESSURE or -k WOPR:A-1H \n"
              << "-l Only do comparison for the last Report Step. This option is only valid for restart files.\n"
              << "-n Do not throw on errors.\n"
              << "-p Print keywords in both cases and exit.\n"
              << "-r compare a spesific report time step number in a restart file.\n"
              << "-t Specify ECLIPSE filetype to compare, (default behaviour is that all files are compared if found). Different possible arguments are:\n"
              << "    -t UNRST \t Compare two unified restart files (.UNRST). This the default value, so it is the same as not passing option -t.\n"
              << "    -t EGRID  \t Compare two EGrid files (.EGRID).\n"
              << "    -t INIT  \t Compare two initial files (.INIT).\n"
              << "    -t RFT   \t Compare two RFT files (.RFT).\n"
              << "    -t SMRY  \t Compare two cases consistent of (unified) summary files.\n"
              << "-x Allow extra keywords in case number 2. These additional keywords (not found in case number1) will be ignored in the comparison.\n"
              << "\nExample usage of the program: \n\n"
              << "compareECL -k PRESSURE <path to first casefile> <path to second casefile> 1e-3 1e-5\n"
              << "compareECL -t INIT -k PORO <path to first casefile> <path to second casefile> 1e-3 1e-5\n"
              << "compareECL -i <path to first casefile> <path to second casefile> 0.01 1e-6\n\n"
              << "Exceptions are thrown (and hence program exits) when deviations are larger than the specified "
              << "tolerances, or when the number of cells does not match -- either in the grid file or for a "
              << "specific keyword. Information about the keyword, keyword occurrence (zero based) and cell "
              << "coordinate is printed when an exception is thrown. For more information about how the cases "
              << "are compared, see the documentation of the EclFilesComparator class.\n\n";
}



static bool has_result_files(const std::string& rootName)
{
    std::vector<std::string> extList = { "EGRID", "INIT", "UNRST", "SMSPEC", "RFT" };

    for (const auto& ext : extList) {
        std::ifstream is(rootName + '.' + ext);
        if (is) {
            return true;
        }
    }

    return false;
}

//------------------------------------------------//

int main(int argc, char** argv) {
    bool integrationTest           = false;
    bool onlyLastSequence          = false;
    bool reportStepOnly            = false;
    bool printKeywords             = false;
    bool specificKeyword           = false;
    bool specificReportStepNumber  = false;
    bool specificFileType          = false;
    bool throwOnError              = true;
    bool restartFile              = false;
    bool acceptExtraKeywords       = false;
    bool analysis                  = false;
    char* keyword                  = nullptr;
    int c                          = 0;
    int reportStepNumber           = -1;
    std::string fileTypeString;

    while ((c = getopt(argc, argv, "hik:alnpt:Rr:xd")) != -1) {
        switch (c) {
        case 'a':
            analysis = true;
            throwOnError = false;
            break;
        case 'h':
            printHelp();
            return 0;
        case 'd':
            reportStepOnly = true;
            break;
        case 'i':
            integrationTest = true;
            break;
        case 'k':
            specificKeyword = true;
            keyword = optarg;
            break;
        case 'l':
            onlyLastSequence = true;
            break;
        case 'n':
            throwOnError = false;
            break;
        case 'p':
            printKeywords = true;
            break;
        case 'r':
            specificReportStepNumber=true;
            reportStepNumber = atoi(optarg);
            break;
        case 'R':
            restartFile = true;
            break;
        case 't':
            specificFileType = true;
            fileTypeString=optarg;
            break;
        case 'x':
            acceptExtraKeywords = true;
            break;
        case '?':
            if (optopt == 'k' || optopt == 'm' || optopt == 's') {
                std::cerr << "Option " << optopt << " requires a keyword as argument, see manual (-h) for more information." << std::endl;
                return EXIT_FAILURE;
            }
            else if (optopt == 't') {
                std::cerr << "Option t requires an ECLIPSE filetype as argument, see manual (-h) for more information." << std::endl;
                return EXIT_FAILURE;
            }
            else {
                std::cerr << "Unknown option." << std::endl;
                return EXIT_FAILURE;
            }
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    if (argc != argOffset + 4) {
        std::cerr << "Error: The number of options and arguments given is not correct. "
                  << "Please run compareECL -h to see manual." << std::endl;
        return EXIT_FAILURE;
    }

    std::string basename1 = argv[argOffset];
    std::string basename2 = argv[argOffset + 1];
    double absTolerance   = strtod(argv[argOffset + 2], nullptr);
    double relTolerance   = strtod(argv[argOffset + 3], nullptr);

    std::cout << "Comparing '" << basename1 << "' to '" << basename2 << "'." << std::endl;

    if (!has_result_files(basename1)){
        std::cerr << "No files found for reference case." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        ECLRegressionTest comparator(basename1, basename2, absTolerance, relTolerance);

        comparator.throwOnErrors(throwOnError);
        comparator.doAnalysis(analysis);
        comparator.setAcceptExtraKeywords(acceptExtraKeywords);

        if (integrationTest) {
            comparator.setIntegrationTest(true);
        }

        if (printKeywords) {
            comparator.setPrintKeywordOnly(printKeywords);
        }

        if (onlyLastSequence) {
            comparator.setOnlyLastReportNumber(true);
        }

        if (reportStepOnly) {
            comparator.setReportStepOnly(true);
        }

        if (specificKeyword) {
            comparator.compareSpesificKeyword(keyword);
        }

        if (specificReportStepNumber) {
            comparator.compareSpesificRstReportStepNumber(reportStepNumber);
        }

        if (restartFile) {
            comparator.setLoadBaseRunData(true);
        }

        comparator.loadGrids();

        if (integrationTest && specificFileType) {
            if (fileTypeString=="EGRID" || fileTypeString=="INIT" || fileTypeString=="RFT") {
                std::cerr << "Integration test and spesific file type, only valid for UNRST and SMRY" << std::endl;
                return EXIT_FAILURE;
            }
        }

        if (specificFileType) {
            if (fileTypeString == "EGRID") {
                comparator.gridCompare();
            } else if (fileTypeString == "INIT") {
                comparator.results_init();
            } else if (fileTypeString == "UNRST") {
                comparator.results_rst();
            } else if (fileTypeString == "SMRY") {
                comparator.results_smry();
            } else if (fileTypeString == "RFT") {
                comparator.results_rft();
            } else {
                std::cerr << "Unknown ECLIPSE filetype specified with option -t. Please run compareECL -h to see manual." << std::endl;
                return EXIT_FAILURE;
            }

        } else if (integrationTest) {
            comparator.results_rst();
            comparator.results_smry();
        } else {
            comparator.gridCompare();
            comparator.results_init();
            comparator.results_rst();
            comparator.results_smry();
            comparator.results_rft();
        }

        if (comparator.getNoErrors() > 0)
            OPM_THROW(std::runtime_error, comparator.getNoErrors() << " errors encountered in comparisons.");
    }

    catch (const std::exception& e) {
        std::cerr << "Program threw an exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\nProgram finished \n" << std::endl;

    return 0;
}
