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

#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <opm/json/JsonObject.hpp>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserConst.hpp>
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>

#include "raw/RawConsts.hpp"
#include "raw/RawKeyword.hpp"
#include "raw/RawRecord.hpp"

namespace Opm {
KeywordSize::KeywordSize(const std::string& in_keyword, const std::string& in_item, int in_shift)
    : KeywordSize(in_keyword, in_item, false, in_shift)
{
}

KeywordSize::KeywordSize(const std::string& in_keyword, const std::string& in_item)
    : KeywordSize(in_keyword, in_item, false, 0)
{
}

KeywordSize::KeywordSize(const std::string& in_keyword, const std::string& in_item, bool table_collection, int in_shift)
    : shift(in_shift)
    , is_table_collection(table_collection)
    , m_size_type(OTHER_KEYWORD_IN_DECK)
    , m_max_size(std::make_pair(in_keyword, in_item))
{
}

KeywordSize::KeywordSize(std::size_t in_min_size, const std::string& in_keyword, const std::string& in_item, bool table_collection, int in_shift)
    : KeywordSize(in_keyword, in_item, table_collection, in_shift)
{
    this->min_size(in_min_size);
}

KeywordSize::KeywordSize()
    : KeywordSize(SLASH_TERMINATED)
{
}

KeywordSize::KeywordSize(ParserKeywordSizeEnum size_type)
    : m_size_type(size_type)
{
    if (size_type == SLASH_TERMINATED)
        return;

    if (size_type == UNKNOWN)
        return;

    if (size_type == DOUBLE_SLASH_TERMINATED)
        return;

    throw std::logic_error("This constructor only allows size type UNKNOWN and SLASH_TERMINATED");
}

KeywordSize::KeywordSize(std::size_t fixed_size)
    : KeywordSize(fixed_size, false)
{
}

KeywordSize::KeywordSize(std::size_t fixed_size, bool code)
    : m_size_type(code ? FIXED_CODE : FIXED)
    , m_max_size(fixed_size)
    , is_code(code)
{
}

KeywordSize::KeywordSize(std::size_t in_min_size, std::size_t fixed_size, bool code)
    : KeywordSize(fixed_size, code)
{
    this->min_size(in_min_size);
}

bool
KeywordSize::operator==(const KeywordSize&) const
{
    return true;
}

bool
KeywordSize::operator!=(const KeywordSize& other) const
{
    return !(*this == other);
}

bool
KeywordSize::table_collection() const
{
    return this->is_table_collection;
}

ParserKeywordSizeEnum
KeywordSize::size_type() const
{
    return this->m_size_type;
}

bool
KeywordSize::code() const
{
    return this->is_code;
}

int
KeywordSize::size_shift() const
{
    return this->shift;
}

const std::string&
KeywordSize::keyword() const
{
    return std::get<1>(this->m_max_size.value()).first;
}

const std::string&
KeywordSize::item() const
{
    return std::get<1>(this->m_max_size.value()).second;
}

std::optional<std::size_t> KeywordSize::min_size() const
{
    return this->m_min_size;
}

const std::optional<std::variant<std::size_t, std::pair<std::string, std::string>>>&
KeywordSize::max_size() const
{
    return this->m_max_size;
}

void KeywordSize::min_size(int s) {
    this->m_min_size = s;
}

std::string
KeywordSize::construct() const
{
    if (this->m_size_type == UNKNOWN || this->m_size_type == DOUBLE_SLASH_TERMINATED || this->m_size_type == SLASH_TERMINATED)
        return fmt::format("KeywordSize({})", ParserKeywordSizeEnum2String(this->m_size_type));

    if (this->m_size_type == FIXED || this->m_size_type == FIXED_CODE) {
        if (this->min_size().has_value())
            return fmt::format("KeywordSize({}, {}, {})", this->min_size().value(), std::get<std::size_t>(this->m_max_size.value()), this->is_code);
        else
            return fmt::format("KeywordSize({}, {})", std::get<std::size_t>(this->m_max_size.value()), this->is_code);
    }

    if (this->m_size_type == OTHER_KEYWORD_IN_DECK) {
        const auto& [size_kw, size_item] = std::get<1>(this->m_max_size.value());
        if (this->min_size().has_value())
            return fmt::format("KeywordSize({}, \"{}\", \"{}\", {}, {})", this->min_size().value(), size_kw, size_item, this->is_table_collection, this->shift);
        else
            return fmt::format("KeywordSize(\"{}\", \"{}\", {}, {})", size_kw, size_item, this->is_table_collection, this->shift);
    }

    throw std::logic_error("No string serialization known?");
}


