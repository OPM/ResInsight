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

#include <opm/io/eclipse/ExtESmry.hpp>

#include <opm/common/ErrorMacros.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>

#include <algorithm>
#include <numeric>
#include <chrono>
#include <exception>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include "cross-platform/windows/Substitutes.hpp"
#else
#include <fnmatch.h>
#endif

#include <fstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iostream>

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


namespace Opm { namespace EclIO {

ExtESmry::ExtESmry(const std::string &filename, bool loadBaseRunData) :
    m_inputFileName { filename },
    m_loadBaseRun(loadBaseRunData)
{
    if (m_inputFileName.extension()=="")
        m_inputFileName+=".ESMRY";

    if (m_inputFileName.extension()!=".ESMRY")
        throw std::invalid_argument("Input file should have extension .ESMRY");

    m_lodsmry_files.push_back(m_inputFileName);

    Opm::filesystem::path rootName = m_inputFileName.parent_path() / m_inputFileName.stem();
    Opm::filesystem::path path = Opm::filesystem::current_path();

    Opm::filesystem::path rstRootN;

    updatePathAndRootName(path, rootName);

    LodsmryHeadType lodsmry_head;

    auto lod_offset = open_esmry(m_inputFileName, lodsmry_head);

    m_startdat = std::get<0>(lodsmry_head);

    m_lod_offset.push_back(lod_offset);

    std::map<std::string, int> key_index;

    auto keyword = std::get<2>(lodsmry_head);
    auto units = std::get<3>(lodsmry_head);

    for (size_t n = 0; n < keyword.size(); n++){
        key_index[keyword[n]] = n;
        m_keyword.push_back(keyword[n]);
    }

    m_keyword_index.push_back(key_index);

    for (size_t n = 0; n < m_keyword.size(); n++)
        kwunits[m_keyword[n]] = units[n];

    RstEntry rst_entry = std::get<1>(lodsmry_head);

    m_rstep_v.push_back(std::get<4>(lodsmry_head));
    m_tstep_v.push_back(std::get<5>(lodsmry_head));

    m_nTstep_v.push_back(m_tstep_v.back().size());

    auto lod_arr_size = sizeOnDiskBinary(m_nTstep_v.back(), Opm::EclIO::REAL, sizeOfReal);

    m_lod_arr_size.push_back(lod_arr_size);

    m_tstep_range.push_back(std::make_tuple(0, m_tstep_v.back().size() - 1));

    if ((loadBaseRunData) && (!std::get<0>(rst_entry).empty())) {

        auto restart = std::get<0>(rst_entry);
        auto rstNum = std::get<1>(rst_entry);

        int sim_ind = 0;
        while (!restart.empty()){
            sim_ind++;

            rstRootN = Opm::filesystem::path(restart);

            updatePathAndRootName(path, rstRootN);

            Opm::filesystem::path rstLodSmryFile = path / rstRootN;
            rstLodSmryFile += ".ESMRY";

            m_lodsmry_files.push_back(rstLodSmryFile);

            lod_offset = open_esmry(rstLodSmryFile, lodsmry_head);

            m_lod_offset.push_back(lod_offset);

            m_rstep_v.push_back(std::get<4>(lodsmry_head));
            m_tstep_v.push_back(std::get<5>(lodsmry_head));

            m_nTstep_v.push_back(m_tstep_v.back().size());

            lod_arr_size = sizeOnDiskBinary(m_nTstep_v.back(), Opm::EclIO::REAL, sizeOfReal);
            m_lod_arr_size.push_back(lod_arr_size);

            int cidx = 0;

            auto it = std::find_if(m_rstep_v[sim_ind].begin(), m_rstep_v[sim_ind].end(),
                           [&cidx, &rstNum](const int & val)
                           {
                              if (val == 1)
                                  ++cidx;

                              return cidx == rstNum;
                           });

            size_t ind =  std::distance(m_rstep_v[sim_ind].begin(), it);

            m_tstep_range.push_back(std::make_tuple(0, ind));

            key_index.clear();
            keyword = std::get<2>(lodsmry_head);

            for (size_t n = 0; n < keyword.size(); n++)
                key_index[keyword[n]] = n;

            m_keyword_index.push_back(key_index);

            rst_entry = std::get<1>(lodsmry_head);
            restart = std::get<0>(rst_entry);
            rstNum = std::get<1>(rst_entry);
        }
    }

    m_nVect = m_keyword.size();

    m_vectorData.resize(m_nVect, {});
    m_vectorLoaded.resize(m_nVect, false);

    int ind = static_cast<int>(m_tstep_range.size()) - 1 ;

    while (ind > -1) {
        int to_ind = std::get<1>(m_tstep_range[ind]);
        m_rstep.insert(m_rstep.end(), m_rstep_v[ind].begin(), m_rstep_v[ind].begin() + to_ind + 1);
        m_tstep.insert(m_tstep.end(), m_tstep_v[ind].begin(), m_tstep_v[ind].begin() + to_ind + 1);
        ind--;
    }

    m_nTstep = m_rstep.size();

    for (size_t m = 0; m < m_rstep.size(); m++)
        if (m_rstep[m] == 1)
            m_seqIndex.push_back(m);
}


std::vector<float> ExtESmry::get_at_rstep(const std::string& name)
{
    auto full_vect = this->get(name);

    std::vector<float> rs_vect;
    rs_vect.reserve(m_seqIndex.size());

    for (auto r : m_seqIndex)
        rs_vect.push_back(full_vect[r]);

    return rs_vect;
}

std::string& ExtESmry::get_unit(const std::string& name)
{
    if ( m_keyword_index[0].find(name) == m_keyword_index[0].end() )
        throw std::invalid_argument("summary key '" + name + "' not found");

    return kwunits.at(name);
}

bool ExtESmry::all_steps_available()
{
    for (size_t n = 1; n < m_tstep.size(); n++)
        if ((m_tstep[n] - m_tstep[n-1]) > 1)
            return false;

    return true;
}

uint64_t ExtESmry::open_esmry(Opm::filesystem::path& inputFileName, LodsmryHeadType& lodsmry_head)
{
    std::fstream fileH;

    fileH.open(inputFileName, std::ios::in |  std::ios::binary);

    if (!fileH)
        throw std::runtime_error("Can not open file ");


    std::string arrName;
    int64_t arr_size;
    Opm::EclIO::eclArrType arrType;
    int sizeOfElement;

    Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);

