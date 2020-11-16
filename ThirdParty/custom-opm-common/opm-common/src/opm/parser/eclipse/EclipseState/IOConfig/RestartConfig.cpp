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
#include <climits>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <sstream>

#include <opm/parser/eclipse/Utility/Functional.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>

#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>

#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>




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

    RestartSchedule RestartSchedule::serializeObject()
    {
        RestartSchedule result(1, 2, 3);
        result.rptsched_restart_set = true;
        result.rptsched_restart = 4;

        return result;
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

    return mnemonics;
}

inline std::map< std::string, int >
RPTSCHED_integer( const std::vector< int >& ints ) {
    const size_t size = std::min( ints.size(), sizeof( SCHEDIntegerKeywords ) );

    std::map< std::string, int > mnemonics;
    for( size_t i = 0; i < size; ++i )
        mnemonics[ SCHEDIntegerKeywords[ i ] ] = ints[ i ];

    return mnemonics;
}

template< typename F, typename G >
inline std::map< std::string, int > RPT( const DeckKeyword& keyword,
                                         const ParseContext& parseContext,
                                         ErrorGuard& errors,
                                         F is_mnemonic,
                                         G integer_mnemonic ) {

    std::vector<std::string> items;
    const auto& deck_items = keyword.getStringData();
    const auto ints = std::any_of( deck_items.begin(), deck_items.end(), is_int );
    const auto strs = !std::all_of( deck_items.begin(), deck_items.end(), is_int );

    /* if any of the values are pure integers we assume this is meant to be the
     * slash-terminated list of integers way of configuring. If integers and
     * non-integers are mixed, this is an error; however if the error mode
     * RPT_MIXED_STYLE is permissive we try some desperate heuristics to
     * interpret this as list of mnemonics. See the the documentation of the
     * RPT_MIXED_STYLE error handler for more details.
     */
    auto stoi = []( const std::string& str ) { return std::stoi( str ); };
    if( !strs )
        return integer_mnemonic( fun::map( stoi, deck_items ) );


    if (ints && strs) {
        const auto& location = keyword.location();
        std::string msg = "Mixed style input is not allowed for keyword: " + keyword.name() + " at " + location.filename + "(" + std::to_string( location.lineno ) + ")";
        parseContext.handleError(ParseContext::RPT_MIXED_STYLE, msg, errors);

        std::vector<std::string> stack;
        for (size_t index=0; index < deck_items.size(); index++) {
            if (is_int(deck_items[index])) {

                if (stack.size() < 2) {
                    std::string errmsg = "Can not interpret " + keyword.name() + " at " + location.filename + "(" + std::to_string( location.lineno ) + ")";
                    throw std::invalid_argument(errmsg);
                }

                if (stack.back() == "=") {
                    stack.pop_back();
                    std::string mnemonic = stack.back();
                    stack.pop_back();

                    items.insert(items.begin(), stack.begin(), stack.end());
                    stack.clear();
                    items.push_back( mnemonic + "=" + deck_items[index]);
                } else {
                    std::string errmsg = "Can not interpret " + keyword.name() + " at " + location.filename + "(" + std::to_string( location.lineno ) + ")";
                    throw std::invalid_argument(errmsg);
                }

            } else
                stack.push_back(deck_items[index]);
        }
        items.insert(items.begin(), stack.begin(), stack.end());
    } else
        items = deck_items;

    std::map< std::string, int > mnemonics;
    for( const auto& mnemonic : items ) {
        const auto sep_pos = mnemonic.find_first_of( "= " );

        std::string base = mnemonic.substr( 0, sep_pos );
        if( !is_mnemonic( base ) ) {
            parseContext.handleError(ParseContext::RPT_UNKNOWN_MNEMONIC, "The mnemonic: " + base + " is not recognized.", errors);
            continue;
        }

        int val = 1;
        if (sep_pos != std::string::npos) {
            const auto value_pos = mnemonic.find_first_not_of("= ", sep_pos);
            if (value_pos != std::string::npos)
                val = std::stoi(mnemonic.substr(value_pos));
        }

        mnemonics.emplace( base, val );
    }

    return mnemonics;
}

