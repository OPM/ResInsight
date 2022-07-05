/*
  Copyright 2013 Statoil ASA.

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

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include <fmt/format.h>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/utility/OpmInputError.hpp>
#include <opm/input/eclipse/Deck/ImportContainer.hpp>

#include <opm/json/JsonObject.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/common/utility/String.hpp>

#include "raw/RawConsts.hpp"
#include "raw/RawEnums.hpp"
#include "raw/RawRecord.hpp"
#include "raw/RawKeyword.hpp"
#include "raw/StarToken.hpp"

namespace Opm {

namespace {
namespace str {
const std::string emptystr = "";

struct find_comment {
    /*
     * A note on performance: using a function to plug functionality into
     * find_terminator rather than plain functions because it almost ensures
     * inlining, where the plain function can reduce to a function pointer.
     */
    template< typename Itr >
    Itr operator()( Itr begin, Itr end ) const {
        auto itr = std::find( begin, end, '-' );
        for( ; itr != end; itr = std::find( itr + 1, end, '-' ) )
            if( (itr + 1) != end &&  *( itr + 1 ) == '-' ) return itr;

        return end;
    }
};

template< typename Itr, typename Term >
inline Itr find_terminator( Itr begin, Itr end, Term terminator ) {

    auto pos = terminator( begin, end );

    if( pos == begin || pos == end) return pos;

    auto qbegin = std::find_if( begin, end, RawConsts::is_quote() );

    if( qbegin == end || qbegin > pos )
        return pos;

    auto qend = std::find( qbegin + 1, end, *qbegin );

    // Quotes are not balanced - probably an error?!
    if( qend == end ) return end;

    return find_terminator( qend + 1, end, terminator );
}

/**
    This function will return a copy of the input string where all
    characters following '--' are removed. The copy is a view and relies on
    the source string to remain alive. The function handles quoting with
    single quotes and double quotes:

    ABC --Comment                =>  ABC
    ABC '--Comment1' --Comment2  =>  ABC '--Comment1'
    ABC "-- Not balanced quote?  =>  ABC "-- Not balanced quote?
*/
static inline std::string_view strip_comments( std::string_view str ) {
    auto terminator = find_terminator( str.begin(), str.end(), find_comment() );
    std::size_t size = std::distance(str.begin(), terminator);
    return { &*str.begin(), size };
}

template< typename Itr >
inline Itr trim_left( Itr begin, Itr end ) {
    return std::find_if_not( begin, end, RawConsts::is_separator() );
}

template< typename Itr >
inline Itr trim_right( Itr begin, Itr end ) {

    std::reverse_iterator< Itr > rbegin( end );
    std::reverse_iterator< Itr > rend( begin );

    return std::find_if_not( rbegin, rend, RawConsts::is_separator() ).base();
}

inline std::string_view trim( std::string_view str ) {
    auto fst = trim_left( str.begin(), str.end() );
    auto lst = trim_right( fst, str.end() );
    std::size_t size = std::distance(fst, lst);
    return { &*fst, size };
}

inline std::string_view del_after_first_slash( std::string_view view ) {
    using itr = std::string_view::const_iterator;
    const auto term = []( itr begin, itr end ) {
        return std::find( begin, end, '/' );
    };

    auto begin = view.begin();
    auto end = view.end();
    auto slash = find_terminator( begin, end, term );

    /* we want to preserve terminating slashes */
    if( slash != end ) ++slash;
    std::size_t size = std::distance(begin, slash);
    return { &*begin, size };
}

inline std::string_view del_after_last_slash( std::string_view view ) {
  auto begin = view.begin();
  auto end = view.end();
  auto slash = end;

  while (true) {
      if (slash == begin)
          break;

      if (*slash == '/')
          break;

      slash--;
  }
  if (slash == begin && *slash != '/')
      slash = end;

  /* we want to preserve terminating slashes */
  if( slash != end ) ++slash;
  std::size_t size = std::distance(begin, slash);
  return { &*begin, size };
}

inline std::string_view del_after_slash(std::string_view view, bool raw_strings) {
    if (raw_strings)
        return del_after_last_slash(view);
    else
        return del_after_first_slash(view);
}


inline bool getline( std::string_view& input, std::string_view& line ) {
    if( input.empty() ) return false;

    auto end = std::find( input.begin(), input.end(), '\n' );

    line = std::string_view(&*input.begin(), end - input.begin() );
    input = std::string_view(&*(end + 1), input.end() - (end + 1));
    return true;

    /* we know that we always append a newline onto the input string, so we can
     * safely assume that end+1 will either be end-of-input (i.e. empty range)
     * or the start of the next line
     */
}

/*
 * Read the input file and remove everything that isn't interesting data,
 * including stripping comments, removing leading/trailing whitespaces and
 * everything after (terminating) slashes. Manually copying into the string for
 * performance.
 */
inline std::string fast_clean( const std::string& str ) {
    std::string dst;
    dst.resize( str.size() );

    std::string_view input( str ), line;
    auto dsti = dst.begin();
    while( true ) {

        if ( getline( input, line ) ) {
            line = trim( strip_comments(line));

            std::copy( line.begin(), line.end(), dsti );
            dsti += std::distance( line.begin(), line.end() );
            *dsti++ = '\n';
        } else
            break;
    }

    dst.resize( std::distance( dst.begin(), dsti ) );
    return dst;
}

inline bool starts_with(const std::string_view& view, const std::string& str) {
    auto str_size = str.size();
    if (str_size > view.size())
        return false;

    auto str_data = str.data();
    auto pos = view.begin();

    std::size_t si = 0;
    while (true) {
        if (*pos != str_data[si])
            return false;

        ++pos;
        ++si;

        if (si == str_size)
            return true;
    }
}

