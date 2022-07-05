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
#ifndef PARSER_KEYWORD_H
#define PARSER_KEYWORD_H

#include <iosfwd>
#include <optional>
#include <regex>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>

#include <opm/input/eclipse/Parser/ParserEnums.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>

namespace Json {
    class JsonObject;
}

namespace Opm {
    class Deck;
    class DeckKeyword;
    class ParseContext;
    class ErrorGuard;
    class ParserDoubleItem;
    class RawKeyword;
    class ErrorGuard;

    /*
      Small helper struct to assemble the information needed to infer the size
      of a keyword based on another keyword in the deck.
    */
    class KeywordSize {
    public:
        KeywordSize();
        KeywordSize(const std::string& in_keyword, const std::string& in_item, int in_shift);
        KeywordSize(const std::string& in_keyword, const std::string& in_item);
        KeywordSize(const std::string& in_keyword, const std::string& in_item, bool table_collection, int in_shift);

        KeywordSize(std::size_t min_size, const std::string& in_keyword, const std::string& in_item, bool table_collection, int in_shift);
        explicit KeywordSize(ParserKeywordSizeEnum size_type);
        explicit KeywordSize(std::size_t fixed_size);
        KeywordSize(std::size_t fixed_size, bool code);
        KeywordSize(std::size_t min_size, std::size_t fixed_size, bool code);

        bool table_collection() const;
        ParserKeywordSizeEnum size_type() const;
        bool code() const;
        int size_shift() const;
        const std::string& keyword() const;
        const std::string& item() const;
        std::optional<std::size_t> min_size() const;
        void min_size(int s);
        const std::optional<std::variant<std::size_t, std::pair<std::string, std::string>>>& max_size() const;
        std::string construct() const;

        bool operator==(const KeywordSize& ) const;
        bool operator!=(const KeywordSize& other) const;
    private:
        int shift{0};
        bool is_table_collection{false};
        ParserKeywordSizeEnum m_size_type;
        std::optional<std::size_t> m_min_size;
        std::optional<std::variant<std::size_t, std::pair<std::string, std::string>>> m_max_size;
        bool is_code{false};
    };

    class ParserKeyword {
    public:
        ParserKeyword(const std::string& name, KeywordSize kw_size);
        explicit ParserKeyword(const std::string& name);
        explicit ParserKeyword(const Json::JsonObject& jsonConfig);

        void initSizeKeyword( const std::string& sizeKeyword, const std::string& sizeItem, bool table_collection, int size_shift);


        static bool validInternalName(const std::string& name);
        static bool validDeckName(const std::string_view& name);
        bool hasMatchRegex() const;
        void setMatchRegex(const std::string& deckNameRegexp);
        bool matches(const std::string_view& ) const;
        bool hasDimension() const;
        void addRecord( ParserRecord );
        void addDataRecord( ParserRecord );
        const ParserRecord& getRecord(size_t recordIndex) const;
        ParserRecord& getRecord(size_t recordIndex);
        std::vector< ParserRecord >::const_iterator begin() const;
        std::vector< ParserRecord >::const_iterator end() const;
        const std::string className() const;
        const std::string& getName() const;
        std::optional<std::size_t> min_size() const;
        size_t getFixedSize() const;
        bool hasFixedSize() const;
        bool isTableCollection() const;
        std::string getDescription() const;
        void setDescription(const std::string &description);

        bool hasMultipleDeckNames() const;
        void clearDeckNames();
        void addDeckName( const std::string& deckName );
        void setCodeEnd(const std::string& end);
        const std::unordered_set<std::string>& deck_names() const;
        const std::string& codeEnd() const;

        const std::vector<std::string>& requiredKeywords() const;
        const std::vector<std::string>& prohibitedKeywords() const;
        void setRequiredKeywords(const std::vector<std::string>&);
        void setProhibitedKeywords(const std::vector<std::string>&);

        void clearValidSectionNames();
        void addValidSectionName(const std::string& sectionName);
        bool isValidSection(const std::string& sectionName) const;
        const std::unordered_set<std::string>& sections() const;

        DeckKeyword parse(const ParseContext& parseContext, ErrorGuard& errors, RawKeyword& rawKeyword, UnitSystem& active_unitsystem, UnitSystem& default_unitsystem) const;
        enum ParserKeywordSizeEnum getSizeType() const;
        const KeywordSize& getKeywordSize() const;
        bool isDataKeyword() const;
        bool rawStringKeyword() const;
        bool isCodeKeyword() const;
        bool isAlternatingKeyword() const;
        bool isDoubleRecordKeyword() const;
        void setAlternatingKeyword(bool alternating);
        void setDoubleRecordsKeyword(bool double_rec);

        std::string createDeclaration(const std::string& indent) const;
        std::string createDecl() const;
        std::string createCode() const;

        bool operator==( const ParserKeyword& ) const;
        bool operator!=( const ParserKeyword& ) const;

    private:
        std::string m_name;
        KeywordSize keyword_size;
        std::unordered_set<std::string> m_deckNames;
        std::unordered_set<std::string> m_validSectionNames;
        std::string m_matchRegexString;
        std::regex m_matchRegex;
        std::vector< ParserRecord > m_records;
        std::string m_Description;
        bool raw_string_keyword = false;
        bool alternating_keyword = false;
        bool double_records = false;
        std::string code_end;
        std::vector<std::string> m_requires;
        std::vector<std::string> m_prohibits;

        static bool validNameStart(const std::string_view& name);
        void initDeckNames( const Json::JsonObject& jsonConfig );
        void initSectionNames( const Json::JsonObject& jsonConfig );
        void initMatchRegex( const Json::JsonObject& jsonObject );
        void initCode( const Json::JsonObject& jsonConfig );
        void initData( const Json::JsonObject& jsonConfig );
        void initProhibitedKeywords(const Json::JsonObject& keywordList);
        void initRequiredKeywords(const Json::JsonObject& keywordList);
        void initSize( const Json::JsonObject& jsonConfig );
        void initSizeKeyword(bool table_collection, const Json::JsonObject& sizeObject);
        void addItems( const Json::JsonObject& jsonConfig);
        void parseRecords( const Json::JsonObject& recordsConfig);
    };

std::ostream& operator<<( std::ostream&, const ParserKeyword& );
}

#endif
