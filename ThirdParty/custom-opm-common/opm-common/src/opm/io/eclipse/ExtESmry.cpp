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
#include <opm/common/utility/TimeService.hpp>
#include <opm/common/utility/shmatch.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>

#include <algorithm>
#include <numeric>
#include <chrono>
#include <exception>
#include <filesystem>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <fstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>


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
    m_io_opening = 0.0;
    m_io_loading = 0.0;

    auto start = std::chrono::system_clock::now();

    if (m_inputFileName.extension()=="")
        m_inputFileName+=".ESMRY";

    if (m_inputFileName.extension()!=".ESMRY")
        throw std::invalid_argument("Input file should have extension .ESMRY");

    m_esmry_files.push_back(m_inputFileName);

    std::filesystem::path rootName = m_inputFileName.parent_path() / m_inputFileName.stem();
    std::filesystem::path path = std::filesystem::current_path();

    std::filesystem::path rstRootN;

    updatePathAndRootName(path, rootName);

    ExtSmryHeadType ext_esmry_head;

    uint64_t rstep_offset;

    bool res = open_esmry(m_inputFileName, ext_esmry_head, rstep_offset);
    int n_attempts = 1;

    while ((!res) && (n_attempts < 10)){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        res = open_esmry(m_inputFileName, ext_esmry_head, rstep_offset);
        n_attempts ++;
    }

    if (n_attempts == 10)
        OPM_THROW( std::runtime_error, "when opening ESMRY file " + filename );

    m_startdat = std::get<0>(ext_esmry_head);
    m_rstep_offset.push_back(rstep_offset);

    std::map<std::string, int> key_index;

    auto keyword = std::get<2>(ext_esmry_head);
    auto units = std::get<3>(ext_esmry_head);

    for (size_t n = 0; n < keyword.size(); n++){
        key_index[keyword[n]] = n;
        m_keyword.push_back(keyword[n]);
    }

    m_keyword_index.push_back(key_index);

    for (size_t n = 0; n < m_keyword.size(); n++)
        kwunits[m_keyword[n]] = units[n];

    RstEntry rst_entry = std::get<1>(ext_esmry_head);

    m_rstep_v.push_back(std::get<4>(ext_esmry_head));
    m_tstep_v.push_back(std::get<5>(ext_esmry_head));

    m_nTstep_v.push_back(m_tstep_v.back().size());

    m_tstep_range.push_back(std::make_tuple(0, m_tstep_v.back().size() - 1));

    if ((loadBaseRunData) && (!std::get<0>(rst_entry).empty())) {

        auto restart = std::get<0>(rst_entry);
        auto rstNum = std::get<1>(rst_entry);

        int sim_ind = 0;
        while (!restart.empty()){
            sim_ind++;

            rstRootN = std::filesystem::path(restart);

            updatePathAndRootName(path, rstRootN);

            std::filesystem::path rstESmryFile = path / rstRootN;
            rstESmryFile += ".ESMRY";

            m_esmry_files.push_back(rstESmryFile);

            if (!open_esmry(rstESmryFile, ext_esmry_head, rstep_offset))
                OPM_THROW( std::runtime_error, "when opening ESMRY file" + rstESmryFile.string() );

            m_rstep_offset.push_back(rstep_offset);

            m_rstep_v.push_back(std::get<4>(ext_esmry_head));
            m_tstep_v.push_back(std::get<5>(ext_esmry_head));

            m_nTstep_v.push_back(m_tstep_v.back().size());

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
            keyword = std::get<2>(ext_esmry_head);

            for (size_t n = 0; n < keyword.size(); n++)
                key_index[keyword[n]] = n;

            m_keyword_index.push_back(key_index);

            rst_entry = std::get<1>(ext_esmry_head);
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

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    m_io_opening += elapsed_seconds.count();
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

bool ExtESmry::open_esmry(const std::filesystem::path& inputFileName, ExtSmryHeadType& ext_smry_head, uint64_t& rstep_offset)
{
    std::fstream fileH;

    fileH.open(inputFileName, std::ios::in |  std::ios::binary);

    if (!fileH)
        return false;

    std::string arrName;
    int64_t arr_size;
    Opm::EclIO::eclArrType arrType;
    int sizeOfElement;

    try {
        Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    if (arrName != "START   " || arrType != Opm::EclIO::INTE)
        OPM_THROW(std::invalid_argument, "reading start, invalid esmry file " + inputFileName.string() );

    std::vector<int> start_vect;
    try {
        start_vect = Opm::EclIO::readBinaryInteArray(fileH, arr_size);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    auto startdat = make_date(start_vect);

    try {
       Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

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
        OPM_THROW(std::invalid_argument, "reading keycheck, invalid esmry file " + inputFileName.string() );

    std::vector<std::string> keywords;

    try {
        keywords = Opm::EclIO::readBinaryC0nnArray(fileH, arr_size, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }


    try {
        Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    if (arrName != "UNITS   ")
        OPM_THROW(std::invalid_argument, "reading UNITS, invalid esmry file " + inputFileName.string() );

    std::vector<std::string> units;

    try {
        units = Opm::EclIO::readBinaryC0nnArray(fileH, arr_size, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    if (keywords.size() != units.size())
        OPM_THROW( std::runtime_error, "invalid ESMRY file " + inputFileName.string() + ". Size of UNITS not equal size of KEYCHECK");

    rstep_offset = static_cast<uint64_t>(fileH.tellg());

    try {
        Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    if (arrName != "RSTEP   " || arrType != Opm::EclIO::INTE)
        OPM_THROW(std::invalid_argument, "Reading RSTEP, invalid esmry file " + inputFileName.string() );

    std::vector<int> rstep;

    try {
        rstep = Opm::EclIO::readBinaryInteArray(fileH, arr_size);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    try {
        Opm::EclIO::readBinaryHeader(fileH, arrName, arr_size, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    if (arrName != "TSTEP   " || arrType != Opm::EclIO::INTE)
        OPM_THROW(std::invalid_argument, "reading TSTEP, invalid esmry file " + inputFileName.string() );

    std::vector<int> tstep;

    try {
        tstep = Opm::EclIO::readBinaryInteArray(fileH, arr_size);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    ext_smry_head = std::make_tuple(startdat, rst_entry, keywords, units, rstep, tstep);

    fileH.close();

    return true;
}


void ExtESmry::updatePathAndRootName(std::filesystem::path& dir, std::filesystem::path& rootN) {

    if (rootN.parent_path().is_absolute()){
        dir = rootN.parent_path();
    } else {
        dir = dir / rootN.parent_path();
    }

    rootN = rootN.stem();
}


bool ExtESmry::load_esmry(const std::vector<std::string>& stringVect, const std::vector<int>& keyIndexVect,
                               const std::vector<int>& loadKeyIndex, int ind, int to_ind )
{
    std::fstream fileH;

    fileH.open(m_esmry_files[ind], std::ios::in |  std::ios::binary);

    if (!fileH)
        return false;

    std::string arrName;
    Opm::EclIO::eclArrType arrType;
    int64_t num_tstep;
    int sizeOfElement;

    // Read actual number of time steps on disk from RSTEP array before loading
    // data. Notice that number of time steps can be different than what it was when
    // the ESMRY file was opened. The simulation may have progressed if this is an
    // ESMRY file from an active run

    fileH.seekg (m_rstep_offset[ind], fileH.beg);

    try {
        Opm::EclIO::readBinaryHeader(fileH, arrName, num_tstep, arrType, sizeOfElement);
    } catch (const std::runtime_error& error)
    {
        return false;
    }

    auto smry_arr_size = sizeOnDiskBinary(num_tstep, Opm::EclIO::REAL, sizeOfReal);

    std::vector<std::vector<float>> smry_data;
    smry_data.resize(loadKeyIndex.size(), {});

    for (size_t n = 0 ; n < loadKeyIndex.size(); n++) {

        const auto& key = stringVect[loadKeyIndex[n]];

        if ( m_keyword_index[ind].find(key) == m_keyword_index[ind].end() ) {

            smry_data[n].resize(to_ind + 1, 0.0 );

        } else {

            int key_ind = m_keyword_index[ind].at(key);

            uint64_t pos = m_rstep_offset[ind] + smry_arr_size*static_cast<uint64_t>(key_ind);

            // adding size of TSTEP and RSTEP INTE data
            pos = pos + 2 * sizeOnDiskBinary(num_tstep, Opm::EclIO::INTE, sizeOfInte);

            pos = pos + static_cast<uint64_t>(2 * 24);  // adding size of binary headers (TSTEP and RSTEP)
            pos = pos + static_cast<uint64_t>(key_ind * 24);  // adding size of binary headers

            fileH.seekg (pos, fileH.beg);

            int64_t size;

            try {
                readBinaryHeader(fileH, arrName, size, arrType, sizeOfElement);
            } catch (const std::runtime_error& error)
            {
                return false;
            }

            arrName = Opm::EclIO::trimr(arrName);

            std::string checkName = "V" + std::to_string(key_ind);

            if (arrName != checkName)
                return false;

            try {
                smry_data[n] = readBinaryRealArray(fileH, size);
            } catch (const std::runtime_error& error)
            {
                return false;
            }
        }
    }

    fileH.close();

    for (size_t n = 0 ; n < loadKeyIndex.size(); n++)
        m_vectorData[keyIndexVect[n]].insert(m_vectorData[keyIndexVect[n]].end(), smry_data[n].begin(), smry_data[n].begin() + to_ind + 1);

    return true;
}


void ExtESmry::loadData(const std::vector<std::string>& stringVect)
{
    auto start = std::chrono::system_clock::now();

    auto num_keys = stringVect.size();
    std::vector<int> keyIndexVect;
    std::vector<int> loadKeyIndex;

    keyIndexVect.reserve(num_keys);
    loadKeyIndex.reserve(num_keys);

    int keyCounter = 0;

    for (const auto& key: stringVect){
        auto key_ind = m_keyword_index[0].at(key);
        if ((!m_vectorLoaded[key_ind]) && (std::find(keyIndexVect.begin(), keyIndexVect.end(), key_ind) == keyIndexVect.end() )){
            keyIndexVect.push_back(key_ind);
            loadKeyIndex.push_back(keyCounter);
        }
        ++keyCounter;
    }

    int ind = static_cast<int>(m_tstep_range.size()) - 1 ;

    while (ind > -1) {

        int to_ind = std::get<1>(m_tstep_range[ind]);

        bool res = load_esmry(stringVect, keyIndexVect, loadKeyIndex, ind, to_ind );

        int n_attempts = 1;

        while ((!res) && (n_attempts < 10)){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            res = load_esmry(stringVect, keyIndexVect, loadKeyIndex, ind, to_ind );
            n_attempts ++;
        }

        if (n_attempts == 10){
            std::string emsry_file_name = m_esmry_files[ind].string();
            OPM_THROW( std::runtime_error, "when loading data from ESMRY file" + emsry_file_name );
        }

        ind--;
    }

    for (auto kind : keyIndexVect)
        m_vectorLoaded[kind] = true;

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    m_io_loading += elapsed_seconds.count();
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
        if (shmatch( pattern, key) )
            list.push_back(key);

    return list;
}

bool ExtESmry::hasKey(const std::string &key) const
{
    return std::find(m_keyword.begin(), m_keyword.end(), key) != m_keyword.end();
}

std::tuple<double, double> ExtESmry::get_io_elapsed() const
{
    std::tuple<double, double> duration = std::make_tuple(m_io_opening, m_io_loading);
    return duration;
}



}} // namespace Opm::ecl