inline std::string clean( const std::vector<std::pair<std::string, std::string>>& code_keywords, const std::string& str ) {
    auto count = std::count_if(code_keywords.begin(), code_keywords.end(), [&str](const std::pair<std::string, std::string>& code_pair)
                                                                  {
                                                                     return str.find(code_pair.first) != std::string::npos;
                                                                   });

    if (count == 0)
        return fast_clean(str);
    else {
        std::string dst;
        dst.resize( str.size() );

        std::string_view input( str ), line;
        auto dsti = dst.begin();
        while( true ) {
            for (const auto& code_pair : code_keywords) {
                const auto& keyword = code_pair.first;

                if (starts_with(input, keyword)) {
                    std::string end_string = code_pair.second;
                    auto end_pos = input.find(end_string);
                    if (end_pos == std::string::npos) {
                        std::copy(input.begin(), input.end(), dsti);
                        dsti += std::distance( input.begin(), input.end() );
                        input = std::string_view(&*input.end(), 0);
                        break;
                    } else {
                        end_pos += end_string.size();
                        std::copy(input.begin(), input.begin() + end_pos, dsti);
                        dsti += end_pos;
                        *dsti++ = '\n';
                        input = std::string_view(&*(input.begin() + end_pos + 1), input.end() - (input.begin() + end_pos + 1));
                        break;
                    }
                }
            }

            if ( getline( input, line ) ) {
                line = trim( strip_comments(line));

                std::copy( line.begin(), line.end(), dsti );
                dsti += std::distance( line.begin(), line.end() );
                *dsti++ = '\n';
            } else
                break;
        }

        dst.resize( std::distance( dst.begin(), dsti ) );
        return dst;
    }
}





inline std::string make_deck_name(const std::string_view& str) {
    auto first_sep = std::find_if( str.begin(), str.end(), RawConsts::is_separator() );
    return uppercase( std::string( str.substr( 0, first_sep - str.begin()) ));
}


inline std::string_view update_record_buffer(const std::string_view& record_buffer, const std::string_view& line) {
    if (record_buffer.empty())
        return line;
    else {
        std::size_t size = std::distance(record_buffer.begin(), line.end());
        return { &*record_buffer.begin(), size };
    }
}


inline bool isTerminator(const std::string_view& line) {
    return (line.size() == 1 && line.back() == RawConsts::slash);
}

inline bool isTerminatedRecordString(const std::string_view& line) {
    return (line.back() == RawConsts::slash);
}

}

struct file {
    file( std::filesystem::path p, const std::string& in ) :
        input( in ), path( p )
    {}

    std::string_view input;
    size_t lineNR = 0;
    std::filesystem::path path;
};


class InputStack : public std::stack< file, std::vector< file > > {
    public:
        void push( std::string&& input, std::filesystem::path p = "<memory string>" );

    private:
        std::list< std::string > string_storage;
        using base = std::stack< file, std::vector< file > >;
};

void InputStack::push( std::string&& input, std::filesystem::path p ) {
    this->string_storage.push_back( std::move( input ) );
    this->emplace( p, this->string_storage.back() );
}

class ParserState {
    public:
        ParserState( const std::vector<std::pair<std::string,std::string>>&,
                     const ParseContext&, ErrorGuard&,
                     const std::set<Opm::Ecl::SectionType>& ignore = {});

        ParserState( const std::vector<std::pair<std::string,std::string>>&,
                     const ParseContext&, ErrorGuard&,
                     std::filesystem::path, const std::set<Opm::Ecl::SectionType>& ignore = {});

        void loadString( const std::string& );
        void loadFile( const std::filesystem::path& );
        void openRootFile( const std::filesystem::path& );

        void handleRandomText(const std::string_view& ) const;
        std::optional<std::filesystem::path> getIncludeFilePath( std::string ) const;
        void addPathAlias( const std::string& alias, const std::string& path );

        const std::filesystem::path& current_path() const;
        size_t line() const;

        bool done() const;
        std::string_view getline();
        void ungetline(const std::string_view& ln);
        void closeFile();

        const std::set<Opm::Ecl::SectionType>& get_ignore() {return ignore_sections; };
        bool check_section_keywords();

    private:
        const std::vector<std::pair<std::string, std::string>> code_keywords;
        InputStack input_stack;

        std::set<Opm::Ecl::SectionType> ignore_sections;
        std::map< std::string, std::string > pathMap;

    public:
        ParserKeywordSizeEnum lastSizeType = SLASH_TERMINATED;
        std::string lastKeyWord;

        Deck deck;
        std::filesystem::path rootPath;
        std::unique_ptr<Python> python;
        const ParseContext& parseContext;
        ErrorGuard& errors;
        bool unknown_keyword = false;
};

const std::filesystem::path& ParserState::current_path() const {
    return this->input_stack.top().path;
}

size_t ParserState::line() const {
    return this->input_stack.top().lineNR;
}

bool ParserState::done() const {

    while( !this->input_stack.empty() &&
            this->input_stack.top().input.empty() )
        const_cast< ParserState* >( this )->input_stack.pop();

    return this->input_stack.empty();
}

std::string_view ParserState::getline() {
    std::string_view ln;

    str::getline( this->input_stack.top().input, ln );
    this->input_stack.top().lineNR++;

    return ln;
}



void ParserState::ungetline(const std::string_view& line) {
    auto& file_view = this->input_stack.top().input;
    if (line.end() + 1 != file_view.begin())
        throw std::invalid_argument("line view does not immediately proceed file_view");

    file_view = std::string_view(&*line.begin(), file_view.end() - line.begin());
    this->input_stack.top().lineNR--;
}




void ParserState::closeFile() {
    this->input_stack.pop();
}

ParserState::ParserState(const std::vector<std::pair<std::string, std::string>>& code_keywords_arg,
                         const ParseContext& __parseContext,
                         ErrorGuard& errors_arg,
                         const std::set<Opm::Ecl::SectionType>& ignore) :
    code_keywords(code_keywords_arg),
    ignore_sections(ignore),
    python( std::make_unique<Python>() ),
    parseContext( __parseContext ),
    errors( errors_arg )
{}

ParserState::ParserState( const std::vector<std::pair<std::string, std::string>>& code_keywords_arg,
                          const ParseContext& context,
                          ErrorGuard& errors_arg,
                          std::filesystem::path p,
                          const std::set<Opm::Ecl::SectionType>& ignore ) :
    code_keywords(code_keywords_arg),
    ignore_sections(ignore),
    rootPath( std::filesystem::canonical( p ).parent_path() ),
    python( std::make_unique<Python>() ),
    parseContext( context ),
    errors( errors_arg )
{
    openRootFile( p );
}

