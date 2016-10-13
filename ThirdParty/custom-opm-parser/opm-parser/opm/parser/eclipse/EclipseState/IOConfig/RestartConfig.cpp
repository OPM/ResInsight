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

#include <algorithm>
#include <iostream>
#include <iterator>

#include <boost/lexical_cast.hpp>

#include <opm/parser/eclipse/Utility/Functional.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <ert/ecl/ecl_util.h>




namespace Opm {

namespace {

inline bool is_int( const std::string& x ) {
    auto is_digit = []( char c ) { return std::isdigit( c ); };

    return !x.empty()
        && ( x.front() == '-' || is_digit( x.front() ) )
        && std::all_of( x.begin() + 1, x.end(), is_digit );
}

constexpr const char* RSTIntegerKeywords[] = { "BASIC",      //  1
                                               "FLOWS",      //  2
                                               "FIP",        //  3
                                               "POT",        //  4
                                               "PBPD",       //  5
                                               "FREQ",       //  6
                                               "PRES",       //  7
                                               "VISC",       //  8
                                               "DEN",        //  9
                                               "DRAIN",      // 10
                                               "KRO",        // 11
                                               "KRW",        // 12
                                               "KRG",        // 13
                                               "PORO",       // 14
                                               "NOGRAD",     // 15
                                               "NORST",      // 16 NORST - not supported
                                               "SAVE",       // 17
                                               "SFREQ",      // 18 SFREQ=?? - not supported
                                               "ALLPROPS",   // 19
                                               "ROCKC",      // 20
                                               "SGTRAP",     // 21
                                               "",           // 22 - Blank - ignored.
                                               "RSSAT",      // 23
                                               "RVSAT",      // 24
                                               "GIMULT",     // 25
                                               "SURFBLK",    // 26
                                               "",           // 27 - PCOW, PCOG, special cased
                                               "STREAM",     // 28 STREAM=?? - not supported
                                               "RK",         // 29
                                               "VELOCITY",   // 30
                                               "COMPRESS" }; // 31

constexpr const char* SCHEDIntegerKeywords[] = { "PRES",    // 1
                                                 "SOIL",    // 2
                                                 "SWAT",    // 3
                                                 "SGAS",    // 4
                                                 "RS",      // 5
                                                 "RV",      // 6
                                                 "RESTART", // 7
                                                 "FIP",     // 8
                                                 "WELLS",   // 9
                                                 "VFPPROD", // 10
                                                 "SUMMARY", // 11
                                                 "CPU",     // 12
                                                 "AQUCT",   // 13
                                                 "WELSPECS",// 14
                                                 "NEWTON",  // 15
                                                 "POILD",   // 16
                                                 "PWAT",    // 17
                                                 "PWATD",   // 18
                                                 "PGAS",    // 19
                                                 "PGASD",   // 20
                                                 "FIPVE",   // 21
                                                 "WOC",     // 22
                                                 "GOC",     // 23
                                                 "WOCDIFF", // 24
                                                 "GOCDIFF", // 25
                                                 "WOCGOC",  // 26
                                                 "ODGAS",   // 27
                                                 "ODWAT",   // 28
                                                 "GDOWAT",  // 29
                                                 "WDOGAS",  // 30
                                                 "OILAPI",  // 31
                                                 "FIPITR",  // 32
                                                 "TBLK",    // 33
                                                 "PBLK",    // 34
                                                 "SALT",    // 35
                                                 "PLYADS",  // 36
                                                 "RK",      // 37
                                                 "FIPSALT", // 38
                                                 "TUNING",  // 39
                                                 "GI",      // 40
                                                 "ROCKC",   // 41
                                                 "SPENWAT", // 42
                                                 "FIPSOL",  // 43
                                                 "SURFBLK", // 44
                                                 "SURFADS", // 45
                                                 "FIPSURF", // 46
                                                 "TRADS",   // 47
                                                 "VOIL",    // 48
                                                 "VWAT",    // 49
                                                 "VGAS",    // 50
                                                 "DENO",    // 51
                                                 "DENW",    // 52
                                                 "DENG",    // 53
                                                 "GASCONC", // 54
                                                 "PB",      // 55
                                                 "PD",      // 56
                                                 "KRW",     // 57
                                                 "KRO",     // 58
                                                 "KRG",     // 59
                                                 "MULT",    // 60
                                                 "UNKNOWN", // 61 61 and 62 are not listed in the manual
                                                 "UNKNOWN", // 62
                                                 "FOAM",    // 63
                                                 "FIPFOAM", // 64
                                                 "TEMP",    // 65
                                                 "FIPTEMP", // 66
                                                 "POTC",    // 67
                                                 "FOAMADS", // 68
                                                 "FOAMDCY", // 69
                                                 "FOAMMOB", // 70
                                                 "RECOV",   // 71
                                                 "FLOOIL",  // 72
                                                 "FLOWAT",  // 73
                                                 "FLOGAS",  // 74
                                                 "SGTRAP",  // 75
                                                 "FIPRESV", // 76
                                                 "FLOSOL",  // 77
                                                 "KRN",     // 78
                                                 "GRAD",    // 79
                                               };

bool is_RPTRST_mnemonic( const std::string& kw ) {
    /* all eclipse 100 keywords we want to not simply ignore. The list is
     * sorted, so we can use binary_search for log(n) lookup. It is important
     * that the list is sorted, but these are all the keywords listed in the
     * manual and unlikely to change at all
     */
    static constexpr const char* valid[] = {
        "ACIP",     "ACIS",     "ALLPROPS", "BASIC",  "BG",       "BO",
        "BW",       "CELLINDX", "COMPRESS", "CONV",   "DEN",      "DRAIN",
        "DRAINAGE", "DYNREG",   "FIP",      "FLORES", "FLOWS",    "FREQ",
        "GIMULT",   "HYDH",     "HYDHFW",   "KRG",    "KRO",      "KRW",
        "NOGRAD",   "NORST",    "NPMREB",   "PBPD",   "PCOG",     "PCOW",
        "PERMREDN", "POIS",     "PORO",     "PORV",   "POT",      "PRES",
        "RFIP",     "RK",       "ROCKC",    "RPORV",  "RSSAT",    "RVSAT",
        "SAVE",     "SDENO",    "SFIP",     "SFREQ",  "SGTRAP",   "SIGM_MOD",
        "STREAM",   "SURFBLK",  "TRAS",     "VELGAS", "VELOCITY", "VELOIL",
        "VELWAT",   "VISC",
    };

    return std::binary_search( std::begin( valid ), std::end( valid ), kw );
}

bool is_RPTSCHED_mnemonic( const std::string& kw ) {
    static constexpr const char* valid[] = {
        "ALKALINE", "ANIONS",  "AQUCT",    "AQUFET",   "AQUFETP",  "BFORG",
        "CATIONS",  "CPU",     "DENG",     "DENO",     "DENW",     "ESALPLY",
        "ESALSUR",  "FFORG",   "FIP",      "FIPFOAM",  "FIPHEAT",  "FIPRESV",
        "FIPSALT",  "FIPSOL",  "FIPSURF",  "FIPTEMP",  "FIPTR",    "FIPVE",
        "FLOGAS",   "FLOOIL",  "FLOSOL",   "FLOWAT",   "FMISC",    "FOAM",
        "FOAMADS",  "FOAMCNM", "FOAMDCY",  "FOAMMOB",  "GASCONC",  "GASSATC",
        "GDOWAT",   "GI",      "GOC",      "GOCDIFF",  "GRAD",     "KRG",
        "KRN",      "KRO",     "KRW",      "MULT",     "NEWTON",   "NOTHING",
        "NPMREB",   "ODGAS",   "ODWAT",    "OILAPI",   "PB",       "PBLK",
        "PBU",      "PD",      "PDEW",     "PGAS",     "PGASD",    "PLYADS",
        "POIL",     "POILD",   "POLYMER",  "POTC",     "POTG",     "POTO",
        "POTW",     "PRES",    "PRESSURE", "PWAT",     "PWATD",    "RECOV",
        "RESTART",  "ROCKC",   "RS",       "RSSAT",    "RV",       "RVSAT",
        "SALT",     "SGAS",    "SGTRAP",   "SIGM_MOD", "SOIL",     "SSOL",
        "SUMMARY",  "SURFADS", "SURFBLK",  "SWAT",     "TBLK",     "TEMP",
        "TRACER",   "TRADS",   "TRDCY",    "TUNING",   "VFPPROD",  "VGAS",
        "VOIL",     "VWAT",    "WDOGAS",   "WELLS",    "WELSPECL", "WELSPECS",
        "WOC",      "WOCDIFF", "WOCGOC",
    };

    return std::binary_search( std::begin( valid ), std::end( valid ), kw );
}

}

