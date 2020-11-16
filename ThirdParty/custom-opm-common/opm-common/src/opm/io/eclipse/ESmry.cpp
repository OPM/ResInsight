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

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <fnmatch.h>
#include <fstream>
#include <cmath>

/*

     KEYWORDS       WGNAMES        NUMS              |   PARAM index   Corresponding ERT key
     ------------------------------------------------+--------------------------------------------------
     WGOR           OP_1           0                 |        0        WGOR:OP_1
     FOPT           +-+-+-+-       0                 |        1        FOPT
     WWCT           OP_1           0                 |        2        WWCT:OP_1
     WIR            OP_1           0                 |        3        WIR:OP_1
     WGOR           WI_1           0                 |        4        WWCT:OP_1
     WWCT           W1_1           0                 |        5        WWCT:WI_1
     BPR            +-+-+-         12675             |        6        BPR:12675, BPR:i,j,k
     RPR            +-+-+-         1                 |        7        RPR:1
     FOPT           +-+-+-         0                 |        8        FOPT
     GGPR           NORTH          0                 |        9        GGPR:NORTH
     COPR           OP_1           5628              |       10        COPR:OP_1:56286, COPR:OP_1:i,j,k
     RXF            +-+-+-         32768*R1(R2 + 10) |       11        RXF:2-3
     SOFX           OP_1           12675             |       12        SOFX:OP_1:12675, SOFX:OP_1:i,j,jk

*/


namespace {

std::chrono::system_clock::time_point make_date(const std::vector<int>& datetime) {
    auto day = datetime[0];
    auto month = datetime[1];
    auto year = datetime[2];
    auto hour = 0;
    auto minute = 0;
    auto second = 0;

    if (datetime.size() == 6) {
        hour = datetime[3];
        minute = datetime[4];
        auto total_usec = datetime[5];
        second = total_usec / 1000000;
    }


    const auto ts = Opm::TimeStampUTC{ Opm::TimeStampUTC::YMD{ year, month, day}}.hour(hour).minutes(minute).seconds(second);
    return std::chrono::system_clock::from_time_t( Opm::asTimeT(ts) );
}


}


