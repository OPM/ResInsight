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
#include <opm/parser/eclipse/Deck/SCHEDULESection.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <ert/ecl/ecl_util.h>




namespace Opm {

    namespace {
        const char* default_dir = ".";

        inline std::string basename( const std::string& path ) {
            return boost::filesystem::path( path ).stem().string();
        }

        inline std::string outputdir( const std::string& path ) {
            auto dir = boost::filesystem::path( path ).parent_path().string();

            if( dir.empty() ) return default_dir;

            return dir;
        }

        inline bool is_int( const std::string& x ) {
            auto is_digit = []( char c ) { return std::isdigit( c ); };

            return !x.empty()
                && ( x.front() == '-' || is_digit( x.front() ) )
                && std::all_of( x.begin() + 1, x.end(), is_digit );
        }
    }

    IOConfig::restartConfig IOConfig::rptrst( const DeckKeyword& kw, size_t step ) {

        const auto& items = kw.getStringData();

        /* if any of the values are pure integers we assume this is meant to be
         * the slash-terminated list of integers way of configuring. If
         * integers and non-integers are mixed, this is an error.
         */
        const auto ints = std::any_of( items.begin(), items.end(), is_int );
        const auto strs = !std::all_of( items.begin(), items.end(), is_int );

        if( ints && strs ) throw std::runtime_error(
                "RPTRST does not support mixed mnemonics and integer list."
            );

        size_t basic = 1;
        size_t freq = 0;
        bool found_basic = false;

        for( const auto& mnemonic : items ) {

            const auto freq_pos = mnemonic.find( "FREQ=" );
            if( freq_pos != std::string::npos ) {
                freq = std::stoul( mnemonic.substr( freq_pos + 5 ) );
            }

            const auto basic_pos = mnemonic.find( "BASIC=" );
            if( basic_pos != std::string::npos ) {
                basic = std::stoul( mnemonic.substr( basic_pos + 6 ) );
                found_basic = true;
            }
        }

        if( found_basic ) return restartConfig( step, basic, freq );

        /* If no BASIC mnemonic is found, either it is not present or we might
         * have an old data set containing integer controls instead of mnemonics.
         * BASIC integer switch is integer control nr 1, FREQUENCY is integer
         * control nr 6.
         */

        /* mnemonics, but without basic and freq. Effectively ignored */
        if( !ints ) return {};

        const int BASIC_index = 0;
        const int FREQ_index = 5;

        if( items.size() > BASIC_index )
            basic = std::stoul( items[ BASIC_index ] );

        // Peculiar special case in eclipse, - not documented
        // This ignore of basic = 0 for the integer mnemonics case
        // is done to make flow write restart file at the same intervals
        // as eclipse for the Norne data set. There might be some rules
        // we are missing here.
        if( 0 == basic ) return {};

        if( items.size() > FREQ_index ) // if frequency is set
            freq = std::stoul( items[ FREQ_index ] );

        return restartConfig( step, basic, freq );
    }

    IOConfig::restartConfig IOConfig::rptsched( const DeckKeyword& keyword ) {
        size_t restart = 0;
        bool restart_found = false;

        const auto& items = keyword.getStringData();
        const auto ints = std::any_of( items.begin(), items.end(), is_int );
        const auto strs = !std::all_of( items.begin(), items.end(), is_int );

        if( ints && strs ) throw std::runtime_error(
                "RPTSCHED does not support mixed mnemonics and integer list."
            );

        for( const auto& mnemonic : items  ) {
            const auto restart_pos = mnemonic.find( "RESTART=" );
            if( restart_pos != std::string::npos ) {
                restart = std::stoul( mnemonic.substr( restart_pos + 8 ) );
                restart_found = true;
            }

            const auto nothing_pos = mnemonic.find( "NOTHING" );
            if( nothing_pos != std::string::npos ) {
                restart = 0;
                restart_found = true;
            }
        }

        if( restart_found ) return restartConfig( restart );


        /* No RESTART or NOTHING found, but it is not an integer list */
        if( strs ) return {};

        /* We might have an old data set containing integer controls instead of
         * mnemonics. Restart integer switch is integer control nr 7
         */

        const int RESTART_index = 6;

        if( items.size() <= RESTART_index ) return {};

        return restartConfig( std::stoul( items[ RESTART_index ] ) );
    }