    if ((arrName != "START   ") or (arrType != Opm::EclIO::INTE))
        OPM_THROW(std::invalid_argument, "reading start, invalid lod file");

    auto start_vect = Opm::EclIO::readBinaryInteArray(fileH, arr_size);

    auto startdat = make_date(start_vect);

    Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);

    Opm::EclIO::RstEntry rst_entry = std::make_tuple("", 0);

    if (arrName == "RESTART "){

        if (m_loadBaseRun) {

            std::vector<std::string> rstfile = Opm::EclIO::readBinaryC0nnArray(fileH, arr_size, sizeOfElement);
            Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
            std::vector<int> rst_num  = Opm::EclIO::readBinaryInteArray(fileH, arr_size);

            rst_entry = std::make_tuple(rstfile[0], rst_num[0]);

        } else {
            uint64_t numIgnore = sizeOnDiskBinary(arr_size, arrType, sizeOfElement);
            numIgnore = numIgnore + 24 + sizeOnDiskBinary(1, Opm::EclIO::INTE, Opm::EclIO::sizeOfInte);
            fileH.seekg(static_cast<std::streamoff>(numIgnore), std::ios_base::cur);
        }

        Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    }

    if (arrName != "KEYCHECK")
        OPM_THROW(std::invalid_argument, "!!reading keycheck, invalid lod file");

    std::vector<std::string> keywords;

    keywords = Opm::EclIO::readBinaryC0nnArray(fileH, arr_size, sizeOfElement);

    Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);

    if (arrName != "UNITS   ")
        OPM_THROW(std::invalid_argument, "reading UNITS, invalid lod file");

    auto units = Opm::EclIO::readBinaryC0nnArray(fileH, arr_size, sizeOfElement);

    if (keywords.size() != units.size())
        throw std::runtime_error("invalied LODSMRY file, size of units not equal size of keywords");

    Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);

    if ((arrName != "RSTEP   ") or (arrType != Opm::EclIO::INTE))
        OPM_THROW(std::invalid_argument, "reading RSTEP, invalid lod file");

    auto rstep = Opm::EclIO::readBinaryInteArray(fileH, arr_size);

    Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);

    if ((arrName != "TSTEP   ") or (arrType != Opm::EclIO::INTE))
        OPM_THROW(std::invalid_argument, "reading TSTEP, invalid lod file");

    auto tstep = Opm::EclIO::readBinaryInteArray(fileH, arr_size);

    lodsmry_head = std::make_tuple(startdat, rst_entry, keywords, units, rstep, tstep);

    uint64_t lodsmry_offset = static_cast<uint64_t>(fileH.tellg());

    fileH.close();

    return lodsmry_offset;
}