namespace Opm { namespace EclIO {

ESmry::ESmry(const std::string &filename, bool loadBaseRunData) :
    inputFileName { filename },
    summaryNodes { }
{

    Opm::filesystem::path rootName = inputFileName.parent_path() / inputFileName.stem();

    // if root name (without any extension) given as first argument in constructor, binary will then be assumed

    if (inputFileName.extension()=="")
        inputFileName+=".SMSPEC";

    if ((inputFileName.extension()!=".SMSPEC") && (inputFileName.extension()!=".FSMSPEC"))
        throw std::invalid_argument("Input file should have extension .SMSPEC or .FSMSPEC");

    const bool formatted = inputFileName.extension()==".SMSPEC" ? false : true;
    formattedFiles.push_back(formatted);

    Opm::filesystem::path path = Opm::filesystem::current_path();

    updatePathAndRootName(path, rootName);

    Opm::filesystem::path smspec_file = path / rootName;
    smspec_file += inputFileName.extension();

    Opm::filesystem::path rstRootN;
    Opm::filesystem::path pathRstFile = path;

    std::set<std::string> keywList;
    std::vector<std::pair<std::string,int>> smryArray;

    const std::unordered_set<std::string> segmentExceptions {
        "SEPARATE",
        "STEPTYPE",
        "SUMTHIN",
    } ;

    // Read data from the summary into local data members.
    {
        EclFile smspec(smspec_file.string());

        smspec.loadData();   // loading all data

        const std::vector<int> dimens = smspec.get<int>("DIMENS");

        nI = dimens[1]; // This is correct -- dimens[0] is something else!
        nJ = dimens[2];
        nK = dimens[3];

        const std::vector<std::string> restartArray = smspec.get<std::string>("RESTART");
        const std::vector<std::string> keywords = smspec.get<std::string>("KEYWORDS");
        const std::vector<std::string> wgnames = smspec.get<std::string>("WGNAMES");
        const std::vector<int> nums = smspec.get<int>("NUMS");
        const std::vector<std::string> units = smspec.get<std::string>("UNITS");

        std::vector<std::string> combindKeyList;
        combindKeyList.reserve(dimens[0]);

        this->startdat = make_date(smspec.get<int>("STARTDAT"));

        for (unsigned int i=0; i<keywords.size(); i++) {
            const std::string keyString = makeKeyString(keywords[i], wgnames[i], nums[i]);
            combindKeyList.push_back(keyString);

            if (keyString.length() > 0) {
                summaryNodes.push_back({
                    keywords[i],
                    SummaryNode::category_from_keyword(keywords[i], segmentExceptions),
                    SummaryNode::Type::Undefined,
                    wgnames[i],
                    nums[i]
                });

                keywList.insert(keyString);
                kwunits[keyString] = units[i];
            }
        }

        keywordListSpecFile.push_back(combindKeyList);
        getRstString(restartArray, pathRstFile, rstRootN);

        smryArray.push_back({smspec_file.string(), dimens[5]});
    }

    // checking if this is a restart run. Supporting nested restarts (restart, from restart, ...)
    // std::set keywList is storing keywords from all runs involved


    while ((rstRootN.string() != "") && (loadBaseRunData)) {

        Opm::filesystem::path rstFile = pathRstFile / rstRootN;
        rstFile += ".SMSPEC";

        bool baseRunFmt = false;

        // if unformatted file not exists, check for formatted file
        if (!Opm::filesystem::exists(rstFile)){
            rstFile = pathRstFile / rstRootN;
            rstFile += ".FSMSPEC";
            baseRunFmt = true;
        }

        EclFile smspec_rst(rstFile.string());
        smspec_rst.loadData();

        const std::vector<int> dimens = smspec_rst.get<int>("DIMENS");
        const std::vector<std::string> restartArray = smspec_rst.get<std::string>("RESTART");
        const std::vector<std::string> keywords = smspec_rst.get<std::string>("KEYWORDS");
        const std::vector<std::string> wgnames = smspec_rst.get<std::string>("WGNAMES");
        const std::vector<int> nums = smspec_rst.get<int>("NUMS");
        const std::vector<std::string> units = smspec_rst.get<std::string>("UNITS");

        std::vector<std::string> combindKeyList;
        combindKeyList.reserve(dimens[0]);

        this->startdat = make_date(smspec_rst.get<int>("STARTDAT"));

        for (size_t i = 0; i < keywords.size(); i++) {
            const std::string keyString = makeKeyString(keywords[i], wgnames[i], nums[i]);
            combindKeyList.push_back(keyString);
            if (keyString.length() > 0) {
                summaryNodes.push_back({
                    keywords[i],
                    SummaryNode::category_from_keyword(keywords[i], segmentExceptions),
                    SummaryNode::Type::Undefined,
                    wgnames[i],
                    nums[i]
                });

                keywList.insert(keyString);
                kwunits[keyString] = units[i];
            }
        }

        smryArray.push_back({rstFile.string(),dimens[5]});
        keywordListSpecFile.push_back(combindKeyList);
        formattedFiles.push_back(baseRunFmt);
        getRstString(restartArray, pathRstFile, rstRootN);
    }

    nSpecFiles = static_cast<int>(smryArray.size());
    nParamsSpecFile.resize(nSpecFiles, 0);

    // arrayPos std::vector of std::map, mapping position in summary file[n]
    for (int i = 0; i < nSpecFiles; i++)
        arrayPos.push_back({});

    std::map<std::string, int> keyIndex;
    {
        size_t m = 0;
        for (auto key : keywList)
            keyIndex[key] = m++;
    }

    int specInd = nSpecFiles - 1;

    while (specInd >= 0){

        auto smry = smryArray[specInd];

        EclFile smspec(std::get<0>(smry));
        smspec.loadData();

        const std::vector<int> dimens = smspec.get<int>("DIMENS");

        nI = dimens[1];
        nJ = dimens[2];
        nK = dimens[3];

        nParamsSpecFile[specInd] = dimens[0];

        const std::vector<std::string> keywords = smspec.get<std::string>("KEYWORDS");
        const std::vector<std::string> wgnames = smspec.get<std::string>("WGNAMES");
        const std::vector<int> nums = smspec.get<int>("NUMS");

        for (size_t i=0; i < keywords.size(); i++) {
            const std::string keyw = makeKeyString(keywords[i], wgnames[i], nums[i]);

            if (keywList.find(keyw) != keywList.end())
                arrayPos[specInd][keyIndex[keyw]]=i;
        }

        specInd--;
    }

    int fromReportStepNumber = 0;
    int toReportStepNumber;
    int step = 0;
    specInd = nSpecFiles - 1;
    nVect = keywList.size();

    int index = 0;
    for (const auto& keyw : keywList){
        keyword.push_back(keyw);
        keyword_index[keyw] = index++;
    }

    vectorData.reserve(nVect);
    vectorLoaded.reserve(nVect);

    for (size_t n = 0; n < nVect; n ++){
        vectorData.push_back({});
        vectorLoaded.push_back(false);
    }

    int dataFileIndex = -1;

    while (specInd >= 0){

        int reportStepNumber = fromReportStepNumber;

        if (specInd > 0) {
            auto rstFrom = smryArray[specInd-1];
            toReportStepNumber = std::get<1>(rstFrom);
        } else {
            toReportStepNumber = std::numeric_limits<int>::max();
        }

        Opm::filesystem::path smspecFile(std::get<0>(smryArray[specInd]));
        rootName = smspecFile.parent_path() / smspecFile.stem();

        // check if multiple or unified result files should be used
        // to import data, no information in smspec file regarding this
        // if both unified and non-unified files exists, will use most recent based on
        // time stamp

        Opm::filesystem::path unsmryFile = rootName;

        unsmryFile += formattedFiles[specInd] ? ".FUNSMRY" : ".UNSMRY";
        const bool use_unified = Opm::filesystem::exists(unsmryFile.string());

        const std::vector<std::string> multFileList = checkForMultipleResultFiles(rootName, formattedFiles[specInd]);

        std::vector<std::string> resultsFileList;

        if ((!use_unified) && (multFileList.size()==0)){
            throw std::runtime_error("neigther unified or non-unified result files found");
        } else if ((use_unified) && (multFileList.size()>0)){
            auto time_multiple = Opm::filesystem::last_write_time(multFileList.back());
            auto time_unified = Opm::filesystem::last_write_time(unsmryFile);

            if (time_multiple > time_unified){
                resultsFileList=multFileList;
            } else {
                resultsFileList.push_back(unsmryFile.string());
            }

        } else if (use_unified){
            resultsFileList.push_back(unsmryFile.string());
        } else {
            resultsFileList=multFileList;
        }

        // make array list with reference to source files (unifed or non unified)

        std::vector<ArrSourceEntry> arraySourceList;

        for (std::string fileName : resultsFileList)
        {
            std::vector<std::tuple <std::string, uint64_t>> arrayList;
            arrayList = this->getListOfArrays(fileName, formattedFiles[specInd]);

            for (size_t n = 0; n < arrayList.size(); n++) {
                ArrSourceEntry  t1 = std::make_tuple(std::get<0>(arrayList[n]), fileName, n, std::get<1>(arrayList[n]));
                arraySourceList.push_back(t1);
            }
        }

        // loop through arrays and for each ministep, store data file, location of params table
        //
        //    2 or 3 arrays pr time step.
        //       If timestep is a report step:  MINISTEP, PARAMS and SEQHDR
        //       else : MINISTEP and PARAMS


        size_t i = std::get<0>(arraySourceList[0]) == "SEQHDR" ? 1 : 0 ;

        while  (i < arraySourceList.size()){

            if (std::get<0>(arraySourceList[i]) != "MINISTEP"){
                std::string message="Reading summary file, expecting keyword MINISTEP, found '" + std::get<0>(arraySourceList[i]) + "'";
                throw std::invalid_argument(message);
            }

            if (std::get<0>(arraySourceList[i+1]) != "PARAMS") {
                std::string message="Reading summary file, expecting keyword PARAMS, found '" + std::get<0>(arraySourceList[i]) + "'";
                throw std::invalid_argument(message);
            }

            i++;

            if (std::find(dataFileList.begin(), dataFileList.end(), std::get<1>(arraySourceList[i])) == dataFileList.end())
            {
                dataFileList.push_back(std::get<1>(arraySourceList[i]));
                dataFileIndex++;
            }

            TimeStepEntry t1 = std::make_tuple(specInd, dataFileIndex, std::get<3>(arraySourceList[i]));
            timeStepList.push_back(t1);

            i++;

            if (i < arraySourceList.size()){
                if (std::get<0>(arraySourceList[i]) == "SEQHDR") {
                    i++;
                    reportStepNumber++;
                    seqIndex.push_back(step);
                }
            } else {
                reportStepNumber++;
                seqIndex.push_back(step);
            }

            if (reportStepNumber >= toReportStepNumber) {
                i = arraySourceList.size();
            }

            step++;
        }

        fromReportStepNumber = toReportStepNumber;

        specInd--;
    }
}


void ESmry::LoadData(const std::vector<std::string>& vectList) const
{
    size_t nvect = vectList.size();
    size_t ntstep = timeStepList.size();

    std::vector<int> keywIndVect;
    keywIndVect.reserve(nvect);

    for (auto key : vectList){
        if (!hasKey(key))
            OPM_THROW(std::invalid_argument, "error loading key " + key );

        auto it = keyword_index.find(key);
        keywIndVect.push_back(it->second);
    }

    for (auto ind : keywIndVect)
        vectorData[ind].reserve(ntstep);

    std::fstream fileH;

    auto specInd = std::get<0>(timeStepList[0]);
    auto dataFileIndex = std::get<1>(timeStepList[0]);
    uint64_t stepFilePos = std::get<2>(timeStepList[0]);

    if (formattedFiles[specInd])
        fileH.open(dataFileList[dataFileIndex], std::ios::in);
    else
        fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);