inline void expand_RPTRST_mnemonics(std::map< std::string, int >& mnemonics) {
    const auto allprops = mnemonics.find( "ALLPROPS");
    if (allprops != mnemonics.end()) {
        const auto value = allprops->second;
        mnemonics.erase( "ALLPROPS" );

        for (const auto& kw : {"BG","BO","BW","KRG","KRO","KRW","VOIL","VGAS","VWAT","DEN"})
            mnemonics[kw] = value;

    }
}


inline std::pair< std::map< std::string, int >, RestartSchedule >
RPTRST( const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors, RestartSchedule prev, size_t step ) {
    auto mnemonics = RPT( keyword, parseContext, errors, is_RPTRST_mnemonic, RPTRST_integer );

    const bool has_freq  = mnemonics.find( "FREQ" )  != mnemonics.end();
    const bool has_basic = mnemonics.find( "BASIC" ) != mnemonics.end();

    expand_RPTRST_mnemonics( mnemonics );

    if( !has_freq && !has_basic ) return { std::move( mnemonics ), {} };

    const auto basic = has_basic ? mnemonics.at( "BASIC" ) : prev.basic;
    const auto freq  = has_freq  ? mnemonics.at( "FREQ"  ) : prev.frequency;

    return { std::move( mnemonics ), { step, basic, freq } };
}

inline std::pair< std::map< std::string, int >, RestartSchedule >
RPTSCHED( const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors ) {
    auto mnemonics = RPT( keyword, parseContext, errors, is_RPTSCHED_mnemonic, RPTSCHED_integer );

    if( mnemonics.count( "NOTHING" ) )
        return { std::move( mnemonics ), { RestartSchedule(0) } };

    if( mnemonics.count( "RESTART" ) )
        return { std::move( mnemonics ), RestartSchedule( size_t( mnemonics.at( "RESTART" )) ) };

    return { std::move( mnemonics ), {} };
}



