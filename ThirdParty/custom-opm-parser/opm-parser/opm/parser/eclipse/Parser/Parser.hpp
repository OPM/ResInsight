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

#ifndef OPM_PARSER_HPP
#define OPM_PARSER_HPP

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Utility/Stringview.hpp>

namespace Json {
    class JsonObject;
}

namespace Opm {

    class Deck;
    class ParseContext;
    class RawKeyword;

    /// The hub of the parsing process.
    /// An input file in the eclipse data format is specified, several steps of parsing is performed
    /// and the semantically parsed result is returned.

    class Parser {
    public:
        Parser(bool addDefault = true);

        static std::string stripComments(const std::string& inputString);

        /// The starting point of the parsing process. The supplied file is parsed, and the resulting Deck is returned.
        std::shared_ptr< Deck > parseFile(const std::string &dataFile, const ParseContext& parseContext) const;
        std::shared_ptr< Deck > parseString(const std::string &data, const ParseContext& parseContext) const;
        std::shared_ptr< Deck > parseStream(std::unique_ptr<std::istream>&& inputStream , const ParseContext& parseContext) const;

        Deck * newDeckFromFile(const std::string &dataFileName, const ParseContext& parseContext) const;
        Deck * newDeckFromString(const std::string &dataFileName, const ParseContext& parseContext) const;

        std::shared_ptr< Deck > parseFile(const std::string &dataFile, bool strict = true) const;
        std::shared_ptr< Deck > parseString(const std::string &data, bool strict = true) const;

        /// Method to add ParserKeyword instances, these holding type and size information about the keywords and their data.
        void addParserKeyword(const Json::JsonObject& jsonKeyword);
        void addParserKeyword(std::unique_ptr< const ParserKeyword >&& parserKeyword);

        /*!
         * \brief Returns whether the parser knows about a keyword
         */
        bool hasKeyword( const std::string& ) const;
        const ParserKeyword* getKeyword(const std::string& name) const;

        bool isRecognizedKeyword( const string_view& deckKeywordName) const;
        const ParserKeyword* getParserKeywordFromDeckName(const string_view& deckKeywordName) const;
        std::vector<std::string> getAllDeckNames () const;

        void loadKeywords(const Json::JsonObject& jsonKeywords);
        bool loadKeywordFromFile(const boost::filesystem::path& configFile);

        void loadKeywordsFromDirectory(const boost::filesystem::path& directory , bool recursive = true);
        void applyUnitsToDeck(Deck& deck) const;

        /*!
         * \brief Returns the approximate number of recognized keywords in decks
         *
         * This is an approximate number because regular expresions are disconsidered.
         */
        size_t size() const;

        template <class T>
        void addKeyword() {
            addParserKeyword( std::unique_ptr< ParserKeyword >( new T ) );
        }

        static EclipseState parse(const Deck& deck,            const ParseContext& context = ParseContext());
        static EclipseState parse(const std::string &filename, const ParseContext& context = ParseContext());
        static EclipseState parseData(const std::string &data, const ParseContext& context = ParseContext());

        /// Parses the deck specified in filename.  If context contains ParseContext::PARSE_PARTIAL_DECK,
        /// we construct only a lean grid, otherwise, we construct a full EclipseState and return the
        /// fully constructed InputGrid
        static std::shared_ptr<const EclipseGrid> parseGrid(const std::string &filename,
                const ParseContext& context = ParseContext());

        /// Parses the provided deck.  If context contains ParseContext::PARSE_PARTIAL_DECK,
        /// we construct only a lean grid, otherwise, we construct a full EclipseState and return the
        /// fully constructed InputGrid
        static std::shared_ptr<const EclipseGrid> parseGrid(const Deck& deck,
                const ParseContext& context = ParseContext());

        /// Parses the provided deck string.  If context contains ParseContext::PARSE_PARTIAL_DECK,
        /// we construct only a lean grid, otherwise, we construct a full EclipseState and return the
        /// fully constructed InputGrid
        static std::shared_ptr<const EclipseGrid> parseGridData(const std::string &data,
                const ParseContext& context = ParseContext());

    private:
        // associative map of the parser internal name and the corresponding ParserKeyword object
        std::vector< std::unique_ptr< const ParserKeyword > > keyword_storage;
        // associative map of deck names and the corresponding ParserKeyword object
        std::map< string_view, const ParserKeyword* > m_deckParserKeywords;
        // associative map of the parser internal names and the corresponding
        // ParserKeyword object for keywords which match a regular expression
        std::map< string_view, const ParserKeyword* > m_wildCardKeywords;

        bool hasWildCardKeyword(const std::string& keyword) const;
        const ParserKeyword* matchingKeyword(const string_view& keyword) const;

        void addDefaultKeywords();
    };


    typedef std::shared_ptr<Parser> ParserPtr;
    typedef std::shared_ptr<const Parser> ParserConstPtr;
} // namespace Opm
#endif  /* PARSER_H */