    ParserKeyword::ParserKeyword(const std::string& name)
        : ParserKeyword(name, KeywordSize{})
    {
    }

    ParserKeyword::ParserKeyword(const std::string& name, KeywordSize kw_size)
        : m_name(name)
        , keyword_size(std::move(kw_size))
    {
        this->m_deckNames.insert(m_name);
    }

    void ParserKeyword::clearDeckNames() {
        m_deckNames.clear();
    }

    void ParserKeyword::addDeckName( const std::string& deckName ) {
        m_deckNames.insert(deckName);
    }

    bool ParserKeyword::hasDimension() const {
        auto have_dim = []( const ParserRecord& r ) {
            return r.hasDimension();
        };

        return std::any_of( this->begin(), this->end(), have_dim );
    }


    bool ParserKeyword::isTableCollection() const {
        return this->keyword_size.table_collection();
    }

    std::string ParserKeyword::getDescription() const {
        return m_Description;
    }

    void ParserKeyword::setDescription(const std::string& description) {
        m_Description = description;
    }


    void ParserKeyword::setCodeEnd(const std::string& end) {
        this->code_end = end;
    }

    const std::string& ParserKeyword::codeEnd() const {
        return this->code_end;
    }

    void ParserKeyword::initSize(const Json::JsonObject& jsonConfig) {
        // The number of record has been set explicitly with the size: keyword
        if (jsonConfig.has_item("size")) {
            Json::JsonObject sizeObject = jsonConfig.get_item("size");

            if (sizeObject.is_number())
                this->keyword_size = KeywordSize( sizeObject.as_int() );
            else
                initSizeKeyword(false, sizeObject);
            return;
        }

        if (jsonConfig.has_item("num_tables")) {
            Json::JsonObject numTablesObject = jsonConfig.get_item("num_tables");

            if (!numTablesObject.is_object())
              throw std::invalid_argument(
                  "The num_tables key must point to a {} object");

            initSizeKeyword(true, numTablesObject);
            return;
        }

        if (jsonConfig.has_item("records_set")) {
           this->keyword_size = KeywordSize(DOUBLE_SLASH_TERMINATED);
           return;
        }

        if (jsonConfig.has_item("code")) {
            this->keyword_size = KeywordSize(1, true);
            return;
        }

        if (jsonConfig.has_item("data")) {
            this->keyword_size = KeywordSize(1);
            return;
        }

        if (jsonConfig.has_item("items") || jsonConfig.has_item("records")) {
            // The number of records is undetermined - the keyword will be '/'
            // terminated.
            this->keyword_size = KeywordSize(SLASH_TERMINATED);
            return;
        }

        // No data associated with this keyword - like e.g. section headers
        this->keyword_size = KeywordSize(0);
    }


    void ParserKeyword::parseRecords( const Json::JsonObject& recordsConfig) {
         if (recordsConfig.is_array()) {
             size_t num_records = recordsConfig.size();
             for (size_t i = 0; i < num_records; i++) {
                  const Json::JsonObject itemsConfig = recordsConfig.get_array_item(i);
                  addItems(itemsConfig);
             }
         } else
             throw std::invalid_argument("The records item must point to an array item");
    }


