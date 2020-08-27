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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserConst.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>

#include "raw/RawConsts.hpp"
#include "raw/RawKeyword.hpp"
#include "raw/RawRecord.hpp"

namespace Opm {

    void ParserKeyword::setSizeType( ParserKeywordSizeEnum sizeType ) {
        m_keywordSizeType = sizeType;
        if (sizeType == FIXED_CODE)
            this->m_fixedSize = 1;
    }

    void ParserKeyword::setFixedSize( size_t keywordSize) {
        m_keywordSizeType = FIXED;
        m_fixedSize = keywordSize;
    }

    void ParserKeyword::setTableCollection(bool _isTableCollection) {
        m_isTableCollection = _isTableCollection;
    }


    void ParserKeyword::commonInit(const std::string& name, ParserKeywordSizeEnum sizeType) {
        m_isTableCollection = false;
        m_name = name;
        m_keywordSizeType = sizeType;
        m_Description = "";
        m_fixedSize = 0;

        m_deckNames.insert(m_name);
    }

    ParserKeyword::ParserKeyword(const std::string& name) {
        commonInit(name, FIXED);
    }


    ParserKeyword::ParserKeyword(const std::string& name, const std::string& sizeKeyword, const std::string& sizeItem, int size_shift, bool _isTableCollection)
    {
        commonInit( name , OTHER_KEYWORD_IN_DECK);
        m_isTableCollection = _isTableCollection;
        initSizeKeyword(sizeKeyword, sizeItem, size_shift);
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
        return m_isTableCollection;
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

            if (sizeObject.is_number()) {
                m_fixedSize = (size_t) sizeObject.as_int();
                m_keywordSizeType = FIXED;
            } else
                initSizeKeyword(sizeObject);

            return;
        }

        if (jsonConfig.has_item("num_tables")) {
            Json::JsonObject numTablesObject = jsonConfig.get_item("num_tables");

            if (!numTablesObject.is_object())
              throw std::invalid_argument(
                  "The num_tables key must point to a {} object");

            initSizeKeyword(numTablesObject);
            m_isTableCollection = true;

            return;
        }

        if (jsonConfig.has_item("records_set")) {
           m_keywordSizeType = DOUBLE_SLASH_TERMINATED;
           return;
        }

        if (jsonConfig.has_item("items") || jsonConfig.has_item("records")) {
            // The number of records is undetermined - the keyword will be '/'
            // terminated.
            m_keywordSizeType = SLASH_TERMINATED;
            return;
        } else {
            m_keywordSizeType = FIXED;
            m_fixedSize = 0;
        }

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