    for (auto ministep : timeStepList) {
        if (dataFileIndex != std::get<1>(ministep)) {
            fileH.close();
            specInd = std::get<0>(ministep);
            dataFileIndex = std::get<1>(ministep);

            if (formattedFiles[specInd])
                fileH.open(dataFileList[dataFileIndex], std::ios::in );
            else
                fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);
        }

        stepFilePos = std::get<2>(ministep);;

        for (auto ind : keywIndVect) {
            auto it = arrayPos[specInd].find(ind);

            if (it == arrayPos[specInd].end()) {
                // undefined vector in current summary file. Typically when loading
                // base restart run and including base run data. Vectors can be added to restart runs
                vectorData[ind].push_back(nanf(""));
            } else {
                int paramPos = it->second;

                if (formattedFiles[specInd]) {
                    uint64_t elementPos = 0;
                    int nBlocks = paramPos /MaxBlockSizeReal;
                    int sizeOfLastBlock = paramPos %  MaxBlockSizeReal;

                    if (nBlocks > 0) {
                        int nLinesBlock = MaxNumBlockReal / numColumnsReal;
                        int rest = MaxNumBlockReal % numColumnsReal;

                        if (rest > 0)
                            nLinesBlock++;

                        uint64_t blockSize = static_cast<uint64_t>(MaxNumBlockReal * numColumnsReal + nLinesBlock);
                        elementPos = static_cast<uint64_t>(nBlocks * blockSize);
                    }

                    int nLines = sizeOfLastBlock / numColumnsReal;
                    elementPos = stepFilePos + elementPos + static_cast<uint64_t>(sizeOfLastBlock * columnWidthReal + nLines);

                    fileH.seekg (elementPos, fileH.beg);

                    char* buffer;
                    size_t size = columnWidthReal;
                    buffer = new char [size];
                    fileH.read (buffer, size);
                    double dtmpv = std::stod(std::string(buffer, size));
                    vectorData[ind].push_back(static_cast<float>(dtmpv));

                    delete[] buffer;

                } else {

                    uint64_t nFullBlocks = static_cast<uint64_t>(paramPos/(MaxBlockSizeReal / sizeOfReal));
                    uint64_t elementPos = ((2 * nFullBlocks) + 1)*static_cast<uint64_t>(sizeOfInte);
                    elementPos += static_cast<uint64_t>(paramPos)* static_cast<uint64_t>(sizeOfReal) + stepFilePos;
                    fileH.seekg (elementPos, fileH.beg);

                    float value;
                    fileH.read(reinterpret_cast<char*>(&value), sizeOfReal);
                    vectorData[ind].push_back(Opm::EclIO::flipEndianFloat(value));
                }
            }
        }
    }