    ParserKeyword::ParserKeyword(const Json::JsonObject& jsonConfig) :
        ParserKeyword(jsonConfig.get_string("name"))
    {

        if (jsonConfig.has_item("deck_names") || jsonConfig.has_item("deck_name_regex") )
            // if either the deck names or the regular expression for deck names are
            // explicitly specified, we do not implicitly add the contents of the 'name'
            // item to the deck names...
            clearDeckNames();

        initSize(jsonConfig);
        if (jsonConfig.has_item("min_size")) {
            auto min_size = jsonConfig.get_int("min_size");
            this->keyword_size.min_size(min_size);
        }

        initDeckNames(jsonConfig);
        initSectionNames(jsonConfig);
        initMatchRegex(jsonConfig);

        if (jsonConfig.has_item("prohibits")) {
            initProhibitedKeywords(jsonConfig.get_item("prohibits"));
        }

        if (jsonConfig.has_item("requires")) {
            initRequiredKeywords(jsonConfig.get_item("requires"));
        }

        if (jsonConfig.has_item("items") && (jsonConfig.has_item("records") ||
                                             jsonConfig.has_item("alternating_records") ||
                                             jsonConfig.has_item("records_set") ))
            throw std::invalid_argument("Fatal error in " + getName() + " configuration. Can NOT have both records: and items:");

        if (jsonConfig.has_item("items")) {
            const Json::JsonObject itemsConfig = jsonConfig.get_item("items");
            addItems(itemsConfig);
        }

        if (jsonConfig.has_item("records")) {
            const Json::JsonObject recordsConfig = jsonConfig.get_item("records");
            parseRecords( recordsConfig );
        }

        if (jsonConfig.has_item("alternating_records")) {
            alternating_keyword = true;
            const Json::JsonObject recordsConfig = jsonConfig.get_item("alternating_records");
            parseRecords( recordsConfig );
        }

        if (jsonConfig.has_item("records_set")) {
            double_records = true;
            const Json::JsonObject recordsConfig = jsonConfig.get_item("records_set");
            parseRecords( recordsConfig );
        }

        if (jsonConfig.has_item("data"))
            initData(jsonConfig);

        if (jsonConfig.has_item("code"))
            this->initCode(jsonConfig);

        if (jsonConfig.has_item("description"))
            m_Description = jsonConfig.get_string("description");

    }

    void ParserKeyword::initProhibitedKeywords(const Json::JsonObject& keywordList) {
        if (!keywordList.is_array())
            throw std::invalid_argument("The 'prohibits' JSON item of keyword "+m_name+" needs to be a list");

        for (std::size_t keywordIndex { 0 } ; keywordIndex != keywordList.size() ; ++keywordIndex) {
            const auto keywordObject { keywordList.get_array_item(keywordIndex) } ;

            if (!keywordObject.is_string())
                throw std::invalid_argument("The sub-items of 'prohibits' of keyword "+m_name+" need to be strings");

            m_prohibits.emplace_back(keywordObject.as_string());
        }
    }

    void ParserKeyword::initRequiredKeywords(const Json::JsonObject& keywordList) {
        if (!keywordList.is_array())
            throw std::invalid_argument("The 'requires' JSON item of keyword "+m_name+" needs to be a list");

        for (std::size_t keywordIndex { 0 } ; keywordIndex != keywordList.size() ; ++keywordIndex) {
            const auto keywordObject { keywordList.get_array_item(keywordIndex) } ;

            if (!keywordObject.is_string())
                throw std::invalid_argument("The sub-items of 'requires' of keyword "+m_name+" need to be strings");

            m_requires.emplace_back(keywordObject.as_string());
        }
    }

    void ParserKeyword::initSizeKeyword(const std::string& sizeKeyword, const std::string& sizeItem, bool table_collection, int size_shift) {
        this->keyword_size = KeywordSize(sizeKeyword, sizeItem, table_collection, size_shift);
    }

    void ParserKeyword::initSizeKeyword(bool table_collection, const Json::JsonObject& sizeObject) {
        if (sizeObject.is_object()) {
            std::string sizeKeyword = sizeObject.get_string("keyword");
            std::string sizeItem = sizeObject.get_string("item");
            int size_shift = 0;
            if (sizeObject.has_item("shift"))
                size_shift = sizeObject.get_int("shift");

            this->keyword_size = KeywordSize{sizeKeyword, sizeItem, table_collection, size_shift};
        } else {
            auto size_type = ParserKeywordSizeEnumFromString( sizeObject.as_string() );
            this->keyword_size = KeywordSize(size_type);
        }
    }


    bool ParserKeyword::validNameStart( const std::string_view& name) {
        if (!isalpha(name[0]))
            return false;

        return true;
    }