bool ParserState::check_section_keywords() {

    std::string_view root_file_str = this->input_stack.top().input;

    int n = 0;
    auto p0 = root_file_str.find_first_not_of(" \t\n");

    while (p0 != std::string::npos){

        auto p1 = root_file_str.find_first_of(" \t\n", p0 + 1);

        if (root_file_str.substr(p0, p1-p0) == "GRID")
            n++;
        else if (root_file_str.substr(p0, p1-p0) == "PROPS")
            n++;
        else if (root_file_str.substr(p0, p1-p0) == "REGIONS")
            n++;
        else if (root_file_str.substr(p0, p1-p0) == "SOLUTION")
            n++;
        else if (root_file_str.substr(p0, p1-p0) == "SUMMARY")
            n++;
        else if (root_file_str.substr(p0, p1-p0) == "SCHEDULE")
            n++;

        p0 = root_file_str.find_first_not_of(" \t\n", p1);
    }

    if (n < 6)
        return false;
    else
        return true;
}

void ParserState::loadString(const std::string& input) {
    this->input_stack.push( str::clean( this->code_keywords, input + "\n" ) );
}

void ParserState::loadFile(const std::filesystem::path& inputFile) {

    const auto closer = []( std::FILE* f ) { std::fclose( f ); };
    std::unique_ptr< std::FILE, decltype( closer ) > ufp(
            std::fopen( inputFile.string().c_str(), "rb"),
            closer
            );

    // make sure the file we'd like to parse is readable
    if( !ufp ) {
        std::string msg = "Could not read from file: " + inputFile.string();
        parseContext.handleError( ParseContext::PARSE_MISSING_INCLUDE , msg, {}, errors);
        return;
    }

    /*
     * read the input file C-style. This is done for performance
     * reasons, as streams are slow
     */

    auto* fp = ufp.get();
    std::string buffer;
    std::fseek( fp, 0, SEEK_END );
    buffer.resize( std::ftell( fp ) + 1 );
    std::rewind( fp );
    const auto readc = std::fread( &buffer[ 0 ], 1, buffer.size() - 1, fp );
    buffer.back() = '\n';

    if( std::ferror( fp ) || readc != buffer.size() - 1 )
        throw std::runtime_error( "Error when reading input file '"
                                  + inputFile.string() + "'" );

    this->input_stack.push( str::clean( this->code_keywords, buffer ), inputFile );
}

/*
 * We have encountered 'random' characters in the input file which
 * are not correctly formatted as a keyword heading, and not part
 * of the data section of any keyword.
 */

void ParserState::handleRandomText(const std::string_view& keywordString ) const {
    std::string errorKey;
    std::string trimmedCopy = std::string( keywordString );
    std::string msg;
    KeywordLocation location{lastKeyWord, this->current_path().string(), this->line()};

    if (trimmedCopy == "/") {
        errorKey = ParseContext::PARSE_RANDOM_SLASH;
        msg = "Extra '/' detected in {file} line {line}";
    }
    else if (lastSizeType == OTHER_KEYWORD_IN_DECK) {
      errorKey = ParseContext::PARSE_EXTRA_RECORDS;
      msg = "Too many records in keyword {keyword}\n"
            "In {} line {}";
    }
    else {
        errorKey = ParseContext::PARSE_RANDOM_TEXT;
        msg = fmt::format("String {} not formatted as valid keyword\n"
                          "In {{file}} line {{line}}.", keywordString);
    }
    parseContext.handleError( errorKey , msg, location, errors );
}


void ParserState::openRootFile( const std::filesystem::path& inputFile) {

    this->loadFile( inputFile );
    this->deck.setDataFile( inputFile.string() );
    const std::filesystem::path& inputFileCanonical = std::filesystem::canonical(inputFile);
    this->rootPath = inputFileCanonical.parent_path();
}

std::optional<std::filesystem::path> ParserState::getIncludeFilePath( std::string path ) const {
    static const std::string pathKeywordPrefix("$");
    static const std::string validPathNameCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");

    size_t positionOfPathName = path.find(pathKeywordPrefix);

    if ( positionOfPathName != std::string::npos) {
        std::string stringStartingAtPathName = path.substr(positionOfPathName+1);
        size_t cutOffPosition = stringStartingAtPathName.find_first_not_of(validPathNameCharacters);
        std::string stringToFind = stringStartingAtPathName.substr(0, cutOffPosition);
        std::string stringToReplace = this->pathMap.at( stringToFind );
        replaceAll(path, pathKeywordPrefix + stringToFind, stringToReplace);
    }

    // Check if there are any backslashes in the path...
    if (path.find('\\') != std::string::npos) {
        // ... if so, replace with slashes and create a warning.
        std::replace(path.begin(), path.end(), '\\', '/');
        OpmLog::warning("Replaced one or more backslash with a slash in an INCLUDE path.");
    }

    std::filesystem::path includeFilePath(path);

    if (includeFilePath.is_relative())
        includeFilePath = this->rootPath / includeFilePath;

    try {
        includeFilePath = std::filesystem::canonical(includeFilePath);
    } catch (const std::filesystem::filesystem_error& fs_error) {
        parseContext.handleError( ParseContext::PARSE_MISSING_INCLUDE , fmt::format("No such file: {}", includeFilePath.string()), {}, errors);
        return {};
    }

    return includeFilePath;
}

void ParserState::addPathAlias( const std::string& alias, const std::string& path ) {
    this->pathMap.emplace( alias, path );
}


