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

#include <opm/common/ErrorMacros.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>


#ifdef _WIN32
#include "cross-platform/windows/Substitutes.hpp"
#else
#include <fnmatch.h>
#endif

#include <fmt/format.h>

/*

     KEYWORDS       WGNAMES        NUMS                 |   PARAM index   Corresponding ERT key
     ---------------------------------------------------+--------------------------------------------------
     WGOR           OP_1           0                    |        0        WGOR:OP_1
     FOPT           :+:+:+:+       0                    |        1        FOPT
     WWCT           OP_1           0                    |        2        WWCT:OP_1
     WIR            OP_1           0                    |        3        WIR:OP_1
     WGOR           WI_1           0                    |        4        WWCT:OP_1
     WWCT           W1_1           0                    |        5        WWCT:WI_1
     BPR            :+:+:+:+       12675                |        6        BPR:12675, BPR:i,j,k
     RPR            :+:+:+:+       1                    |        7        RPR:1
     FOPT           :+:+:+:+       0                    |        8        FOPT
     GGPR           NORTH          0                    |        9        GGPR:NORTH
     COPR           OP_1           5628                 |       10        COPR:OP_1:56286, COPR:OP_1:i,j,k
     RXF            :+:+:+:+       R1 + 32768*(R2 + 10) |       11        RXF:2-3
     SOFX           OP_1           12675                |       12        SOFX:OP_1:12675, SOFX:OP_1:i,j,jk
     AAQX           :+:+:+:+       12                   |       13        AAQX:12

*/


namespace {

Opm::time_point make_date(const std::vector<int>& datetime) {
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
    return Opm::TimeService::from_time_t( Opm::asTimeT(ts) );
}


}