    bool ParserKeyword::validInternalName( const std::string& name ) {
        if( name.length() < 2 ) return false;
        if( !std::isalpha( name[0] ) ) return false;

        const auto ok = []( char c ) { return std::isalnum( c ) || c == '_'; };

        return std::all_of( name.begin() + 1, name.end(), ok );
    }


    bool ParserKeyword::validDeckName( const std::string_view& name) {

        if( !validNameStart( name ) )
            return false;

        const auto valid = []( char c ) {
            return std::isalnum( c ) || c == '-' || c == '_' || c == '+';
        };

        return std::all_of( name.begin() + 1, name.end(), valid );
    }

    bool ParserKeyword::hasMultipleDeckNames() const {
        return m_deckNames.size() > 1;
    }

    void ParserKeyword::initDeckNames(const Json::JsonObject& jsonObject) {
        if (!jsonObject.has_item("deck_names"))
            return;

        const Json::JsonObject namesObject = jsonObject.get_item("deck_names");
        if (!namesObject.is_array())
            throw std::invalid_argument("The 'deck_names' JSON item of keyword "+m_name+" needs to be a list");

        if (namesObject.size() > 0)
            m_deckNames.clear();

        for (size_t nameIdx = 0; nameIdx < namesObject.size(); ++ nameIdx) {
            const Json::JsonObject nameObject = namesObject.get_array_item(nameIdx);

            if (!nameObject.is_string())
                throw std::invalid_argument("The sub-items of 'deck_names' of keyword "+m_name+" need to be strings");

            addDeckName(nameObject.as_string());
        }
    }

    void ParserKeyword::initSectionNames(const Json::JsonObject& jsonObject) {
        if (!jsonObject.has_item("sections"))
            throw std::invalid_argument("The 'sections' JSON item of keyword "+m_name+" needs to be defined");

        const Json::JsonObject namesObject = jsonObject.get_item("sections");

        if (!namesObject.is_array())
            throw std::invalid_argument("The 'sections' JSON item of keyword "+m_name+" needs to be a list");

        m_validSectionNames.clear();
        for (size_t nameIdx = 0; nameIdx < namesObject.size(); ++ nameIdx) {
            const Json::JsonObject nameObject = namesObject.get_array_item(nameIdx);

            if (!nameObject.is_string())
                throw std::invalid_argument("The sub-items of 'sections' of keyword "+m_name+" need to be strings");

            addValidSectionName(nameObject.as_string());
        }
    }

    void ParserKeyword::initMatchRegex(const Json::JsonObject& jsonObject) {
        if (!jsonObject.has_item("deck_name_regex"))
            return;

        const Json::JsonObject regexStringObject = jsonObject.get_item("deck_name_regex");
        if (!regexStringObject.is_string())
            throw std::invalid_argument("The 'deck_name_regex' JSON item of keyword "+m_name+" need to be a string");

        setMatchRegex(regexStringObject.as_string());
    }

    void ParserKeyword::addItems(const Json::JsonObject& itemsConfig) {
        if( !itemsConfig.is_array() )
            throw std::invalid_argument("The 'items' JSON item missing must be an array in keyword "+getName()+".");

        size_t num_items = itemsConfig.size();
        ParserRecord record;

        for (size_t i = 0; i < num_items; i++) {
            const Json::JsonObject& itemConfig = itemsConfig.get_array_item(i);
            record.addItem( ParserItem( itemConfig ) );
        }

        this->addRecord( record );
    }

namespace {

void set_dimensions( ParserItem& item,
                    const Json::JsonObject& json,
                    const std::string& keyword ) {
    if( !json.has_item("dimension") ) return;

    const auto& dim = json.get_item("dimension");
    if( dim.is_string() ) {
        item.push_backDimension( dim.as_string() );
    }
    else if( dim.is_array() ) {
        for (size_t idim = 0; idim < dim.size(); idim++)
            item.push_backDimension( dim.get_array_item( idim ).as_string() );
    }
    else {
        throw std::invalid_argument(
                "The 'dimension' attribute of keyword " + keyword
                + " must be a string or a list of strings" );
    }
}

}