RawKeyword * newRawKeyword(const ParserKeyword& parserKeyword, const std::string& keywordString, ParserState& parserState, const Parser& parser) {
    for (const auto& keyword : parserKeyword.prohibitedKeywords()) {
        if (parserState.deck.hasKeyword(keyword)) {
            parserState
                .parseContext
                .handleError(
                    ParseContext::PARSE_INVALID_KEYWORD_COMBINATION,
                    fmt::format("Incompatible keyword combination: {} declared when {} is already present.", keywordString, keyword),
                    KeywordLocation { keywordString, parserState.current_path().string(), parserState.line()},
                    parserState.errors
                );
        }
    }

    for (const auto& keyword : parserKeyword.requiredKeywords()) {
        if (!parserState.deck.hasKeyword(keyword)) {
            parserState
                .parseContext
                .handleError(
                    ParseContext::PARSE_INVALID_KEYWORD_COMBINATION,
                    fmt::format("Incompatible keyword combination: {} declared, but {} is missing.", keywordString, keyword),
                    KeywordLocation { keywordString, parserState.current_path().string(), parserState.line()},
                    parserState.errors
                );
        }
    }

    bool raw_string_keyword = parserKeyword.rawStringKeyword();

    if( parserKeyword.getSizeType() == SLASH_TERMINATED || parserKeyword.getSizeType() == UNKNOWN || parserKeyword.getSizeType() == DOUBLE_SLASH_TERMINATED) {

        const auto size_type = parserKeyword.getSizeType();
        Raw::KeywordSizeEnum rawSizeType;

        switch(size_type) {
            case SLASH_TERMINATED:        rawSizeType = Raw::SLASH_TERMINATED; break;
            case UNKNOWN:                 rawSizeType = Raw::UNKNOWN; break;
            case DOUBLE_SLASH_TERMINATED: rawSizeType = Raw::DOUBLE_SLASH_TERMINATED; break;
            default:
                throw std::logic_error("Should not be here!");
        }

        return new RawKeyword( keywordString,
                               parserState.current_path().string(),
                               parserState.line(),
                               raw_string_keyword,
                               rawSizeType);
    }

    if( parserKeyword.hasFixedSize() ) {
        auto size_type = Raw::FIXED;
        if (parserKeyword.isCodeKeyword())
            size_type = Raw::CODE;

        return new RawKeyword( keywordString,
                               parserState.current_path().string(),
                               parserState.line(),
                               raw_string_keyword,
                               size_type,
                               parserKeyword.min_size(),
                               parserKeyword.getFixedSize());
    }

    const auto& keyword_size = parserKeyword.getKeywordSize();
    const auto& deck = parserState.deck;
    auto size_type = parserKeyword.isTableCollection() ? Raw::TABLE_COLLECTION : Raw::FIXED;

    if( deck.hasKeyword(keyword_size.keyword() ) ) {
        const auto& sizeDefinitionKeyword = deck[keyword_size.keyword()].back();
        const auto& record = sizeDefinitionKeyword.getRecord(0);
        auto targetSize = record.getItem( keyword_size.item() ).get< int >( 0 ) + keyword_size.size_shift();
        if (parserKeyword.isAlternatingKeyword())
            targetSize *= std::distance( parserKeyword.begin(), parserKeyword.end() );

        return new RawKeyword( keywordString,
                               parserState.current_path().string(),
                               parserState.line(),
                               raw_string_keyword,
                               size_type,
                               parserKeyword.min_size(),
                               targetSize);
    }

    std::string msg_fmt = fmt::format("Problem with {{keyword}} - missing {0}\n"
                                      "In {{file}} line {{line}}\n"
                                      "For the keyword {{keyword}} we expect to read the number of records from keyword {0}, {0} was not found", keyword_size.keyword());
    parserState.parseContext.handleError(ParseContext::PARSE_MISSING_DIMS_KEYWORD ,
                                         msg_fmt,
                                         KeywordLocation{keywordString, parserState.current_path().string(), parserState.line()},
                                         parserState.errors );

    const auto& keyword = parser.getKeyword( keyword_size.keyword() );
    const auto& record = keyword.getRecord(0);
    const auto& int_item = record.get( keyword_size.item());

    const auto targetSize = int_item.getDefault< int >( ) + keyword_size.size_shift();
    return new RawKeyword( keywordString,
                           parserState.current_path().string(),
                           parserState.line(),
                           raw_string_keyword,
                           size_type,
                           parserKeyword.min_size(),
                           targetSize);
}


RawKeyword * newRawKeyword( const std::string& deck_name, ParserState& parserState, const Parser& parser, const std::string_view& line ) {
    if (deck_name.size() > RawConsts::maxKeywordLength) {
        const std::string keyword8 = deck_name.substr(0, RawConsts::maxKeywordLength);
        if (parser.isRecognizedKeyword(keyword8)) {
            std::string msg = "Keyword {keyword} to long - only eight first characters recognized\n"
                              "In {file} line {line}\n";
            parserState.parseContext.handleError(ParseContext::PARSE_LONG_KEYWORD,
                                                 msg,
                                                 KeywordLocation{deck_name, parserState.current_path().string(), parserState.line()},
                                                 parserState.errors);

            parserState.unknown_keyword = false;
            const auto& parserKeyword = parser.getParserKeywordFromDeckName( keyword8 );
            return newRawKeyword(parserKeyword, keyword8, parserState, parser);
        } else {
            parserState.parseContext.handleUnknownKeyword( deck_name, KeywordLocation{}, parserState.errors );
            parserState.unknown_keyword = true;
            return nullptr;
        }
    }

    if (parser.isRecognizedKeyword(deck_name)) {
        parserState.unknown_keyword = false;
        const auto& parserKeyword = parser.getParserKeywordFromDeckName(deck_name);
        return newRawKeyword(parserKeyword, deck_name, parserState, parser);
    }

    if( ParserKeyword::validDeckName(deck_name) ) {
        parserState.parseContext.handleUnknownKeyword( deck_name, KeywordLocation{}, parserState.errors );
        parserState.unknown_keyword = true;
        return nullptr;
    }

    if (!parserState.unknown_keyword)
        parserState.handleRandomText(line);

    return nullptr;
}

// This code is made to skip part of the eclipse file containing the contents of UDT
// UDT is currently not a supported keyword.
// The first record of UDT contains a line, that MAY (although unlikely) contain a keyword.
// Skipping this first line, and the next, skipUDT then searches for the next
// keyword, which marks the end of UDT.
void skipUDT( ParserState& parserState, const Parser& parser) {
  //Skipping first two records of UDT:
  size_t record_count = 0;
  size_t count_max = 2;

  while( !parserState.done() ) {
      auto line = parserState.getline();
      if( line.empty() ) continue;

      if (record_count < count_max)
          record_count++;
      else {
           std::string deck_name = str::make_deck_name( line );
           if (parser.hasKeyword(deck_name)) {
               parserState.ungetline(line);
               return;
           }
      }
  }
}