    DynamicState< IOConfig::restartConfig > IOConfig::rstconf(
            const SCHEDULESection& schedule,
            std::shared_ptr< const TimeMap > timemap ) {

        size_t current_step = 1;
        bool ignore_RPTSCHED_restart = false;
        restartConfig unset;

        DynamicState< IOConfig::restartConfig >
            restart_config( timemap, restartConfig( 0, 0, 1 ) );

        for( const auto& keyword : schedule ) {
            const auto& name = keyword.name();

            if( name == "DATES" ) {
                current_step += keyword.size();
                continue;
            }

            if( name == "TSTEP" ) {
                current_step += keyword.getRecord( 0 ).getItem( 0 ).size();
                continue;
            }

            if( !( name == "RPTRST" || name == "RPTSCHED" ) ) continue;

            if( timemap->size() <= current_step ) continue;

            const bool is_RPTRST = name == "RPTRST";

            if( !is_RPTRST && ignore_RPTSCHED_restart ) continue;

            const auto rs = is_RPTRST
                          ? rptrst( keyword, current_step - 1 )
                          : rptsched( keyword );

            if( is_RPTRST ) ignore_RPTSCHED_restart = rs.basic > 2;

            /* we're using the default state of restartConfig to signal "no
             * update". The default state is non-sensical
             */
            if( rs == unset ) continue;

            if( 6 == rs.rptsched_restart || 6 == rs.basic )
                throw std::runtime_error(
                    "OPM does not support the RESTART=6 setting"
                    "(write restart file every timestep)"
                );

            restart_config.update( current_step, rs );
        }

        return restart_config;
    }

    IOConfig::IOConfig( const Deck& deck ) :
        IOConfig( GRIDSection( deck ),
                  RUNSPECSection( deck ),
                  SCHEDULESection( deck ),
                  std::make_shared< const TimeMap >( deck ),
                  deck.getDataFile() )
    {}

    IOConfig::IOConfig( const std::string& input_path ) :
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) ),
        m_base_name( basename( input_path ) )
    {}

    static inline bool write_egrid_file( const GRIDSection& grid ) {
        if( grid.hasKeyword( "NOGGF" ) ) return false;
        if( !grid.hasKeyword( "GRIDFILE" ) ) return true;

        const auto& keyword = grid.getKeyword( "GRIDFILE" );

        if( keyword.size() == 0 ) return false;

        const auto& rec = keyword.getRecord( 0 );
        const auto& item1 = rec.getItem( 0 );

        if( item1.hasValue( 0 ) && item1.get< int >( 0 ) != 0 ) {
            std::cerr << "IOConfig: Reading GRIDFILE keyword from GRID section: "
                      << "Output of GRID file is not supported. "
                      << "Supported format: EGRID"
                      << std::endl;
            return true;
        }

        if( rec.size() < 1 ) return true;

        const auto& item2 = rec.getItem( 1 );
        return !item2.hasValue( 0 ) || item2.get< int >( 0 ) != 0;
    }

    IOConfig::IOConfig( const GRIDSection& grid,
                        const RUNSPECSection& runspec,
                        const SCHEDULESection& schedule,
                        std::shared_ptr< const TimeMap > timemap,
                        const std::string& input_path ) :
        m_timemap( timemap ),
        m_write_INIT_file( grid.hasKeyword( "INIT" ) ),
        m_write_EGRID_file( write_egrid_file( grid ) ),
        m_UNIFIN( runspec.hasKeyword( "UNIFIN" ) ),
        m_UNIFOUT( runspec.hasKeyword( "UNIFOUT" ) ),
        m_FMTIN( runspec.hasKeyword( "FMTIN" ) ),
        m_FMTOUT( runspec.hasKeyword( "FMTOUT" ) ),
        m_deck_filename( input_path ),
        m_output_dir( outputdir( input_path ) ),
        m_base_name( basename( input_path ) ),
        m_restart_output_config( std::make_shared< DynamicState< restartConfig > >(
                rstconf( schedule, timemap ) ) )
    {}

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
        assertTimeMap( this->m_timemap );
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

            auto rs = rptrst( rptrstkeyword, 0 );
            if( rs != restartConfig() )
                m_restart_output_config->updateInitial( rs );

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

    void IOConfig::overrideRestartWriteInterval(size_t interval) {
        size_t step = 0;
        /* write restart files if the interval is non-zero. The restart
         * mnemonic (setting) that governs restart-on-interval is BASIC=3
         */
        size_t basic = interval > 0 ? 3 : 0;

        restartConfig rs( step, basic, interval );
        m_restart_output_config->globalReset( rs );

        setWriteInitialRestartFile( interval > 0 );
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