    void ParserKeyword::initCode(const Json::JsonObject& jsonConfig) {
        const Json::JsonObject codeConfig = jsonConfig.get_item("code");
        if (!codeConfig.has_item("end"))
            throw std::invalid_argument("The end: is missing from the code block");
        this->setCodeEnd(codeConfig.get_string("end"));

        const std::string itemName("code");
        auto input_type = ParserItem::itype::RAW_STRING;
        ParserItem item( itemName, input_type);
        ParserRecord record;
        item.setSizeType( ParserItem::item_size::ALL );

        record.addItem(item);
        this->addRecord(record);
    }


    void ParserKeyword::initData(const Json::JsonObject& jsonConfig) {
        const Json::JsonObject dataConfig = jsonConfig.get_item("data");
        if (!dataConfig.has_item("value_type") )
            throw std::invalid_argument("The 'value_type' JSON item of keyword "+getName()+" is missing");

        const std::string value_type = dataConfig.get_string("value_type");
        const std::string itemName("data");
        bool hasDefault = dataConfig.has_item("default");
        auto input_type = ParserItem::from_string(dataConfig.get_string("value_type"));
        ParserItem item( itemName, input_type);
        ParserRecord record;

        item.setSizeType( ParserItem::item_size::ALL );

        if (input_type == ParserItem::itype::INT) {
            if(hasDefault) {
                int defaultValue = dataConfig.get_int("default");
                item.setDefault(defaultValue);
            }
            record.addDataItem(item);
            this->addDataRecord(record);
            return;
        }

        if (input_type == ParserItem::itype::STRING || input_type == ParserItem::itype::RAW_STRING) {
            if (hasDefault) {
                std::string defaultValue = dataConfig.get_string("default");
                item.setDefault(defaultValue);
            }
            record.addDataItem(item);
            this->addDataRecord(record);
            return;
        }

        if (input_type == ParserItem::itype::DOUBLE) {
            if (hasDefault) {
                double defaultValue = dataConfig.get_double("default");
                item.setDefault(defaultValue);
            }
            set_dimensions( item, dataConfig, this->getName() );
            record.addDataItem(item);
            this->addDataRecord(record);
            return;
        }

        throw std::invalid_argument("While initializing keyword "+getName()+": Values of type "+dataConfig.get_string("value_type")+" are not implemented.");
    }


    const ParserRecord& ParserKeyword::getRecord(size_t recordIndex) const {
        if( this->m_records.empty() )
            throw std::invalid_argument( "Trying to get record from empty keyword" );

        if( recordIndex >= this->m_records.size() ) {
            if (alternating_keyword) {
                return this->m_records[ recordIndex % this->m_records.size() ];
            }
            else
                return this->m_records.back();
        }

        return this->m_records[ recordIndex ];
    }

    ParserRecord& ParserKeyword::getRecord( size_t index ) {
        return const_cast< ParserRecord& >(
                 const_cast< const ParserKeyword& >( *this ).getRecord( index )
                );
    }


    std::vector< ParserRecord >::const_iterator ParserKeyword::begin() const {
        return m_records.begin();
    }

    std::vector< ParserRecord >::const_iterator ParserKeyword::end() const {
        return m_records.end();
    }

    const std::unordered_set<std::string>& ParserKeyword::deck_names() const {
        return this->m_deckNames;
    }

    const std::unordered_set<std::string>& ParserKeyword::sections() const {
        return this->m_validSectionNames;
    }

    void ParserKeyword::addRecord( ParserRecord record ) {
        m_records.push_back( std::move( record ) );
        if (record.rawStringRecord())
            this->raw_string_keyword = true;
    }


    void ParserKeyword::addDataRecord( ParserRecord record) {
        if (this->keyword_size.size_type() != FIXED)
            throw std::invalid_argument("When calling addDataRecord() for keyword " + getName() + ", it must be configured with fixed size == 1.");

        auto max_size = std::get<std::size_t>(this->keyword_size.max_size().value());
        if (max_size != 1)
            throw std::invalid_argument("When calling addDataRecord() for keyword " + getName() + ", it must be configured with fixed size == 1.");

        this->addRecord(std::move(record));
    }


    const std::string ParserKeyword::className() const {
        return getName();
    }

    const std::string& ParserKeyword::getName() const {
        return m_name;
    }

