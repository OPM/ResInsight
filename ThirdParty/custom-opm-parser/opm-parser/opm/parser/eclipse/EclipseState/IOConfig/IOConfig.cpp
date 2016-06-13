/*
  Copyright 2015 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <iostream>
#include <iterator>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <ert/ecl/ecl_util.h>




namespace Opm {

    IOConfig::IOConfig(const std::string& input_path):
        m_write_INIT_file(false),
        m_write_EGRID_file(true),
        m_write_initial_RST_file(false),
        m_UNIFIN(false),
        m_UNIFOUT(false),
        m_FMTIN(false),
        m_FMTOUT(false),
        m_ignore_RPTSCHED_RESTART(false),
        m_deck_filename(input_path),
        m_output_enabled(true)
    {
        m_output_dir = ".";
        m_base_name = "";
        if (!input_path.empty()) {
            boost::filesystem::path path( input_path );
            m_base_name = path.stem().string();
            m_output_dir = path.parent_path().string();
            if (m_output_dir == "")
                m_output_dir = ".";
        }
    }

    bool IOConfig::getWriteEGRIDFile() const {
        return m_write_EGRID_file;
    }

    bool IOConfig::getWriteINITFile() const {
        return m_write_INIT_file;
    }

    bool IOConfig::getWriteRestartFile(size_t timestep) const {
        bool write_restart_ts = false;

        if (0 == timestep) {
            write_restart_ts = m_write_initial_RST_file;
        } else if (m_restart_output_config) {
            restartConfig ts_restart_config = m_restart_output_config->get(timestep);

            //Look at rptsched restart setting
            if (ts_restart_config.rptsched_restart_set) {
                if (ts_restart_config.rptsched_restart > 0) {
                    write_restart_ts = true;
                }
            } else { //Look at rptrst basic setting
                switch (ts_restart_config.basic) {
                    case 0: //Do not write restart files
                        write_restart_ts = false;
                        break;
                    case 1: //Write restart file every report time
                        write_restart_ts = true;
                        break;
                    case 2: //Write restart file every report time
                        write_restart_ts = true;
                        break;
                    case 3: //Every n'th report time
                        write_restart_ts = getWriteRestartFileFrequency(timestep, ts_restart_config.timestep, ts_restart_config.frequency);
                        break;
                    case 4: //First reportstep of every year, or if n > 1, n'th years
                        write_restart_ts = getWriteRestartFileFrequency(timestep, ts_restart_config.timestep, ts_restart_config.frequency, true);
                        break;
                    case 5: //First reportstep of every month, or if n > 1, n'th months
                        write_restart_ts = getWriteRestartFileFrequency(timestep, ts_restart_config.timestep, ts_restart_config.frequency, false, true);
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

        return write_restart_ts;
    }


    bool IOConfig::getWriteRestartFileFrequency(size_t timestep,
                                                size_t start_timestep,
                                                size_t frequency,
                                                bool   years,
                                                bool   months) const {
        bool write_restart_file = false;
        if ((!years && !months) && (timestep >= start_timestep)) {
            write_restart_file = ((timestep % frequency) == 0) ? true : false;
        } else {
              write_restart_file = m_timemap->isTimestepInFirstOfMonthsYearsSequence(timestep, years, start_timestep, frequency);

        }
        return write_restart_file;
    }


    void IOConfig::handleRPTRSTBasic(TimeMapConstPtr timemap, size_t timestep, size_t basic, size_t frequency, bool update_default, bool reset_global) {

        if (6 == basic )
        {
            throw std::runtime_error("OPM does not support the RPTRST BASIC=6 setting (write restart file every timestep)");
        }

        if (basic > 2) {
            m_ignore_RPTSCHED_RESTART = true;
        } else {
            m_ignore_RPTSCHED_RESTART = false;
        }


        assertTimeMap( timemap );
        {
            restartConfig rs;
            rs.timestep  = timestep;
            rs.basic     = basic;
            rs.frequency = frequency;
            rs.rptsched_restart_set = false;
            rs.rptsched_restart     = 0;

            if (update_default) {
                m_restart_output_config->updateInitial(rs);
            }
            else if (reset_global) {
                m_restart_output_config->globalReset(rs);
            }
            else {
                m_restart_output_config->update(timestep, rs);
            }
        }
    }


    void IOConfig::handleRPTSCHEDRestart(TimeMapConstPtr timemap, size_t timestep, size_t restart) {
        if (6 == restart )
        {
            throw std::runtime_error("OPM does not support the RPTSCHED RESTART=6 setting (write restart file every timestep)");
        }

        if (m_ignore_RPTSCHED_RESTART) { //If previously RPTRST BASIC has been set >2, ignore RPTSCHED RESTART
            return;
        }

        assertTimeMap( timemap );
        {
            restartConfig rs;
            rs.timestep             = 0;
            rs.basic                = 0;
            rs.frequency            = 0;
            rs.rptsched_restart     = restart;
            rs.rptsched_restart_set = true;

            m_restart_output_config->update(timestep, rs);
        }
    }


    void IOConfig::assertTimeMap(TimeMapConstPtr timemap) {
        if (!m_timemap) {
            restartConfig rs;
            rs.timestep  = 0;
            rs.basic     = 0;
            rs.frequency = 1;
            rs.rptsched_restart_set = false;
            rs.rptsched_restart     = 0;

            m_timemap = timemap;
            m_restart_output_config = std::make_shared<DynamicState<restartConfig>>(timemap, rs);
        }
    }


    /*
      Will initialize the two internal variables holding the first
      report step when restart and rft output is queried.

      The reason we are interested in this report step is that when we
      reach this step the output files should be opened with mode 'w'
      - whereas for subsequent steps they should be opened with mode
      'a'.
    */

    void IOConfig::initFirstOutput(const Schedule& schedule) {
        m_first_restart_step = -1;
        m_first_rft_step = -1;
        assertTimeMap( schedule.getTimeMap() );
        {
            size_t report_step = 0;
            while (true) {
                if (getWriteRestartFile(report_step)) {
                    m_first_restart_step = report_step;
                    break;
                }
                report_step++;
                if (report_step == m_timemap->size())
                    break;
            }
        }
        {
            for (const auto& well : schedule.getWells( )) {
                int well_output = well->firstRFTOutput();
                if (well_output >= 0) {
                    if ((m_first_rft_step < 0) || (well_output < m_first_rft_step))
                        m_first_rft_step = well_output;
                }
            }
        }
    }


    void IOConfig::handleSolutionSection(TimeMapConstPtr timemap, std::shared_ptr<const SOLUTIONSection> solutionSection) {
        if (solutionSection->hasKeyword("RPTRST")) {
            const auto& rptrstkeyword        = solutionSection->getKeyword("RPTRST");
            const auto& record = rptrstkeyword.getRecord(0);
            const auto& item     = record.getItem(0);

            bool handleRptrstBasic = false;
            size_t basic = 0;
            size_t freq  = 0;

            for (size_t index = 0; index < item.size(); ++index) {
                if (item.hasValue(index)) {
                    std::string mnemonics = item.get< std::string >(index);
                    std::size_t found_basic = mnemonics.find("BASIC=");
                    if (found_basic != std::string::npos) {
                        std::string basic_no = mnemonics.substr(found_basic+6, found_basic+7);
                        basic = atoi(basic_no.c_str());
                        handleRptrstBasic = true;
                    }

                    std::size_t found_freq = mnemonics.find("FREQ=");
                    if (found_freq != std::string::npos) {
                        std::string freq_no = mnemonics.substr(found_freq+5, found_freq+6);
                        freq = atoi(freq_no.c_str());
                    }
                }
            }

            if (handleRptrstBasic) {
                size_t currentStep = 0;
                handleRPTRSTBasic(timemap, currentStep, basic, freq, true);
            }


            setWriteInitialRestartFile(true); // Guessing on eclipse rules for write of initial RESTART file (at time 0):
                                              // Write of initial restart file is (due to the eclipse reference manual)
                                              // governed by RPTSOL RESTART in solution section,
                                              // if RPTSOL RESTART > 1 initial restart file is written.
                                              // but - due to initial restart file written from Eclipse
                                              // for data where RPTSOL RESTART not set - guessing that
                                              // when RPTRST is set in SOLUTION (no basic though...) -> write inital restart.

        } //RPTRST


        if (solutionSection->hasKeyword("RPTSOL") && (timemap->size() > 0)) {
            handleRPTSOL(solutionSection->getKeyword("RPTSOL"));
        } //RPTSOL
    }



    void IOConfig::handleGridSection(std::shared_ptr<const GRIDSection> gridSection) {
        m_write_INIT_file = gridSection->hasKeyword("INIT");

        if (gridSection->hasKeyword("GRIDFILE")) {
            const auto& gridfilekeyword = gridSection->getKeyword("GRIDFILE");
            if (gridfilekeyword.size() > 0) {
                const auto& rec = gridfilekeyword.getRecord(0);
                const auto& item1 = rec.getItem(0);
                if ((item1.hasValue(0)) && (item1.get< int >(0) !=  0)) {
                    std::cerr << "IOConfig: Reading GRIDFILE keyword from GRID section: Output of GRID file is not supported" << std::endl;
                }
                if (rec.size() > 1) {
                    const auto& item2 = rec.getItem(1);
                    if ((item2.hasValue(0)) && (item2.get< int >(0) ==  0)) {
                        m_write_EGRID_file = false;
                    }
                }
            }
        }
        if (gridSection->hasKeyword("NOGGF")) {
            m_write_EGRID_file = false;
        }
    }


    void IOConfig::handleRunspecSection(std::shared_ptr<const RUNSPECSection> runspecSection) {
        m_FMTIN   = runspecSection->hasKeyword("FMTIN");   //Input files are formatted
        m_FMTOUT  = runspecSection->hasKeyword("FMTOUT");  //Output files are to be formatted
        m_UNIFIN  = runspecSection->hasKeyword("UNIFIN");  //Input files are unified
        m_UNIFOUT = runspecSection->hasKeyword("UNIFOUT"); //Output files are to be unified
    }


    void IOConfig::overrideRestartWriteInterval(size_t interval) {
        if (interval > 0) {
            size_t basic = 3;
            size_t timestep = 0;
            handleRPTRSTBasic(m_timemap, timestep, basic, interval, false, true);
            setWriteInitialRestartFile(true);
        } else {
            size_t basic = 0;
            size_t timestep = 0;
            handleRPTRSTBasic(m_timemap, timestep, basic, interval, false, true);
            setWriteInitialRestartFile(false);
        }
    }

    bool IOConfig::getUNIFIN() const {
        return m_UNIFIN;
    }

    bool IOConfig::getUNIFOUT() const {
        return m_UNIFOUT;
    }

    bool IOConfig::getFMTIN() const {
        return m_FMTIN;
    }

    bool IOConfig::getFMTOUT() const {
        return m_FMTOUT;
    }

    void IOConfig::setWriteInitialRestartFile(bool writeInitialRestartFile) {
        m_write_initial_RST_file = writeInitialRestartFile;
    }


    void IOConfig::handleRPTSOL( const DeckKeyword& keyword) {
        const auto& record = keyword.getRecord(0);

        size_t restart = 0;
        size_t found_mnemonic_RESTART = 0;
        bool handle_RPTSOL_RESTART = false;

        const auto& item = record.getItem(0);


        for (size_t index = 0; index < item.size(); ++index) {
            const std::string& mnemonic = item.get< std::string >(index);

            found_mnemonic_RESTART = mnemonic.find("RESTART=");
            if (found_mnemonic_RESTART != std::string::npos) {
                std::string restart_no = mnemonic.substr(found_mnemonic_RESTART+8, mnemonic.size());
                restart = boost::lexical_cast<size_t>(restart_no);
                handle_RPTSOL_RESTART = true;
            }
        }


        /* If no RESTART mnemonic is found, either it is not present or we might
           have an old data set containing integer controls instead of mnemonics.
           Restart integer switch is integer control nr 7 */

        if (found_mnemonic_RESTART == std::string::npos) {
            if (item.size() >= 7)  {
                const std::string& integer_control = item.get< std::string >(6);
                try {
                    restart = boost::lexical_cast<size_t>(integer_control);
                    handle_RPTSOL_RESTART = true;
                } catch (boost::bad_lexical_cast &) {
                    //do nothing
                }
            }
        }

        if (handle_RPTSOL_RESTART) {
            if (restart > 1) {
                setWriteInitialRestartFile(true);
            } else {
                setWriteInitialRestartFile(false);
            }
        }
    }


    boost::gregorian::date IOConfig::getTimestepDate(size_t reportStep) const {
        auto time = (*m_timemap)[reportStep];
        return time.date();
    }


    void IOConfig::dumpRestartConfig() const {
        for (size_t reportStep = 0; reportStep < m_timemap->size(); reportStep++) {
            if (getWriteRestartFile(reportStep)) {
                auto time = (*m_timemap)[reportStep];
                boost::gregorian::date date = time.date();
                printf("%04zu : %02hu/%02hu/%hu \n" , reportStep ,
		       date.day().as_number() , date.month().as_number() , static_cast<unsigned short>(date.year()));
            }
        }
    }


    std::string IOConfig::getRestartFileName(const std::string& restart_base, int report_step, bool output) const {
        bool unified  = output ? getUNIFOUT() : getUNIFIN();
        bool fmt_file = output ? getFMTOUT()  : getFMTIN();

        ecl_file_enum file_type = (unified) ? ECL_UNIFIED_RESTART_FILE : ECL_RESTART_FILE;
        char * c_str = ecl_util_alloc_filename( NULL , restart_base.c_str() , file_type, fmt_file , report_step);
        std::string restart_filename = c_str;
        free( c_str );

        return restart_filename;
    }


    int IOConfig::getFirstRestartStep() const {
        return m_first_restart_step;
    }


    int IOConfig::getFirstRFTStep() const {
        return m_first_rft_step;
    }

    bool IOConfig::getOutputEnabled(){
        return m_output_enabled;
    }

    void IOConfig::setOutputEnabled(bool enabled){
        m_output_enabled = enabled;
    }

    std::string IOConfig::getOutputDir() {
        return m_output_dir;
    }

    void IOConfig::setOutputDir(const std::string& outputDir) {
        m_output_dir = outputDir;
    }

    const std::string& IOConfig::getBaseName() const {
        return m_base_name;
    }

    void IOConfig::setBaseName(std::string baseName) {
        m_base_name = baseName;
    }

} //namespace Opm
