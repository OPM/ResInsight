#include <chrono>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <getopt.h>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

using namespace Opm::EclIO;
using EclEntry = EclFile::EclEntry;

template <typename T>
void write(EclOutput& outFile, EclFile& file1,
           const std::string& name, int index)
{
    auto vect = file1.get<T>(index);
    outFile.write(name, vect);
}


template<typename T>
void write(EclOutput& outFile, ERst& file1,
           const std::string& name, int index, int reportStepNumber)
{
    auto vect = file1.getRst<T>(index, reportStepNumber);
    outFile.write(name, vect);
}

template<typename T>
void write(EclOutput& outFile, ERst& file1,
           const std::string& name, int index)
{
    auto vect = file1.get<T>(index);
    outFile.write(name, vect);
}


template <typename T>
void writeArray(std::string name, eclArrType arrType, T& file1, int index, EclOutput& outFile) {

    if (arrType == INTE) {
        write<int>(outFile, file1, name, index);
    } else if (arrType == REAL) {
        write<float>(outFile, file1, name,index);
    } else if (arrType == DOUB) {
        write<double>(outFile, file1, name, index);
    } else if (arrType == LOGI) {
        write<bool>(outFile, file1, name, index);
    } else if (arrType == CHAR) {
        write<std::string>(outFile, file1, name, index);
    } else if (arrType == MESS) {
        outFile.message(name);
    } else {
        std::cout << "unknown array type " << std::endl;
        exit(1);
    }
}

template <typename T>
void writeArray(std::string name, eclArrType arrType, T& file1, int index, int reportStepNumber, EclOutput& outFile) {

    if (arrType == INTE) {
        write<int>(outFile, file1, name, index, reportStepNumber);
    } else if (arrType == REAL) {
        write<float>(outFile, file1, name, index, reportStepNumber);
    } else if (arrType == DOUB) {
        write<double>(outFile, file1, name, index, reportStepNumber);
    } else if (arrType == LOGI) {
        write<bool>(outFile, file1, name, index, reportStepNumber);
    } else if (arrType == CHAR) {
        write<std::string>(outFile, file1, name, index, reportStepNumber);
    } else if (arrType == MESS) {
        outFile.message(name);
    } else {
        std::cout << "unknown array type " << std::endl;
        exit(1);
    }
}

void writeArrayList(std::vector<EclEntry>& arrayList, EclFile file1, EclOutput& outFile) {

    for (size_t index = 0; index < arrayList.size(); index++) {
        std::string name = std::get<0>(arrayList[index]);
        eclArrType arrType = std::get<1>(arrayList[index]);
        writeArray(name, arrType, file1, index, outFile);
    }
}

void writeArrayList(std::vector<EclEntry>& arrayList, ERst file1, int reportStepNumber, EclOutput& outFile) {

    for (size_t index = 0; index < arrayList.size(); index++) {
        std::string name = std::get<0>(arrayList[index]);
        eclArrType arrType = std::get<1>(arrayList[index]);
        writeArray(name, arrType, file1, index , reportStepNumber, outFile);
    }
}

static void printHelp() {

    std::cout << "\nconvertECL needs one argument which is the input file to be converted. If this is a binary file the output file will be formatted. If the input file is formatted the output will be binary. \n"
              << "\nIn addition, the program takes these options (which must be given before the arguments):\n\n"
              << "-h Print help and exit.\n"
              << "-l list report step numbers in the selected restart file.\n"
              << "-r extract and convert a spesific report time step number from a unified restart file. \n\n";
}