namespace Opm { namespace EclIO {

ESmry::ESmry(const std::string &filename, bool loadBaseRunData) :
    inputFileName { filename },
    summaryNodes { }
{
    fromSingleRun = !loadBaseRunData;

    Opm::filesystem::path rootName = inputFileName.parent_path() / inputFileName.stem();

    // if only root name (without any extension) given as first argument in constructor
    // binary will then be assumed

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

    std::vector<EclFile> smspecList;
    std::vector<std::string> vectList = {"DIMENS", "RESTART", "KEYWORDS", "NUMS", "UNITS"};

    // Read data from the summary into local data members.
    {
        smspecList.emplace_back(smspec_file.string());

        auto arrays = smspecList.back().getList();
        std::vector<int> vectIndices;

        for (size_t n = 0; n < arrays.size(); n++)
            if(std::find(vectList.begin(), vectList.end(), std::get<0>(arrays[n])) != vectList.end())
               vectIndices.push_back(static_cast<int>(n));

        smspecList.back().loadData(vectIndices);

        const std::vector<int> dimens = smspecList.back().get<int>("DIMENS");

        nI = dimens[1]; // This is correct -- dimens[0] is something else!
        nJ = dimens[2];
        nK = dimens[3];

        const std::vector<std::string> restartArray = smspecList.back().get<std::string>("RESTART");
        const std::vector<std::string> keywords = smspecList.back().get<std::string>("KEYWORDS");
        std::vector<std::string> wgnames;

        if (smspecList.back().hasKey("WGNAMES"))
            wgnames = smspecList.back().get<std::string>("WGNAMES");
        else
            wgnames = smspecList.back().get<std::string>("NAMES");

        const std::vector<int> nums = smspecList.back().get<int>("NUMS");
        const std::vector<std::string> units = smspecList.back().get<std::string>("UNITS");

        std::vector<std::string> lgrs;
        std::vector<int> numlx;
        std::vector<int> numly;
        std::vector<int> numlz;

        if (smspecList.back().hasKey("LGRS")) lgrs = smspecList.back().get<std::string>("LGRS");
        if (smspecList.back().hasKey("NUMLX")) numlx = smspecList.back().get<int>("NUMLX");
        if (smspecList.back().hasKey("NUMLY")) numly = smspecList.back().get<int>("NUMLY");
        if (smspecList.back().hasKey("NUMLZ")) numlz = smspecList.back().get<int>("NUMLZ");

        std::vector<std::string> combindKeyList;
        combindKeyList.reserve(dimens[0]);

        this->startdat = make_date(smspecList.back().get<int>("STARTDAT"));

        for (unsigned int i=0; i<keywords.size(); i++) {

            std::string lgrName;
            int lgri = -1;
            int lgrj = -1;
            int lgrk = -1;

            if (i < lgrs.size()) lgrName = lgrs[i];
            if (i < numlx.size()) lgri = numlx[i];
            if (i < numly.size()) lgrj = numly[i];
            if (i < numlz.size()) lgrk = numlz[i];

            const std::string keyString = makeKeyString(keywords[i], wgnames[i], nums[i]);
            combindKeyList.push_back(keyString);

            if (! keyString.empty()) {
                summaryNodes.push_back({
                    keywords[i],
                    SummaryNode::category_from_keyword(keywords[i], segmentExceptions),
                    SummaryNode::Type::Undefined,
                    wgnames[i],
                    nums[i],
                    lgrName,
                    lgri,
                    lgrj,
                    lgrk,
                    i,
                    ""
                });

                keywList.insert(keyString);
                kwunits[keyString] = units[i];
            }
        }

        keywordListSpecFile.push_back(combindKeyList);
        getRstString(restartArray, pathRstFile, rstRootN);

        if ((rstRootN.string() != "") && (loadBaseRunData)){

            if (! Opm::filesystem::exists(pathRstFile))
                OPM_THROW(std::runtime_error, "path to restart file not found, '" + pathRstFile.string() + "'");

            auto abs_rst_file = Opm::filesystem::canonical(pathRstFile) / rstRootN;
            Opm::filesystem::path rel_path;

            if (inputFileName.parent_path().string().empty())
                rel_path = Opm::proximate(abs_rst_file);
            else
                rel_path =  Opm::proximate(abs_rst_file, inputFileName.parent_path());

            if (abs_rst_file.string().size() < rel_path.string().size())
                restart_info = std::make_tuple(abs_rst_file.string(), dimens[5]);
            else
                restart_info = std::make_tuple(rel_path.string(), dimens[5]);
        }
        else
            restart_info = std::make_tuple("", 0);

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

        smspecList.emplace_back(EclFile(rstFile.string()));

        auto arrays = smspecList.back().getList();
        std::vector<int> vectIndices;

        for (size_t n = 0; n < arrays.size(); n++)
            if(std::find(vectList.begin(), vectList.end(), std::get<0>(arrays[n])) != vectList.end())
               vectIndices.push_back(static_cast<int>(n));

        smspecList.back().loadData(vectIndices);

        const std::vector<int> dimens = smspecList.back().get<int>("DIMENS");
        const std::vector<std::string> restartArray = smspecList.back().get<std::string>("RESTART");
        const std::vector<std::string> keywords = smspecList.back().get<std::string>("KEYWORDS");
        std::vector<std::string> wgnames;

        if (smspecList.back().hasKey("WGNAMES"))
            wgnames = smspecList.back().get<std::string>("WGNAMES");
        else
            wgnames = smspecList.back().get<std::string>("NAMES");

        const std::vector<int> nums = smspecList.back().get<int>("NUMS");
        const std::vector<std::string> units = smspecList.back().get<std::string>("UNITS");

        std::vector<std::string> lgrs;
        std::vector<int> numlx;
        std::vector<int> numly;
        std::vector<int> numlz;

        if (smspecList.back().hasKey("LGRS")) lgrs = smspecList.back().get<std::string>("LGRS");
        if (smspecList.back().hasKey("NUMLX")) numlx = smspecList.back().get<int>("NUMLX");
        if (smspecList.back().hasKey("NUMLY")) numly = smspecList.back().get<int>("NUMLY");
        if (smspecList.back().hasKey("NUMLZ")) numlz = smspecList.back().get<int>("NUMLZ");

        std::vector<std::string> combindKeyList;
        combindKeyList.reserve(dimens[0]);

        this->startdat = make_date(smspecList.back().get<int>("STARTDAT"));

        for (size_t i = 0; i < keywords.size(); i++) {

            std::string lgrName;
            int lgri = -1;
            int lgrj = -1;
            int lgrk = -1;

            if (i < lgrs.size()) lgrName = lgrs[i];
            if (i < numlx.size()) lgri = numlx[i];
            if (i < numly.size()) lgrj = numly[i];
            if (i < numlz.size()) lgrk = numlz[i];

            const std::string keyString = makeKeyString(keywords[i], wgnames[i], nums[i]);
            combindKeyList.push_back(keyString);

            if (! keyString.empty()) {
                summaryNodes.push_back({
                    keywords[i],
                    SummaryNode::category_from_keyword(keywords[i], segmentExceptions),
                    SummaryNode::Type::Undefined,
                    wgnames[i],
                    nums[i],
                    lgrName,
                    lgri,
                    lgrj,
                    lgrk,
                    i,
                    ""
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

        const std::vector<int> dimens = smspecList[specInd].get<int>("DIMENS");

        nI = dimens[1];
        nJ = dimens[2];
        nK = dimens[3];

        nParamsSpecFile[specInd] = dimens[0];

        const std::vector<std::string> keywords = smspecList[specInd].get<std::string>("KEYWORDS");
        std::vector<std::string> wgnames;

        if (smspecList.back().hasKey("WGNAMES"))
            wgnames = smspecList.back().get<std::string>("WGNAMES");
        else
            wgnames = smspecList.back().get<std::string>("NAMES");

        const std::vector<int> nums = smspecList[specInd].get<int>("NUMS");

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

    while (specInd >= 0) {

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

        if ((!use_unified) && (multFileList.size()==0)) {
            throw std::runtime_error("neigther unified or non-unified result files found");
        } else if ((use_unified) && (multFileList.size()>0)) {
            auto time_multiple = Opm::filesystem::last_write_time(multFileList.back());
            auto time_unified = Opm::filesystem::last_write_time(unsmryFile);

            if (time_multiple > time_unified) {
                resultsFileList=multFileList;
            } else {
                resultsFileList.push_back(unsmryFile.string());
            }

        } else if (use_unified) {
            resultsFileList.push_back(unsmryFile.string());
        } else {
            resultsFileList=multFileList;
        }

        std::vector<ArrSourceEntry> arraySourceList;

        for (std::string fileName : resultsFileList)
        {
            std::vector<std::tuple <std::string, std::uint64_t>> arrayList;
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

        while  (i < arraySourceList.size()) {

            if (std::get<0>(arraySourceList[i]) != "MINISTEP") {
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

            TimeStepEntry mst1 = std::make_tuple(specInd, dataFileIndex, std::get<3>(arraySourceList[i-1]));
            miniStepList.push_back(mst1);

            TimeStepEntry t1 = std::make_tuple(specInd, dataFileIndex, std::get<3>(arraySourceList[i]));
            timeStepList.push_back(t1);

            i++;

            if (i < arraySourceList.size()) {
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

        nTstep = timeStepList.size();
    }
}

void ESmry::read_ministeps_from_disk()
{
    auto specInd = std::get<0>(miniStepList[0]);
    auto dataFileIndex = std::get<1>(miniStepList[0]);
    uint64_t stepFilePos;

    std::fstream fileH;

    if (formattedFiles[specInd])
        fileH.open(dataFileList[dataFileIndex], std::ios::in);
    else
        fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);

    int ministep_value;

    for (size_t n = 0; n < miniStepList.size(); n++) {

        if (dataFileIndex != std::get<1>(miniStepList[n])) {
            fileH.close();
            specInd = std::get<0>(miniStepList[n]);
            dataFileIndex = std::get<1>(miniStepList[n]);

            if (formattedFiles[specInd])
                fileH.open(dataFileList[dataFileIndex], std::ios::in );
            else
                fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);
        }

        stepFilePos = std::get<2>(miniStepList[n]);

        fileH.seekg (stepFilePos , fileH.beg);

        if (formattedFiles[specInd]) {
            ministep_value = read_ministep_formatted(fileH);
        } else {
            std::function<int(int)> f = Opm::EclIO::flipEndianInt;
            auto ministep_vect = readBinaryArray<int,int>(fileH, 1, Opm::EclIO::INTE, f, sizeOfInte);
            ministep_value = ministep_vect[0];
        }

        mini_steps.push_back(ministep_value);
    }

    fileH.close();
}

bool ESmry::all_steps_available()
{
    if (mini_steps.size() == 0)
        this->read_ministeps_from_disk();

    for (size_t n = 1; n < mini_steps.size(); n++)
        if ((mini_steps[n] - mini_steps[n-1]) > 1)
            return false;

    return true;
}

int ESmry::read_ministep_formatted(std::fstream& fileH)
{
    const std::size_t size = sizeOnDiskFormatted(1, Opm::EclIO::INTE, 4)+1;
    auto buffer = std::vector<char>(size);
    fileH.read (buffer.data(), size);

    const auto fileStr = std::string(buffer.data(), size);

    const auto ministep_vect = readFormattedInteArray(fileStr, 1, 0);

    return ministep_vect[0];
}


std::string ESmry::read_string_from_disk(std::fstream& fileH, uint64_t size) const
{
    std::vector<char> buffer(size);
    fileH.read (buffer.data(), size);

    return { buffer.data(), size };
}


void ESmry::LoadData(const std::vector<std::string>& vectList) const
{
    size_t nvect = vectList.size();

    std::vector<int> keywIndVect;
    keywIndVect.reserve(nvect);

    for (auto key : vectList) {
        if (!hasKey(key))
            OPM_THROW(std::invalid_argument, "error loading key " + key );

        auto it = keyword_index.find(key);
        keywIndVect.push_back(it->second);
    }

    for (auto ind : keywIndVect)
        vectorData[ind].reserve(nTstep);

    std::fstream fileH;

    auto specInd = std::get<0>(timeStepList[0]);
    auto dataFileIndex = std::get<1>(timeStepList[0]);
    std::uint64_t blockSize_f;

    {
        const int rest = MaxBlockSizeReal % numColumnsReal;
        const int nLinesBlock = MaxBlockSizeReal / numColumnsReal + (rest > 0);

        blockSize_f= static_cast<std::uint64_t>(MaxNumBlockReal * numColumnsReal * columnWidthReal + nLinesBlock);
    }

    if (formattedFiles[specInd])
        fileH.open(dataFileList[dataFileIndex], std::ios::in);
    else
        fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);

    for (const auto& ministep : timeStepList) {
        if (dataFileIndex != std::get<1>(ministep)) {
            fileH.close();
            specInd = std::get<0>(ministep);
            dataFileIndex = std::get<1>(ministep);

            if (formattedFiles[specInd])
                fileH.open(dataFileList[dataFileIndex], std::ios::in );
            else
                fileH.open(dataFileList[dataFileIndex], std::ios::in |  std::ios::binary);
        }

        const auto stepFilePos = std::get<2>(ministep);;

        for (auto ind : keywIndVect) {
            auto it = arrayPos[specInd].find(ind);
            if (it == arrayPos[specInd].end()) {
                // undefined vector in current summary file. Typically when loading
                // base restart run and including base run data. Vectors can be added to restart runs
                vectorData[ind].push_back(std::nanf(""));
            }
            else {
                int paramPos = it->second;

                if (formattedFiles[specInd]) {
                    std::uint64_t elementPos = 0;
                    int nBlocks = paramPos / MaxBlockSizeReal;
                    int sizeOfLastBlock = paramPos %  MaxBlockSizeReal;

                    if (nBlocks > 0)
                        elementPos = static_cast<uint64_t>(nBlocks * blockSize_f);

                    int nLines = sizeOfLastBlock / numColumnsReal;
                    elementPos = stepFilePos + elementPos + static_cast<std::uint64_t>(sizeOfLastBlock*columnWidthReal + nLines);

                    fileH.seekg (elementPos, fileH.beg);

                    const std::size_t size = columnWidthReal;
                    std::vector<char> buffer(size);
                    fileH.read (buffer.data(), size);
                    vectorData[ind].push_back(std::strtof(buffer.data(), nullptr));
                }
                else {
                    const std::uint64_t nFullBlocks = static_cast<std::uint64_t>(paramPos/(MaxBlockSizeReal / sizeOfReal));
                    std::uint64_t elementPos = ((2 * nFullBlocks) + 1) * static_cast<std::uint64_t>(sizeOfInte);
                    elementPos += static_cast<std::uint64_t>(paramPos) * static_cast<std::uint64_t>(sizeOfReal) + stepFilePos;

                    fileH.seekg (elementPos, fileH.beg);

                    float value;
                    fileH.read(reinterpret_cast<char*>(&value), sizeOfReal);

                    vectorData[ind].push_back(Opm::EclIO::flipEndianFloat(value));
                }
            }
        }
    }

    fileH.close();

    for (const auto& ind : keywIndVect)
        vectorLoaded[ind] = true;
}

std::vector<int> ESmry::makeKeywPosVector(int specInd) const
{
    std::vector<int> keywpos(nParamsSpecFile[specInd], -1);

    auto has_index = [&keywpos](const int ix)
    {
        return std::find(keywpos.begin(), keywpos.end(), ix) != keywpos.end();
    };

    const auto& kwList = keywordListSpecFile[specInd];
    for (int n = 0; n < nParamsSpecFile[specInd]; ++n) {
        auto it = keyword_index.find(kwList[n]);
        if ((it == keyword_index.end()) || has_index(it->second)) {
            continue;
        }

        keywpos[n] = it->second;
    }

    return keywpos;
}

void ESmry::LoadData() const
{
    std::fstream fileH;

    auto specInd = std::get<0>(timeStepList[0]);
    auto dataFileIndex = std::get<1>(timeStepList[0]);

    std::vector<int> keywpos = makeKeywPosVector(specInd);

    auto openMode = formattedFiles[specInd]
                    ? std::ios::in
                    : std::ios::in | std::ios::binary;

    fileH.open(dataFileList[dataFileIndex], openMode);

    for (const auto& ministep : timeStepList) {
        if (dataFileIndex != std::get<1>(ministep)) {
            fileH.close();

            if (specInd != std::get<0>(ministep)) {
                specInd = std::get<0>(ministep);
                keywpos = makeKeywPosVector(specInd);
            }

            dataFileIndex = std::get<1>(ministep);

            openMode = formattedFiles[specInd]
                       ? std::ios::in
                       : std::ios::in | std::ios::binary;

            fileH.open(dataFileList[dataFileIndex], openMode);
        }

        const auto stepFilePos = std::get<2>(ministep);
        const auto maxNumberOfElements = MaxBlockSizeReal / sizeOfReal;
        fileH.seekg (stepFilePos, fileH.beg);

        if (formattedFiles[specInd]) {
            const std::size_t size = sizeOnDiskFormatted(nParamsSpecFile[specInd], Opm::EclIO::REAL, sizeOfReal) + 1;
            std::vector<char> buffer(size);
            fileH.read (buffer.data(), size);

            const auto fileStr = std::string_view(buffer.data(), size);
            std::size_t p = 0;
            std::int64_t p1= 0;

            for (int i=0; i< nParamsSpecFile[specInd]; ++i, ++p) {
                p1 = fileStr.find_first_not_of(' ',p1);
                const std::int64_t p2 = fileStr.find_first_of(' ', p1);

                if ((keywpos[p] > -1) && !vectorLoaded[keywpos[p]]) {
                    const auto dtmpv = std::strtof(fileStr.substr(p1, p2-p1).data(), nullptr);
                    vectorData[keywpos[p]].push_back(dtmpv);
                }

                p1 = fileStr.find_first_not_of(' ',p2);
            }
        }
        else {
            std::int64_t rest = static_cast<int64_t>(nParamsSpecFile[specInd]);
            std::size_t p = 0;

            while (rest > 0) {
                int dhead;
                fileH.read(reinterpret_cast<char*>(&dhead), sizeof(dhead));
                dhead = Opm::EclIO::flipEndianInt(dhead);

                const int num = dhead / sizeOfInte;
                if ((num > maxNumberOfElements) || (num < 0))
                    OPM_THROW(std::runtime_error, "??Error reading binary data, inconsistent header data or incorrect number of elements");

                for (int i = 0; i < num; ++i, ++p) {
                    float value;
                    fileH.read(reinterpret_cast<char*>(&value), sizeOfReal);

                    if ((keywpos[p] > -1) && !vectorLoaded[keywpos[p]])
                        vectorData[keywpos[p]].push_back(Opm::EclIO::flipEndianFloat(value));
                }

                rest -= num;

                if (( num < maxNumberOfElements && rest != 0) ||
                        (num == maxNumberOfElements && rest < 0))
                {
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

    std::fill_n(vectorLoaded.begin(), nVect, true);
}


std::vector<std::tuple <std::string, uint64_t>>
ESmry::getListOfArrays(std::string filename, bool formatted)
{
    std::vector<std::tuple <std::string, uint64_t>> resultVect;

    FILE *ptr;
    char arrName[9];
    char numstr[13];

    int64_t num;

    if (formatted)
        ptr = fopen(filename.c_str(),"r");  // r for read, files opened as text files
    else
        ptr = fopen(filename.c_str(),"rb");  // r for read, b for binary

    bool endOfFile = false;

    while (!endOfFile)
    {
        Opm::EclIO::eclArrType arrType;

        if (formatted)
        {
            fseek(ptr, 2, SEEK_CUR);

            if (fread(arrName, 8, 1, ptr) != 1 )
                throw std::runtime_error("fread error when loading summary data");

            arrName[8]='\0';

            fseek(ptr, 1, SEEK_CUR);

            if (fread(numstr, 12, 1, ptr) != 1)
                throw std::runtime_error("fread error when loading summary data");

            numstr[12]='\0';

            int num_int = std::stoi(numstr);
            num = static_cast<int64_t>(num_int);

            fseek(ptr, 8, SEEK_CUR);

            if ((strcmp(arrName, "SEQHDR  ") == 0) || (strcmp(arrName, "MINISTEP") == 0))
                arrType = Opm::EclIO::INTE;
            else if (strcmp(arrName, "PARAMS  ") == 0)
                arrType = Opm::EclIO::REAL;
            else {
                throw std::invalid_argument("unknown array in summary data file ");
            }

        } else {
            int num_int;

            fseek(ptr, 4, SEEK_CUR);

            if (fread(arrName, 8, 1, ptr) != 1)
                throw std::runtime_error("fread error when loading summary data");

            arrName[8]='\0';

            if (fread(&num_int, 4, 1, ptr) != 1)
                throw std::runtime_error("fread error when loading summary data");

            num = static_cast<int64_t>(Opm::EclIO::flipEndianInt(num_int));

            fseek(ptr, 8, SEEK_CUR);

            if ((strcmp(arrName, "SEQHDR  ") == 0) || (strcmp(arrName, "MINISTEP") == 0))
                arrType = Opm::EclIO::INTE;
            else if (strcmp(arrName, "PARAMS  ") == 0)
                arrType = Opm::EclIO::REAL;
            else {
                throw std::invalid_argument("unknown array in UNSMRY file ");
            }
        }

        uint64_t filePos = static_cast<uint64_t>(ftell(ptr));

        std::tuple <std::string, uint64_t> t1;
        t1 = std::make_tuple(Opm::EclIO::trimr(arrName), filePos);
        resultVect.push_back(t1);

        if (num > 0) {
            if (formatted) {
                uint64_t sizeOfNextArray = sizeOnDiskFormatted(num, arrType, 4);
                fseek(ptr, static_cast<long int>(sizeOfNextArray), SEEK_CUR);
            } else {
                uint64_t sizeOfNextArray = sizeOnDiskBinary(num, arrType, 4);
                fseek(ptr, static_cast<long int>(sizeOfNextArray), SEEK_CUR);
            }
        }

        if (fgetc(ptr) == EOF)
            endOfFile = true;
        else
            fseek(ptr, -1, SEEK_CUR);
    }

    fclose(ptr);

    return resultVect;
}

bool ESmry::make_esmry_file()
{
    // check that loadBaseRunData is not set, this function only works for single smspec files
    // function will not replace existing lodsmry files (since this is already loaded by this class)
    // if lodsmry file exist, this function will return false and do nothing.

    if (!fromSingleRun)
        OPM_THROW(std::invalid_argument, "creating esmry file only possible when loadBaseRunData=false");

    if (mini_steps.size() == 0)
        this->read_ministeps_from_disk();

    Opm::filesystem::path path = inputFileName.parent_path();
    Opm::filesystem::path rootName = inputFileName.stem();
    Opm::filesystem::path smryDataFile;

    smryDataFile = path / rootName += ".ESMRY";

    if (Opm::EclIO::fileExists(smryDataFile.string()))
    {
        return false;

    } else {

        std::vector<int> is_rstep;
        is_rstep.reserve(timeStepList.size());

        for (size_t i = 0; i < timeStepList.size(); i++)
            if(std::find(seqIndex.begin(), seqIndex.end(), i) != seqIndex.end())
                is_rstep.push_back(1);
            else
                is_rstep.push_back(0);

        this->LoadData();

        {
            Opm::TimeStampUTC ts( std::chrono::system_clock::to_time_t( startdat ));

            std::vector<int> start_date_vect = {ts.day(), ts.month(), ts.year(), ts.hour(),
                                                ts.minutes(), ts.seconds(), 0 };

            std::vector<std::string> units;
            units.reserve(keyword.size());

            for (auto key : keyword)
                units.push_back(kwunits.at(key));

            Opm::EclIO::EclOutput outFile(smryDataFile.string(), false, std::ios::out);

            outFile.write<int>("START", start_date_vect);

            if (std::get<0>(restart_info) != ""){
                auto rst_file = std::get<0>(restart_info);
                outFile.write<std::string>("RESTART", {rst_file});
                outFile.write<int>("RSTNUM", {std::get<1>(restart_info)});
            }

            outFile.write("KEYCHECK", keyword);
            outFile.write("UNITS", units);
            outFile.write<int>("RSTEP", is_rstep);
            outFile.write<int>("TSTEP", mini_steps);

            for (size_t n = 0; n < vectorData.size(); n++ ) {
                const std::string vect_name = fmt::format("V{}", n);
                outFile.write<float>(vect_name, vectorData[n]);
            }
        }

        return true;
    }
}



std::vector<std::string> ESmry::checkForMultipleResultFiles(const Opm::filesystem::path& rootN, bool formatted) const {

    std::vector<std::string> fileList;
    const std::string pathRootN = rootN.parent_path().string();

    const std::string fileFilter = formatted ? rootN.stem().string()+".A" : rootN.stem().string()+".S";

    for (Opm::filesystem::directory_iterator itr(pathRootN); itr != Opm::filesystem::directory_iterator(); ++itr)
    {
        const std::string file = itr->path().filename().string();

        if (file.find(fileFilter) != std::string::npos) {
            std::string num_string = itr->path().extension().string().substr(2);

            if (Opm::EclIO::is_number(num_string))
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


void ESmry::ijk_from_global_index(int glob, int &i, int &j, int &k) const
{
    glob -= 1;

    i = 1 + (glob % this->nI);  glob /= this->nI;
    j = 1 + (glob % this->nJ);
    k = 1 + (glob / this->nJ);
}


std::string ESmry::makeKeyString(const std::string& keywordArg, const std::string& wgname, int num) const
{
    const auto no_wgname = std::string_view(":+:+:+:+");

    const auto first = keywordArg[0];

    if (first == 'A') {
        if (num <= 0) {
            return "";
        }

        return fmt::format("{}:{}", keywordArg, num);
    }

    if (first == 'B') {
        if (num <= 0) {
            return "";
        }

        int _i, _j, _k;
        ijk_from_global_index(num, _i, _j, _k);

        return fmt::format("{}:{},{},{}", keywordArg, _i, _j, _k);
    }

    if (first == 'C') {
        if (num <= 0) {
            return "";
        }

        int _i, _j, _k;
        ijk_from_global_index(num, _i, _j, _k);

        return fmt::format("{}:{}:{},{},{}", keywordArg, wgname, _i, _j, _k);
    }

    if (first == 'G') {
        if (wgname == no_wgname) {
            return "";
        }

        return fmt::format("{}:{}", keywordArg, wgname);
    }

    if (first == 'R') {
        if (num <= 0) {
            return "";
        }

        if (keywordArg[1] == 'F') {
            // NUMS = R1 + 32768*(R2 + 10)
            const auto r1 =  num % (1UL << 15);
            const auto r2 = (num / (1UL << 15)) - 10;

            return fmt::format("{}:{}-{}", keywordArg, r1, r2);
        }

        return fmt::format("{}:{}", keywordArg, num);
    }

    if (first == 'S') {
        const std::vector<std::string> segmExcep= {"STEPTYPE", "SEPARATE", "SUMTHIN"};

        auto it = std::find(segmExcep.begin(), segmExcep.end(), keywordArg);
        if (it != segmExcep.end()) {
            return keywordArg;
        }

        if (wgname == no_wgname) {
            return "";
        }

        if (num <= 0) {
            return "";
        }

        return fmt::format("{}:{}:{}", keywordArg, wgname, num);
    }

    if (first == 'W') {
        if (wgname == no_wgname) {
            return "";
        }

        return fmt::format("{}:{}", keywordArg, wgname);
    }

    return keywordArg;
}

std::string ESmry::unpackNumber(const SummaryNode& node) const
{
    if (node.category == SummaryNode::Category::Block ||
        node.category == SummaryNode::Category::Connection) {
        int _i,_j,_k;
        ijk_from_global_index(node.number, _i, _j, _k);

        return fmt::format("{},{},{}", _i, _j, _k);
    }
    else if (node.category == SummaryNode::Category::Region && node.keyword[2] == 'F') {
        const auto r1 =  node.number % (1 << 15);
        const auto r2 = (node.number / (1 << 15)) - 10;

        return fmt::format("{}-{}", r1, r2);
    }
    else {
        return fmt::format("{}", node.number);
    }
}

std::string ESmry::lookupKey(const SummaryNode& node) const {
    return node.unique_key([this](const auto& num) { return this->unpackNumber(num); });
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

std::vector<Opm::time_point> ESmry::dates() const {
    double time_unit = 24 * 3600;
    std::vector<Opm::time_point> d;

    for (const auto& t : this->get("TIME"))
        d.push_back( this->startdat + std::chrono::duration_cast<std::chrono::seconds>( std::chrono::duration<double, std::chrono::seconds::period>( t * time_unit)));

    return d;
}

std::vector<time_point> ESmry::dates_at_rstep() const {
    const auto& full_vector = this->dates();
    return this->rstep_vector(full_vector);
}
}} // namespace Opm::EclIO