std::unique_ptr<RawKeyword> tryParseKeyword( ParserState& parserState, const Parser& parser) {
    bool is_title = false;
    std::unique_ptr<RawKeyword> rawKeyword;
    std::string_view record_buffer(str::emptystr);
    std::optional<ParserKeyword> parserKeyword;
    while( !parserState.done() ) {
        auto line = parserState.getline();

        if( line.empty() && !rawKeyword ) continue;
        if( line.empty() && !is_title ) continue;

        std::string keywordString;

        if( !rawKeyword ) {
            /*
              Extracting a possible keywordname from a line of deck input
              involves several steps.

              1. The make_deck_name() function will strip off everyhing
                 following the first white-space separator and uppercase the
                 string.

              2. The ParserKeyword::validDeckName() verifies that the keyword
                 candidate only contains valid characters.

              3. In the newRawKeyword() function the first 8 characters of the
                 deck_name is used to look for the keyword in the Parser
                 container.
            */
            std::string deck_name = str::make_deck_name( line );
            if (ParserKeyword::validDeckName(deck_name)) {
                auto ptr = newRawKeyword( deck_name, parserState, parser, line );
                if (ptr) {
                    rawKeyword.reset( ptr );
                    parserKeyword = parser.getParserKeywordFromDeckName(rawKeyword->getKeywordName());
                    if (deck_name == "UDT") {
                        skipUDT(parserState, parser);
                        return nullptr;
                    }
                    parserState.lastSizeType = parserKeyword->getSizeType();
                    parserState.lastKeyWord = deck_name;
                    if (rawKeyword->isFinished())
                        return rawKeyword;

                    if (deck_name == "TITLE")
                        is_title = true;
                }
            } else {
                /* We are looking at some random gibberish?! */
                if (!parserState.unknown_keyword)
                    parserState.handleRandomText( line );
            }
        } else {
            if (rawKeyword->getSizeType() == Raw::CODE) {
                auto end_pos = line.find(parserKeyword->codeEnd());
                if (end_pos != std::string::npos) {
                    std::string_view line_content = { &*line.begin(), end_pos};
                    record_buffer = str::update_record_buffer( record_buffer, line_content );

                    RawRecord record(record_buffer, rawKeyword->location(), true);
                    rawKeyword->addRecord(record);
                    return rawKeyword;
                } else
                    record_buffer = str::update_record_buffer( &*record_buffer.begin(), line );

                continue;
            }

            if (rawKeyword->can_complete()) {
                /*
                  When we are spinning through a keyword of size type UNKNOWN it
                  is essential to recognize a string as the next keyword. The
                  line starting a new keyword can have arbitrary rubbish
                  following the keyword name - i.e. this  line

                    PORO  Here comes some random gibberish which should be ignored
                       10000*0.15 /

                  To ensure the keyword 'PORO' is recognized in the example
                  above we remove everything following the first space in the
                  line variable before we check if it is the start of a new
                  keyword.
                */
                std::string deck_name = str::make_deck_name( line );
                if( parser.isRecognizedKeyword( deck_name ) ) {
                    rawKeyword->terminateKeyword();
                    parserState.ungetline(line);
                    return rawKeyword;
                }
            }

            line = str::del_after_slash(line, rawKeyword->rawStringKeyword());
            record_buffer = str::update_record_buffer(record_buffer, line);
            if (is_title) {
                if (record_buffer.empty()) {
                    RawRecord record("opm/flow simulation", rawKeyword->location());
                    rawKeyword->addRecord(record);
                } else {
                    std::size_t size = std::distance(record_buffer.begin(),record_buffer.end());
                    RawRecord record( std::string_view{ &*record_buffer.begin(), size }, rawKeyword->location());
                    rawKeyword->addRecord(record);
                }
                return rawKeyword;
            }


            if (str::isTerminator(record_buffer)) {
                if (rawKeyword->terminateKeyword())
                    return rawKeyword;
            }


            if (str::isTerminatedRecordString(record_buffer)) {
                std::size_t size = std::distance(record_buffer.begin(), record_buffer.end()) - 1;
                RawRecord record( std::string_view{ &*record_buffer.begin(), size }, rawKeyword->location());
                if (rawKeyword->addRecord(record))
                    return rawKeyword;

                record_buffer = str::emptystr;
            }
        }
    }

    if (rawKeyword) {
        if (rawKeyword->can_complete())
            rawKeyword->terminateKeyword();

        if (!rawKeyword->isFinished()) {
            throw OpmInputError {
                "Keyword is not properly terminated.",
                rawKeyword->location()
            };
        }
    }

    return rawKeyword;
}

std::string_view advance_parser_state( ParserState& parserState, const std::string& to_keyw )
{
    auto line = parserState.getline();

    while (line != to_keyw) {
        line = parserState.getline();
    }

    return line;
}

