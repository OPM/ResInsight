/*
  Copyright 2021 Equinor ASA.

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
#include <optional>
#include <fmt/format.h>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Schedule/RSTConfig.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Utility/Functional.hpp>
#include <opm/common/utility/OpmInputError.hpp>


namespace Opm {

namespace {

inline bool is_int( const std::string& x ) {
    auto is_digit = []( char c ) { return std::isdigit( c ); };

    return !x.empty()
        && ( x.front() == '-' || is_digit( x.front() ) )
        && std::all_of( x.begin() + 1, x.end(), is_digit );
}

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

inline std::map< std::string, int >
RPTSCHED_integer( const std::vector< int >& ints ) {
    const size_t size = std::min( ints.size(), sizeof( SCHEDIntegerKeywords ) );

    std::map< std::string, int > mnemonics;
    for( size_t i = 0; i < size; ++i )
        mnemonics[ SCHEDIntegerKeywords[ i ] ] = ints[ i ];

    return mnemonics;
}

inline std::map< std::string, int >
RPTRST_integer( const std::vector< int >& ints ) {
    const size_t PCO_index = 26;
    const size_t BASIC_index = 0;

    std::map< std::string, int > mnemonics;
    const size_t size = std::min( ints.size(), sizeof( RSTIntegerKeywords ) );

    /* fun with special cases. Eclipse seems to ignore the BASIC=0, interpreting
     * it as sort-of "don't modify". Handle this by *not* adding/updating the
     * integer list sourced BASIC mnemonic, should it be zero. I'm not sure if
     * this applies to other mnemonics, but the eclipse manual indicates that
     * any zero here should disable the output.
     *
     * See https://github.com/OPM/opm-parser/issues/886 for reference
     *
     * The current treatment of a mix on RPTRST and RPTSCHED integer keywords is
     * probably not correct, but it is extremely difficult to comprehend exactly
     * how it should be. Current code is a rather arbitrary hack to get through
     * the tests.
     */

    if (size >= 26) {
        for( size_t i = 0; i < std::min( size, PCO_index ); ++i )
            mnemonics[ RSTIntegerKeywords[ i ] ] = ints[ i ];
    } else {
        if( size > 0 && ints[ BASIC_index ] != 0)
            mnemonics[ RSTIntegerKeywords[ BASIC_index ] ] = ints[ BASIC_index ];

        for( size_t i = 1; i < std::min( size, PCO_index ); ++i )
            mnemonics[ RSTIntegerKeywords[ i ] ] = ints[ i ];
    }

    for( size_t i = PCO_index + 1; i < size; ++i )
        mnemonics[ RSTIntegerKeywords[ i ] ] = ints[ i ];

    /* item 27 (index 26) sets both PCOW and PCOG, so we special case it */
    if( ints.size() >= PCO_index ) {
        mnemonics[ "PCOW" ] = ints[ PCO_index ];
        mnemonics[ "PCOG" ] = ints[ PCO_index ];
    }

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
        std::string msg = "Error in keyword {keyword}, mixing mnemonics and integers is not allowed\n"
                          "In {file} line {line}.";
        parseContext.handleError(ParseContext::RPT_MIXED_STYLE, msg, location, errors);

        std::vector<std::string> stack;
        for (size_t index=0; index < deck_items.size(); index++) {
            if (is_int(deck_items[index])) {

                if (stack.size() < 2)
                    throw OpmInputError("Problem processing {keyword}\nIn {file} line {line}.", location);

                if (stack.back() == "=") {
                    stack.pop_back();
                    std::string mnemonic = stack.back();
                    stack.pop_back();

                    items.insert(items.begin(), stack.begin(), stack.end());
                    stack.clear();
                    items.push_back( mnemonic + "=" + deck_items[index]);
                } else
                    throw OpmInputError("Problem processing {keyword}\nIn {file} line {line}.", location);

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
            std::string msg_fmt = fmt::format("Error in keyword {{keyword}}, unrecognized mnemonic {}\nIn {{file}} line {{line}}.", base);
            parseContext.handleError(ParseContext::RPT_UNKNOWN_MNEMONIC, msg_fmt, keyword.location(), errors);
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
    const auto allprops_iter = mnemonics.find( "ALLPROPS");
    if (allprops_iter != mnemonics.end()) {
        const auto value = allprops_iter->second;
        mnemonics.erase( allprops_iter );

        for (const auto& kw : {"BG","BO","BW","KRG","KRO","KRW","VOIL","VGAS","VWAT","DEN"})
            mnemonics[kw] = value;
    }
}

std::optional<int> extract(std::map<std::string, int>& mnemonics, const std::string& key) {
    auto iter = mnemonics.find(key);
    if (iter == mnemonics.end())
        return {};

    int value = iter->second;
    mnemonics.erase(iter);
    return value;
}


inline std::pair< std::map< std::string, int >, std::pair<std::optional<int>, std::optional<int>>>
RPTRST( const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors) {
    auto mnemonics = RPT( keyword, parseContext, errors, is_RPTRST_mnemonic, RPTRST_integer );
    std::optional<int> basic = extract(mnemonics, "BASIC");
    std::optional<int> freq  = extract(mnemonics, "FREQ");

    expand_RPTRST_mnemonics( mnemonics );
    return {mnemonics, { basic, freq }};
}


template <typename T>
void update_optional(std::optional<T>& target, const std::optional<T>& src) {
    if (src.has_value())
        target = src;
}


}

// The handleRPTSOL() function is only invoked from the constructor which uses
// the SOLUTION section, and the only information actually extracted is whether
// to write the initial restart file.