    ParserKeyword::ParserKeyword(const Json::JsonObject& jsonConfig) {

      if (jsonConfig.has_item("name")) {
            ParserKeywordSizeEnum sizeType = UNKNOWN;
            commonInit(jsonConfig.get_string("name"), sizeType);
        } else
            throw std::invalid_argument("Json object is missing the 'name' property");

        if (jsonConfig.has_item("deck_names") || jsonConfig.has_item("deck_name_regex") )
            // if either the deck names or the regular expression for deck names are
            // explicitly specified, we do not implicitly add the contents of the 'name'
            // item to the deck names...
            clearDeckNames();

        initSize(jsonConfig);
        initDeckNames(jsonConfig);
        initSectionNames(jsonConfig);
        initMatchRegex(jsonConfig);

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



    void ParserKeyword::initSizeKeyword(const std::string& sizeKeyword, const std::string& sizeItem, int size_shift) {
        keyword_size = KeywordSize(sizeKeyword, sizeItem, size_shift); 
        m_keywordSizeType = OTHER_KEYWORD_IN_DECK;
    }

    void ParserKeyword::initSizeKeyword(const Json::JsonObject& sizeObject) {
        if (sizeObject.is_object()) {
            std::string sizeKeyword = sizeObject.get_string("keyword");
            std::string sizeItem = sizeObject.get_string("item");
            int size_shift = 0;
            if (sizeObject.has_item("shift"))
                size_shift = sizeObject.get_int("shift");

            initSizeKeyword(sizeKeyword, sizeItem, size_shift);
        } else {
            m_keywordSizeType = ParserKeywordSizeEnumFromString( sizeObject.as_string() );
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
        this->m_fixedSize = 1U;
        this->m_keywordSizeType = FIXED_CODE;

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
        this->m_fixedSize = 1U;
        this->m_keywordSizeType = FIXED;

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



    void ParserKeyword::addRecord( ParserRecord record ) {
        m_records.push_back( std::move( record ) );
        if (record.rawStringRecord())
            this->raw_string_keyword = true;
    }


    void ParserKeyword::addDataRecord( ParserRecord record) {
        if ((m_keywordSizeType == FIXED) && (m_fixedSize == 1U))
            addRecord( std::move( record ) );
        else
            throw std::invalid_argument("When calling addDataRecord() for keyword " + getName() + ", it must be configured with fixed size == 1.");
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

    ParserKeyword::SectionNameSet::const_iterator ParserKeyword::validSectionNamesBegin() const {
        return m_validSectionNames.begin();
    }

    ParserKeyword::SectionNameSet::const_iterator ParserKeyword::validSectionNamesEnd() const  {
        return m_validSectionNames.end();
    }

    ParserKeyword::DeckNameSet::const_iterator ParserKeyword::deckNamesBegin() const {
        return m_deckNames.begin();
    }

    ParserKeyword::DeckNameSet::const_iterator ParserKeyword::deckNamesEnd() const  {
        return m_deckNames.end();
    }

    DeckKeyword ParserKeyword::parse(const ParseContext& parseContext,
                                     ErrorGuard& errors,
                                     RawKeyword& rawKeyword,
                                     UnitSystem& active_unitsystem,
                                     UnitSystem& default_unitsystem,
                                     const std::string& filename) const {

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
                     keyword.addRecord( this->getRecord( record_nr ).parse( parseContext, errors, rawRecord, active_unitsystem, default_unitsystem, rawKeyword.getKeywordName(), filename ) );
                     record_nr++;
                }
            }
        }
        else {
            size_t record_nr = 0;
            for( auto& rawRecord : rawKeyword ) {
                if( m_records.size() == 0 && rawRecord.size() > 0 )
                    throw std::invalid_argument("Missing item information " + rawKeyword.getKeywordName());

                keyword.addRecord( this->getRecord( record_nr ).parse( parseContext, errors, rawRecord, active_unitsystem, default_unitsystem, rawKeyword.getKeywordName(), filename ) );
                record_nr++;
            }
        }

        if (this->hasFixedSize( ))
            keyword.setFixedSize( );

        if (this->m_keywordSizeType == OTHER_KEYWORD_IN_DECK) {
            if (!m_isTableCollection)
                keyword.setFixedSize( );
        }

        if (this->m_keywordSizeType == UNKNOWN)
            keyword.setFixedSize( );

        return keyword;
    }

    size_t ParserKeyword::getFixedSize() const {
        if (!hasFixedSize())
            throw std::logic_error("The parser keyword "+getName()+" does not have a fixed size!");
        return m_fixedSize;
    }

    bool ParserKeyword::hasFixedSize() const {
        return (this->m_keywordSizeType == FIXED || this->m_keywordSizeType == FIXED_CODE);
    }

    enum ParserKeywordSizeEnum ParserKeyword::getSizeType() const {
        return m_keywordSizeType;
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
        return (this->m_keywordSizeType == FIXED_CODE);
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

        else if (hasMatchRegex())
            return std::regex_match( name.begin(), name.end(), m_matchRegex);

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

        ss << className() << "::" << className() << "( ) : ParserKeyword(\"" << m_name << "\")" << '\n' << "{" << '\n';
        {
            const std::string sizeString(ParserKeywordSizeEnum2String(m_keywordSizeType));
            ss << indent;
            switch (m_keywordSizeType) {
            case SLASH_TERMINATED:
            case FIXED_CODE:
            case DOUBLE_SLASH_TERMINATED:
            case UNKNOWN:
                ss << "setSizeType(" << sizeString << ");" << '\n';
                break;
            case FIXED:
                ss << "setFixedSize( (size_t) " << m_fixedSize << ");" << '\n';
                break;
            case OTHER_KEYWORD_IN_DECK:
                ss << "setSizeType(" << sizeString << ");" << '\n';
                ss << indent << "initSizeKeyword(\"" << keyword_size.keyword << "\",\"" << keyword_size.item << "\"," << keyword_size.shift << ");" << '\n';
                if (m_isTableCollection)
                    ss << indent << "setTableCollection( true );" << '\n';
                break;
            }
        }

        // add the valid sections for the keyword
        for (auto sectionNameIt = m_validSectionNames.begin();
             sectionNameIt != m_validSectionNames.end();
             ++sectionNameIt)
        {
            ss << indent << "addValidSectionName(\"" << *sectionNameIt << "\");" << '\n';
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

        if (this->m_keywordSizeType == FIXED_CODE)
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
            || m_keywordSizeType   != rhs.m_keywordSizeType
            || isCodeKeyword()     != rhs.isCodeKeyword()
            || isDataKeyword()     != rhs.isDataKeyword()
            || m_isTableCollection != rhs.m_isTableCollection )
          return false;


        switch( m_keywordSizeType ) {
            case FIXED:
                if( m_fixedSize != rhs.m_fixedSize )
                    return false;
                break;

            case OTHER_KEYWORD_IN_DECK:
                if (this->keyword_size != rhs.keyword_size)
                    return false;
                break;
            default:
                break;
        }

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