    fileH.close();

    for (auto ind : keywIndVect)
        vectorLoaded[ind] = true;
}

std::vector<int> ESmry::makeKeywPosVector(int specInd) const {

    std::vector<int> keywpos;
    keywpos.reserve(nParamsSpecFile[specInd]);

    for (int n = 0; n < nParamsSpecFile[specInd]; n++){
        std::string tmpstr = keywordListSpecFile[specInd][n];
        auto it = keyword_index.find(tmpstr);

        if (it == keyword_index.end())
            keywpos.push_back(-1);
        else
            if (std::find(keywpos.begin(), keywpos.end(), it->second) != keywpos.end())
                keywpos.push_back(-1);
            else
               keywpos.push_back(it->second);
    }

    return keywpos;
}

void ESmry::LoadData() const
{
    std::fstream fileH;

    auto specInd = std::get<0>(timeStepList[0]);
    auto dataFileIndex = std::get<1>(timeStepList[0]);
    uint64_t stepFilePos = std::get<2>(timeStepList[0]);

    std::vector<int> keywpos = makeKeywPosVector(specInd);

    if (formattedFiles[specInd])
        fileH.open(dataFileList[dataFileIndex], std::ios::in);
    else
        fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);

    for (auto ministep : timeStepList) {

        if (dataFileIndex != std::get<1>(ministep)) {
            fileH.close();

            if (specInd != std::get<0>(ministep)){
                specInd = std::get<0>(ministep);
                keywpos = makeKeywPosVector(specInd);
            }

            dataFileIndex = std::get<1>(ministep);

            if (formattedFiles[specInd])
                fileH.open(dataFileList[dataFileIndex], std::ios::in );
            else
                fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);
        }

        stepFilePos = std::get<2>(ministep);
        int maxNumberOfElements = MaxBlockSizeReal / sizeOfReal;
        fileH.seekg (stepFilePos, fileH.beg);

        if (formattedFiles[specInd]) {
            char* buffer;
            size_t size = sizeOnDiskFormatted(nParamsSpecFile[specInd], Opm::EclIO::REAL)+1;
            buffer = new char [size];
            fileH.read (buffer, size);

            std::string fileStr = std::string(buffer, size);
            size_t p = 0;
            int64_t p1= 0;

            for (int i=0; i< nParamsSpecFile[specInd]; i++) {
                p1 = fileStr.find_first_not_of(' ',p1);
                int64_t p2 = fileStr.find_first_of(' ', p1);

                if (keywpos[p] > -1) {
                    double dtmpv = std::stod(fileStr.substr(p1, p2-p1));
                    vectorData[keywpos[p]].push_back(static_cast<float>(dtmpv));
                }

                p1 = fileStr.find_first_not_of(' ',p2);
                p++;
            }

            delete[] buffer;
        } else {
            int64_t rest = static_cast<int64_t>(nParamsSpecFile[specInd]);
            size_t p = 0;

            while (rest > 0) {
                int dhead;
                fileH.read(reinterpret_cast<char*>(&dhead), sizeof(dhead));
                dhead = Opm::EclIO::flipEndianInt(dhead);
                int num = dhead / sizeOfInte;

                if ((num > maxNumberOfElements) || (num < 0))
                    OPM_THROW(std::runtime_error, "??Error reading binary data, inconsistent header data or incorrect number of elements");

                for (int i = 0; i < num; i++) {
                    float value;
                    fileH.read(reinterpret_cast<char*>(&value), sizeOfReal);

                    if (keywpos[p] > -1)
                        vectorData[keywpos[p]].push_back(Opm::EclIO::flipEndianFloat(value));

                    p++;
                }

                rest -= num;

                if (( num < maxNumberOfElements && rest != 0) ||
                        (num == maxNumberOfElements && rest < 0)) {
                    std::string message = "Error reading binary data, incorrect number of elements";
                    OPM_THROW(std::runtime_error, message);
                }

                int dtail;
                fileH.read(reinterpret_cast<char*>(&dtail), sizeof(dtail));
                dtail = Opm::EclIO::flipEndianInt(dtail);

                if (dhead != dtail)
                    OPM_THROW(std::runtime_error, "Error reading binary data, tail not matching header.");
            }
        }
    }

    for (size_t n=0; n < nVect; n++)
        vectorLoaded[n] = true;
}


