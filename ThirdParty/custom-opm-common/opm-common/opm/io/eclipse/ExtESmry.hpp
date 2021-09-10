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

#ifndef OPM_IO_ExtESmry_HPP
#define OPM_IO_ExtESmry_HPP

#include <chrono>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <stdint.h>

#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/utility/TimeService.hpp>

namespace Opm { namespace EclIO {

using ArrSourceEntry = std::tuple<std::string, std::string, int, uint64_t>;
using TimeStepEntry = std::tuple<int, int, uint64_t>;
using RstEntry = std::tuple<std::string, int>;

// start, rstart + rstnum, keycheck, units, rstep, tstep
using LodsmryHeadType = std::tuple<time_point, RstEntry, std::vector<std::string>, std::vector<std::string>,
                                    std::vector<int>, std::vector<int>>;

class ExtESmry
{
public:

    // input is esmry, only binary supported.
    explicit ExtESmry(const std::string& filename, bool loadBaseRunData=false);

    const std::vector<float>& get(const std::string& name);
    std::vector<float> get_at_rstep(const std::string& name);
    std::string& get_unit(const std::string& name);

    void loadData();
    void loadData(const std::vector<std::string>& stringVect);

    time_point startdate() const { return m_startdat; }

    bool hasKey(const std::string& key) const;

    size_t numberOfTimeSteps() const { return m_nTstep; }
    size_t numberOfVectors() const { return m_nVect; }

    const std::vector<std::string>& keywordList() const { return m_keyword;}
    std::vector<std::string> keywordList(const std::string& pattern) const;

    std::vector<time_point> dates();

    bool all_steps_available();


private:
    filesystem::path m_inputFileName;
    std::vector<filesystem::path> m_lodsmry_files;

    bool m_loadBaseRun;
    std::vector<std::map<std::string, int>> m_keyword_index;
    std::vector<std::tuple<int,int>> m_tstep_range;
    std::vector<std::string> m_keyword;
    std::vector<int> m_rstep;
    std::vector<int> m_tstep;
    std::vector<std::vector<int>> m_rstep_v;
    std::vector<std::vector<int>> m_tstep_v;
    std::vector<std::vector<float>> m_vectorData;
    std::vector<bool> m_vectorLoaded;
    std::unordered_map<std::string, std::string> kwunits;

    size_t m_nVect;
    std::vector<size_t> m_nTstep_v;
    size_t m_nTstep;
    std::vector<int> m_seqIndex;

    std::vector<uint64_t> m_lod_offset;
    std::vector<uint64_t> m_lod_arr_size;

    time_point m_startdat;

    uint64_t open_esmry(Opm::filesystem::path& inputFileName, LodsmryHeadType& lodsmry_head);

    void updatePathAndRootName(Opm::filesystem::path& dir, Opm::filesystem::path& rootN);
};

}} // namespace Opm::EclIO


#endif // OPM_IO_ExtESmry_HPP