bool parseState( ParserState& parserState, const Parser& parser ) {
    std::string filename = parserState.current_path().string();

    auto ignore = parserState.get_ignore();

    if (ignore.size() > 0)
        if (!parserState.check_section_keywords())
            throw std::runtime_error("Parsing individual sections not possible when section keywords in root input file");

    bool ignore_grid = ignore.find(Opm::Ecl::GRID) !=ignore.end()  ? true : false;
    bool ignore_props = ignore.find(Opm::Ecl::PROPS) !=ignore.end()  ? true : false;
    bool ignore_regions = ignore.find(Opm::Ecl::REGIONS) !=ignore.end()  ? true : false;
    bool ignore_solution = ignore.find(Opm::Ecl::SOLUTION) !=ignore.end()  ? true : false;
    bool ignore_summary = ignore.find(Opm::Ecl::SUMMARY) !=ignore.end()  ? true : false;
    bool ignore_schedule = ignore.find(Opm::Ecl::SCHEDULE) !=ignore.end()  ? true : false;

    while( !parserState.done() ) {
        auto rawKeyword = tryParseKeyword( parserState, parser);

        if( !rawKeyword )
            continue;

        std::string_view keyw = rawKeyword->getKeywordName();

        if ((ignore_grid) && (keyw=="GRID"))
            keyw = advance_parser_state( parserState, "PROPS" );

        if ((ignore_props) && (keyw=="PROPS"))
            keyw = advance_parser_state( parserState, "REGIONS" );

        if ((ignore_regions) && (keyw=="REGIONS"))
            keyw = advance_parser_state( parserState, "SOLUTION" );

        if ((ignore_solution) && (keyw=="SOLUTION"))
            keyw = advance_parser_state( parserState, "SUMMARY" );

        if ((ignore_summary) && (keyw=="SUMMARY"))
            keyw = advance_parser_state( parserState, "SCHEDULE" );

        if ((ignore_schedule) && (keyw=="SCHEDULE"))
            return true;

        if (rawKeyword->getKeywordName() == Opm::RawConsts::end)
            return true;

        if (rawKeyword->getKeywordName() == Opm::RawConsts::endinclude) {
            parserState.closeFile();
            continue;
        }

        if (rawKeyword->getKeywordName() == Opm::RawConsts::paths) {
            for( const auto& record : *rawKeyword ) {
                std::string pathName = readValueToken<std::string>(record.getItem(0));
                std::string pathValue = readValueToken<std::string>(record.getItem(1));
                parserState.addPathAlias( pathName, pathValue );
            }

            continue;
        }

        if (rawKeyword->getKeywordName() == Opm::RawConsts::include) {
            auto& firstRecord = rawKeyword->getFirstRecord( );
            std::string includeFileAsString = readValueToken<std::string>(firstRecord.getItem(0));
            const auto& includeFile = parserState.getIncludeFilePath( includeFileAsString );

            if (includeFile.has_value()) {
                auto& deck_tree = parserState.deck.tree();
                deck_tree.add_include(std::filesystem::absolute(parserState.current_path()).string(), includeFile.value().string());
                parserState.loadFile( includeFile.value() );
            }
            continue;
        }

        if( parser.isRecognizedKeyword( rawKeyword->getKeywordName() ) ) {
            const auto& kwname = rawKeyword->getKeywordName();
            const auto& parserKeyword = parser.getParserKeywordFromDeckName( kwname );
            {
                const auto& location = rawKeyword->location();
                auto msg = fmt::format("{:5} Reading {:<8} in {} line {}", parserState.deck.size(), rawKeyword->getKeywordName(), location.filename, location.lineno);
                OpmLog::info(msg);
            }
            try {
                if (rawKeyword->getKeywordName() ==  Opm::RawConsts::pyinput) {
                    if (parserState.python) {
                        std::string python_string = rawKeyword->getFirstRecord().getRecordString();
                        parserState.python->exec(python_string, parser, parserState.deck);
                    }
                    else
                        throw std::logic_error("Cannot yet embed Python while still running Python.");
                }
                else {
                    auto deck_keyword = parserKeyword.parse( parserState.parseContext,
                                                             parserState.errors,
                                                             *rawKeyword,
                                                             parserState.deck.getActiveUnitSystem(),
                                                             parserState.deck.getDefaultUnitSystem());

                    if (deck_keyword.name() == ParserKeywords::IMPORT::keywordName) {
                        bool formatted = deck_keyword.getRecord(0).getItem(1).get<std::string>(0)[0] == 'F';
                        const auto& import_file = parserState.getIncludeFilePath(deck_keyword.getRecord(0).getItem(0).get<std::string>(0));

                        ImportContainer import(parser, parserState.deck.getActiveUnitSystem(), import_file.value().string(), formatted, parserState.deck.size());
                        for (auto kw : import)
                            parserState.deck.addKeyword(std::move(kw));
                    } else
                        parserState.deck.addKeyword( std::move(deck_keyword) );
                }
            } catch (const OpmInputError& opm_error) {
                throw;
            } catch (const std::exception& e) {
                /*
                  This catch-all of parsing errors is to be able to write a good
                  error message; the parser is quite confused at this state and
                  we should not be tempted to continue the parsing.

                  We log a error message with the name of the problematic
                  keyword and the location in the input deck. We rethrow the
                  same exception without updating the what() message of the
                  exception.
                */
                const OpmInputError opm_error { e, rawKeyword->location() } ;

                OpmLog::error(opm_error.what());

                std::throw_with_nested(opm_error);
            }
        } else {
            const std::string msg = "The keyword " + rawKeyword->getKeywordName() + " is not recognized - ignored";
            KeywordLocation location(rawKeyword->getKeywordName(), parserState.current_path().string(), parserState.line());
            OpmLog::warning(Log::fileMessage(location, msg));
        }
    }

    return true;
}

}


    /* stripComments only exists so that the unit tests can verify it.
     * strip_comment is the actual (internal) implementation
     */
    std::string Parser::stripComments( const std::string& str ) {
        return { str.begin(),
                 str::find_terminator( str.begin(), str.end(), str::find_comment() ) };
    }

    Parser::Parser(bool addDefault) {
        // The addDefaultKeywords() method is implemented in a source file
        // ${PROJECT_BINARY_DIR}/ParserInit.cpp which is generated by the build
        // system.

        if (addDefault)
            this->addDefaultKeywords();
    }


    /*
     About INCLUDE: Observe that the ECLIPSE parser is slightly unlogical
     when it comes to nested includes; the path to an included file is always
     interpreted relative to the filesystem location of the DATA file, and
     not the location of the file issuing the INCLUDE command. That behaviour
     is retained in the current implementation.
     */

    inline void assertFullDeck(const ParseContext& context) {
        if (context.hasKey(ParseContext::PARSE_MISSING_SECTIONS))
            throw new std::logic_error("Cannot construct a state in partial deck context");
    }

    EclipseState Parser::parse(const std::string &filename, const ParseContext& context, ErrorGuard& errors) {
        assertFullDeck(context);
        return EclipseState( Parser{}.parseFile( filename, context, errors ));
    }

    EclipseState Parser::parse(const Deck& deck, const ParseContext& context) {
        assertFullDeck(context);
        return EclipseState(deck);
    }

    EclipseState Parser::parseData(const std::string &data, const ParseContext& context, ErrorGuard& errors) {
        assertFullDeck(context);
        Parser p;
        auto deck = p.parseString(data, context, errors);
        return parse(deck, context);
    }

    EclipseGrid Parser::parseGrid(const std::string &filename, const ParseContext& context , ErrorGuard& errors) {
        if (context.hasKey(ParseContext::PARSE_MISSING_SECTIONS))
            return EclipseGrid{ filename };
        return parse(filename, context, errors).getInputGrid();
    }

    EclipseGrid Parser::parseGrid(const Deck& deck, const ParseContext& context)
    {
        if (context.hasKey(ParseContext::PARSE_MISSING_SECTIONS))
            return EclipseGrid{ deck };
        return parse(deck, context).getInputGrid();
    }

    EclipseGrid Parser::parseGridData(const std::string &data, const ParseContext& context, ErrorGuard& errors) {
        Parser parser;
        auto deck = parser.parseString(data, context, errors);
        if (context.hasKey(ParseContext::PARSE_MISSING_SECTIONS)) {
            return EclipseGrid{ deck };
        }
        return parse(deck, context).getInputGrid();
    }

    Deck Parser::parseFile(const std::string &dataFileName, const ParseContext& parseContext,
                           ErrorGuard& errors, const std::vector<Opm::Ecl::SectionType>& sections) const {

        std::set<Opm::Ecl::SectionType> ignore_sections;

        if (sections.size() > 0) {

            std::set<Opm::Ecl::SectionType> all_sections;
            all_sections = {Opm::Ecl::GRID, Opm::Ecl::PROPS, Opm::Ecl::REGIONS, Opm::Ecl::SOLUTION, Opm::Ecl::SUMMARY, Opm::Ecl::SCHEDULE};

            std::set<Opm::Ecl::SectionType> read_sections;

            for (auto sec : sections)
                 read_sections.insert(sec);

            std::set_difference(all_sections.begin(), all_sections.end(), read_sections.begin(), read_sections.end(),
                            std::inserter(ignore_sections, ignore_sections.end()));
        }

        /*
          The following rules apply to the .DATA file argument which is
          internalized in the deck:

           1. It is normalized by removing uneccessary '.' characters and
              resolving symlinks.

           2. The relative/abolute status of the path is retained.
        */
        std::string data_file;
        if (dataFileName[0] == '/')
            data_file = std::filesystem::canonical(dataFileName).string();
        else
            data_file = std::filesystem::proximate( std::filesystem::canonical(dataFileName) ).string();

        ParserState parserState( this->codeKeywords(), parseContext, errors, data_file, ignore_sections);
        parseState( parserState, *this );
        return std::move( parserState.deck );
    }

    Deck Parser::parseFile(const std::string& dataFileName,
                           const ParseContext& parseContext) const {
        ErrorGuard errors;
        return this->parseFile(dataFileName, parseContext, errors, {});
    }

    Deck Parser::parseFile(const std::string& dataFileName,
                           const ParseContext& parseContext,
                           const std::vector<Opm::Ecl::SectionType>& sections) const {
        ErrorGuard errors;
        return this->parseFile(dataFileName, parseContext, errors, sections);
    }

    Deck Parser::parseFile(const std::string& dataFileName) const {
        ErrorGuard errors;
        return this->parseFile(dataFileName, ParseContext(), errors);
    }




    Deck Parser::parseString(const std::string &data, const ParseContext& parseContext, ErrorGuard& errors) const {
        ParserState parserState( this->codeKeywords(), parseContext, errors );
        parserState.loadString( data );
        parseState( parserState, *this );
        return std::move( parserState.deck );
    }

    Deck Parser::parseString(const std::string &data, const ParseContext& parseContext) const {
        ErrorGuard errors;
        return this->parseString(data, parseContext, errors);
    }

    Deck Parser::parseString(const std::string &data) const {
        ErrorGuard errors;
        return this->parseString(data, ParseContext(), errors);
    }

    size_t Parser::size() const {
        return m_deckParserKeywords.size();
    }

    const ParserKeyword* Parser::matchingKeyword(const std::string_view& name) const {
        for (auto iter = m_wildCardKeywords.begin(); iter != m_wildCardKeywords.end(); ++iter) {
            if (iter->second->matches(name))
                return iter->second;
        }
        return nullptr;
    }

    bool Parser::hasWildCardKeyword(const std::string& internalKeywordName) const {
        return (m_wildCardKeywords.count(internalKeywordName) > 0);
    }

    bool Parser::isRecognizedKeyword(const std::string_view& name ) const {
        if( !ParserKeyword::validDeckName( name ) )
            return false;

        if( m_deckParserKeywords.count( name ) )
            return true;

        return bool( matchingKeyword( name ) );
    }

