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

#include <opm/io/eclipse/EclUtil.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ExtSmryOutput.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <stdexcept>
#include <string>
#include <filesystem>


namespace Opm { namespace EclIO {


ExtSmryOutput::ExtSmryOutput(const std::vector<std::string>& valueKeys, const std::vector<std::string>& valueUnits,
                 const EclipseState& es, const time_t start_time)
{
    m_nVect = valueKeys.size();
    m_nTimeSteps = 0;
    m_last_write = std::chrono::system_clock::now();

    IOConfig ioconf = es.getIOConfig();

    m_restart_rootn = "";
    m_restart_step = -1;

    InitConfig initcfg = es.getInitConfig();

    if (initcfg.restartRequested()) {
        m_restart_rootn = initcfg.getRestartRootName();
        m_restart_step = initcfg.getRestartStep();
    }

    m_fmt = es.cfg().io().getFMTOUT();

    auto dims = es.gridDims();

    m_outputFileName = ioconf.getOutputDir() + "/" + ioconf.getBaseName() + ".ESMRY";

    m_smry_keys = this->make_modified_keys(valueKeys, dims);
    m_smryUnits = valueUnits;

    Opm::time_point startdat = Opm::TimeService::from_time_t(start_time);

    Opm::TimeStampUTC ts( std::chrono::system_clock::to_time_t( startdat ));

    m_start_date_vect = {ts.day(), ts.month(), ts.year(),
        ts.hour(), ts.minutes(), ts.seconds(), 0 };

    for (size_t n = 0; n < static_cast<size_t>(m_nVect); n++)
        m_smrydata.push_back({});
}


void ExtSmryOutput::write(const std::vector<float>& ts_data, int report_step, bool is_final_summary)
{

    if (ts_data.size() != static_cast<size_t>(m_nVect))
        throw std::invalid_argument("size of ts_data vector not same as number of smry vectors");

    auto current = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = current - m_last_write;

    m_rstep.push_back(report_step);

    // flow is yet not supporting rptonly in summary
    // tstep = {0,1,2 .. , m_nTimeSteps-1}

    if (m_tstep.size()==0)
        m_tstep.push_back(0);
    else
        m_tstep.push_back(m_tstep.back()+1);

    for (size_t n = 0; n < static_cast<size_t>(m_nVect); n++)
        m_smrydata[n].push_back(ts_data[n]);

    if ((is_final_summary) || (elapsed_seconds.count() > m_min_write_interval))
    {
        const auto tp = std::chrono::system_clock::now();
        auto sec_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
        std::string tmp_file_name = "TMP_" + std::to_string(sec_since_epoch) + ".ESMRY";

        {
            Opm::EclIO::EclOutput outFile(tmp_file_name, m_fmt, std::ios::out);

            outFile.write<int>("START", m_start_date_vect);

            if (m_restart_rootn.size() > 0) {
                outFile.write<std::string>("RESTART", {m_restart_rootn});
                outFile.write<int>("RSTNUM", {m_restart_step});
            }

            outFile.write("KEYCHECK", m_smry_keys);
            outFile.write("UNITS", m_smryUnits);

            outFile.write<int>("RSTEP", m_rstep);
            outFile.write<int>("TSTEP", m_tstep);

            for (size_t n = 0; n < static_cast<size_t>(m_nVect); n++ ) {
                std::string vect_name="V" + std::to_string(n);
                outFile.write<float>(vect_name, m_smrydata[n]);
            }
        }

        const std::filesystem::path from_file = tmp_file_name;
        const std::filesystem::path to_file = m_outputFileName;
        std::filesystem::rename(from_file, to_file);

        m_last_write = std::chrono::system_clock::now();
    }

    m_nTimeSteps++;
}


std::vector<std::string> ExtSmryOutput::make_modified_keys(const std::vector<std::string>& valueKeys, const GridDims& dims)
{
    std::vector<std::string> mod_keys;
    mod_keys.reserve(valueKeys.size());

    for (size_t n=0; n < valueKeys.size(); n++){
        if (valueKeys[n].substr(0,1) == "C"){
            size_t p = valueKeys[n].find_first_of(":");
            p = valueKeys[n].find_first_of(":", p + 1);

            int num = std::stod(valueKeys[n].substr(p + 1)) - 1;

            auto ijk = ijk_from_global_index(dims, num);

            std::string mod_key = valueKeys[n].substr(0, p + 1);
            mod_key = mod_key + std::to_string(ijk[0] + 1) + "," + std::to_string(ijk[1] + 1) + "," + std::to_string(ijk[2] + 1);

            mod_keys.push_back(mod_key);

        } else if (valueKeys[n].substr(0,1) == "B"){

            size_t p = valueKeys[n].find_first_of(":");

            int num = std::stod(valueKeys[n].substr(p + 1)) - 1;

            auto ijk = ijk_from_global_index(dims, num);

            std::string mod_key = valueKeys[n].substr(0, p + 1);
            mod_key = mod_key + std::to_string(ijk[0] + 1) + "," + std::to_string(ijk[1] + 1) + "," + std::to_string(ijk[2] + 1);

            mod_keys.push_back(mod_key);

        } else if (valueKeys[n].substr(0,1) == "R"){
            std::string str34 = valueKeys[n].substr(2,2);
            std::string str45 = valueKeys[n].substr(3,2);

            if (valueKeys[n].substr(0,5) == "RORFR"){
                mod_keys.push_back(valueKeys[n]);

            } else if ((str34 == "FR") || (str34 == "FT") || (str45 == "FR") || (str45 == "FT")) {
                auto p = valueKeys[n].find(":");
                if (p != std::string::npos) {
                    int num = std::stoi(valueKeys[n].substr(p+1));
                    const auto& [r1, r2] = splitSummaryNumber(num);
                    std::string mod_key = valueKeys[n].substr(0,p) + ":" + std::to_string(r1) + "-" + std::to_string(r2);
                    mod_keys.push_back(mod_key);
                } else {
                    mod_keys.push_back(valueKeys[n]);
                }

            } else {
                mod_keys.push_back(valueKeys[n]);
            }

        } else {
            mod_keys.push_back(valueKeys[n]);
        }
    }

    return mod_keys;

}

std::array<int, 3> ExtSmryOutput::ijk_from_global_index(const GridDims& dims, int globInd) const
{

    if (globInd < 0 || static_cast<size_t>(globInd) >= dims[0] * dims[1] * dims[2])
        throw std::invalid_argument("global index out of range");

    std::array<int, 3> result;

    result[0] = globInd % dims[0];
    globInd /= dims[0];
    result[1] = globInd % dims[1];
    result[2] = globInd / dims[1];

    return result;
}


}} // namespace Opm::EclIO
