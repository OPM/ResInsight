/*
   Copyright 2019 Statoil ASA.

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

#ifndef OPM_IO_ExtSmryOutput_HPP
#define OPM_IO_ExtSmryOutput_HPP

#include <string>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>


namespace Opm {

class EclipseState;

}

namespace Opm { namespace EclIO {


class ExtSmryOutput
{

public:
    ExtSmryOutput(const std::vector<std::string>& valueKeys, const std::vector<std::string>& valueUnits,
                 const EclipseState& es, const time_t start_time);

    void write(const std::vector<float>& ts_data, int report_step, bool is_final_summary);

private:

    const int m_min_write_interval = 15;  // at least 15 seconds betwen each write
    std::chrono::time_point<std::chrono::system_clock> m_last_write;

    std::string m_outputFileName;
    int m_nTimeSteps;
    int m_nVect;
    bool m_fmt;

    std::vector<int> m_start_date_vect;
    std::string m_restart_rootn;
    int m_restart_step;
    std::vector<std::string> m_smry_keys;
    std::vector<std::string> m_smryUnits;
    std::vector<int> m_rstep;
    std::vector<int> m_tstep;
    std::vector<std::vector<float>> m_smrydata;

    std::array<int, 3> ijk_from_global_index(const GridDims& dims, int globInd) const;
    std::vector<std::string> make_modified_keys(const std::vector<std::string>& valueKeys, const GridDims& dims);
};


}} // namespace Opm::EclIO

#endif // OPM_IO_ExtSmryOutput_HPP