int main(int argc, char **argv) {

    int c                          = 0;
    int reportStepNumber           = -1;
    bool specificReportStepNumber  = false;
    bool listProperties            = false;

    while ((c = getopt(argc, argv, "hr:l")) != -1) {
        switch (c) {
        case 'h':
            printHelp();
            return 0;
        case 'l':
            listProperties=true;
            break;
        case 'r':
            specificReportStepNumber=true;
            reportStepNumber = atoi(optarg);
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int argOffset = optind;

    // start reading
    auto start = std::chrono::system_clock::now();
    std::string filename = argv[argOffset];

    EclFile file1(filename);
    bool formattedOutput = file1.formattedInput() ? false : true;

    int p = filename.find_last_of(".");
    int l = filename.length();

    std::string rootN = filename.substr(0,p);
    std::string extension = filename.substr(p,l-p);
    std::string resFile;

    if (listProperties) {

        if (extension==".UNRST") {

            ERst rst1(filename);
            rst1.loadData("INTEHEAD");

            std::vector<int> reportStepList=rst1.listOfReportStepNumbers();

            for (auto seqn : reportStepList) {

                std::vector<int> inteh = rst1.getRst<int>("INTEHEAD", seqn, 0);

                std::cout << "Report step number: "
                          << std::setfill(' ') << std::setw(4) << seqn << "   Date: " << inteh[66] << "/"
                          << std::setfill('0') << std::setw(2) << inteh[65] << "/"
                          << std::setfill('0') << std::setw(2) << inteh[64] << std::endl;
            }

            std::cout << std::endl;

        } else {
            std::cout << "\n!ERROR, option -l only only available for unified restart files (*.UNRST) " << std::endl;
            exit(1);
        }

        return 0;
    }

    std::map<std::string, std::string> to_formatted = {{".EGRID", ".FEGRID"}, {".INIT", ".FINIT"}, {".SMSPEC", ".FSMSPEC"},
        {".UNSMRY", ".FUNSMRY"}, {".UNRST", ".FUNRST"}, {".RFT", ".FRFT"}, {".LODSMRY", ".FLODSMRY"}};

    std::map<std::string, std::string> to_binary = {{".FEGRID", ".EGRID"}, {".FINIT", ".INIT"}, {".FSMSPEC", ".SMSPEC"},
        {".FUNSMRY", ".UNSMRY"}, {".FUNRST", ".UNRST"}, {".FRFT", ".RFT"}, {".FLODSMRY", ".LODSMRY"}};

    if (formattedOutput) {

        auto search = to_formatted.find(extension);

        if (search != to_formatted.end()){
            resFile = rootN + search->second;
        } else if (extension.substr(1,1)=="X"){
            resFile = rootN + ".F" + extension.substr(2);
        } else if (extension.substr(1,1)=="S"){
            resFile = rootN + ".A" + extension.substr(2);
        } else {
            std::cout << "\n!ERROR, unknown file type for input file '" << rootN + extension << "'\n" << std::endl;
            exit(1);
        }
    } else {

        auto search = to_binary.find(extension);

        if (search != to_binary.end()){
            resFile = rootN + search->second;
        } else if (extension.substr(1,1)=="F"){
            resFile = rootN + ".X" + extension.substr(2);
        } else if (extension.substr(1,1)=="A"){
            resFile = rootN + ".S" + extension.substr(2);
        } else {
            std::cout << "\n!ERROR, unknown file type for input file '" << rootN + extension << "'\n" << std::endl;
            exit(1);
        }
    }

    std::cout << "\033[1;31m" << "\nconverting  " << argv[argOffset] << " -> " << resFile << "\033[0m\n" << std::endl;

    EclOutput outFile(resFile, formattedOutput);

    if (specificReportStepNumber) {

        if (extension!=".UNRST") {
            std::cout << "\n!ERROR, option -r only can only be used with unified restart files (*.UNRST) " << std::endl;
            exit(1);
        }

        ERst rst1(filename);

        if (!rst1.hasReportStepNumber(reportStepNumber)) {
            std::cout << "\n!ERROR, selected unified restart file doesn't have report step number " << reportStepNumber << "\n" << std::endl;
            exit(1);
        }

        rst1.loadReportStepNumber(reportStepNumber);

        auto arrayList = rst1.listOfRstArrays(reportStepNumber);

        writeArrayList(arrayList, rst1, reportStepNumber, outFile);

    } else {

        file1.loadData();
        auto arrayList = file1.getList();

        writeArrayList(arrayList, file1, outFile);
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "runtime  : " << argv[argOffset] << ": " << elapsed_seconds.count() << " seconds\n" << std::endl;

    return 0;
}