void RestartConfig::handleScheduleSection(const SCHEDULESection& schedule, const ParseContext& parseContext, ErrorGuard& errors) {
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
            current_step += keyword.getRecord( 0 ).getItem( 0 ).data_size();
            continue;
        }

        if( this->m_timemap.size() <= current_step ) continue;

        if (name == "SAVE") {
            this->save_keywords.at(current_step) = true;
        } else {
            this->save_keywords.at(current_step) = false;
        }

        if( !( name == "RPTRST" || name == "RPTSCHED" ) ) continue;

        const bool is_RPTRST = name == "RPTRST";
        const auto& prev_sched = this->restart_schedule.back();

        auto config = is_RPTRST ? RPTRST( keyword, parseContext, errors, prev_sched, current_step )
            : RPTSCHED( keyword , parseContext, errors);

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

    template<typename T>
    RestartConfig::RestartConfig( const TimeMap& time_map, const Deck& deck, const ParseContext& parseContext, T&& errors ) :
        RestartConfig(time_map, deck, parseContext, errors)
    {}

    RestartConfig::RestartConfig( const TimeMap& time_map, const Deck& deck) :
        RestartConfig(time_map, deck, ParseContext(), ErrorGuard())
    {}


    RestartConfig::RestartConfig( const TimeMap& time_map, const Deck& deck, const ParseContext& parseContext, ErrorGuard& errors ) :
        m_timemap( time_map ),
        m_first_restart_step( -1 ),
        restart_schedule( m_timemap, {0,0,1}),
        restart_keywords( m_timemap, {} ),
        save_keywords( m_timemap.size(), false )
    {
        handleSolutionSection( SOLUTIONSection(deck), parseContext, errors );
        handleScheduleSection( SCHEDULESection(deck), parseContext, errors );
        initFirstOutput( );
    }

    RestartConfig RestartConfig::serializeObject()
    {
        RestartConfig result;
        result.m_timemap = TimeMap::serializeObject();
        result.m_first_restart_step = 2;
        result.m_write_initial_RST_file = true;
        result.restart_schedule = {{RestartSchedule::serializeObject()}, 2};
        result.restart_keywords = {{{{"test",3}}}, 3};
        result.save_keywords = {false, true};

        return result;
    }

    RestartSchedule RestartConfig::getNode( size_t timestep ) const{
        return restart_schedule.get(timestep);
    }


    bool RestartConfig::getWriteRestartFile(size_t timestep, bool log) const {
        if (0 == timestep)
            return m_write_initial_RST_file;

        if (save_keywords[timestep]) {
            if ( log ) {
                std::string logstring = "Fast restart using SAVE is not supported. Standard restart file is written instead";
                Opm::OpmLog::warning("Unhandled output keyword", logstring);
            }
            return true;
        }

        {
            RestartSchedule ts_restart_config = getNode( timestep );
            return ts_restart_config.writeRestartFile( timestep , m_timemap );
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
            if (report_step == m_timemap.size())
                break;
        }
    }


    void RestartConfig::handleSolutionSection(const SOLUTIONSection& solutionSection, const ParseContext& parseContext, ErrorGuard& errors) {
        if (solutionSection.hasKeyword("RPTRST")) {
            const auto& rptrstkeyword        = solutionSection.getKeyword("RPTRST");

            const auto rptrst = RPTRST( rptrstkeyword, parseContext, errors, {}, 0 );
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


        if (solutionSection.hasKeyword("RPTSOL") && (m_timemap.size() > 0)) {
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

        for (size_t index = 0; index < item.data_size(); ++index) {
            const std::string& mnemonic = item.get< std::string >(index);

            found_mnemonic_RESTART = mnemonic.find("RESTART=");
            if (found_mnemonic_RESTART != std::string::npos) {
                std::string restart_no = mnemonic.substr(found_mnemonic_RESTART+8, mnemonic.size());
                restart = std::strtoul(restart_no.c_str(), nullptr, 10);
                handle_RPTSOL_RESTART = true;
            }
        }


        /* If no RESTART mnemonic is found, either it is not present or we might
           have an old data set containing integer controls instead of mnemonics.
           Restart integer switch is integer control nr 7 */

        if (found_mnemonic_RESTART == std::string::npos) {
            if (item.data_size() >= 7)  {
                const std::string& integer_control = item.get< std::string >(6);
                restart = std::strtoul(integer_control.c_str(), nullptr, 10);
                if (restart != ULONG_MAX)
                    handle_RPTSOL_RESTART = true;
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

        auto ext = std::string{};
        if (unified) {
            ext = fmt_file ? "FUNRST" : "UNRST";
        }
        else {
            std::ostringstream os;

            const char* fmt_prefix   = "FGH";
            const char* unfmt_prefix = "XYZ";

            const int cycle = 10 * 1000;
            const int p_ix  = report_step / cycle;
            const int n     = report_step % cycle;

            os << (fmt_file ? fmt_prefix[p_ix] : unfmt_prefix[p_ix])
               << std::setw(4) << std::setfill('0') << n;

            ext = os.str();
        }

        return restart_base + '.' + ext;
    }


    int RestartConfig::getFirstRestartStep() const {
        return m_first_restart_step;
    }


    bool RestartConfig::operator==(const RestartConfig& data) const {
        return this->m_timemap == data.m_timemap &&
               this->m_first_restart_step == data.m_first_restart_step &&
               this->m_write_initial_RST_file == data.m_write_initial_RST_file &&
               this->restart_schedule == data.restart_schedule &&
               this->restart_keywords == data.restart_keywords &&
               this->save_keywords == data.save_keywords;
    }
}