void Parser::addParserKeyword( ParserKeyword parserKeyword ) {
    /* Store the keywords in the keyword storage. They aren't free'd until the
     * parser gets destroyed, even if there is no reasonable way to reach them
     * (effectively making them leak). This is not a big problem because:
     *
     * * A keyword can be added that overwrites some *but not all* deckname ->
     *   keyword mappings. Keeping track of this is more hassle than worth for
     *   what is essentially edge case usage.
     * * We can store (and search) via std::string_view's from the keyword added
     *   first because we know that it will be kept around, i.e. we don't have to
     *   deal with subtle lifetime issues.
     * * It means we aren't reliant on some internal name mapping, and can only
     * be concerned with interesting behaviour.
     * * Finally, these releases would in practice never happen anyway until
     *   the parser went out of scope, and now they'll also be cleaned up in the
     *   same sweep.
     */

    this->keyword_storage.push_back( std::move( parserKeyword ) );
    const ParserKeyword * ptr = std::addressof(this->keyword_storage.back());
    std::string_view name( ptr->getName() );

    for (const auto& deck_name : ptr->deck_names())
    {
        m_deckParserKeywords[deck_name] = ptr;
    }

    if (ptr->hasMatchRegex())
        m_wildCardKeywords[ name ] = ptr;

    if (ptr->isCodeKeyword())
        this->code_keywords.emplace_back( ptr->getName(), ptr->codeEnd() );
}


void Parser::addParserKeyword(const Json::JsonObject& jsonKeyword) {
    addParserKeyword( ParserKeyword( jsonKeyword ) );
}

bool Parser::hasKeyword( const std::string& name ) const {
    return this->m_deckParserKeywords.find( std::string_view( name ) )
        != this->m_deckParserKeywords.end();
}

const ParserKeyword& Parser::getKeyword( const std::string& name ) const {
    return getParserKeywordFromDeckName( std::string_view( name ) );
}

const ParserKeyword& Parser::getParserKeywordFromDeckName(const std::string_view& name ) const {
    auto candidate = m_deckParserKeywords.find( name );

    if( candidate != m_deckParserKeywords.end() ) return *candidate->second;

    const auto* wildCardKeyword = matchingKeyword( name );

    if ( !wildCardKeyword )
        throw std::invalid_argument( "Do not have parser keyword for parsing: " + std::string(name) );

    return *wildCardKeyword;
}