std::vector<std::tuple <std::string, uint64_t>>
ESmry::getListOfArrays(std::string filename, bool formatted)
{
    std::vector<std::tuple <std::string, uint64_t>> resultVect;
    std::fstream fileH;

    if (formatted)
        fileH.open(filename, std::ios::in);
    else
        fileH.open(filename, std::ios::in |  std::ios::binary);

    while (!Opm::EclIO::isEOF(&fileH)) {
        std::string arrName(8,' ');
        Opm::EclIO::eclArrType arrType;

        int64_t num;
        if (formatted)
            Opm::EclIO::readFormattedHeader(fileH,arrName,num,arrType);
        else
            Opm::EclIO::readBinaryHeader(fileH,arrName,num,arrType);

        uint64_t filePos = fileH.tellg();

        std::tuple <std::string, uint64_t> t1;
        t1 = std::make_tuple(Opm::EclIO::trimr(arrName), filePos);
        resultVect.push_back(t1);

        if (num > 0) {
            if (formatted) {
                uint64_t sizeOfNextArray = sizeOnDiskFormatted(num, arrType);
                fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);

            } else {
                uint64_t sizeOfNextArray = sizeOnDiskBinary(num, arrType);
                fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);
            }
        }
    }

    fileH.close();

    return resultVect;
}