    void ParserKeyword::clearValidSectionNames() {
        m_validSectionNames.clear();
    }

    void ParserKeyword::addValidSectionName( const std::string& sectionName ) {
        m_validSectionNames.insert(sectionName);
    }

    bool ParserKeyword::isValidSection(const std::string& sectionName) const {
        return m_validSectionNames.size() == 0 || m_validSectionNames.count(sectionName) > 0;
    }

    const std::vector<std::string>& ParserKeyword::requiredKeywords() const {
        return m_requires;
    }

    const std::vector<std::string>& ParserKeyword::prohibitedKeywords() const {
        return m_prohibits;
    }

    void ParserKeyword::setRequiredKeywords(const std::vector<std::string>& keywordNames) {
        m_requires = keywordNames;
    }

    void ParserKeyword::setProhibitedKeywords(const std::vector<std::string>& keywordNames) {
        m_prohibits = keywordNames;
    }

    DeckKeyword ParserKeyword::parse(const ParseContext& parseContext,
                                     ErrorGuard& errors,
                                     RawKeyword& rawKeyword,
                                     UnitSystem& active_unitsystem,
                                     UnitSystem& default_unitsystem) const {

        if( !rawKeyword.isFinished() )
            throw std::invalid_argument("Tried to create a deck keyword from an incomplete raw keyword " + rawKeyword.getKeywordName());

        DeckKeyword keyword( rawKeyword.location(), rawKeyword.getKeywordName() );
        keyword.setDataKeyword( isDataKeyword() );

        if (double_records)
            keyword.setDoubleRecordKeyword();

        if (double_records) {
            /* Note: this merely dumps all records sequentially into m_recordList.
               Each block of records is separated by an empty DeckRecord.
            */
            size_t record_nr = 0;
            for (auto& rawRecord : rawKeyword) {
                if (rawRecord.size() == 0) {
                     keyword.addRecord( DeckRecord() );
                     record_nr = 0;
                }
                else {
                    keyword.addRecord( this->getRecord( record_nr ).parse( parseContext, errors, rawRecord, active_unitsystem, default_unitsystem, rawKeyword.location() ) );
                    record_nr++;
                }
            }
        }
        else {
            size_t record_nr = 0;
            for( auto& rawRecord : rawKeyword ) {
                if( m_records.size() == 0 && rawRecord.size() > 0 )
                    throw std::invalid_argument("Missing item information " + rawKeyword.getKeywordName());

                keyword.addRecord( this->getRecord( record_nr ).parse( parseContext, errors, rawRecord, active_unitsystem, default_unitsystem, rawKeyword.location() ) );
                record_nr++;
            }
        }

        if (this->hasFixedSize( ))
            keyword.setFixedSize( );

        const auto& kw_size = this->keyword_size;
        if (kw_size.size_type() == OTHER_KEYWORD_IN_DECK) {
            if (!kw_size.table_collection())
                keyword.setFixedSize( );
        }

        if (kw_size.size_type() == UNKNOWN)
            keyword.setFixedSize( );

        return keyword;
    }

    std::optional<std::size_t> ParserKeyword::min_size() const {
        return this->keyword_size.min_size();
    }

    size_t ParserKeyword::getFixedSize() const {
        if (!hasFixedSize())
            throw std::logic_error("The parser keyword "+getName()+" does not have a fixed size!");
        const auto& max_size = this->keyword_size.max_size();
        return std::get<std::size_t>(max_size.value());
    }

    bool ParserKeyword::hasFixedSize() const {
        auto size_type = this->keyword_size.size_type();
        return (size_type == FIXED || size_type == FIXED_CODE || this->m_records.empty());
    }

    enum ParserKeywordSizeEnum ParserKeyword::getSizeType() const {
        return this->keyword_size.size_type();
    }

    bool ParserKeyword::rawStringKeyword() const {
        return this->raw_string_keyword;
    }

    const KeywordSize& ParserKeyword::getKeywordSize() const {
        return keyword_size;
    }

    bool ParserKeyword::isDataKeyword() const {
        if( this->m_records.empty() ) return false;

        return this->m_records.front().isDataRecord();
    }

    bool ParserKeyword::isCodeKeyword() const {
        return this->keyword_size.code();
    }