std::vector<std::string> Parser::getAllDeckNames () const {
    std::vector<std::string> keywords;
    for (auto iterator = m_deckParserKeywords.begin(); iterator != m_deckParserKeywords.end(); iterator++) {
        keywords.push_back(std::string(iterator->first));
    }
    for (auto iterator = m_wildCardKeywords.begin(); iterator != m_wildCardKeywords.end(); iterator++) {
        keywords.push_back(std::string(iterator->first));
    }
    return keywords;
}


    void Parser::loadKeywords(const Json::JsonObject& jsonKeywords) {
        if (jsonKeywords.is_array()) {
            for (size_t index = 0; index < jsonKeywords.size(); index++) {
                Json::JsonObject jsonKeyword = jsonKeywords.get_array_item(index);
                addParserKeyword( ParserKeyword( jsonKeyword ) );
            }
        } else
            throw std::invalid_argument("Input JSON object is not an array");
    }

    bool Parser::loadKeywordFromFile(const std::filesystem::path& configFile) {

        try {
            Json::JsonObject jsonKeyword(configFile);
            addParserKeyword( ParserKeyword( jsonKeyword ) );
            return true;
        }
        catch (...) {
            return false;
        }
    }


    void Parser::loadKeywordsFromDirectory(const std::filesystem::path& directory, bool recursive) {
        if (!std::filesystem::exists(directory))
            throw std::invalid_argument("Directory: " + directory.string() + " does not exist.");
        else {
            std::filesystem::directory_iterator end;
            for (std::filesystem::directory_iterator iter(directory); iter != end; iter++) {
                if (std::filesystem::is_directory(*iter)) {
                    if (recursive)
                        loadKeywordsFromDirectory(*iter, recursive);
                } else {
                    if (ParserKeyword::validInternalName(iter->path().filename().string())) {
                        if (!loadKeywordFromFile(*iter))
                            std::cerr << "** Warning: failed to load keyword from file:" << iter->path() << std::endl;
                    }
                }
            }
        }
    }

    const std::vector<std::pair<std::string,std::string>> Parser::codeKeywords() const {
        return this->code_keywords;
    }


#if 0
    void Parser::applyUnitsToDeck(Deck& deck) const {

        for( auto& deckKeyword : deck ) {

            if( !isRecognizedKeyword( deckKeyword.name() ) ) continue;

            const auto& parserKeyword = getParserKeywordFromDeckName( deckKeyword.name() );
            if( !parserKeyword.hasDimension() ) continue;

            parserKeyword.applyUnitsToDeck(deck , deckKeyword);
        }
    }
#endif


    static bool isSectionDelimiter( const DeckKeyword& keyword ) {
        const auto& name = keyword.name();
        for( const auto& x : { "RUNSPEC", "GRID", "EDIT", "PROPS",
                               "REGIONS", "SOLUTION", "SUMMARY", "SCHEDULE" } )
            if( name == x ) return true;

        return false;
    }

    bool DeckSection::checkSectionTopology(const Deck& deck,
                                           const Parser& parser,
                                           bool ensureKeywordSectionAffiliation)
    {
        if( deck.size() == 0 ) {
            std::string msg = "empty decks are invalid\n";
            OpmLog::warning(msg);
            return false;
        }

        bool deckValid = true;

        if( deck[0].name() != "RUNSPEC" ) {
            std::string msg = "The first keyword of a valid deck must be RUNSPEC\n";
            auto curKeyword = deck[0];
            OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
            deckValid = false;
        }

        std::string curSectionName = deck[0].name();
        size_t curKwIdx = 1;
        for (; curKwIdx < deck.size(); ++curKwIdx) {
            const auto& curKeyword = deck[curKwIdx];
            const std::string& curKeywordName = curKeyword.name();

            if (!isSectionDelimiter( curKeyword )) {
                if( !parser.isRecognizedKeyword( curKeywordName ) )
                    // ignore unknown keywords for now (i.e. they can appear in any section)
                    continue;

                const auto& parserKeyword = parser.getParserKeywordFromDeckName( curKeywordName );
                if (ensureKeywordSectionAffiliation && !parserKeyword.isValidSection(curSectionName)) {
                    std::string msg =
                        "The keyword '"+curKeywordName+"' is located in the '"+curSectionName
                        +"' section where it is invalid";
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                continue;
            }

            if (curSectionName == "RUNSPEC") {
                if (curKeywordName != "GRID") {
                    std::string msg =
                        "The RUNSPEC section must be followed by GRID instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "GRID") {
                if (curKeywordName != "EDIT" && curKeywordName != "PROPS") {
                    std::string msg =
                        "The GRID section must be followed by EDIT or PROPS instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "EDIT") {
                if (curKeywordName != "PROPS") {
                    std::string msg =
                        "The EDIT section must be followed by PROPS instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "PROPS") {
                if (curKeywordName != "REGIONS" && curKeywordName != "SOLUTION") {
                    std::string msg =
                        "The PROPS section must be followed by REGIONS or SOLUTION instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "REGIONS") {
                if (curKeywordName != "SOLUTION") {
                    std::string msg =
                        "The REGIONS section must be followed by SOLUTION instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "SOLUTION") {
                if (curKeywordName != "SUMMARY" && curKeywordName != "SCHEDULE") {
                    std::string msg =
                        "The SOLUTION section must be followed by SUMMARY or SCHEDULE instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "SUMMARY") {
                if (curKeywordName != "SCHEDULE") {
                    std::string msg =
                        "The SUMMARY section must be followed by SCHEDULE instead of "+curKeywordName;
                    OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                    deckValid = false;
                }

                curSectionName = curKeywordName;
            }
            else if (curSectionName == "SCHEDULE") {
                // schedule is the last section, so every section delimiter after it is wrong...
                std::string msg =
                    "The SCHEDULE section must be the last one ("
                    +curKeywordName+" specified after SCHEDULE)";
                OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
                deckValid = false;
            }
        }

        // SCHEDULE is the last section and it is mandatory, so make sure it is there
        if (curSectionName != "SCHEDULE") {
            const auto& curKeyword = deck[deck.size() - 1];
            std::string msg =
                "The last section of a valid deck must be SCHEDULE (is "+curSectionName+")";
            OpmLog::warning(Log::fileMessage(curKeyword.location(), msg) );
            deckValid = false;
        }

        return deckValid;
    }

} // namespace Opm