void ExtESmry::updatePathAndRootName(Opm::filesystem::path& dir, Opm::filesystem::path& rootN) {

    if (rootN.parent_path().is_absolute()){
        dir = rootN.parent_path();
    } else {
        dir = dir / rootN.parent_path();
    }

    rootN = rootN.stem();
}


void ExtESmry::loadData(const std::vector<std::string>& stringVect)
{
    std::vector<int> keyIndexVect;

    for (const auto& key: stringVect)
        keyIndexVect.push_back(m_keyword_index[0].at(key));

    std::fstream fileH;

    int ind = static_cast<int>(m_tstep_range.size()) - 1 ;

    while (ind > -1) {

        int to_ind = std::get<1>(m_tstep_range[ind]);

        fileH.open(m_lodsmry_files[ind], std::ios::in |  std::ios::binary);

        if (!fileH)
            throw std::runtime_error("Can not open file lodFile");

        for (size_t n = 0 ; n < stringVect.size(); n++) {

            std::string key = stringVect[n];

            std::string arrName;
            Opm::EclIO::eclArrType arrType;

            if ( m_keyword_index[ind].find(key) == m_keyword_index[ind].end() ) {

                for (int m = 0; m < to_ind + 1; m++)
                    m_vectorData[keyIndexVect[n]].push_back(0.0);

            } else {

                int key_ind = m_keyword_index[ind].at(key);

                uint64_t pos = m_lod_offset[ind] + m_lod_arr_size[ind]*static_cast<uint64_t>(key_ind);
                pos = pos + static_cast<uint64_t>(key_ind * 24);  // adding size of binary headers

                fileH.seekg (pos, fileH.beg);

                int64_t size;
                int sizeOfElement;
                readBinaryHeader(fileH, arrName, size, arrType, sizeOfElement);

                arrName = Opm::EclIO::trimr(arrName);

                std::string checkName = "V" + std::to_string(key_ind);

                if (arrName != checkName)
                    OPM_THROW(std::invalid_argument, "lodsmry, wrong header expecting  " + checkName + " found " +  arrName);

                auto smry_data = readBinaryRealArray(fileH, size);

                m_vectorData[keyIndexVect[n]].insert(m_vectorData[keyIndexVect[n]].end(), smry_data.begin(), smry_data.begin() + to_ind + 1);
            }
        }

        fileH.close();
        ind--;
    }

    for (auto kind : keyIndexVect)
        m_vectorLoaded[kind] = true;
}

void ExtESmry::loadData()
{
    this->loadData(m_keyword);
}

const std::vector<float>& ExtESmry::get(const std::string& name)
{
    if ( m_keyword_index[0].find(name) == m_keyword_index[0].end() )
        throw std::invalid_argument("summary key '" + name + "' not found");

    int index = m_keyword_index[0].at(name);

    if (!m_vectorLoaded[index]){
        loadData({name});
    }

    return m_vectorData[index];
}

std::vector<Opm::time_point> ExtESmry::dates() {
    double time_unit = 24 * 3600;
    std::vector<Opm::time_point> d;

    for (const auto& t : this->get("TIME"))
        d.push_back( this->m_startdat + std::chrono::duration_cast<std::chrono::seconds>( std::chrono::duration<double, std::chrono::seconds::period>( t * time_unit)));

    return d;
}

std::vector<std::string> ExtESmry::keywordList(const std::string& pattern) const
{
    std::vector<std::string> list;

    for (const auto& key : m_keyword)
        if (fnmatch( pattern.c_str(), key.c_str(), 0 ) == 0 )
            list.push_back(key);

    return list;
}

bool ExtESmry::hasKey(const std::string &key) const
{
    return std::find(m_keyword.begin(), m_keyword.end(), key) != m_keyword.end();
}



}} // namespace Opm::ecl