    bool ParserKeyword::isAlternatingKeyword() const {
        return alternating_keyword;
    }

    bool ParserKeyword::isDoubleRecordKeyword() const {
        return double_records;
    }

    void ParserKeyword::setAlternatingKeyword(bool alternating) {
        alternating_keyword = alternating;
    }

    void ParserKeyword::setDoubleRecordsKeyword(bool double_rec) {
        double_records = double_rec;
    }

    bool ParserKeyword::hasMatchRegex() const {
        return !m_matchRegexString.empty();
    }

    void ParserKeyword::setMatchRegex(const std::string& deckNameRegexp) {
        try {
            m_matchRegex = std::regex(deckNameRegexp);
            m_matchRegexString = deckNameRegexp;
        }
        catch (const std::exception &e) {
            std::cerr << "Warning: Malformed regular expression for keyword '" << getName() << "':\n"
                      << "\n"
                      << e.what() << "\n"
                      << "\n"
                      << "Ignoring expression!\n";
        }
    }

    bool ParserKeyword::matches(const std::string_view& name ) const {
        if (!validDeckName(name ))
            return false;

        else if( m_deckNames.count( std::string(name) ) )
            return true;

        else if (hasMatchRegex()) {
            bool match = std::regex_match( name.begin(), name.end(), m_matchRegex);
            if (match)
                return true;
        }

        // Last desperate attempt - go through the deckNames list and
        // interpret the elements as a regular expression.
        for (const auto& deck_name : this->m_deckNames) {
            if (std::regex_match(name.begin(), name.end(), std::regex(deck_name)))
                return true;
        }
        return false;
    }

    std::string ParserKeyword::createDeclaration(const std::string& indent) const {
        std::stringstream ss;
        ss << indent << "class " << className() << " : public ParserKeyword {" << '\n';
        ss << indent << "public:" << '\n';
        {
            std::string local_indent = indent + "    ";
            ss << local_indent << className() << "();" << '\n';
            ss << local_indent << "static const std::string keywordName;" << '\n';
            if (m_records.size() > 0 ) {
                for( const auto& record : *this ) {
                    for( const auto& item : record ) {
                        ss << '\n';
                        item.inlineClass(ss , local_indent );
                    }
                }
            }
        }
        ss << indent << "};" << '\n' << '\n' << '\n';
        return ss.str();
    }


    std::string ParserKeyword::createDecl() const {
        return className() + "::" + className() + "()";
    }