    RestartSchedule::RestartSchedule( size_t sched_restart) :
        rptsched_restart_set( true ),
        rptsched_restart( sched_restart )
    {
    }

    RestartSchedule::RestartSchedule( size_t step, size_t b, size_t freq) :
        timestep( step ),
        basic( b ),
        frequency( basic > 2 ? std::max( freq, size_t{ 1 } ) : freq )
    {
        /*
         * if basic > 2 and freq is default (zero) we're looking at an error
         * (every Nth step where N = 0). Instead of throwing we default this to
         * 1 so that basic > 2 and freq unset essentially is
         * write-every-timestep. It could've just as easily been an exception,
         * but to be more robust handling poorly written decks we instead set a
         * reasonable default and carry on.
         */
    }

    bool RestartSchedule::operator!=(const RestartSchedule & rhs) const {
        return !( *this == rhs );
    }

    bool RestartSchedule::operator==( const RestartSchedule& rhs ) const {
        if( this->rptsched_restart_set ) {
            return rhs.rptsched_restart_set
                && this->rptsched_restart == rhs.rptsched_restart;
        }

        return this->timestep == rhs.timestep &&
            this->basic == rhs.basic &&
            this->frequency == rhs.frequency;
    }

inline std::map< std::string, int >
RPTRST_integer( const std::vector< int >& ints ) {
    const size_t PCO_index = 26;
    const size_t BASIC_index = 0;

    std::map< std::string, int > mnemonics;
    const size_t size = std::min( ints.size(), sizeof( RSTIntegerKeywords ) );

    /* fun with special cases. Eclipse seems to ignore the BASIC=0,
     * interpreting it as sort-of "don't modify". Handle this by *not*
     * adding/updating the integer list sourced BASIC mnemonic, should it be
     * zero. I'm not sure if this applies to other mnemonics, but the eclipse
     * manual indicates that any zero here should disable the output.
     *
     * See https://github.com/OPM/opm-parser/issues/886 for reference
     */
    if( size > 0 && ints[ BASIC_index ] != 0 )
        mnemonics[ RSTIntegerKeywords[ BASIC_index ] ] = ints[ BASIC_index ];

    for( size_t i = 1; i < std::min( size, PCO_index ); ++i )
        mnemonics[ RSTIntegerKeywords[ i ] ] = ints[ i ];

    for( size_t i = PCO_index + 1; i < size; ++i )
        mnemonics[ RSTIntegerKeywords[ i ] ] = ints[ i ];

    /* item 27 (index 26) sets both PCOW and PCOG, so we special case it */
    if( ints.size() >= PCO_index ) {
        mnemonics[ "PCOW" ] = ints[ PCO_index ];
        mnemonics[ "PCOG" ] = ints[ PCO_index ];
    }

    return std::move( mnemonics );
}

inline std::map< std::string, int >
RPTSCHED_integer( const std::vector< int >& ints ) {
    const size_t size = std::min( ints.size(), sizeof( SCHEDIntegerKeywords ) );

    std::map< std::string, int > mnemonics;
    for( size_t i = 0; i < size; ++i )
        mnemonics[ SCHEDIntegerKeywords[ i ] ] = ints[ i ];

    return std::move( mnemonics );
}

template< typename F, typename G >
inline std::map< std::string, int > RPT( const DeckKeyword& keyword,
                                         F is_mnemonic,
                                         G integer_mnemonic ) {

    const auto& items = keyword.getStringData();
    const auto ints = std::any_of( items.begin(), items.end(), is_int );
    const auto strs = !std::all_of( items.begin(), items.end(), is_int );

    /* if any of the values are pure integers we assume this is meant to be
     * the slash-terminated list of integers way of configuring. If
     * integers and non-integers are mixed, this is an error.
     */
    if( ints && strs ) throw std::runtime_error(
            "RPTRST does not support mixed mnemonics and integer list."
            );

    auto stoi = []( const std::string& str ) { return std::stoi( str ); };
    if( ints )
        return integer_mnemonic( fun::map( stoi, items ) );

    std::map< std::string, int > mnemonics;

    for( const auto& mnemonic : items ) {
        const auto pos = mnemonic.find( '=' );

        std::string base = mnemonic.substr( 0, pos );
        if( !is_mnemonic( base ) ) continue;

        const int val = pos != std::string::npos
                      ? std::stoi( mnemonic.substr( pos + 1 ) )
                      : 1;

        mnemonics.emplace( base, val );
    }

    return std::move( mnemonics );
}

void expand_RPTRST_mnemonics(std::map< std::string, int >& mnemonics) {
    const auto allprops = mnemonics.find( "ALLPROPS");
    if (allprops != mnemonics.end()) {
        const auto value = allprops->second;
        mnemonics.erase( "ALLPROPS" );

        for (const auto& kw : {"BG","BO","BW","KRG","KRO","KRW","VOIL","VGAS","VWAT","DEN"})
            mnemonics[kw] = value;

    }
}


inline std::pair< std::map< std::string, int >, RestartSchedule >
RPTRST( const DeckKeyword& keyword, RestartSchedule prev, size_t step ) {
    auto mnemonics = RPT( keyword, is_RPTRST_mnemonic, RPTRST_integer );

    const bool has_freq  = mnemonics.find( "FREQ" )  != mnemonics.end();
    const bool has_basic = mnemonics.find( "BASIC" ) != mnemonics.end();

    expand_RPTRST_mnemonics( mnemonics );

    if( !has_freq && !has_basic ) return { std::move( mnemonics ), {} };

    const auto basic = has_basic ? mnemonics.at( "BASIC" ) : prev.basic;
    const auto freq  = has_freq  ? mnemonics.at( "FREQ"  ) : prev.frequency;

    return { std::move( mnemonics ), { step, basic, freq } };
}

inline std::pair< std::map< std::string, int >, RestartSchedule >
RPTSCHED( const DeckKeyword& keyword ) {
    auto mnemonics = RPT( keyword, is_RPTSCHED_mnemonic, RPTSCHED_integer );

    if( mnemonics.count( "NOTHING" ) )
        return { std::move( mnemonics ), { 0 } };

    if( mnemonics.count( "RESTART" ) )
        return { std::move( mnemonics ), size_t( mnemonics.at( "RESTART" ) ) };

    return { std::move( mnemonics ), {} };
}



void RestartConfig::handleScheduleSection(const SCHEDULESection& schedule) {
    size_t current_step = 1;
    RestartSchedule unset;

    auto ignore_RPTSCHED_RESTART = []( decltype( restart_schedule )& x ) {
        return x.back().basic > 2;
    };

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
        if( this->m_timemap->size() <= current_step ) continue;

        const bool is_RPTRST = name == "RPTRST";
        const auto& prev_sched = this->restart_schedule.back();

        auto config = is_RPTRST
                      ? RPTRST( keyword, prev_sched, current_step )
                      : RPTSCHED( keyword );

        /* add the missing entries from the previous step */
        {
            auto& mnemonics = config.first;
            const auto& prev_mnemonics = this->restart_keywords.back();
            mnemonics.insert( prev_mnemonics.begin(), prev_mnemonics.end() );

            if( mnemonics.find( "NOTHING" ) != mnemonics.end() )
                mnemonics.clear();

            this->restart_keywords.update( current_step , mnemonics );
        }
        const bool ignore_RESTART =
            !is_RPTRST && ignore_RPTSCHED_RESTART( this->restart_schedule );

        const auto& rs = config.second;
        if( rs == unset || ignore_RESTART ) continue;

        if( 6 == rs.rptsched_restart || 6 == rs.basic )
            throw std::runtime_error(
                    "OPM does not support the RESTART=6 setting "
                    "(write restart file every timestep)"
                );

        this->restart_schedule.update( current_step, rs );
    }
}

    bool RestartSchedule::writeRestartFile( size_t input_timestep , const TimeMap& timemap) const {
        if (this->rptsched_restart_set && (this->rptsched_restart > 0))
            return true;

        switch (basic) {
         //Do not write restart files
        case 0:  return false;

        //Write restart file every report time
        case 1: return true;

        //Write restart file every report time
        case 2: return true;

        //Every n'th report time
        case 3: return  ((input_timestep % this->frequency) == 0) ? true : false;

        //First reportstep of every year, or if n > 1, n'th years
        case 4: return timemap.isTimestepInFirstOfMonthsYearsSequence(input_timestep, true , this->timestep, this->frequency);

         //First reportstep of every month, or if n > 1, n'th months
        case 5: return timemap.isTimestepInFirstOfMonthsYearsSequence(input_timestep, false , this->timestep, this->frequency);

        default: return false;
        }
    }



    RestartConfig::RestartConfig( const Deck& deck ) :
        RestartConfig( SCHEDULESection( deck ),
                       SOLUTIONSection( deck ),
                       std::make_shared< const TimeMap >( deck ))
    {}


    RestartConfig::RestartConfig( const SCHEDULESection& schedule,
                                  const SOLUTIONSection& solution,
                                  std::shared_ptr< const TimeMap > timemap) :
        m_timemap( timemap ),
        m_first_restart_step( -1 ),
        restart_schedule( timemap, { 0, 0, 1 } ),
        restart_keywords( timemap, {} )
    {
        handleSolutionSection( solution );
        handleScheduleSection( schedule );

        initFirstOutput( );
    }


    RestartSchedule RestartConfig::getNode( size_t timestep ) const{
        return restart_schedule.get(timestep);
    }


    bool RestartConfig::getWriteRestartFile(size_t timestep) const {
        if (0 == timestep)
            return m_write_initial_RST_file;

        {
            RestartSchedule ts_restart_config = getNode( timestep );
            return ts_restart_config.writeRestartFile( timestep , *m_timemap );
        }
    }


    const std::map< std::string, int >& RestartConfig::getRestartKeywords( size_t timestep ) const {
        return restart_keywords.at( timestep );
    }


    int RestartConfig::getKeyword( const std::string& keyword, size_t timeStep) const {
        const std::map< std::string, int >& keywords = this->getRestartKeywords( timeStep );
        const auto iter  = keywords.find( keyword );
        if (iter == keywords.end()) {
            if (is_RPTRST_mnemonic( keyword ))
                return 0;
            else
                throw std::invalid_argument("The mnenomic " + keyword + " is not recognized");
        } else
            return iter->second;
    }

    /*
      Will initialize the internal variable holding the first report
      step when restart output is queried.

      The reason we are interested in this report step is that when we
      reach this step the output files should be opened with mode 'w'
      - whereas for subsequent steps they should be opened with mode
      'a'.
    */

    void RestartConfig::initFirstOutput( ) {
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


    void RestartConfig::handleSolutionSection(const SOLUTIONSection& solutionSection) {
        if (solutionSection.hasKeyword("RPTRST")) {
            const auto& rptrstkeyword        = solutionSection.getKeyword("RPTRST");

            const auto rptrst = RPTRST( rptrstkeyword, {}, 0 );
            this->restart_keywords.updateInitial( rptrst.first );
            this->restart_schedule.updateInitial( rptrst.second );
            setWriteInitialRestartFile(true); // Guessing on eclipse rules for write of initial RESTART file (at time 0):
                                              // Write of initial restart file is (due to the eclipse reference manual)
                                              // governed by RPTSOL RESTART in solution section,
                                              // if RPTSOL RESTART > 1 initial restart file is written.
                                              // but - due to initial restart file written from Eclipse
                                              // for data where RPTSOL RESTART not set - guessing that
                                              // when RPTRST is set in SOLUTION (no basic though...) -> write inital restart.
        } //RPTRST


        if (solutionSection.hasKeyword("RPTSOL") && (m_timemap->size() > 0)) {
            handleRPTSOL(solutionSection.getKeyword("RPTSOL"));
        } //RPTSOL
    }


    void RestartConfig::overrideRestartWriteInterval(size_t interval) {
        size_t step = 0;
        /* write restart files if the interval is non-zero. The restart
         * mnemonic (setting) that governs restart-on-interval is BASIC=3
         */
        size_t basic = interval > 0 ? 3 : 0;

        RestartSchedule rs( step, basic, interval );
        restart_schedule.globalReset( rs );

        setWriteInitialRestartFile( interval > 0 );
    }


    void RestartConfig::setWriteInitialRestartFile(bool writeInitialRestartFile) {
        m_write_initial_RST_file = writeInitialRestartFile;
    }


    void RestartConfig::handleRPTSOL( const DeckKeyword& keyword) {
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



    std::string RestartConfig::getRestartFileName(const std::string& restart_base, int report_step, bool unified , bool fmt_file) {

        ecl_file_enum file_type = (unified) ? ECL_UNIFIED_RESTART_FILE : ECL_RESTART_FILE;
        char * c_str = ecl_util_alloc_filename( NULL , restart_base.c_str() , file_type, fmt_file , report_step);
        std::string restart_filename = c_str;
        free( c_str );

        return restart_filename;
    }


    int RestartConfig::getFirstRestartStep() const {
        return m_first_restart_step;
    }

}
