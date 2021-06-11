#include <chrono>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <getopt.h>

#include <fstream>
#include <iostream>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ESmry.hpp>

using namespace Opm::EclIO;
using EclEntry = EclFile::EclEntry;


static void printHelp() {

    std::cout << "\nSmall test program used to test performance for ESmry class. Two arguments needed.\n"
              << "\nfirst name of smspec file, and second is vector list:\n"
              << "\nExample\n > test_esmry_lod -p out.txt TEST.SMSPEC \"FOPR FGPR\" \n\n"
              << "\nIn addition, the program takes these options (which must be given before the arguments):\n\n"
              << "-h Print help and exit.\n"
              << "-p write selected vectors to file .\n\n";
}


std::vector<std::string> splitString(std::string str){

    std::vector<std::string> res_vect;

    int p1 = 0;

    while (p1 != -1 ) {
        p1 = str.find_first_not_of(' ', p1);
        int p2 = str.find_first_of(" ", p1+1);
        res_vect.push_back(str.substr(p1, p2 - p1) );
        p1 = p2;
    }

    return res_vect;
}


int main(int argc, char **argv) {

    int c                          = 0;
    bool output                    = false;
    bool compare                   = false;
    bool loadOnDemand              = false;
    std::string outFileName        = "";
    std::string refFileName        = "";

    while ((c = getopt(argc, argv, "hp:c:l")) != -1) {
        switch (c) {
        case 'h':
            printHelp();
            return 0;
        case 'p':
            output=true;
            outFileName = optarg;
            break;
        case 'c':
            compare = true;
            refFileName = optarg;
            break;
        case 'l':
            loadOnDemand = true;
            break;

        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    if (compare) {

        std::cout << "\ncompare " << argv[argOffset] << " vs ref: " << refFileName << std::endl;

        std::string filename = argv[argOffset];

        auto lap1 = std::chrono::system_clock::now();

        ESmry smry1(filename, false);
        ESmry smry2(refFileName, false);

        smry1.LoadData();
        smry2.LoadData();

        auto kwlist1 = smry1.keywordList();
        auto kwlist2 = smry2.keywordList();

        if (kwlist1.size() !=kwlist2.size())
            throw std::runtime_error("size of keyword lists differ");

        for (size_t n = 0; n < kwlist1.size(); n++)
            if (kwlist1[n] != kwlist1[n])
                throw std::runtime_error("keyword element differ");

        std::cout << kwlist1.size() << " vs " << kwlist2.size() << std::endl;

        for (auto keyw : kwlist1){
            auto v1 = smry1.get(keyw);
            auto v2 = smry2.get(keyw);

            if (v1.size() != v2.size())
                throw std::runtime_error("size of vector differ");

            for (size_t m = 0; m < v1.size(); m++) {

                float absDiff = abs( v1[m]- v2[m]);

                if (v2[m] > 0.0)
                    absDiff = absDiff / v2[m];

                if (absDiff > 0.0000001) {
                    std::cout << "not equal. keyword: " << keyw << " v1: " << v1[m] << " vs v2: " << v2[m] << std::endl;
                    exit(1);
                }
            }
        }

        auto lap2 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds2 = lap2-lap1;


        std::cout << "runtime  for test : " << elapsed_seconds2.count() << " seconds\n" << std::endl;
        exit(0);
    }
    /*
    do {
        std::cout << '\n' << "Press a key to continue...";
    } while ( std::cin.get() != '\n' );
    */

    // start reading
    auto start = std::chrono::system_clock::now();
    std::string filename = argv[argOffset];


    ESmry smry1(filename, true);
    //ESmry smry1(filename, false);

    auto lap1 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds1 = lap1-start;


    if (argc < 3)
       throw std::invalid_argument("second argument should be a vector list, example \"FOPR FWCT\"");

    std::string inputVectors(argv[argOffset+1]);
    std::vector<std::string> vectKeyList = splitString(inputVectors);

    if (loadOnDemand) {
        std::cout << "\nloading vectors ";
        for (auto key : vectKeyList)
            std::cout << key << " ";

        std::cout << "\n" << std::endl;

        smry1.LoadData(vectKeyList);
    } else {

        std::cout << "\nloading all vectors \n" << std::endl;
        smry1.LoadData();
    }

    auto lap2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds2 = lap2-lap1;


    std::vector<std::vector<float>> dataList;

    for (auto key : vectKeyList){
        auto vect = smry1.get_at_rstep(key);
        dataList.push_back(vect);
    }

    auto lap3 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds3 = lap3-lap2;


    std::cout << "runtime  : " << argv[argOffset] << ": " << elapsed_seconds1.count() << " seconds\n" << std::endl;
    std::cout << "runtime  loading vector list: " << elapsed_seconds2.count() << " seconds\n" << std::endl;
    std::cout << "runtime  getting vectors vector list: " << elapsed_seconds3.count() << " seconds\n" << std::endl;

    std::chrono::duration<double> elapsed_total = lap3-start;
    std::cout << "runtime  total: " << elapsed_total.count() << " seconds\n" << std::endl;

    if (output){

        int colw = 15;
        std::cout << "write to file " << outFileName << std::endl;

        std::ofstream outfile;
        outfile.open(outFileName);

        outfile << std::setw(colw) << "ts";
        for (auto key : vectKeyList)
            outfile << std::setw(colw) << key;

        outfile << std::endl;


        for (size_t n =1; n < dataList.size(); n++)
            if (dataList[n].size() != dataList[n-1].size())
                throw std::runtime_error("size of vectors differs ");

        for (size_t t = 0; t < dataList[0].size(); t++){
            outfile << std::setw(colw) << t;

            for (size_t n = 0; n < dataList.size(); n++)
                outfile  << std::setw(colw) << std::scientific << std::setprecision(8) << dataList[n][t];

            outfile << std::endl;
        }

        outfile.close();
    }

    /*
    do {
        std::cout << '\n' << "Press a key to exit ...";
    } while ( std::cin.get() != '\n' );
    */

    return 0;
}
