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
#include <stdexcept>
#include <string>

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserConst.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/MessageContainer.hpp>
#include <opm/parser/eclipse/RawDeck/RawConsts.hpp>
#include <opm/parser/eclipse/RawDeck/RawKeyword.hpp>

namespace Opm {

    void ParserKeyword::setSizeType( ParserKeywordSizeEnum sizeType ) {
        m_keywordSizeType = sizeType;
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


    ParserKeyword::ParserKeyword(const std::string& name, const std::string& sizeKeyword, const std::string& sizeItem, bool _isTableCollection)
    {
        commonInit( name , OTHER_KEYWORD_IN_DECK);
        m_isTableCollection = _isTableCollection;
        initSizeKeyword(sizeKeyword, sizeItem);
    }

    void ParserKeyword::clearDeckNames() {
        m_deckNames.clear();
    }

    void ParserKeyword::addDeckName( const std::string& deckName ) {
        m_deckNames.insert(deckName);
    }

    bool ParserKeyword::hasDimension() const {
        for( const auto& record : m_records )
            if( record->hasDimension() ) return true;

        return false;
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

    void ParserKeyword::initSize(const Json::JsonObject& jsonConfig) {
        // The number of record has been set explicitly with the size: keyword
        if (jsonConfig.has_item("size")) {
            Json::JsonObject sizeObject = jsonConfig.get_item("size");

            if (sizeObject.is_number()) {
                m_fixedSize = (size_t) sizeObject.as_int();
                m_keywordSizeType = FIXED;
            } else
                initSizeKeyword(sizeObject);

        } else {
            if (jsonConfig.has_item("num_tables")) {
                Json::JsonObject numTablesObject = jsonConfig.get_item("num_tables");

                if (!numTablesObject.is_object())
                    throw std::invalid_argument("The num_tables key must point to a {} object");

                initSizeKeyword(numTablesObject);
                m_isTableCollection = true;
            } else {
                if (jsonConfig.has_item("items") || jsonConfig.has_item("records"))
                    // The number of records is undetermined - the keyword will be '/' terminated.
                    m_keywordSizeType = SLASH_TERMINATED;
                else {
                    m_keywordSizeType = FIXED;
                    if (jsonConfig.has_item("data"))
                        m_fixedSize = 1;
                    else
                        m_fixedSize = 0;
                }
            }
        }
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

        if (jsonConfig.has_item("items") && jsonConfig.has_item("records"))
            throw std::invalid_argument("Fatal error in " + getName() + " configuration. Can NOT have both records: and items:");

        if (jsonConfig.has_item("items")) {
            const Json::JsonObject itemsConfig = jsonConfig.get_item("items");
            addItems(itemsConfig);
        }

        if (jsonConfig.has_item("records")) {
            const Json::JsonObject recordsConfig = jsonConfig.get_item("records");
            if (recordsConfig.is_array()) {
                size_t num_records = recordsConfig.size();
                for (size_t i = 0; i < num_records; i++) {
                    const Json::JsonObject itemsConfig = recordsConfig.get_array_item(i);
                    addItems(itemsConfig);
                }
            } else
                throw std::invalid_argument("The records item must point to an array item");
        }

        if (jsonConfig.has_item("data"))
            initData(jsonConfig);

        if (jsonConfig.has_item("description")) {
            m_Description = jsonConfig.get_string("description");
        }

    }



    void ParserKeyword::initSizeKeyword(const std::string& sizeKeyword, const std::string& sizeItem) {
        m_sizeDefinitionPair = std::pair<std::string, std::string>(sizeKeyword, sizeItem);
        m_keywordSizeType = OTHER_KEYWORD_IN_DECK;
    }

    void ParserKeyword::initSizeKeyword(const Json::JsonObject& sizeObject) {
        if (sizeObject.is_object()) {
            std::string sizeKeyword = sizeObject.get_string("keyword");
            std::string sizeItem = sizeObject.get_string("item");
            initSizeKeyword(sizeKeyword, sizeItem);
        } else {
            m_keywordSizeType = ParserKeywordSizeEnumFromString( sizeObject.as_string() );
        }
    }


    bool ParserKeyword::validNameStart( const string_view& name) {
        if (name.length() > ParserConst::maxKeywordLength)
            return false;

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

    string_view ParserKeyword::getDeckName( const string_view& str ) {

        auto first_sep = std::find_if( str.begin(), str.end(), RawConsts::is_separator );

        // only look at the first 8 characters (at most)
        if( std::distance( str.begin(), first_sep ) < 9 )
            return { str.begin(), first_sep };

        return { str.begin(), str.begin() + 9 };
    }

    bool ParserKeyword::validDeckName( const string_view& name) {

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
        if (itemsConfig.is_array()) {
            size_t num_items = itemsConfig.size();
            std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();

            for (size_t i = 0; i < num_items; i++) {
                const Json::JsonObject itemConfig = itemsConfig.get_array_item(i);

                if (itemConfig.has_item("value_type")) {
                    ParserValueTypeEnum valueType = ParserValueTypeEnumFromString(itemConfig.get_string("value_type"));
                    switch (valueType) {
                    case INT:
                        {
                            ParserIntItemConstPtr item = ParserIntItemConstPtr(new ParserIntItem(itemConfig));
                            record->addItem( item );
                        }
                        break;
                    case STRING:
                        {
                            ParserStringItemConstPtr item = ParserStringItemConstPtr(new ParserStringItem(itemConfig));
                            record->addItem(item);
                        }
                        break;
                    case DOUBLE:
                        {
                            ParserDoubleItemPtr item = ParserDoubleItemPtr(new ParserDoubleItem(itemConfig));
                            initDoubleItemDimension( item , itemConfig );
                            record->addItem(item);
                        }
                        break;
                    default:
                        throw std::invalid_argument("While parsing "+getName()+": Values of type "+itemConfig.get_string("value_type")+" are not implemented.");
                    }
                } else
                    throw std::invalid_argument("'value_type' JSON item missing for keyword "+getName()+".");
            }
            addRecord(record);
        } else
            throw std::invalid_argument("The 'items' JSON item missing must be an array in keyword "+getName()+".");
    }

    void ParserKeyword::initDoubleItemDimension( ParserDoubleItemPtr item, const Json::JsonObject itemConfig) {
        if (itemConfig.has_item("dimension")) {
            const Json::JsonObject dimensionConfig = itemConfig.get_item("dimension");
            if (dimensionConfig.is_string())
                item->push_backDimension( dimensionConfig.as_string() );
            else if (dimensionConfig.is_array()) {
                for (size_t idim = 0; idim < dimensionConfig.size(); idim++) {
                    Json::JsonObject dimObject = dimensionConfig.get_array_item( idim );
                    item->push_backDimension( dimObject.as_string());
                }
            } else
                throw std::invalid_argument("The 'dimension' attribute of keyword "+getName()+" must be a string or a list of strings");
        }
    }



    void ParserKeyword::initData(const Json::JsonObject& jsonConfig) {
        m_fixedSize = 1U;
        m_keywordSizeType = FIXED;

        const Json::JsonObject dataConfig = jsonConfig.get_item("data");
        if (dataConfig.has_item("value_type")) {
            ParserValueTypeEnum valueType = ParserValueTypeEnumFromString(dataConfig.get_string("value_type"));
            const std::string itemName("data");
            bool hasDefault = dataConfig.has_item("default");
            std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();

            switch (valueType) {
                case INT:
                {
                    ParserIntItemPtr item = ParserIntItemPtr(new ParserIntItem(itemName, ALL));
                    if (hasDefault) {
                        int defaultValue = dataConfig.get_int("default");
                        item->setDefault(defaultValue);
                    }
                    record->addDataItem( item );
                }
                break;
                case STRING:
                {
                    ParserStringItemPtr item = ParserStringItemPtr(new ParserStringItem(itemName, ALL));
                    if (hasDefault) {
                        std::string defaultValue = dataConfig.get_string("default");
                        item->setDefault(defaultValue);
                    }
                    record->addItem( item );
                }
                break;
                case DOUBLE:
                {
                    ParserDoubleItemPtr item = ParserDoubleItemPtr(new ParserDoubleItem(itemName, ALL));
                    if (hasDefault) {
                        double defaultValue = dataConfig.get_double("default");
                        item->setDefault(defaultValue);
                    }
                    initDoubleItemDimension( item , dataConfig );
                    record->addDataItem( item );
                }
                break;
                default:
                    throw std::invalid_argument("While initializing keyword "+getName()+": Values of type "+dataConfig.get_string("value_type")+" are not implemented.");
            }
            addDataRecord( record );
        } else
            throw std::invalid_argument("The 'value_type' JSON item of keyword "+getName()+" is missing");
    }

    ParserRecordPtr ParserKeyword::getRecord(size_t recordIndex) const {
        return m_records.get( recordIndex );
    }


    std::vector<ParserRecordPtr>::const_iterator ParserKeyword::recordBegin() const {
        return m_records.begin();
    }

    std::vector<ParserRecordPtr>::const_iterator ParserKeyword::recordEnd() const {
        return m_records.end();
    }



    void ParserKeyword::addRecord(std::shared_ptr<ParserRecord> record) {
        m_records.push_back( record );
    }


    void ParserKeyword::addDataRecord(std::shared_ptr<ParserRecord> record) {
        if ((m_keywordSizeType == FIXED) && (m_fixedSize == 1U))
            addRecord(record);
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

    DeckKeyword ParserKeyword::parse(const ParseContext& parseContext , MessageContainer& msgContainer, RawKeywordPtr rawKeyword) const {
        if( !rawKeyword->isFinished() )
            throw std::invalid_argument("Tried to create a deck keyword from an incomplete raw keyword " + rawKeyword->getKeywordName());

        DeckKeyword keyword( rawKeyword->getKeywordName() );
        keyword.setLocation( rawKeyword->getFilename(), rawKeyword->getLineNR() );
        keyword.setDataKeyword( isDataKeyword() );

        size_t record_nr = 0;
        for( auto& rawRecord : *rawKeyword ) {
            if( m_records.size() == 0 && rawRecord.size() > 0 )
                throw std::invalid_argument("Missing item information " + rawKeyword->getKeywordName());

            keyword.addRecord( getRecord( record_nr )->parse( parseContext, msgContainer, rawRecord ) );
            record_nr++;
        }

        return keyword;
    }

    size_t ParserKeyword::getFixedSize() const {
        if (!hasFixedSize())
            throw std::logic_error("The parser keyword "+getName()+" does not have a fixed size!");
        return m_fixedSize;
    }

    bool ParserKeyword::hasFixedSize() const {
        return m_keywordSizeType == FIXED;
    }

    enum ParserKeywordSizeEnum ParserKeyword::getSizeType() const {
        return m_keywordSizeType;
    }

    const std::pair<std::string, std::string>& ParserKeyword::getSizeDefinitionPair() const {
        return m_sizeDefinitionPair;
    }


    bool ParserKeyword::isDataKeyword() const {
        if (m_records.size() > 0) {
            auto record = m_records.get(0);
            return record->isDataRecord();
        } else
            return false;
    }


    bool ParserKeyword::hasMatchRegex() const {
        return !m_matchRegexString.empty();
    }

    void ParserKeyword::setMatchRegex(const std::string& deckNameRegexp) {
        try {
#ifdef HAVE_REGEX
            m_matchRegex = std::regex(deckNameRegexp, std::regex::extended);
#else
            m_matchRegex = boost::regex(deckNameRegexp);
#endif
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

    bool ParserKeyword::matches(const string_view& name ) const {
        if (!validDeckName(name ))
            return false;

        else if( m_deckNames.count( name.string() ) )
            return true;

        else if (hasMatchRegex()) {
#ifdef HAVE_REGEX
            return std::regex_match( name.begin(), name.end(), m_matchRegex);
#else
            return boost::regex_match( name.begin(), name.end(), m_matchRegex);
#endif
        }

        return false;
    }

    bool ParserKeyword::equal(const ParserKeyword& other) const {
        // compare the deck names. we don't care about the ordering of the strings.
        if (m_deckNames != other.m_deckNames)
            return false;

        if(    m_name != other.m_name
            || m_matchRegexString != other.m_matchRegexString
            || m_keywordSizeType != other.m_keywordSizeType
            || isDataKeyword() != other.isDataKeyword()
            || m_isTableCollection != other.m_isTableCollection )
                return false;

        switch( m_keywordSizeType ) {
            case FIXED:
                if( m_fixedSize != other.m_fixedSize )
                    return false;
                break;

            case OTHER_KEYWORD_IN_DECK:
                if(  m_sizeDefinitionPair.first != other.m_sizeDefinitionPair.first
                  || m_sizeDefinitionPair.second != other.m_sizeDefinitionPair.second )
                    return false;
                break;

            default:
                break;
        }

        for( size_t i = 0; i < m_records.size(); i++ ) {
            if( !getRecord( i )->equal( *other.getRecord( i ) ) )
                    return false;
        }

        return true;
    }


    std::string ParserKeyword::createDeclaration(const std::string& indent) const {
        std::stringstream ss;
        ss << indent << "class " << className() << " : public ParserKeyword {" << std::endl;
        ss << indent << "public:" << std::endl;
        {
            std::string local_indent = indent + "    ";
            ss << local_indent << className() << "();" << std::endl;
            ss << local_indent << "static const std::string keywordName;" << std::endl;
            if (m_records.size() > 0 ) {
                for (auto iter = recordBegin(); iter != recordEnd(); ++iter) {
                    std::shared_ptr<ParserRecord> record = *iter;
                    for (size_t i = 0; i < record->size(); i++) {
                        ParserItemConstPtr item = record->get(i);
                        ss << std::endl;
                        item->inlineClass(ss , local_indent );
                    }
                }
            }
        }
        ss << indent << "};" << std::endl << std::endl << std::endl;
        return ss.str();
    }


    std::string ParserKeyword::createDecl() const {
        return className() + "::" + className() + "()";
    }


    std::string ParserKeyword::createCode() const {
        std::stringstream ss;
        const std::string lhs = "keyword";
        const std::string indent = "  ";

        ss << className() << "::" << className() << "( ) : ParserKeyword(\"" << m_name << "\") {" << std::endl;
        {
            const std::string sizeString(ParserKeywordSizeEnum2String(m_keywordSizeType));
            ss << indent;
            switch (m_keywordSizeType) {
                case SLASH_TERMINATED:
                    ss << "setSizeType(" << sizeString << ");" << std::endl;
                    break;
                case UNKNOWN:
                    ss << "setSizeType(" << sizeString << ");" << std::endl;
                    break;
                case FIXED:
                    ss << "setFixedSize( (size_t) " << m_fixedSize << ");" << std::endl;
                    break;
                case OTHER_KEYWORD_IN_DECK:
                    ss << "setSizeType(" << sizeString << ");" << std::endl;
                    ss << indent << "initSizeKeyword(\"" << m_sizeDefinitionPair.first << "\",\"" << m_sizeDefinitionPair.second << "\");" << std::endl;
                    if (m_isTableCollection)
                        ss << "setTableCollection( true );" << std::endl;
                    break;
            }
        }
        ss << indent << "setDescription(\"" << getDescription() << "\");" << std::endl;

        // add the valid sections for the keyword
        ss << indent << "clearValidSectionNames();\n";
        for (auto sectionNameIt = m_validSectionNames.begin();
             sectionNameIt != m_validSectionNames.end();
             ++sectionNameIt)
        {
            ss << indent << "addValidSectionName(\"" << *sectionNameIt << "\");" << std::endl;
        }

        // add the deck names
        ss << indent << "clearDeckNames();\n";
        for (auto deckNameIt = m_deckNames.begin();
             deckNameIt != m_deckNames.end();
             ++deckNameIt)
        {
            ss << indent << "addDeckName(\"" << *deckNameIt << "\");" << std::endl;
        }

        // set the deck name match regex
        if (hasMatchRegex())
            ss << indent << "setMatchRegex(\"" << m_matchRegexString << "\");" << std::endl;

        {
            if (m_records.size() > 0 ) {
                for (auto iter = recordBegin(); iter != recordEnd(); ++iter) {
                    std::shared_ptr<ParserRecord> record = *iter;
                    const std::string local_indent = indent + "   ";
                    ss << indent << "{" << std::endl;
                    ss << local_indent << "std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();" << std::endl;
                    for (size_t i = 0; i < record->size(); i++) {
                        ParserItemConstPtr item = record->get(i);
                        ss << local_indent << "{" << std::endl;
                        {
                            std::string indent3 = local_indent + "   ";
                            ss << indent3 << "ParserItemPtr item(" << item->createCode() << ");" << std::endl;
                            ss << indent3 << "item->setDescription(\"" << item->getDescription() << "\");" << std::endl;
                            for (size_t idim=0; idim < item->numDimensions(); idim++)
                                ss << indent3 <<"item->push_backDimension(\"" << item->getDimension( idim ) << "\");" << std::endl;
                            {
                                std::string addItemMethod = "addItem";
                                if (isDataKeyword())
                                    addItemMethod = "addDataItem";

                                ss << indent3 << "record->" << addItemMethod << "(item);" << std::endl;
                            }
                        }
                        ss << local_indent << "}" << std::endl;
                    }

                    if (record->isDataRecord())
                        ss << local_indent << "addDataRecord( record );" << std::endl;
                    else
                        ss << local_indent << "addRecord( record );" << std::endl;

                    ss << indent << "}" << std::endl;
                }
            }
        }
        ss << "}" << std::endl;

        ss << "const std::string " << className() << "::keywordName = \"" << getName() << "\";" << std::endl;
        for (auto iter = recordBegin(); iter != recordEnd(); ++iter) {
            std::shared_ptr<ParserRecord> record = *iter;
            for (size_t i = 0; i < record->size(); i++) {
                ParserItemConstPtr item = record->get(i);
                ss << item->inlineClassInit(className());
            }
        }
        ss << std::endl;
        return ss.str();
    }


    void ParserKeyword::applyUnitsToDeck( Deck& deck, DeckKeyword& deckKeyword) const {
        for (size_t index = 0; index < deckKeyword.size(); index++) {
            std::shared_ptr<const ParserRecord> parserRecord = getRecord(index);
            auto& deckRecord = deckKeyword.getRecord(index);
            parserRecord->applyUnitsToDeck( deck , deckRecord );
        }
    }
}