    std::string ParserKeyword::createCode() const {
        std::stringstream ss;
        const std::string lhs = "keyword";
        const std::string indent = "  ";

        ss << fmt::format("{0}::{0}() : ParserKeyword(\"{1}\", {2}) {{\n",
                          this->className(),
                          this->m_name,
                          this->keyword_size.construct());

        //{
        //    const std::string sizeString(ParserKeywordSizeEnum2String(m_keywordSizeType));
        //    ss << indent;
        //    switch (m_keywordSizeType) {
        //    case SLASH_TERMINATED:
        //    case FIXED_CODE:
        //    case DOUBLE_SLASH_TERMINATED:
        //    case UNKNOWN:
        //        ss << "setSizeType(" << sizeString << ");" << '\n';
        //        break;
        //    case FIXED:
        //        ss << "setFixedSize( (size_t) " << m_fixedSize << ");" << '\n';
        //        break;
        //    case OTHER_KEYWORD_IN_DECK:
        //        ss << "setSizeType(" << sizeString << ");" << '\n';
        //        ss << indent << "initSizeKeyword(\"" << keyword_size.keyword << "\",\"" << keyword_size.item << "\"," << fmt::format("{}", this->isTableCollection()) << "," << keyword_size.shift << ");" << '\n';
        //        break;
        //    }
        //}

        // add the valid sections for the keyword
        for (auto sectionNameIt = m_validSectionNames.begin();
             sectionNameIt != m_validSectionNames.end();
             ++sectionNameIt)
        {
            ss << indent << "addValidSectionName(\"" << *sectionNameIt << "\");" << '\n';
        }

        // set required and prohibited keywords
        if (!m_prohibits.empty()) {
            ss << indent << "setProhibitedKeywords({" << '\n';
            for (const auto& keyword : m_prohibits) {
                ss << indent << indent << '"' << keyword << '"' << "," << '\n';
            }
            ss << indent << "});" << '\n';
        }

        if (!m_requires.empty()) {
            ss << indent << "setRequiredKeywords({" << '\n';
            for (const auto& keyword : m_requires) {
                ss << indent << indent << '"' << keyword << '"' << "," << '\n';
            }
            ss << indent << "});"  << '\n';
        }

        // add the deck names
        ss << indent << "clearDeckNames();\n";
        for (auto deckNameIt = m_deckNames.begin();
             deckNameIt != m_deckNames.end();
             ++deckNameIt)
        {
            ss << indent << "addDeckName(\"" << *deckNameIt << "\");" << '\n';
        }

        // set AlternatingRecords
        if (alternating_keyword)
            ss << indent << "setAlternatingKeyword(true);" << '\n';

        // set DoubleRecords
        if (double_records)
            ss << indent << "setDoubleRecordsKeyword(true);" << '\n';

        // set the deck name match regex
        if (hasMatchRegex())
            ss << indent << "setMatchRegex(\"" << m_matchRegexString << "\");" << '\n';

        if (this->keyword_size.code())
            ss << indent << "setCodeEnd(\"" << this->code_end << "\");" << '\n';

        {
            if (m_records.size() > 0 ) {
                for( const auto& record : *this ) {
                    const std::string local_indent = indent + "   ";
                    ss << indent << "{" << '\n';
                    ss << local_indent << "ParserRecord record;" << '\n';
                    for( const auto& item : record ) {
                        ss << local_indent << "{" << '\n';
                        {
                            std::string indent3 = local_indent + "   ";
                            ss << item.createCode(indent3);
                            {
                                std::string addItemMethod = "addItem";
                                if (isDataKeyword())
                                    addItemMethod = "addDataItem";

                                ss << indent3 << "record." << addItemMethod << "(item);" << '\n';
                            }
                        }
                        ss << local_indent << "}" << '\n';
                    }

                    if (record.isDataRecord())
                        ss << local_indent << "addDataRecord( record );" << '\n';
                    else
                        ss << local_indent << "addRecord( record );" << '\n';

                    ss << indent << "}" << '\n';
                }
            }
        }
        ss << "}" << '\n';

        ss << "const std::string " << className() << "::keywordName = \"" << getName() << "\";" << '\n';
        for( const auto& record : *this ) {
            for( const auto& item : record ) {
                ss << item.inlineClassInit(className());
            }
        }
        ss << '\n';

        return ss.str();
    }



    bool ParserKeyword::operator==( const ParserKeyword& rhs ) const {
        // compare the deck names. we don't care about the ordering of the strings.
        if (m_deckNames != rhs.m_deckNames)
            return false;

        if(    m_name              != rhs.m_name
            || this->code_end      != rhs.code_end
            || m_matchRegexString  != rhs.m_matchRegexString
            || isCodeKeyword()     != rhs.isCodeKeyword()
            || isDataKeyword()     != rhs.isDataKeyword())
          return false;

        return true;

        //switch( m_keywordSizeType ) {
        //    case FIXED:
        //        if( m_fixedSize != rhs.m_fixedSize )
        //            return false;
        //        break;

        //    case OTHER_KEYWORD_IN_DECK:
        //        if (this->keyword_size != rhs.keyword_size)
        //            return false;
        //        break;
        //    default:
        //        break;
        //}

        if (this->keyword_size != rhs.keyword_size)
            return false;

        return this->m_records.size() == rhs.m_records.size()
            && std::equal( this->begin(), this->end(), rhs.begin() );
    }

    bool ParserKeyword::operator!=( const ParserKeyword& rhs ) const {
        return !( *this == rhs );
    }

    std::ostream& operator<<( std::ostream& stream, const ParserKeyword& kw ) {
        stream << "ParserKeyword " << kw.getName() << " { " << std::endl
               << "records: [";

        if( kw.begin() != kw.end() ) stream << std::endl;

        for( const auto& record : kw )
            stream << record << std::endl;
        stream << "]";

        return stream << std::endl << "}";
    }

}