std::vector<std::string> ESmry::checkForMultipleResultFiles(const Opm::filesystem::path& rootN, bool formatted) const {

    std::vector<std::string> fileList;
    const std::string pathRootN = rootN.parent_path().string();

    const std::string fileFilter = formatted ? rootN.stem().string()+".A" : rootN.stem().string()+".S";

    for (Opm::filesystem::directory_iterator itr(pathRootN); itr != Opm::filesystem::directory_iterator(); ++itr)
    {
        const std::string file = itr->path().filename().string();

        if ((file.find(fileFilter) != std::string::npos) && (file.find("SMSPEC") == std::string::npos)) {
            fileList.push_back(pathRootN + "/" + file);
        }
    }

    std::sort(fileList.begin(), fileList.end());

    return fileList;
}

void ESmry::getRstString(const std::vector<std::string>& restartArray, Opm::filesystem::path& pathRst, Opm::filesystem::path& rootN) const {

    std::string rootNameStr="";

    for (const auto& str : restartArray) {
        rootNameStr = rootNameStr + str;
    }

    rootN = Opm::filesystem::path(rootNameStr);

    updatePathAndRootName(pathRst, rootN);
}

void ESmry::updatePathAndRootName(Opm::filesystem::path& dir, Opm::filesystem::path& rootN) const {

    if (rootN.parent_path().is_absolute()){
        dir = rootN.parent_path();
    } else {
        dir = dir / rootN.parent_path();
    }

    rootN = rootN.stem();
}

bool ESmry::hasKey(const std::string &key) const
{
    return std::find(keyword.begin(), keyword.end(), key) != keyword.end();
}


void ESmry::ijk_from_global_index(int glob,int &i,int &j,int &k) const
{
    const int tmpGlob = glob - 1;

    k = 1 + tmpGlob / (nI * nJ);
    const int rest = tmpGlob % (nI * nJ);

    j = 1 + rest / nI;
    i = 1 + rest % nI;
}


std::string ESmry::makeKeyString(const std::string& keywordArg, const std::string& wgname, int num) const
{
    std::string keyStr;
    const std::vector<std::string> segmExcep= {"STEPTYPE", "SEPARATE", "SUMTHIN"};

    if (keywordArg.substr(0, 1) == "A") {
        keyStr = keywordArg + ":" + std::to_string(num);
    } else if (keywordArg.substr(0, 1) == "B") {
        int _i,_j,_k;
        ijk_from_global_index(num, _i, _j, _k);

        keyStr = keywordArg + ":" + std::to_string(_i) + "," + std::to_string(_j) + "," + std::to_string(_k);

    } else if (keywordArg.substr(0, 1) == "C") {
        if (num > 0) {
            int _i,_j,_k;
            ijk_from_global_index(num, _i, _j, _k);
            keyStr = keywordArg + ":" + wgname+ ":" + std::to_string(_i) + "," + std::to_string(_j) + "," + std::to_string(_k);
        }
    } else if (keywordArg.substr(0, 1) == "G") {
        if ( wgname != ":+:+:+:+") {
            keyStr = keywordArg + ":" + wgname;
        }
    } else if (keywordArg.substr(0, 1) == "R" && keywordArg.substr(2, 1) == "F") {
        // NUMS = R1 + 32768*(R2 + 10)
        int r2 = 0;
        int y = 32768 * (r2 + 10) - num;

        while (y <0 ) {
            r2++;
            y = 32768 * (r2 + 10) - num;
        }

        r2--;
        const int r1 = num - 32768 * (r2 + 10);

        keyStr = keywordArg + ":" + std::to_string(r1) + "-" + std::to_string(r2);
    } else if (keywordArg.substr(0, 1) == "R") {
        keyStr = keywordArg + ":" + std::to_string(num);
    } else if (keywordArg.substr(0, 1) == "S") {
        auto it = std::find(segmExcep.begin(), segmExcep.end(), keywordArg);
        if (it != segmExcep.end()) {
            keyStr = keywordArg;
        } else {
            keyStr = keywordArg + ":" + wgname + ":" + std::to_string(num);
        }
    } else if (keywordArg.substr(0,1) == "W") {
        if (wgname != ":+:+:+:+") {
            keyStr = keywordArg + ":" + wgname;
        }
    } else {
        keyStr = keywordArg;
    }

    return keyStr;
}