void RSTConfig::handleRPTSOL( const DeckKeyword& keyword) {
    const auto& record = keyword.getRecord(0);
    const auto& item = record.getItem(0);
    for (const auto& mnemonic : item.getData<std::string>()) {
        auto mnemonic_RESTART_pos = mnemonic.find("RESTART=");
        if (mnemonic_RESTART_pos != std::string::npos) {
            std::string restart_no = mnemonic.substr(mnemonic_RESTART_pos + 8, mnemonic.size());
            auto restart = std::strtoul(restart_no.c_str(), nullptr, 10);
            this->write_rst_file = (restart > 1);
            return;
        }
    }


    /* If no RESTART mnemonic is found, either it is not present or we might
       have an old data set containing integer controls instead of mnemonics.
       Restart integer switch is integer control nr 7 */

    if (item.data_size() >= 7) {
        const std::string& integer_control = item.get<std::string>(6);
        auto restart = std::strtoul(integer_control.c_str(), nullptr, 10);
        this->write_rst_file = (restart > 1);
        return;
    }
}

bool RSTConfig::operator==(const RSTConfig& other) const {
    return this->write_rst_file == other.write_rst_file &&
           this->keywords == other.keywords &&
           this->basic == other.basic &&
           this->freq == other.freq &&
           this->save == other.save;
}


void RSTConfig::update_schedule(const std::pair<std::optional<int>, std::optional<int>>& basic_freq) {
    update_optional(this->basic, basic_freq.first);
    update_optional(this->freq, basic_freq.second);
    if (this->basic.has_value()) {
        auto basic_value = this->basic.value();
        if (basic_value == 0)
            this->write_rst_file = false;
        else if (basic_value == 1 || basic_value == 2)
            this->write_rst_file = true;
        else
            this->write_rst_file = {};
    }
}


void RSTConfig::handleRPTRST(const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors) {
    const auto& [mnemonics, basic_freq] = RPTRST(keyword, parseContext, errors);
    this->update_schedule(basic_freq);
    for (const auto& [kw,num] : mnemonics)
        this->keywords[kw] = num;
}

void RSTConfig::handleRPTSCHED(const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors) {
    auto mnemonics = RPT( keyword, parseContext, errors, is_RPTSCHED_mnemonic, RPTSCHED_integer );
    auto nothing = extract(mnemonics, "NOTHING");
    if (nothing.has_value()) {
        this->basic = {};
        this->keywords.clear();
    }

    if (this->basic.value_or(2) <= 2) {
        auto restart = extract(mnemonics, "RESTART");
        if (restart.has_value()) {
            auto basic_value = std::min(2, restart.value());
            this->update_schedule({basic_value , 1});
        }
    }

    for (const auto& [kw,num] : mnemonics)
        this->keywords[kw] = num;
}


RSTConfig::RSTConfig(const SOLUTIONSection& solution_section, const ParseContext& parseContext, ErrorGuard& errors) :
      write_rst_file(false)
{
    if (solution_section.hasKeyword<ParserKeywords::RPTRST>()) {
        const auto& keyword = solution_section.get<ParserKeywords::RPTRST>().back();
        this->handleRPTRST(keyword, parseContext, errors);

        // Guessing on eclipse rules for write of initial RESTART file (at time 0):
        // Write of initial restart file is (due to the eclipse reference manual)
        // governed by RPTSOL RESTART in solution section,
        // if RPTSOL RESTART > 1 initial restart file is written.
        // but - due to initial restart file written from Eclipse
        // for data where RPTSOL RESTART not set - guessing that
        // when RPTRST is set in SOLUTION (no basic though...) -> write inital restart.
        this->write_rst_file = true;
    }

    if (solution_section.hasKeyword<ParserKeywords::RPTSOL>()) {
        const auto& keyword = solution_section.get<ParserKeywords::RPTSOL>().back();
        this->handleRPTSOL(keyword);
    }
}


void RSTConfig::update(const DeckKeyword& keyword, const ParseContext& parseContext, ErrorGuard& errors) {
    if (keyword.name() == ParserKeywords::RPTRST::keywordName)
        this->handleRPTRST(keyword, parseContext, errors);
    else if (keyword.name() == ParserKeywords::RPTSCHED::keywordName) {
        this->handleRPTSCHED(keyword, parseContext, errors);
    } else
        throw std::logic_error("The RSTConfig object can only use RPTRST and RPTSCHED keywords");
}


RSTConfig RSTConfig::serializeObject() {
    RSTConfig rst_config;
    rst_config.basic = 10;
    rst_config.freq = {};
    rst_config.write_rst_file = true;
    rst_config.save = true;
    rst_config.keywords = {{"S1", 1}, {"S2", 2}};
    return rst_config;
}

/*
  The RPTRST keyword is treated differently in the SOLUTION section and in the
  SCHEDULE section. This function takes a RSTConfig object created from the
  solution section and creates a transformed copy suitable as the first
  RSTConfig to represent the Schedule section.
*/
RSTConfig RSTConfig::first(const RSTConfig& solution_config ) {
    RSTConfig rst_config(solution_config);
    auto basic = rst_config.basic;
    if (!basic.has_value()) {
        rst_config.write_rst_file = false;
        return rst_config;
    }

    auto basic_value = basic.value();
    if (basic_value == 0)
        rst_config.write_rst_file = false;
    else if (basic_value == 1 || basic_value == 2)
        rst_config.write_rst_file = true;
    else if (basic_value >= 3)
        rst_config.write_rst_file = {};

    return rst_config;
}

}