std::string ESmry::unpackNumber(const SummaryNode& node) const {
    if (node.category == SummaryNode::Category::Block ||
        node.category == SummaryNode::Category::Connection) {
        int _i,_j,_k;
        ijk_from_global_index(node.number, _i, _j, _k);

        return std::to_string(_i) + "," + std::to_string(_j) + "," + std::to_string(_k);
    } else if (node.category == SummaryNode::Category::Region && node.keyword[2] == 'F') {
        const auto r1 =  node.number % (1 << 15);
        const auto r2 = (node.number / (1 << 15)) - 10;

        return std::to_string(r1) + "-" + std::to_string(r2);
    } else {
        return std::to_string(node.number);
    }
}

std::string ESmry::lookupKey(const SummaryNode& node) const {
    return node.unique_key(std::bind( &ESmry::unpackNumber, this, std::placeholders::_1 ));
}

const std::vector<float>& ESmry::get(const SummaryNode& node) const {
    return get(lookupKey(node));
}

std::vector<float> ESmry::get_at_rstep(const SummaryNode& node) const {
    return get_at_rstep(lookupKey(node));
}

const std::string& ESmry::get_unit(const SummaryNode& node) const {
    return get_unit(lookupKey(node));
}

const std::vector<float>& ESmry::get(const std::string& name) const
{
    auto it = std::find(keyword.begin(), keyword.end(), name);

    if (it == keyword.end()) {
        const std::string message="keyword " + name + " not found ";
        OPM_THROW(std::invalid_argument, message);
    }

    int ind = std::distance(keyword.begin(), it);

    if (!vectorLoaded[ind]){
        LoadData({name});
        vectorLoaded[ind]=true;
    }

    return vectorData[ind];
}

std::vector<float> ESmry::get_at_rstep(const std::string& name) const
{
    return this->rstep_vector( this->get(name) );
}


int ESmry::timestepIdxAtReportstepStart(const int reportStep) const
{
    const auto nReport = static_cast<int>(seqIndex.size());

    if ((reportStep < 1) || (reportStep > nReport)) {
        throw std::invalid_argument {
            "Report step " + std::to_string(reportStep)
            + " outside valid range 1 .. " + std::to_string(nReport)
        };
    }

    return seqIndex[reportStep - 1];
}

const std::string& ESmry::get_unit(const std::string& name) const {
    return kwunits.at(name);
}

const std::vector<std::string>& ESmry::keywordList() const
{
    return keyword;
}

std::vector<std::string> ESmry::keywordList(const std::string& pattern) const
{
    std::vector<std::string> list;

    for (auto key : keyword)
        if (fnmatch( pattern.c_str(), key.c_str(), 0 ) == 0 )
            list.push_back(key);

    return list;
}



const std::vector<SummaryNode>& ESmry::summaryNodeList() const {
    return summaryNodes;
}

std::vector<std::chrono::system_clock::time_point> ESmry::dates() const {
    double time_unit = 24 * 3600;
    std::vector<std::chrono::system_clock::time_point> d;

    using namespace std::chrono;
    using TP      = time_point<system_clock>;
    using DoubSec = duration<double, seconds::period>;

    for (const auto& t : this->get("TIME"))
        d.push_back( this->startdat + duration_cast<TP::duration>(DoubSec(t * time_unit)));

    return d;
}

std::vector<std::chrono::system_clock::time_point> ESmry::dates_at_rstep() const {
    const auto& full_vector = this->dates();
    return this->rstep_vector(full_vector);
}


}} // namespace Opm::ecl
