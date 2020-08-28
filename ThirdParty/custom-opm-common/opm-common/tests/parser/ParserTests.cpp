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

#define BOOST_TEST_MODULE ParserTests
#include <boost/test/unit_test.hpp>

#include <opm/json/JsonObject.hpp>
#include <iostream>

#include <opm/parser/eclipse/Utility/Typetools.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>

#include "src/opm/parser/eclipse/Parser/raw/RawKeyword.hpp"
#include "src/opm/parser/eclipse/Parser/raw/RawRecord.hpp"

#include <iostream>

using namespace Opm;

namespace {

constexpr ParserItem::itype INT = ParserItem::itype::INT;
constexpr ParserItem::itype STRING= ParserItem::itype::STRING;
constexpr ParserItem::itype DOUBLE = ParserItem::itype::DOUBLE;


std::string prefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

ParserKeyword createDynamicSized(const std::string& kw) {
    ParserKeyword pkw(kw);
    pkw.setSizeType(SLASH_TERMINATED);
    return pkw;
}

ParserKeyword createFixedSized(const std::string& kw , size_t size) {
    ParserKeyword pkw(kw);
    pkw.setFixedSize( size );
    return pkw;
}

ParserKeyword createTable(const std::string& name,
                          const std::string& sizeKeyword,
                          const std::string& sizeItem,
                          bool isTableCollection) {
    ParserKeyword pkw(name);
    pkw.initSizeKeyword(sizeKeyword , sizeItem, 0);
    pkw.setTableCollection(isTableCollection);
    return pkw;
}

}

/************************Basic structural tests**********************'*/

BOOST_AUTO_TEST_CASE(Initializing) {
    BOOST_CHECK_NO_THROW(Parser parser);
}

BOOST_AUTO_TEST_CASE(addKeyword_keyword_doesntfail) {
    Parser parser;
    parser.addParserKeyword( createDynamicSized( "EQUIL" ) );
}


BOOST_AUTO_TEST_CASE(canParseDeckKeyword_returnstrue) {
    Parser parser;
    parser.addParserKeyword(createDynamicSized("FJAS"));
    BOOST_CHECK(parser.isRecognizedKeyword("FJAS"));
}


BOOST_AUTO_TEST_CASE(getKeyword_haskeyword_returnskeyword) {
    Parser parser;
    parser.addParserKeyword( createDynamicSized( "FJAS" ) );
    BOOST_CHECK_EQUAL("FJAS", parser.getParserKeywordFromDeckName("FJAS").getName());
}

BOOST_AUTO_TEST_CASE(getKeyword_hasnotkeyword_getKeywordThrowsException) {
    Parser parser;
    parser.addParserKeyword( createDynamicSized( "FJAS" ) );
    BOOST_CHECK_THROW(parser.getParserKeywordFromDeckName("FJASS"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(getAllDeckNames_hasTwoKeywords_returnsCompleteList) {
    Parser parser( false );
    std::cout << parser.getAllDeckNames().size() << std::endl;
    parser.addParserKeyword( createDynamicSized( "FJAS" ) );
    parser.addParserKeyword( createDynamicSized( "SAJF" ) );
    BOOST_CHECK_EQUAL(2U, parser.getAllDeckNames().size());
}

BOOST_AUTO_TEST_CASE(getAllDeckNames_hasNoKeywords_returnsEmptyList) {
    Parser parser( false );
    BOOST_CHECK_EQUAL(0U, parser.getAllDeckNames().size());
}



/************************ JSON config related tests **********************'*/


BOOST_AUTO_TEST_CASE(addParserKeywordJSON_isRecognizedKeyword_returnstrue) {
    Parser parser;
    Json::JsonObject jsonConfig("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 ,  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");
    parser.addParserKeyword( jsonConfig );
    BOOST_CHECK(parser.isRecognizedKeyword("BPR"));
}


BOOST_AUTO_TEST_CASE(addParserKeywordJSON_size_isObject_allGood) {
    Parser parser;
    Json::JsonObject jsonConfig("{\"name\": \"EQUIXL\", \"sections\":[], \"size\" : {\"keyword\":\"EQLDIMS\" , \"item\" : \"NTEQUL\"},  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");
    parser.addParserKeyword( jsonConfig );
    BOOST_CHECK(parser.isRecognizedKeyword("EQUIXL"));
}



BOOST_AUTO_TEST_CASE(loadKeywordsJSON_notArray_throw) {
    Parser parser;
    Json::JsonObject jsonConfig( "{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"]}");

    BOOST_CHECK_THROW(parser.loadKeywords( jsonConfig ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(loadKeywordsJSON_noSectionsItem_throw) {
    Parser parser;
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}]");

    BOOST_CHECK_THROW(parser.loadKeywords( jsonConfig ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(loadKeywordsJSON_isRecognizedKeyword_returnstrue) {
    Parser parser;
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"], \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}]");

    parser.loadKeywords( jsonConfig );
    BOOST_CHECK(parser.isRecognizedKeyword("BPR"));
}


BOOST_AUTO_TEST_CASE(empty_sizeReturns0) {
    Parser parser( false );
    BOOST_CHECK_EQUAL( 0U , parser.size());
}



BOOST_AUTO_TEST_CASE(loadKeywordsJSON_manyKeywords_returnstrue) {
    Parser parser( false );
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"] ,  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}, {\"name\" : \"WWCT\", \"sections\":[\"SUMMARY\"], \"size\" : 0} , {\"name\" : \"EQUIL\", \"sections\":[\"PROPS\"], \"size\" : 0}]");

    parser.loadKeywords( jsonConfig );
    BOOST_CHECK(parser.isRecognizedKeyword("BPR"));
    BOOST_CHECK(parser.isRecognizedKeyword("WWCT"));
    BOOST_CHECK(parser.isRecognizedKeyword("EQUIL"));
    BOOST_CHECK_EQUAL( 3U , parser.size() );
}




/*****************************************************************/


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_fileDoesNotExist_returnsFalse) {
    Parser parser;
    Opm::filesystem::path configFile("File/does/not/exist");
    BOOST_CHECK_EQUAL( false , parser.loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_invalidJson_returnsFalse) {
    Parser parser;
    Opm::filesystem::path configFile(prefix() + "json/example_invalid_json");
    BOOST_CHECK_EQUAL( false , parser.loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_invalidConfig_returnsFalse) {
    Parser parser;
    Opm::filesystem::path configFile(prefix() + "json/example_missing_name.json");
    BOOST_CHECK_EQUAL( false , parser.loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_validKeyword_returnsTrueHasKeyword) {
    Parser parser( false );
    Opm::filesystem::path configFile(prefix() + "json/BPR");
    BOOST_CHECK_EQUAL( true , parser.loadKeywordFromFile( configFile ));
    BOOST_CHECK_EQUAL( 1U , parser.size() );
    BOOST_CHECK_EQUAL( true , parser.isRecognizedKeyword("BPR") );
}



BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_directoryDoesNotexist_throws) {
        Parser parser;
        Opm::filesystem::path configPath("path/does/not/exist");
        BOOST_CHECK_THROW(parser.loadKeywordsFromDirectory( configPath), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_notRecursive_allNames) {
        Parser parser( false );
        BOOST_CHECK_EQUAL(false , parser.isRecognizedKeyword("BPR"));
        Opm::filesystem::path configPath(prefix() + "config/directory1");
        BOOST_CHECK_NO_THROW(parser.loadKeywordsFromDirectory( configPath, false));
        BOOST_CHECK(parser.isRecognizedKeyword("WWCT"));
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(false , parser.isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_notRecursive_strictNames) {
        Parser parser( false );
        Opm::filesystem::path configPath(prefix() + "config/directory1");
        BOOST_CHECK_NO_THROW(parser.loadKeywordsFromDirectory( configPath, false));
        BOOST_CHECK(parser.isRecognizedKeyword("WWCT"));
        // the file name for the following keyword is "Bpr", but that
        // does not matter
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(false , parser.isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_Recursive_allNames) {
        Parser parser( false );
        BOOST_CHECK_EQUAL(false , parser.isRecognizedKeyword("BPR"));
        Opm::filesystem::path configPath(prefix() + "config/directory1");
        BOOST_CHECK_NO_THROW(parser.loadKeywordsFromDirectory( configPath, true));
        BOOST_CHECK(parser.isRecognizedKeyword("WWCT"));
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_default) {
        Parser parser( false );
        BOOST_CHECK_EQUAL(false , parser.isRecognizedKeyword("BPR"));
        Opm::filesystem::path configPath(prefix() + "config/directory1");
        BOOST_CHECK_NO_THROW(parser.loadKeywordsFromDirectory( configPath ));
        BOOST_CHECK(parser.isRecognizedKeyword("WWCT"));
        // the file name for the following keyword is "Bpr", but that
        // does not matter
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(true , parser.isRecognizedKeyword("DIMENS"));
}

BOOST_AUTO_TEST_CASE(ReplaceKeyword) {
    Parser parser;
    BOOST_CHECK( parser.loadKeywordFromFile( prefix() + "parser/EQLDIMS2" ) );

    const auto& eqldims = parser.getParserKeywordFromDeckName("EQLDIMS");
    const auto& record = eqldims.getRecord(0);
    BOOST_CHECK(record.hasItem("NEW"));
}


BOOST_AUTO_TEST_CASE(WildCardTest) {
    Parser parser;
    BOOST_CHECK(!parser.isRecognizedKeyword("TVDP*"));
    BOOST_CHECK(!parser.isRecognizedKeyword("TVDP"));
    BOOST_CHECK(parser.isRecognizedKeyword("TVDPXXX"));
    BOOST_CHECK(!parser.isRecognizedKeyword("TVD"));

    BOOST_CHECK(!parser.isRecognizedKeyword("TVDP"));

    const auto& keyword1 = parser.getParserKeywordFromDeckName("TVDPA");
    const auto& keyword2 = parser.getParserKeywordFromDeckName("TVDPBC");
    const auto& keyword3 = parser.getParserKeywordFromDeckName("TVDPXXX");

    BOOST_CHECK_EQUAL( keyword1 , keyword2 );
    BOOST_CHECK_EQUAL( keyword1 , keyword3 );
}


BOOST_AUTO_TEST_CASE( quoted_comments ) {
    BOOST_CHECK_EQUAL( Parser::stripComments( "ABC" ) , "ABC");
    BOOST_CHECK_EQUAL( Parser::stripComments( "--ABC") , "");
    BOOST_CHECK_EQUAL( Parser::stripComments( "ABC--DEF") , "ABC");
    BOOST_CHECK_EQUAL( Parser::stripComments( "'ABC'--DEF") , "'ABC'");
    BOOST_CHECK_EQUAL( Parser::stripComments( "\"ABC\"--DEF") , "\"ABC\"");
    BOOST_CHECK_EQUAL( Parser::stripComments( "ABC--'DEF'") , "ABC");
    BOOST_CHECK_EQUAL( Parser::stripComments("ABC'--'DEF") , "ABC'--'DEF");
    BOOST_CHECK_EQUAL( Parser::stripComments("ABC'--'DEF\"--\"GHI") , "ABC'--'DEF\"--\"GHI");
    BOOST_CHECK_EQUAL( Parser::stripComments("ABC'--'DEF'GHI") , "ABC'--'DEF'GHI");
    BOOST_CHECK_EQUAL( Parser::stripComments("ABC'--'DEF'--GHI") , "ABC'--'DEF'--GHI");
}

BOOST_AUTO_TEST_CASE( PATHS_has_global_scope ) {
    Parser parser;
    ParseContext parseContext;
    ErrorGuard errors;

    parseContext.update( ParseContext::PARSE_MISSING_INCLUDE , Opm::InputError::THROW_EXCEPTION);
    const auto deck = parser.parseFile( prefix() + "parser/PATHSInInclude.data", parseContext, errors );
    BOOST_CHECK(deck.hasKeyword("OIL"));
    BOOST_CHECK_THROW( parser.parseFile( prefix() + "parser/PATHSInIncludeInvalid.data", parseContext, errors ), std::invalid_argument );
}

BOOST_AUTO_TEST_CASE( PATHS_with_backslashes ) {
    Parser parser;
    ParseContext parseContext;
    ErrorGuard errors;

    parseContext.update( ParseContext::PARSE_MISSING_INCLUDE , Opm::InputError::THROW_EXCEPTION);
    const auto deck = parser.parseFile( prefix() + "parser/PATHSWithBackslashes.data", parseContext, errors );
    BOOST_CHECK(deck.hasKeyword("OIL"));
}

BOOST_AUTO_TEST_CASE( handle_empty_title ) {
    const auto* input_deck = "RUNSPEC\n\n"
                             "TITLE\n\n"
                             "DIMENS\n10 10 10/\n"
                             "EQLDIMS\n/\n";

    Parser parser;
    const auto deck = parser.parseString( input_deck);
    BOOST_CHECK_EQUAL( "opm/flow", deck.getKeyword( "TITLE" ).getStringData().front() );
    BOOST_CHECK_EQUAL( "simulation", deck.getKeyword( "TITLE" ).getStringData().back() );
 }

BOOST_AUTO_TEST_CASE( deck_comma_separated_fields ) {
    const char* deck = R"(
TABDIMS
    2*    24 2*    20    20 1*     1 7* /

SWOF
    0.1000,  0.0000e+00,  8.0000e-01  0
    0.2000,  0,           8.0000e-01  0
    0.2500,  2.7310e-04,  5.8082e-01  0
    0.3000,  2.1848e-03,  4.1010e-01  0
    0.3500,  7.3737e-03,  2.8010e-01  0
    0.4000,  1.7478e-02,  1.8378e-01  0
    0.4500,  3.4138e-02,  1.1473e-01  0
    0.5000,  5.8990e-02,  6.7253e-02  0
    0.5500,  9.3673e-02,  3.6301e-02  0
    0.6000,  1.3983e-01,  1.7506e-02  0
    0.6500,  1.9909e-01,  7.1706e-03  0
    0.7000,  2.7310e-01,  2.2688e-03  0
    0.7500,  3.6350e-01,  4.4820e-04  0
    0.8000,  4.7192e-01,  2.8000e-05  0
    0.8500,  6.0000e-01,  0.0000e+00  0
    0.9000,  7.4939e-01,  0.0000e+00  0
/
)";

    BOOST_CHECK_NO_THROW( Parser().parseString( deck ) );
 }


BOOST_AUTO_TEST_CASE(ParseTNUM) {
    const char * deck1 =
        "REGIONS\n"
        "TNUMFSGS\n"
        " 100*1/\n"
        "\n"
        "TNUMFXXX\n"
        " 100*1/\n"
        "\n";

    Opm::Parser parser;
    auto deck = parser.parseString( deck1 );
    BOOST_CHECK( deck.hasKeyword("TNUMFSGS"));
    BOOST_CHECK( deck.hasKeyword("TNUMFXXX"));
}


BOOST_AUTO_TEST_CASE(ScalarCheck) {
    ParserItem item1("ITEM1", INT);
    ParserItem item2("ITEM1", INT); item2.setSizeType( ParserItem::item_size::ALL );


    BOOST_CHECK(  item1.scalar());
    BOOST_CHECK( !item2.scalar());
}

BOOST_AUTO_TEST_CASE(Initialize_DefaultSizeType) {
    ParserItem item1(std::string("ITEM1"), INT);
    BOOST_CHECK_EQUAL( ParserItem::item_size::SINGLE, item1.sizeType());
}



BOOST_AUTO_TEST_CASE(Initialize_Default) {
    ParserItem item1(std::string("ITEM1"), INT);
    ParserItem item2(std::string("ITEM1"), INT); item2.setDefault(88);

    BOOST_CHECK(!item1.hasDefault());
    BOOST_CHECK_THROW(item1.getDefault< int >(), std::invalid_argument);
    BOOST_CHECK(item2.hasDefault());
    BOOST_CHECK_EQUAL(item2.getDefault< int >(), 88);
}


BOOST_AUTO_TEST_CASE(Initialize_Default_Double) {
    ParserItem item1(std::string("ITEM1"), DOUBLE);
    ParserItem item2("ITEM1", DOUBLE); item2.setDefault(88.91);

    BOOST_CHECK(!item1.hasDefault());
    BOOST_CHECK_THROW( item1.getDefault< double >(), std::invalid_argument );
    BOOST_CHECK_EQUAL( 88.91 , item2.getDefault< double >());
}

BOOST_AUTO_TEST_CASE(Initialize_Default_String) {
    ParserItem item1(std::string("ITEM1"), STRING);
    BOOST_CHECK(!item1.hasDefault());
    BOOST_CHECK_THROW(item1.getDefault< std::string >(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(scan_PreMatureTerminator_defaultUsed) {
    ParserItem itemInt("ITEM2", INT);
    itemInt.setDefault(123);

    RawRecord rawRecord1( "" );
    UnitSystem unit_system;
    const auto defaulted = itemInt.scan(rawRecord1, unit_system, unit_system);

    BOOST_CHECK(defaulted.defaultApplied(0));
    BOOST_CHECK_EQUAL(defaulted.get< int >(0), 123);
}

BOOST_AUTO_TEST_CASE(InitializeIntItem_setDescription_canReadBack) {
    ParserItem itemInt("ITEM1", INT);
    std::string description("This is the description");
    itemInt.setDescription(description);

    BOOST_CHECK_EQUAL( description, itemInt.getDescription() );
}


/******************************************************************/
/* <Json> */
BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject_missingName_throws) {
    Json::JsonObject jsonConfig("{\"nameX\": \"ITEM1\" , \"size_type\" : \"ALL\"}");
    BOOST_CHECK_THROW( ParserItem item1( jsonConfig ) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject_defaultSizeType) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\", \"value_type\": \"INT\" }");
    ParserItem item1( jsonConfig );
    BOOST_CHECK_EQUAL( ParserItem::item_size::SINGLE , item1.sizeType());
}



BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"size_type\" : \"ALL\", \"value_type\": \"INT\" }");
    ParserItem item1( jsonConfig );
    BOOST_CHECK_EQUAL( "ITEM1" , item1.name() );
    BOOST_CHECK_EQUAL( ParserItem::item_size::ALL, item1.sizeType() );
    BOOST_CHECK_THROW(item1.getDefault< int >(), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject_withDefault) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"size_type\" : \"SINGLE\", \"default\" : 100, \"value_type\": \"INT\" }");
    ParserItem item1( jsonConfig );
    BOOST_CHECK_EQUAL( 100 , item1.getDefault< int >() );
}


BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject_withDefaultInvalid_throws) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"size_type\" : \"SINGLE\", \"default\" : \"100X\"}");
    BOOST_CHECK_THROW( ParserItem item1( jsonConfig ) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(InitializeIntItem_FromJsonObject_withSizeTypeALL_throws) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"value_type\": \"INT\", \"size_type\" : \"ALL\", \"default\" : 100}");
    BOOST_CHECK_THROW( ParserItem item1( jsonConfig ) , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(InitializeIntItem_WithDescription_DescriptionPropertyShouldBePopulated) {
    std::string description("Description goes here");
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"value_type\": \"INT\", \"description\" : \"Description goes here\"}");
    ParserItem item(jsonConfig);

    BOOST_CHECK_EQUAL( "Description goes here", item.getDescription() );
}


BOOST_AUTO_TEST_CASE(InitializeIntItem_WithoutDescription_DescriptionPropertyShouldBeEmpty) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\", \"value_type\": \"INT\" }");
    ParserItem item(jsonConfig);

    BOOST_CHECK_EQUAL( "", item.getDescription() );
}



/* </Json> */
/******************************************************************/

/* EQUAL */


BOOST_AUTO_TEST_CASE(IntItem_Equal_ReturnsTrue) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", INT); item1.setSizeType(sizeType);
    ParserItem item2("ITEM1", INT); item2.setSizeType(sizeType);
    ParserItem item3 = item1;

    BOOST_CHECK_EQUAL( item1, item2 );
    BOOST_CHECK_EQUAL( item1, item3 );
}


BOOST_AUTO_TEST_CASE(IntItem_Different_ReturnsFalse) {
    ParserItem item1("ITEM1", INT); item1.setSizeType(ParserItem::item_size::ALL);
    ParserItem item2("ITEM2", INT); item2.setSizeType(ParserItem::item_size::ALL);
    ParserItem item3("ITEM1", INT);
    ParserItem item4("ITEM1", INT); item4.setDefault(52);

    BOOST_CHECK_NE( item1, item2 );
    BOOST_CHECK_NE( item1, item3 );
    BOOST_CHECK_NE( item2, item3 );
    BOOST_CHECK_NE( item4, item3 );
}

BOOST_AUTO_TEST_CASE(DoubleItem_DimEqual_ReturnsTrue) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", DOUBLE); item1.setSizeType(sizeType);
    ParserItem item2("ITEM1", DOUBLE); item2.setSizeType(sizeType);

    item1.push_backDimension("Length*Length");
    item2.push_backDimension("Length*Length");

    BOOST_CHECK_EQUAL( item1, item2 );
}


BOOST_AUTO_TEST_CASE(DoubleItem_DimDifferent_ReturnsFalse) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", DOUBLE); item1.setSizeType(sizeType);    // Dim: []
    ParserItem item2("ITEM1", DOUBLE); item2.setSizeType(sizeType);    // Dim: [Length]
    ParserItem item3("ITEM1", DOUBLE); item3.setSizeType(sizeType);    // Dim: [Length ,Length]
    ParserItem item4("ITEM1", DOUBLE); item4.setSizeType(sizeType);    // Dim: [t]

    item2.push_backDimension("Length");
    item3.push_backDimension("Length");
    item3.push_backDimension("Length");
    item4.push_backDimension("Time");

    BOOST_CHECK_NE(item1, item2 );
    BOOST_CHECK_NE(item2, item3 );
    BOOST_CHECK_NE(item2, item1 );
    BOOST_CHECK_NE(item2, item4 );
    BOOST_CHECK_NE(item1, item3 );
    BOOST_CHECK_NE(item3, item1 );
    BOOST_CHECK_NE(item4, item2 );
}


BOOST_AUTO_TEST_CASE(DoubleItem_Different_ReturnsFalse) {
    ParserItem item1("ITEM1", DOUBLE); item1.setDefault(0.0); item1.setSizeType(ParserItem::item_size::ALL);
    ParserItem item2("ITEM2", DOUBLE); item2.setDefault(0.0); item2.setSizeType(ParserItem::item_size::ALL);
    ParserItem item3("ITEM1", DOUBLE); item3.setDefault(0.0);
    ParserItem item4("ITEM1", DOUBLE); item4.setDefault(42.89);

    BOOST_CHECK_NE( item1, item2 );
    BOOST_CHECK_NE( item1, item3 );
    BOOST_CHECK_NE( item2, item3 );
    BOOST_CHECK_NE( item4, item3 );
}


BOOST_AUTO_TEST_CASE(StringItem_Equal_ReturnsTrue) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", STRING); item1.setSizeType(sizeType);
    ParserItem item2("ITEM1", STRING); item2.setSizeType(sizeType);
    ParserItem item3 = item1;

    BOOST_CHECK_EQUAL( item1, item2 );
    BOOST_CHECK_EQUAL( item1, item3 );
}


BOOST_AUTO_TEST_CASE(StringItem_Different_ReturnsFalse) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", STRING); item1.setDefault( std::string("")); item1.setSizeType(sizeType);
    ParserItem item2("ITEM2", STRING); item2.setDefault( std::string("")); item2.setSizeType(sizeType);
    ParserItem item3("ITEM1", STRING); item3.setDefault( std::string(""));
    ParserItem item4("ITEM1", STRING); item4.setDefault( std::string("42.89"));

    BOOST_CHECK_NE( item1, item2 );
    BOOST_CHECK_NE( item1, item3 );
    BOOST_CHECK_NE( item2, item3 );
    BOOST_CHECK_NE( item4, item3 );
}




/******************************************************************/

BOOST_AUTO_TEST_CASE(Name_ReturnsCorrectName) {
    auto sizeType = ParserItem::item_size::ALL;

    ParserItem item1("ITEM1", INT); item1.setSizeType(sizeType);
    BOOST_CHECK_EQUAL("ITEM1", item1.name());

    ParserItem item2("", INT); item2.setSizeType(sizeType);
    BOOST_CHECK_EQUAL("", item2.name());
}

BOOST_AUTO_TEST_CASE(Size_ReturnsCorrectSizeTypeSingle) {
    auto sizeType = ParserItem::item_size::SINGLE;
    ParserItem item1("ITEM1", INT);
    BOOST_CHECK_EQUAL(sizeType, item1.sizeType());
}

BOOST_AUTO_TEST_CASE(Size_ReturnsCorrectSizeTypeAll) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem item1("ITEM1", INT); item1.setSizeType(sizeType);
    BOOST_CHECK_EQUAL(sizeType, item1.sizeType());
}

BOOST_AUTO_TEST_CASE(Scan_All_CorrectIntSetInDeckItem) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem itemInt("ITEM", INT); itemInt.setSizeType(sizeType);

    RawRecord rawRecord( "100 443 10*77 10*1 25" );
    UnitSystem unit_system;
    const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(23U, deckIntItem.data_size());
    BOOST_CHECK_EQUAL(77,  deckIntItem.get< int >(3));
    BOOST_CHECK_EQUAL(1,   deckIntItem.get< int >(21));
    BOOST_CHECK_EQUAL(25,  deckIntItem.get< int >(22));
}

BOOST_AUTO_TEST_CASE(Scan_All_WithDefaults) {
    auto sizeType = ParserItem::item_size::ALL;
    ParserItem itemInt("ITEM", INT); itemInt.setSizeType(sizeType);
    itemInt.setInputType( ParserItem::itype::INT );

    RawRecord rawRecord( "100 10* 10*1 25" );
    UnitSystem unit_system;
    const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(22U, deckIntItem.data_size());
    BOOST_CHECK(!deckIntItem.defaultApplied(0));
    BOOST_CHECK( deckIntItem.defaultApplied(1));
    BOOST_CHECK(!deckIntItem.defaultApplied(11));
    BOOST_CHECK(!deckIntItem.defaultApplied(21));
    BOOST_CHECK_EQUAL(1,  deckIntItem.get< int >(20));
    BOOST_CHECK_EQUAL(25, deckIntItem.get< int >(21));
}

BOOST_AUTO_TEST_CASE(Scan_SINGLE_CorrectIntSetInDeckItem) {
    ParserItem itemInt(std::string("ITEM2"), INT);

    RawRecord rawRecord("100 44.3 'Heisann'" );
    UnitSystem unit_system;
    const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(100, deckIntItem.get< int >(0));
}

BOOST_AUTO_TEST_CASE(Scan_SeveralInts_CorrectIntsSetInDeckItem) {
    ParserItem itemInt1(std::string("ITEM1"), INT);
    ParserItem itemInt2(std::string("ITEM2"), INT);
    ParserItem itemInt3(std::string("ITEM3"), INT);

    RawRecord rawRecord( "100 443 338932 222.33 'Heisann' " );
    UnitSystem unit_system;
    const auto deckIntItem1 = itemInt1.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(100, deckIntItem1.get< int >(0));

    const auto deckIntItem2 = itemInt2.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(443, deckIntItem2.get< int >(0));

    const auto deckIntItem3 = itemInt3.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(338932, deckIntItem3.get< int >(0));
}





BOOST_AUTO_TEST_CASE(Scan_Multiplier_CorrectIntsSetInDeckItem) {
    ParserItem itemInt("ITEM2", INT);

    RawRecord rawRecord( "3*4 " );
    UnitSystem unit_system;
    itemInt.setSizeType(ParserItem::item_size::ALL);
    const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(4, deckIntItem.get< int >(0));
    BOOST_CHECK_EQUAL(4, deckIntItem.get< int >(1));
    BOOST_CHECK_EQUAL(4, deckIntItem.get< int >(2));
}

BOOST_AUTO_TEST_CASE(Scan_StarNoMultiplier_ExceptionThrown) {
    ParserItem itemInt("ITEM2", INT);

    UnitSystem unit_system;
    RawRecord rawRecord( "*45 " );
    BOOST_CHECK_THROW(itemInt.scan(rawRecord, unit_system, unit_system), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Scan_MultipleItems_CorrectIntsSetInDeckItem) {
    ParserItem itemInt1(std::string("ITEM1"), INT);
    ParserItem itemInt2(std::string("ITEM2"), INT);

    RawRecord rawRecord( "10 20" );
    UnitSystem unit_system;
    const auto deckIntItem1 = itemInt1.scan(rawRecord, unit_system, unit_system);
    const auto deckIntItem2 = itemInt2.scan(rawRecord, unit_system, unit_system);

    BOOST_CHECK_EQUAL(10, deckIntItem1.get< int >(0));
    BOOST_CHECK_EQUAL(20, deckIntItem2.get< int >(0));
}

BOOST_AUTO_TEST_CASE(Scan_MultipleDefault_CorrectIntsSetInDeckItem) {
    ParserItem itemInt1("ITEM1", INT); itemInt1.setDefault(10);
    ParserItem itemInt2("ITEM2", INT); itemInt2.setDefault(20);

    RawRecord rawRecord( "* * " );
    UnitSystem unit_system;
    const auto deckIntItem1 = itemInt1.scan(rawRecord, unit_system, unit_system);
    const auto deckIntItem2 = itemInt2.scan(rawRecord, unit_system, unit_system);

    BOOST_CHECK_EQUAL(10, deckIntItem1.get< int >(0));
    BOOST_CHECK_EQUAL(20, deckIntItem2.get< int >(0));
}

BOOST_AUTO_TEST_CASE(Scan_MultipleWithMultiplier_CorrectIntsSetInDeckItem) {
    ParserItem itemInt1("ITEM1", INT);
    ParserItem itemInt2("ITEM2", INT);

    RawRecord rawRecord( "2*30" );
    UnitSystem unit_system;
    const auto deckIntItem1 = itemInt1.scan(rawRecord, unit_system, unit_system);
    const auto deckIntItem2 = itemInt2.scan(rawRecord, unit_system, unit_system);

    BOOST_CHECK_EQUAL(30, deckIntItem1.get< int >(0));
    BOOST_CHECK_EQUAL(30, deckIntItem2.get< int >(0));
}

BOOST_AUTO_TEST_CASE(Scan_MalformedMultiplier_Throw) {
    ParserItem itemInt1("ITEM1", INT);

    UnitSystem unit_system;
    RawRecord rawRecord( "2.10*30" );
    BOOST_CHECK_THROW(itemInt1.scan(rawRecord, unit_system, unit_system), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Scan_MalformedMultiplierChar_Throw) {
    ParserItem itemInt1("ITEM1", INT);

    RawRecord rawRecord( "210X30" );
    UnitSystem unit_system;
    BOOST_CHECK_THROW(itemInt1.scan(rawRecord, unit_system, unit_system), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Scan_MultipleWithMultiplierDefault_CorrectIntsSetInDeckItem) {
    ParserItem itemInt1("ITEM1", INT); itemInt1.setDefault(10);
    ParserItem itemInt2("ITEM2", INT); itemInt2.setDefault(20);

    RawRecord rawRecord( "2*" );
    UnitSystem unit_system;
    const auto deckIntItem1 = itemInt1.scan(rawRecord, unit_system, unit_system);
    const auto deckIntItem2 = itemInt2.scan(rawRecord, unit_system, unit_system);

    BOOST_CHECK_EQUAL(10, deckIntItem1.get< int >(0));
    BOOST_CHECK_EQUAL(20, deckIntItem2.get< int >(0));
}

BOOST_AUTO_TEST_CASE(Scan_RawRecordErrorInRawData_ExceptionThrown) {
    ParserItem itemInt(std::string("ITEM2"), INT);

    // Wrong type
    RawRecord rawRecord2( "333.2 /" );
    UnitSystem unit_system;
    BOOST_CHECK_THROW(itemInt.scan(rawRecord2, unit_system, unit_system), std::invalid_argument);

    // Wrong type
    RawRecord rawRecord3( "100X /" );
    BOOST_CHECK_THROW(itemInt.scan(rawRecord3, unit_system, unit_system), std::invalid_argument);

    // Wrong type
    RawRecord rawRecord5( "astring /" );
    BOOST_CHECK_THROW(itemInt.scan(rawRecord5, unit_system, unit_system), std::invalid_argument);
}

/*********************String************************'*/
/*****************************************************************/
/*</json>*/

BOOST_AUTO_TEST_CASE(InitializeStringItem_FromJsonObject_missingName_throws) {
    Json::JsonObject jsonConfig("{\"nameX\": \"ITEM1\" , \"size_type\" : \"ALL\"}");
    BOOST_CHECK_THROW( ParserItem item1( jsonConfig ) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(InitializeStringItem_FromJsonObject_withDefault) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"value_type\": \"STRING\", \"size_type\" : \"SINGLE\", \"default\" : \"100\"}");
    ParserItem item1( jsonConfig );
    BOOST_CHECK_EQUAL( "100" , item1.getDefault< std::string >() );
}

BOOST_AUTO_TEST_CASE(InitializeStringItem_FromJsonObject_withDefaultInvalid_throws) {
    Json::JsonObject jsonConfig("{\"name\": \"ITEM1\" , \"size_type\" : \"ALL\", \"default\" : [1,2,3]}");
    BOOST_CHECK_THROW( ParserItem item1( jsonConfig ) , std::invalid_argument );
}
/*</json>*/
/*****************************************************************/

BOOST_AUTO_TEST_CASE(init_defaultvalue_defaultset) {
    ParserItem itemString(std::string("ITEM1"), STRING);
    RawRecord rawRecord0( "'1*'" );
    UnitSystem unit_system;
    itemString.setDefault(std::string("DEFAULT"));
    BOOST_CHECK_EQUAL("1*", itemString.scan( rawRecord0, unit_system, unit_system ).get< std::string >(0) );

    RawRecord rawRecord1( "13*" );
    BOOST_CHECK_EQUAL("DEFAULT" , itemString.scan( rawRecord1, unit_system, unit_system ).get< std::string >(0) );

    RawRecord rawRecord2( "*" );
    BOOST_CHECK_EQUAL("DEFAULT", itemString.scan( rawRecord2, unit_system, unit_system ).get< std::string >(0) );
}

BOOST_AUTO_TEST_CASE(scan_all_valuesCorrect) {
    ParserItem itemString("ITEMWITHMANY", STRING);
    RawRecord rawRecord( "'WELL1' FISK BANAN 3*X OPPLEGG_FOR_DATAANALYSE 'Foo$*!% BAR' " );
    UnitSystem unit_system;
    itemString.setSizeType( ParserItem::item_size::ALL );
    const auto deckItem = itemString.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(8U, deckItem.data_size());

    BOOST_CHECK_EQUAL("WELL1", deckItem.get< std::string >(0));
    BOOST_CHECK_EQUAL("FISK", deckItem.get< std::string >(1));
    BOOST_CHECK_EQUAL("BANAN", deckItem.get< std::string >(2));
    BOOST_CHECK_EQUAL("X", deckItem.get< std::string >(3));
    BOOST_CHECK_EQUAL("X", deckItem.get< std::string >(4));
    BOOST_CHECK_EQUAL("X", deckItem.get< std::string >(5));
    BOOST_CHECK_EQUAL("OPPLEGG_FOR_DATAANALYSE", deckItem.get< std::string >(6));
    BOOST_CHECK_EQUAL("Foo$*!% BAR", deckItem.get< std::string >(7));
}

BOOST_AUTO_TEST_CASE(scan_all_withdefaults) {
    ParserItem itemString("ITEMWITHMANY", INT);
    RawRecord rawRecord( "10*1 10* 10*2 " );
    UnitSystem unit_system;
    itemString.setDefault(0);
    itemString.setSizeType( ParserItem::item_size::ALL );
    const auto deckItem = itemString.scan(rawRecord, unit_system, unit_system);

    BOOST_CHECK_EQUAL(30U, deckItem.data_size());

    BOOST_CHECK( !deckItem.defaultApplied(0) );
    BOOST_CHECK( !deckItem.defaultApplied(9) );
    BOOST_CHECK(  deckItem.defaultApplied(10) );
    BOOST_CHECK(  deckItem.defaultApplied(19) );
    BOOST_CHECK( !deckItem.defaultApplied(20) );
    BOOST_CHECK( !deckItem.defaultApplied(29) );

    BOOST_CHECK_THROW(deckItem.get< int >(30), std::out_of_range);
    BOOST_CHECK_THROW(deckItem.defaultApplied(30), std::out_of_range);

    BOOST_CHECK_EQUAL(1, deckItem.get< int >(0));
    BOOST_CHECK_EQUAL(1, deckItem.get< int >(9));
    BOOST_CHECK_EQUAL(2, deckItem.get< int >(20));
    BOOST_CHECK_EQUAL(2, deckItem.get< int >(29));
}

BOOST_AUTO_TEST_CASE(scan_single_dataCorrect) {
    ParserItem itemString( "ITEM1", STRING);
    RawRecord rawRecord( "'WELL1' 'WELL2'" );
    UnitSystem unit_system;
    const auto deckItem = itemString.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL("WELL1", deckItem.get< std::string >(0));
}

BOOST_AUTO_TEST_CASE(scan_singleWithMixedRecord_dataCorrect) {
    ParserItem itemString("ITEM1", STRING);
    ParserItem itemInt("ITEM1", INT);
    UnitSystem unit_system;
    RawRecord rawRecord( "2 'WELL1' /" );
    itemInt.scan(rawRecord, unit_system, unit_system);
    const auto deckItem = itemString.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL("WELL1", deckItem.get< std::string >(0));
}

/******************String and int**********************/
BOOST_AUTO_TEST_CASE(scan_intsAndStrings_dataCorrect) {
    RawRecord rawRecord( "'WELL1' 2 2 2*3" );
    UnitSystem unit_system;
    ParserItem itemSingleString(std::string("ITEM1"), STRING);
    const auto deckItemWell1 = itemSingleString.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL("WELL1", deckItemWell1.get< std::string >(0));

    ParserItem itemSomeInts("SOMEINTS", INT);
    itemSomeInts.setSizeType( ParserItem::item_size::ALL );
    const auto deckItemInts = itemSomeInts.scan(rawRecord, unit_system, unit_system);
    BOOST_CHECK_EQUAL(2, deckItemInts.get< int >(0));
    BOOST_CHECK_EQUAL(2, deckItemInts.get< int >(1));
    BOOST_CHECK_EQUAL(3, deckItemInts.get< int >(2));
    BOOST_CHECK_EQUAL(3, deckItemInts.get< int >(3));
}

/*****************************************************************/


BOOST_AUTO_TEST_CASE(ParserDefaultHasDimensionReturnsFalse) {
    ParserItem intItem(std::string("SOMEINTS"), INT);
    ParserItem stringItem(std::string("SOMESTRING"), STRING);
    ParserItem doubleItem(std::string("SOMEDOUBLE"), DOUBLE);

    BOOST_CHECK( intItem.dimensions().empty() );
    BOOST_CHECK( stringItem.dimensions().empty() );
    BOOST_CHECK( doubleItem.dimensions().empty() );
}

BOOST_AUTO_TEST_CASE(ParserIntItemGetDimensionThrows) {
    ParserItem intItem(std::string("SOMEINT"), INT);

    BOOST_CHECK_THROW( intItem.push_backDimension("Length") , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(ParserDoubleItemAddMultipleDimensionToSIngleSizeThrows) {
    ParserItem doubleItem(std::string("SOMEDOUBLE"), DOUBLE);

    doubleItem.push_backDimension("Length*Length");
    BOOST_CHECK_THROW( doubleItem.push_backDimension("Length*Length"), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(ParserDoubleItemWithDimensionHasReturnsCorrect) {
    ParserItem doubleItem("SOMEDOUBLE", DOUBLE);

    BOOST_CHECK( doubleItem.dimensions().empty() );
    doubleItem.push_backDimension("Length*Length");
    BOOST_CHECK( !doubleItem.dimensions().empty() );
}

BOOST_AUTO_TEST_CASE(ParserDoubleItemGetDimension) {
    ParserItem doubleItem( "SOMEDOUBLE", DOUBLE);
    doubleItem.setSizeType( ParserItem::item_size::ALL );
    doubleItem.setDefault(0.0);

    doubleItem.push_backDimension("Length");
    doubleItem.push_backDimension("Length*Length");
    doubleItem.push_backDimension("Length*Length*Length");

    const auto& dimensions = doubleItem.dimensions();
    BOOST_CHECK_EQUAL( "Length" , dimensions[0]);
    BOOST_CHECK_EQUAL( "Length*Length" ,dimensions[1]);
    BOOST_CHECK_EQUAL( "Length*Length*Length" , dimensions[2]);
    BOOST_CHECK_EQUAL(3, dimensions.size());
}


BOOST_AUTO_TEST_CASE(DefaultConstructor_NoParams_NoThrow) {
    BOOST_CHECK_NO_THROW(ParserRecord record);
}

BOOST_AUTO_TEST_CASE(Size_NoElements_ReturnsZero) {
    ParserRecord record;
    BOOST_CHECK_EQUAL(0U, record.size());
}

BOOST_AUTO_TEST_CASE(Size_OneItem_Return1) {
    ParserItem itemInt("ITEM1", INT);
    ParserRecord record;
    record.addItem(itemInt);
    BOOST_CHECK_EQUAL(1U, record.size());
}

BOOST_AUTO_TEST_CASE(Get_OneItem_Return1) {
    ParserItem itemInt("ITEM1", INT);
    ParserRecord record;
    record.addItem(itemInt);

    BOOST_CHECK_EQUAL(record.get(0), itemInt);
}

BOOST_AUTO_TEST_CASE(Get_outOfRange_Throw) {
    BOOST_CHECK_THROW(ParserRecord{}.get(0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(Get_KeyNotFound_Throw) {
    ParserRecord record;
    BOOST_CHECK_THROW(record.get("Hei"), std::out_of_range );
}

BOOST_AUTO_TEST_CASE(Get_KeyFound_OK) {
    ParserItem itemInt("ITEM1", INT);
    ParserRecord record;
    record.addItem(itemInt);
    BOOST_CHECK_EQUAL(record.get("ITEM1"), itemInt);
}

BOOST_AUTO_TEST_CASE(Get_GetByNameAndIndex_OK) {
    ParserItem itemInt("ITEM1", INT);
    ParserRecord record;
    record.addItem(itemInt);

    const auto& itemByName = record.get("ITEM1");
    const auto& itemByIndex = record.get(0);
    BOOST_CHECK_EQUAL(itemInt, itemByName);
    BOOST_CHECK_EQUAL(itemInt, itemByIndex);
}

BOOST_AUTO_TEST_CASE(addItem_SameName_Throw) {
    ParserItem itemInt1("ITEM1", INT);
    ParserItem itemInt2("ITEM1", INT);
    ParserRecord record;
    record.addItem(itemInt1);
    BOOST_CHECK_THROW(record.addItem(itemInt2), std::invalid_argument);
}

static ParserRecord createSimpleParserRecord() {
    ParserItem itemInt1("ITEM1", INT);
    ParserItem itemInt2("ITEM2", INT);
    ParserRecord record;

    record.addItem(itemInt1);
    record.addItem(itemInt2);
    return record;
}

BOOST_AUTO_TEST_CASE(parse_validRecord_noThrow) {
    auto record = createSimpleParserRecord();
    ParseContext parseContext;
    ErrorGuard errors;
    RawRecord raw( string_view( "100 443" ) );
    UnitSystem unit_system;
    BOOST_CHECK_NO_THROW(record.parse(parseContext, errors, raw , unit_system, unit_system, "KEYWORD", "filename") );
}

BOOST_AUTO_TEST_CASE(parse_validRecord_deckRecordCreated) {
    auto record = createSimpleParserRecord();
    RawRecord rawRecord( string_view( "100 443" ) );
    ParseContext parseContext;
    ErrorGuard errors;
    UnitSystem unit_system;
    const auto deckRecord = record.parse(parseContext , errors, rawRecord, unit_system, unit_system, "KEYWORD", "filename");
    BOOST_CHECK_EQUAL(2U, deckRecord.size());
}


// INT INT DOUBLE DOUBLE INT DOUBLE

static ParserRecord createMixedParserRecord() {
    ParserItem itemInt1( "INTITEM1", INT);
    ParserItem itemInt2( "INTITEM2", INT);
    ParserItem itemInt3( "INTITEM3", INT);
    ParserItem itemDouble1( "DOUBLEITEM1", DOUBLE);
    ParserItem itemDouble2( "DOUBLEITEM2", DOUBLE);
    ParserItem itemDouble3( "DOUBLEITEM3", DOUBLE);

    ParserRecord record;
    record.addItem(itemInt1);
    record.addItem(itemInt2);
    record.addItem(itemDouble1);
    record.addItem(itemDouble2);
    record.addItem(itemInt3);
    record.addItem(itemDouble3);

    return record;
}

BOOST_AUTO_TEST_CASE(parse_validMixedRecord_noThrow) {
    auto record = createMixedParserRecord();
    RawRecord rawRecord( string_view( "1 2 10.0 20.0 4 90.0") );
    ParseContext parseContext;
    ErrorGuard errors;
    UnitSystem unit_system;
    BOOST_CHECK_NO_THROW(record.parse(parseContext, errors, rawRecord, unit_system, unit_system, "KEYWORD", "filename"));
}

BOOST_AUTO_TEST_CASE(Equal_Equal_ReturnsTrue) {
    auto record1 = createMixedParserRecord();
    auto record2 = createMixedParserRecord();

    BOOST_CHECK(record1.equal(record1));
    BOOST_CHECK(record1.equal(record2));
}

BOOST_AUTO_TEST_CASE(Equal_Different_ReturnsFalse) {
    ParserItem    itemInt( "INTITEM1", INT);
    ParserItem itemDouble( "DOUBLEITEM1", DOUBLE);
    ParserItem itemString( "STRINGITEM1", STRING);
    ParserRecord record1;
    ParserRecord record2;
    ParserRecord record3;

    record1.addItem(itemInt);
    record1.addItem(itemDouble);

    record2.addItem(itemInt);
    record2.addItem(itemDouble);
    record2.addItem(itemString);

    record3.addItem(itemDouble);
    record3.addItem(itemInt);
    BOOST_CHECK(!record1.equal(record2));
    BOOST_CHECK(!record1.equal(record3));

}

BOOST_AUTO_TEST_CASE(ParseWithDefault_defaultAppliedCorrectInDeck) {
    ParserRecord parserRecord;
    ParserItem itemInt("ITEM1", INT);    itemInt.setDefault(100);
    ParserItem itemString("ITEM2", STRING); itemString.setDefault(std::string("DEFAULT"));
    ParserItem itemDouble("ITEM3", DOUBLE); itemDouble.setDefault(3.14);

    parserRecord.addItem(itemInt);
    parserRecord.addItem(itemString);
    parserRecord.addItem(itemDouble);

    // according to the RM, this is invalid ("an asterisk by itself is not sufficient"),
    // but it seems to appear in the wild. Thus, we interpret this as "1*"...
    {
        RawRecord rawRecord( "* " );
        UnitSystem unit_system;
        const auto& deckStringItem = itemString.scan(rawRecord, unit_system, unit_system);
        const auto& deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
        const auto& deckDoubleItem = itemDouble.scan(rawRecord, unit_system, unit_system);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }

    {
        RawRecord rawRecord( "" );
        UnitSystem unit_system;
        const auto deckStringItem = itemString.scan(rawRecord, unit_system, unit_system);
        const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
        const auto deckDoubleItem = itemDouble.scan(rawRecord, unit_system, unit_system);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }


    {
        RawRecord rawRecord( "TRYGVE 10 2.9 " );

        // let the raw record be "consumed" by the items. Note that the scan() method
        // modifies the rawRecord object!
        UnitSystem unit_system;
        const auto& deckStringItem = itemString.scan(rawRecord, unit_system, unit_system);
        const auto& deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
        const auto& deckDoubleItem = itemDouble.scan(rawRecord, unit_system, unit_system);

        BOOST_CHECK(!deckStringItem.defaultApplied(0));
        BOOST_CHECK(!deckIntItem.defaultApplied(0));
        BOOST_CHECK(!deckDoubleItem.defaultApplied(0));
    }

    // again this is invalid according to the RM, but it is used anyway in the wild...
    {
        RawRecord rawRecord( "* * *" );
        UnitSystem unit_system;
        const auto deckStringItem = itemString.scan(rawRecord, unit_system, unit_system);
        const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
        const auto deckDoubleItem = itemDouble.scan(rawRecord, unit_system, unit_system);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }

    {
        RawRecord rawRecord(  "3*" );
        UnitSystem unit_system;
        const auto deckStringItem = itemString.scan(rawRecord, unit_system, unit_system);
        const auto deckIntItem = itemInt.scan(rawRecord, unit_system, unit_system);
        const auto deckDoubleItem = itemDouble.scan(rawRecord, unit_system, unit_system);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }
}

BOOST_AUTO_TEST_CASE(Parse_RawRecordTooManyItems_Throws) {
    ParserRecord parserRecord;
    ParserItem itemI( "I", INT);
    ParserItem itemJ( "J", INT);
    ParserItem itemK( "K", INT);
    ParseContext parseContext;
    ErrorGuard errors;

    parserRecord.addItem(itemI);
    parserRecord.addItem(itemJ);
    parserRecord.addItem(itemK);


    RawRecord rawRecord(  "3 3 3 " );
    UnitSystem unit_system;
    BOOST_CHECK_NO_THROW(parserRecord.parse(parseContext, errors, rawRecord, unit_system, unit_system, "KEYWORD", "filename"));

    RawRecord rawRecordOneExtra(  "3 3 3 4 " );
    BOOST_CHECK_THROW(parserRecord.parse(parseContext, errors, rawRecordOneExtra, unit_system, unit_system, "KEYWORD", "filename"), std::invalid_argument);

    RawRecord rawRecordForgotRecordTerminator(  "3 3 3 \n 4 4 4 " );
    BOOST_CHECK_THROW(parserRecord.parse(parseContext, errors, rawRecordForgotRecordTerminator, unit_system, unit_system, "KEYWORD", "filename"), std::invalid_argument);

}


BOOST_AUTO_TEST_CASE(Parse_RawRecordTooFewItems) {
    ParserRecord parserRecord;
    ParserItem itemI( "I" , INT);
    ParserItem itemJ( "J" , INT);
    ParserItem itemK( "K" , INT);

    parserRecord.addItem(itemI);
    parserRecord.addItem(itemJ);
    parserRecord.addItem(itemK);

    ParseContext parseContext;
    ErrorGuard errors;
    RawRecord rawRecord(  "3 3  " );
    UnitSystem unit_system;
    // no default specified for the third item, record can be parsed just fine but trying
    // to access the data will raise an exception...;
    BOOST_CHECK_NO_THROW(parserRecord.parse(parseContext, errors, rawRecord, unit_system, unit_system, "KEWYORD", "filename"));
    auto record = parserRecord.parse(parseContext, errors , rawRecord, unit_system, unit_system, "KEYWORD", "filename");
    BOOST_CHECK_NO_THROW(record.getItem(2));
    BOOST_CHECK_THROW(record.getItem(2).get< int >(0), std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(ParseRecordHasDimensionCorrect) {
    ParserRecord parserRecord;
    ParserItem itemI( "I", INT);

    BOOST_CHECK( !parserRecord.hasDimension() );

    parserRecord.addItem( itemI );
    BOOST_CHECK( !parserRecord.hasDimension() );

    ParserItem item2( "ID", DOUBLE);
    item2.push_backDimension("Length*Length/Time");
    parserRecord.addItem( item2 );
    BOOST_CHECK( parserRecord.hasDimension() );
}


BOOST_AUTO_TEST_CASE(DefaultNotDataRecord) {
    ParserRecord record;
    BOOST_CHECK_EQUAL( false , record.isDataRecord() );
}

BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws1) {
    ParserRecord record;
    ParserItem dataItem( "ACTNUM", INT);
    ParserItem item ( "XXX", INT);
    record.addDataItem( dataItem );
    BOOST_CHECK_THROW( record.addItem( item ) , std::invalid_argument);
    BOOST_CHECK_THROW( record.addItem( dataItem ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws2) {
    ParserRecord record;
    ParserItem dataItem( "ACTNUM", INT);
    ParserItem item    ( "XXX", INT);

    record.addItem( item );
    BOOST_CHECK_THROW( record.addDataItem( dataItem ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(construct_withname_nameSet) {
    const auto& parserKeyword = createDynamicSized("BPR");
    BOOST_CHECK_EQUAL(parserKeyword.getName(), "BPR");
}

BOOST_AUTO_TEST_CASE(NamedInit) {
    std::string keyword("KEYWORD");
    const auto& parserKeyword = createFixedSized(keyword, (size_t) 100);
    BOOST_CHECK_EQUAL(parserKeyword.getName(), keyword);
}

BOOST_AUTO_TEST_CASE(ParserKeyword_default_SizeTypedefault) {
    std::string keyword("KEYWORD");
    const auto& parserKeyword = createDynamicSized(keyword);
    BOOST_CHECK_EQUAL(parserKeyword.getSizeType() , SLASH_TERMINATED);
}


BOOST_AUTO_TEST_CASE(ParserKeyword_withSize_SizeTypeFIXED) {
    std::string keyword("KEYWORD");
    const auto& parserKeyword = createFixedSized(keyword, (size_t) 100);
    BOOST_CHECK_EQUAL(parserKeyword.getSizeType() , FIXED);
}

BOOST_AUTO_TEST_CASE(ParserKeyword_withOtherSize_SizeTypeOTHER) {
    std::string keyword("KEYWORD");
    const auto& parserKeyword = createTable(keyword, "EQUILDIMS" , "NTEQUIL" , false);
    const auto& keyword_size = parserKeyword.getKeywordSize();
    BOOST_CHECK_EQUAL(OTHER_KEYWORD_IN_DECK , parserKeyword.getSizeType() );
    BOOST_CHECK_EQUAL("EQUILDIMS", keyword_size.keyword );
    BOOST_CHECK_EQUAL("NTEQUIL" , keyword_size.item );
}

BOOST_AUTO_TEST_CASE(ParserKeyword_validDeckName) {
    BOOST_CHECK_EQUAL( true , ParserKeyword::validDeckName("SUMMARY"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validDeckName("MixeCase"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validDeckName("STRING88"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("88STRING"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("KEY.EXT"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("STRING~"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validDeckName("MINUS-"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validDeckName("PLUS+"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("SHARP#"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("-MINUS"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("+PLUS"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("#SHARP"));

    BOOST_CHECK_EQUAL( false  , ParserKeyword::validDeckName("TVDP*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validDeckName("*"));
}

BOOST_AUTO_TEST_CASE(ParserKeyword_validInternalName) {
    BOOST_CHECK_EQUAL( true , ParserKeyword::validInternalName("SUMMARY"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validInternalName("MixeCase"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validInternalName("NAMEISQUITELONG"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validInternalName("I_have_underscores"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("WHATABOUT+"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("ORMINUS-"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("NOSHARP#"));
    BOOST_CHECK_EQUAL( true , ParserKeyword::validInternalName("STRING88"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("88STRING"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("KEY.EXT"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("STRING~"));

    BOOST_CHECK_EQUAL( false  , ParserKeyword::validInternalName("TVDP*"));
    BOOST_CHECK_EQUAL( false , ParserKeyword::validInternalName("*"));
}

BOOST_AUTO_TEST_CASE(ParserKeywordMatches) {
    auto parserKeyword = createFixedSized("HELLO", (size_t) 1);
    parserKeyword.clearDeckNames();
    parserKeyword.setMatchRegex("WORLD.+");
    BOOST_CHECK_EQUAL( false , parserKeyword.matches("HELLO"));
    BOOST_CHECK_EQUAL( false , parserKeyword.matches("WORLD"));
    BOOST_CHECK_EQUAL( true , parserKeyword.matches("WORLDABC"));
    BOOST_CHECK_EQUAL( false , parserKeyword.matches("WORLD#BC"));
}

BOOST_AUTO_TEST_CASE(AddDataKeyword_correctlyConfigured) {
    auto parserKeyword = createFixedSized("PORO", (size_t) 1);
    ParserItem item( "ACTNUM", INT);
    ParserRecord record;

    BOOST_CHECK( !parserKeyword.isDataKeyword() );
    record.addDataItem( item );
    parserKeyword.addRecord( record );
    BOOST_CHECK( parserKeyword.isDataKeyword() );

    BOOST_CHECK( parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL(1U , parserKeyword.getFixedSize() );
}

BOOST_AUTO_TEST_CASE(WrongConstructor_addDataItem_throws) {
    auto parserKeyword = createDynamicSized("PORO");
    ParserItem dataItem( "ACTNUM", INT);
    ParserRecord record;
    record.addDataItem( dataItem );
    BOOST_CHECK_THROW( parserKeyword.addDataRecord( record ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(DefaultConstructur_setDescription_canReadBack) {
    auto parserKeyword = createDynamicSized("BPR");
    std::string description("This is the description");
    parserKeyword.setDescription(description);
    BOOST_CHECK_EQUAL( description, parserKeyword.getDescription());
}

/*****************************************************************/
/* json */
BOOST_AUTO_TEST_CASE(ConstructFromJsonObject) {
    Json::JsonObject jsonObject("{\"name\": \"XXX\", \"sections\":[], \"size\" : 0}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL("XXX" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
}

BOOST_AUTO_TEST_CASE(ConstructMultiNameFromJsonObject) {
    const auto jsonString =
        "{"
        "  \"name\": \"XXX\" ,"
        "  \"sections\":[],"
        "  \"size\" : 0,"
        "  \"deck_names\" : ["
        "    \"XXA\","
        "    \"XXB\","
        "    \"XXC\""
        "  ]"
        "}";
    Json::JsonObject jsonObject(jsonString);
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL("XXX" , parserKeyword.getName());
    BOOST_CHECK(parserKeyword.matches("XXA"));
    BOOST_CHECK(parserKeyword.matches("XXB"));
    BOOST_CHECK(parserKeyword.hasMultipleDeckNames());
    BOOST_CHECK(!parserKeyword.matches("XXD"));
    BOOST_CHECK(!parserKeyword.matches("XXX"));
}


BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withSize) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");

    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_nosize_notItems_OK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"]}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 0U , parserKeyword.getFixedSize());
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withSizeOther) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : {\"keyword\" : \"Bjarne\" , \"item\": \"BjarneIgjen\"},  \"items\" :[{\"name\":\"ItemX\" ,  \"value_type\" : \"DOUBLE\"}]}");
    ParserKeyword parserKeyword(jsonObject);
    const auto& keyword_size = parserKeyword.getKeywordSize();
    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( false , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL(OTHER_KEYWORD_IN_DECK , parserKeyword.getSizeType());
    BOOST_CHECK_EQUAL("Bjarne", keyword_size.keyword);
    BOOST_CHECK_EQUAL("BjarneIgjen" , keyword_size.item );
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_missingName_throws) {
    Json::JsonObject jsonObject("{\"nameXX\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100}");
    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject) , std::invalid_argument);
}

/*
  "items": [{"name" : "I" , "size_type" : "SINGLE" , "value_type" : "int"}]
*/
BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_invalidItems_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : 100}");
    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemMissingName_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : [{\"nameX\" : \"I\"  , \"value_type\" : \"INT\"}]}");
    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemMissingValueType_throws) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"Xvalue_type\" : \"INT\"}]}");
    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_ItemInvalidEnum_throws) {
    Json::JsonObject jsonObject1("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"XSINGLE\" , \"value_type\" : \"INT\"}]}");
    Json::JsonObject jsonObject2("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INTX\"}]}");

    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject1) , std::invalid_argument);
    BOOST_CHECK_THROW(std::make_shared<const ParserKeyword>(jsonObject2) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObjectItemsOK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" : [{\"name\" : \"I\", \"value_type\" : \"INT\"}]}");
    const auto& parserKeyword = std::make_shared<const ParserKeyword>(jsonObject);
    const auto& record = parserKeyword->getRecord(0);
    const auto& item = record.get( 0 );
    BOOST_CHECK_EQUAL( 1U , record.size( ) );
    BOOST_CHECK_EQUAL( "I" , item.name( ) );
    BOOST_CHECK_EQUAL( ParserItem::item_size::SINGLE , item.sizeType());
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_sizeFromOther) {
    Json::JsonObject jsonConfig("{\"name\": \"EQUILX\", \"sections\":[\"PROPS\"], \"size\" : {\"keyword\":\"EQLDIMS\" , \"item\" : \"NTEQUL\"},  \"items\" :[{\"name\":\"ItemX\" ,\"value_type\" : \"DOUBLE\"}]}");
    BOOST_CHECK_NO_THROW( std::make_shared<const ParserKeyword>(jsonConfig) );
}

BOOST_AUTO_TEST_CASE(Default_NotData) {
    auto parserKeyword = createDynamicSized("BPR");
    BOOST_CHECK_EQUAL( false , parserKeyword.isDataKeyword());
}


BOOST_AUTO_TEST_CASE(AddDataKeywordFromJson_defaultThrows) {
    Json::JsonObject jsonConfig("{\"name\": \"ACTNUM\", \"sections\":[\"GRID\"], \"data\" : {\"value_type\": \"INT\" , \"default\" : 100}}");
    BOOST_CHECK_THROW( new ParserKeyword(jsonConfig), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(AddDataKeywordFromJson_correctlyConfigured) {
    Json::JsonObject jsonConfig("{\"name\": \"ACTNUM\", \"sections\":[\"GRID\"], \"data\" : {\"value_type\": \"INT\"}}");
    ParserKeyword parserKeyword(jsonConfig);
    const auto& parserRecord = parserKeyword.getRecord(0);
    const auto& item = parserRecord.get(0);


    BOOST_CHECK( parserKeyword.isDataKeyword() );
    BOOST_CHECK( parserKeyword.hasFixedSize( ) );
    BOOST_CHECK_EQUAL(1U , parserKeyword.getFixedSize() );

    BOOST_CHECK_EQUAL( item.name() , ParserKeywords::ACTNUM::data::itemName );
    BOOST_CHECK_EQUAL( ParserItem::item_size::ALL, item.sizeType() );
}

BOOST_AUTO_TEST_CASE(AddkeywordFromJson_numTables_incoorect_throw) {
    Json::JsonObject jsonConfig("{\"name\": \"PVTG\", \"sections\":[\"PROPS\"], \"num_tables\" : 100}");
    BOOST_CHECK_THROW(new ParserKeyword(jsonConfig) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(AddkeywordFromJson_isTableCollection) {
    Json::JsonObject jsonConfig("{\"name\": \"PVTG\", \"sections\":[\"PROPS\"], \"num_tables\" : {\"keyword\": \"TABDIMS\" , \"item\" : \"NTPVT\"} , \"items\" : [{\"name\" : \"data\", \"value_type\" : \"DOUBLE\"}]}");
    ParserKeyword parserKeyword(jsonConfig);
    BOOST_CHECK_EQUAL( true , parserKeyword.isTableCollection() );
    BOOST_CHECK_EQUAL( false , parserKeyword.isDataKeyword());
    BOOST_CHECK_EQUAL( false , parserKeyword.hasFixedSize( ));
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_InvalidSize_throws) {
    Json::JsonObject jsonObject1("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : \"string\" , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");
    Json::JsonObject jsonObject2("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : [1,2,3]    , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");

    BOOST_CHECK_THROW(ParserKeyword kw(jsonObject1) , std::invalid_argument);
    BOOST_CHECK_THROW(ParserKeyword kw(jsonObject2) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_SizeUNKNOWN_OK) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : \"UNKNOWN\" , \"items\" : [{\"name\" : \"I\" , \"size_type\" : \"SINGLE\" , \"value_type\" : \"INT\"}]}");
    ParserKeyword parserKeyword(jsonObject);

    BOOST_CHECK_EQUAL( UNKNOWN , parserKeyword.getSizeType() );
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_WithDescription_DescriptionPropertyShouldBePopulated) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"description\" : \"Description\"}");
    ParserKeyword parserKeyword(jsonObject);

    BOOST_CHECK_EQUAL( "Description", parserKeyword.getDescription() );
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_WithoutDescription_DescriptionPropertyShouldBeEmpty) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"]}");
    ParserKeyword parserKeyword(jsonObject);
    BOOST_CHECK_EQUAL( "", parserKeyword.getDescription() );
}

/* </Json> */
/*****************************************************************/
BOOST_AUTO_TEST_CASE(getFixedSize_sizeObjectHasFixedSize_sizeReturned) {
    const auto& parserKeyword = createFixedSized("JA", (size_t) 3);
    BOOST_CHECK_EQUAL(3U, parserKeyword.getFixedSize());

}

BOOST_AUTO_TEST_CASE(getFixedSize_sizeObjectDoesNotHaveFixedSizeObjectSet_ExceptionThrown) {
    const auto& parserKeyword = createDynamicSized("JA");
    BOOST_CHECK_THROW(parserKeyword.getFixedSize(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(hasFixedSize_hasFixedSizeObject_returnstrue) {
    const auto& parserKeyword = createFixedSized("JA", (size_t) 2);
    BOOST_CHECK(parserKeyword.hasFixedSize());
}

BOOST_AUTO_TEST_CASE(hasFixedSize_sizeObjectDoesNotHaveFixedSize_returnsfalse) {
    const auto& parserKeyword = createDynamicSized("JA");
    BOOST_CHECK(!parserKeyword.hasFixedSize());
}

/******/
/* Tables: */
BOOST_AUTO_TEST_CASE(DefaultIsNot_TableKeyword) {
    const auto& parserKeyword = createDynamicSized("JA");
    BOOST_CHECK(!parserKeyword.isTableCollection());
}

BOOST_AUTO_TEST_CASE(ConstructorIsTableCollection) {
    auto parserKeyword = createTable("JA" , "TABDIMS" , "NTPVT" , true);
    BOOST_CHECK(parserKeyword.isTableCollection());
    BOOST_CHECK(!parserKeyword.hasFixedSize());

    auto keyword_size = parserKeyword.getKeywordSize();
    BOOST_CHECK_EQUAL( parserKeyword.getSizeType() , OTHER_KEYWORD_IN_DECK);
    BOOST_CHECK_EQUAL("TABDIMS", keyword_size.keyword );
    BOOST_CHECK_EQUAL("NTPVT" , keyword_size.item);
}

BOOST_AUTO_TEST_CASE(ParseEmptyRecord) {
    auto tabdimsKeyword = createFixedSized("TEST" , 1);
    ParserRecord record;
    ParserItem item("ITEM", INT);
    RawKeyword rawkeyword( tabdimsKeyword.getName() , "FILE" , 10U , false, Raw::FIXED, 1);
    ParseContext parseContext;
    ErrorGuard errors;
    UnitSystem unit_system;

    BOOST_CHECK_EQUAL( Raw::FIXED , rawkeyword.getSizeType());
    rawkeyword.addRecord( RawRecord("") );
    record.addItem(item);
    tabdimsKeyword.addRecord( record );

    const auto deckKeyword = tabdimsKeyword.parse( parseContext, errors, rawkeyword , unit_system, unit_system, "filename");
    BOOST_REQUIRE_EQUAL( 1U , deckKeyword.size());

    const auto& deckRecord = deckKeyword.getRecord(0);
    BOOST_REQUIRE_EQUAL( 1U , deckRecord.size());
}



/*****************************************************************/
/* Dimension */
BOOST_AUTO_TEST_CASE(ParseKeywordHasDimensionCorrect) {
    auto parserKeyword = createDynamicSized("JA");
    ParserItem item1("I",  DOUBLE);
    ParserItem item2("ID", DOUBLE);
    ParserRecord record;

    BOOST_CHECK( !parserKeyword.hasDimension());
    BOOST_CHECK_EQUAL( 0U , item1.dimensions().size() );
    item2.push_backDimension("Length");
    BOOST_CHECK_EQUAL( 1U , item2.dimensions().size() );

    record.addItem( item1 );
    parserKeyword.addRecord( record );
    BOOST_CHECK( !parserKeyword.hasDimension() );
    BOOST_CHECK_EQUAL( 0U , item1.dimensions().size() );

    record.addItem( item2 );

    auto parserKeyword2 = createDynamicSized("JA");
    parserKeyword2.addRecord( record );
    BOOST_CHECK( parserKeyword2.hasDimension() );
    BOOST_CHECK_EQUAL( 1U , item2.dimensions().size() );
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withDimension) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\" , \"dimension\" : \"Length*Length/Time\"}]}");
    ParserKeyword parserKeyword(jsonObject);

    const auto& record = parserKeyword.getRecord(0);
    const auto& item = record.get("ItemX");

    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK( parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

    BOOST_CHECK( parserKeyword.hasDimension() );
    BOOST_CHECK_EQUAL( 1U , item.dimensions().size() );
}

BOOST_AUTO_TEST_CASE(ConstructFromJsonObject_withDimensionList) {
    Json::JsonObject jsonObject("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 , \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"ALL\" , \"value_type\" : \"DOUBLE\" , \"dimension\" : [\"Length*Length/Time\" , \"Time\", \"1\"]}]}");
    ParserKeyword parserKeyword(jsonObject);
    const auto& record = parserKeyword.getRecord(0);
    const auto& item = record.get("ItemX");

    BOOST_CHECK_EQUAL("BPR" , parserKeyword.getName());
    BOOST_CHECK_EQUAL( true , parserKeyword.hasFixedSize() );
    BOOST_CHECK_EQUAL( 100U , parserKeyword.getFixedSize() );

    BOOST_CHECK( parserKeyword.hasDimension() );
    BOOST_CHECK_EQUAL( 3U , item.dimensions().size() );
}





BOOST_AUTO_TEST_CASE(ConstructFromJson_withRecords) {
    const std::string json_string1 = "{\"name\" : \"MULTFLT\", \"sections\" : [\"GRID\", \"EDIT\", \"SCHEDULE\"], \"records\" : [["
        "{\"name\" : \"fault\" , \"value_type\" : \"STRING\"},"
        "{\"name\" : \"factor\" , \"value_type\" : \"DOUBLE\"}]]}";

    const std::string json_string2 = "{\"name\" : \"MULTFLT\", \"sections\" : [\"GRID\", \"EDIT\", \"SCHEDULE\"], \"items\" : ["
        "{\"name\" : \"fault\" , \"value_type\" : \"STRING\"},"
        "{\"name\" : \"factor\" , \"value_type\" : \"DOUBLE\"}]}";


    Json::JsonObject jsonObject1( json_string1 );
    Json::JsonObject jsonObject2( json_string2 );
    ParserKeyword kw1(jsonObject1);
    ParserKeyword kw2(jsonObject2);

    BOOST_CHECK_EQUAL( kw1, kw2 );
}

BOOST_AUTO_TEST_CASE(ConstructFromJson_withRecords_and_items_throws) {
    const std::string json_string = "{\"name\" : \"MULTFLT\", \"sections\" : [\"GRID\", \"EDIT\", \"SCHEDULE\"], \"records\" : [["
        "{\"name\" : \"fault\" , \"value_type\" : \"STRING\"},"
        "{\"name\" : \"factor\" , \"value_type\" : \"DOUBLE\"}]],\"items\" : ["
        "{\"name\" : \"fault\" , \"value_type\" : \"STRING\"},"
        "{\"name\" : \"factor\" , \"value_type\" : \"DOUBLE\"}]}";
    Json::JsonObject jsonObject( json_string );
    BOOST_CHECK_THROW( ParserKeyword kw( jsonObject ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ConstructFromJson_withAlternatingRecords) {
    const std::string json_string = R"(
    {"name" : "STOG", "sections" : ["PROPS"] , "num_tables" : {"keyword" : "TABDIMS" , "item" : "NTPVT"}, "alternating_records" : [[
      {"name" : "ref_oil_pressure", "value_type" : "DOUBLE"}], [
      {"name" : "oil_phase_pressure" , "value_type" : "DOUBLE"},
      {"name" : "surface_rension", "value_type" : "DOUBLE"}]]}
    )";
    Json::JsonObject jsonObject( json_string );
    ParserKeyword kw( jsonObject );
    BOOST_CHECK( kw.isAlternatingKeyword() );
    BOOST_CHECK_EQUAL(kw.getRecord(0).size(), 1);
    BOOST_CHECK_EQUAL(kw.getRecord(1).size(), 2);
}

BOOST_AUTO_TEST_CASE(ConstructFromJson_withAlternatingRecordswithItems) {
    const std::string json_string = R"(
    {"name" : "STOG", "sections" : ["PROPS"] , "num_tables" : {"keyword" : "TABDIMS" , "item" : "NTPVT"}, "alternating_records" : [[
      {"name" : "ref_oil_pressure", "value_type" : "DOUBLE"}], [
      {"name" : "oil_phase_pressure" , "value_type" : "DOUBLE"},
      {"name" : "surface_rension", "value_type" : "DOUBLE"}]], "items" : [{"name" : "w", "value_type" : "STRING"}]}
    )";
    Json::JsonObject jsonObject( json_string );
    BOOST_CHECK_THROW( ParserKeyword kw( jsonObject ), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(GetAlternatingKeywordFromParser) {
    Parser parser;
    BOOST_CHECK(parser.hasKeyword("STOG"));
    ParserKeyword kw = parser.getKeyword("STOG");
    BOOST_CHECK(kw.isAlternatingKeyword());
    auto record0 = kw.getRecord(0);
    auto record1 = kw.getRecord(1);
    BOOST_CHECK_EQUAL(record0.get(0).name(), "REF_OIL_PHASE_PRESSURE");
    BOOST_CHECK_EQUAL(record1.get(0).name(), "table");
    BOOST_CHECK(kw.getRecord(2) == record0);
    BOOST_CHECK(kw.getRecord(3) == record1);
    BOOST_CHECK(kw.getRecord(12) == record0);
    BOOST_CHECK(kw.getRecord(13) == record1);
}

BOOST_AUTO_TEST_CASE(ConstructFromJson_withDoubleRecords) {
    const std::string json_string = R"(
    {"name" : "CECONT", "sections" : ["PROPS"] , "records_set" : [[
      {"name" : "WELL", "value_type" : "STRING"}, 
      {"name" : "I", "value_type" : "INT"},
      {"name" : "J", "value_type" : "INT"},
      {"name" : "K", "value_type" : "INT"}], [
      {"name" : "TRACER" , "value_type" : "STRING"},
      {"name" : "rate", "value_type" : "DOUBLE"}]]}
    )";
    Json::JsonObject jsonObject( json_string );
    ParserKeyword kw( jsonObject );
    BOOST_CHECK( kw.isDoubleRecordKeyword() );
    BOOST_CHECK_EQUAL(kw.getRecord(0).size(), 4);
    BOOST_CHECK_EQUAL(kw.getRecord(1).size(), 2);
}

BOOST_AUTO_TEST_CASE(GetDoubleRecordKeywordFromParser) {
    Parser parser;
    BOOST_CHECK(parser.hasKeyword("CECONT"));
    ParserKeyword kw = parser.getKeyword("CECONT");
    BOOST_CHECK(kw.isDoubleRecordKeyword());
    BOOST_CHECK(kw.getSizeType() == DOUBLE_SLASH_TERMINATED);
    auto record0 = kw.getRecord(0);
    auto record1 = kw.getRecord(1);
    BOOST_CHECK_EQUAL(record0.get(0).name(), "WELL");
    BOOST_CHECK_EQUAL(record1.get(0).name(), "TRACER");
    BOOST_CHECK(kw.getRecord(11) == record1);
    BOOST_CHECK(kw.getRecord(13) == record1);
}



BOOST_AUTO_TEST_CASE(Create1Arg) {
    ParserKeyword kw("GRID");
    BOOST_CHECK_EQUAL( false , kw.hasDimension() );
    BOOST_CHECK( kw.hasFixedSize() );
    BOOST_CHECK_EQUAL( kw.getFixedSize( ) , 0 );

    BOOST_CHECK_THROW( kw.getRecord( 0 ) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(TestKeywordSizeEnum2String) {
    BOOST_CHECK_EQUAL( "SLASH_TERMINATED" , ParserKeywordSizeEnum2String(SLASH_TERMINATED));
    BOOST_CHECK_EQUAL( "FIXED"     , ParserKeywordSizeEnum2String(FIXED));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(OTHER_KEYWORD_IN_DECK));
    BOOST_CHECK_EQUAL( "UNKNOWN"     , ParserKeywordSizeEnum2String(UNKNOWN));
}


BOOST_AUTO_TEST_CASE(TestKeywordSizeEnumFromString) {
    BOOST_CHECK_THROW( ParserKeywordSizeEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( FIXED     , ParserKeywordSizeEnumFromString("FIXED"));
    BOOST_CHECK_EQUAL( SLASH_TERMINATED , ParserKeywordSizeEnumFromString("SLASH_TERMINATED"));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(OTHER_KEYWORD_IN_DECK));
    BOOST_CHECK_EQUAL( "UNKNOWN"     , ParserKeywordSizeEnum2String(UNKNOWN));
}



BOOST_AUTO_TEST_CASE(TestKeywordSizeEnumLoop) {
    BOOST_CHECK_EQUAL( FIXED     , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( FIXED ) ));
    BOOST_CHECK_EQUAL( SLASH_TERMINATED , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( SLASH_TERMINATED ) ));
    BOOST_CHECK_EQUAL( OTHER_KEYWORD_IN_DECK     , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( OTHER_KEYWORD_IN_DECK ) ));
    BOOST_CHECK_EQUAL( UNKNOWN , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( UNKNOWN ) ));

    BOOST_CHECK_EQUAL( "FIXED"     , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "FIXED" ) ));
    BOOST_CHECK_EQUAL( "SLASH_TERMINATED" , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "SLASH_TERMINATED" ) ));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "OTHER_KEYWORD_IN_DECK" ) ));
    BOOST_CHECK_EQUAL( "UNKNOWN" , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "UNKNOWN" ) ));
}



/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestKeywordActionEnum2String) {
    BOOST_CHECK_EQUAL( "INTERNALIZE"     , ParserKeywordActionEnum2String(INTERNALIZE));
    BOOST_CHECK_EQUAL( "IGNORE"          , ParserKeywordActionEnum2String(IGNORE));
    BOOST_CHECK_EQUAL( "IGNORE_WARNING"  , ParserKeywordActionEnum2String(IGNORE_WARNING));
    BOOST_CHECK_EQUAL( "THROW_EXCEPTION" , ParserKeywordActionEnum2String(THROW_EXCEPTION));
}


BOOST_AUTO_TEST_CASE(TestKeywordActionEnumFromString) {
    BOOST_CHECK_THROW( ParserKeywordActionEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( INTERNALIZE    , ParserKeywordActionEnumFromString("INTERNALIZE"));
    BOOST_CHECK_EQUAL( IGNORE_WARNING , ParserKeywordActionEnumFromString("IGNORE_WARNING"));
    BOOST_CHECK_EQUAL( IGNORE  , ParserKeywordActionEnumFromString("IGNORE"));
    BOOST_CHECK_EQUAL( THROW_EXCEPTION  , ParserKeywordActionEnumFromString("THROW_EXCEPTION"));
}



BOOST_AUTO_TEST_CASE(TestKeywordActionEnumLoop) {
    BOOST_CHECK_EQUAL( INTERNALIZE    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( INTERNALIZE ) ));
    BOOST_CHECK_EQUAL( IGNORE , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( IGNORE ) ));
    BOOST_CHECK_EQUAL( IGNORE_WARNING    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( IGNORE_WARNING ) ));
    BOOST_CHECK_EQUAL( THROW_EXCEPTION    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( THROW_EXCEPTION ) ));

    BOOST_CHECK_EQUAL( "INTERNALIZE"    , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "INTERNALIZE" ) ));
    BOOST_CHECK_EQUAL( "IGNORE" , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "IGNORE" ) ));
    BOOST_CHECK_EQUAL( "IGNORE_WARNING"    , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "IGNORE_WARNING" ) ));
    BOOST_CHECK_EQUAL( "THROW_EXCEPTION" , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "THROW_EXCEPTION" ) ));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(ParseUnitConventions)
{
    const auto* deck_string = R"(
METRIC
FIELD
LAB
PVT-M
)";

    Parser parser;
    const auto deck = parser.parseString( deck_string );

    BOOST_CHECK( deck.hasKeyword( "METRIC" ) );
    BOOST_CHECK( deck.hasKeyword( "FIELD" ) );
    BOOST_CHECK( deck.hasKeyword( "LAB" ) );
    BOOST_CHECK( deck.hasKeyword( "PVT-M" ) );
}



BOOST_AUTO_TEST_CASE(ParseAQUTAB) {
  const auto * deck_string = R"(
RUNSPEC

AQUDIMS
 * * 2 /

PROPS

AQUTAB
  0    1
  0.10 1.1
  0.20 1.2 /
)";

  Parser parser;
  const auto deck = parser.parseString( deck_string);
  const auto& aqutab = deck.getKeyword("AQUTAB");
  BOOST_CHECK_EQUAL( 1, aqutab.size());
}


BOOST_AUTO_TEST_CASE(ParseRAW_STRING) {
    const std::string deck_string = R"(UDQ
   DEFINE 'WUBHP' 'P*X*' /
   DEFINE 'WUBHP' 'P*X*' 5*(1 + LOG(WBHP)) /
/
)";
    Parser parser;
    const auto deck = parser.parseString( deck_string);
    const auto& udq = deck.getKeyword("UDQ");
    const std::vector<std::string> expected0 = {"'P*X*'"};
    const std::vector<std::string> expected1 = {"'P*X*'", "5*(1", "+", "LOG(WBHP))"};
    const auto& data0 = RawString::strings( udq.getRecord(0).getItem("DATA").getData<RawString>() );
    const auto& data1 = RawString::strings( udq.getRecord(1).getItem("DATA").getData<RawString>() );
    BOOST_CHECK_EQUAL_COLLECTIONS( data0.begin(), data0.end(), expected0.begin(), expected0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS( data1.begin(), data1.end(), expected1.begin(), expected1.end());

    std::stringstream ss;
    ss << udq;
    BOOST_CHECK_EQUAL(ss.str(), deck_string);
}


BOOST_AUTO_TEST_CASE(ParseThreePhaseRelpermModels) {
    {
        const auto deck_string = std::string{ R"(
STONE1
STONE2
)" };

        const auto deck = Parser{}.parseString( deck_string);

        BOOST_CHECK( !deck.hasKeyword( "STONE" ) );
        BOOST_CHECK(  deck.hasKeyword( "STONE1" ) );
        BOOST_CHECK(  deck.hasKeyword( "STONE2" ) );
    }

    {
        const auto deck_string = std::string{ R"(
STONE
)" };

        const auto deck = Parser{}.parseString( deck_string );

        BOOST_CHECK(  deck.hasKeyword( "STONE" ) );
        BOOST_CHECK( !deck.hasKeyword( "STONE1" ) );
        BOOST_CHECK( !deck.hasKeyword( "STONE2" ) );
    }
}

BOOST_AUTO_TEST_CASE(ParseDynamicr) {
    const auto deck_string = std::string { R"(RUNSPEC
FIELD
SOLUTION
DYNAMICR
FIPYYY
1 /
"IN" FIPXXX 1 /
/
FIPYYY
2 /
"ALL" FIPXXX 2 FIPXXX 3 /
/
FIPYYY
3 /
BOIP <= 500000 /
/ENDDYN
SCHEDULE
GCUTBACK
G1 0.6 3* 0.9 LIQ /
/
)"};

Parser parser;
auto deck = parser.parseString(deck_string);
BOOST_CHECK( deck.hasKeyword("GCUTBACK") );

}

BOOST_AUTO_TEST_CASE(ParseRSConst) {
    // Check that parsing RSCONST does not bleed into next keyword.

    const auto deck_string = std::string { R"(RUNSPEC
FIELD
TABDIMS
  1* 2
/

PROPS
RSCONST
  0.35  932 /

DENSITY
  50.91  62.4 /
  51.90  64.2 /
)" };

    const auto deck = Parser{}.parseString( deck_string );

    BOOST_CHECK( deck.hasKeyword( "RSCONST" ) );

    const auto& rsconst = deck.getKeyword("RSCONST");
    BOOST_CHECK_EQUAL( rsconst.size( ), 1 );

    const auto& rec = rsconst.getRecord( 0 );
    BOOST_CHECK_EQUAL( rec.size( ), 2 );

    const auto& rs   = rec.getItem( 0 );
    const auto& pbub = rec.getItem( 1 );

    BOOST_CHECK_EQUAL( rs.name( ), "RS" );
    BOOST_CHECK_EQUAL( pbub.name( ), "PB" );

    BOOST_CHECK( ! rs.defaultApplied( 0 ) );
    BOOST_CHECK( ! pbub.defaultApplied( 0 ) );

    BOOST_CHECK_CLOSE( rs.get< double >( 0 ), 0.35, 1.0e-10 );
    BOOST_CHECK_CLOSE( pbub.get< double >( 0 ), 932.0, 1.0e-10 );

    BOOST_CHECK_CLOSE( rs.getSIDouble( 0 ), 62.337662337662323, 1.0e-10 );
    BOOST_CHECK_CLOSE( pbub.getSIDouble( 0 ), 6.425913797232911e+06, 1.0e-10 );
}


BOOST_AUTO_TEST_CASE(DefaultedRSConst) {
    // Defaulted RSCONST should *probably* fail at the input stage.
    // In other words, Parser::parseString() should probably fail here.
    //
    // We currently just construct a single record of empty items
    // for which 'defaultApplied()' returns true.

    const auto deck_string = std::string { R"(RUNSPEC
FIELD
TABDIMS
/

PROPS
RSCONST
/

DENSITY
  50.91  62.4 /
)" };

    const auto deck = Parser{}.parseString( deck_string);

    BOOST_CHECK( deck.hasKeyword( "RSCONST" ) );

    const auto& rsconst = deck.getKeyword("RSCONST");
    BOOST_CHECK_EQUAL( rsconst.size( ), 1 );

    const auto& rec = rsconst.getRecord( 0 );
    BOOST_CHECK_EQUAL( rec.size( ), 2 );

    const auto& rs   = rec.getItem( 0 );
    const auto& pbub = rec.getItem( 1 );

    BOOST_CHECK_EQUAL( rs.name( ), "RS" );
    BOOST_CHECK_EQUAL( pbub.name( ), "PB" );

    BOOST_CHECK( rs.defaultApplied( 0 ) );
    BOOST_CHECK( pbub.defaultApplied( 0 ) );
}

BOOST_AUTO_TEST_CASE(IGNORE_SOH) {
    // Check that parsing RSCONSTT does not bleed into next keyword.

    const auto deck_string = std::string { R"(
FIELD
TABDIMS
  1* 2
/
-- The ^A should be here - that is ASCII character 1 which is sometimes
-- inserted by the windows editor Notepad++
PROPS
RSCONSTT
  0.35  932 /
  0.40  945 /
)" };

    const auto deck = Parser{}.parseString( deck_string );
}

BOOST_AUTO_TEST_CASE(InvalidLateUnits) {
    const auto deck_string = std::string { R"(RUNSPEC
DX
   100*100/

FIELD

)" };
    /*
      The opm-parser will fail hard if you change unit system with one of the
      FIELD, METRIC, LAB or PVT-M keywords after you have loaded a keyword with
      a dimension. This test verifies that you indeed get an exception in this
      case.

      The faulty situation should be "impossible" because the unit system
      keywords should be in the RUNSPEC section, and all the other keywords in
      the RUNSPEC section are integer keywords without dimension. Hence the deck
      used in this test has incorrectly put the DX keyword in the RUNSPEC
      section in order to provoke the exception; if at some stage the opm parser
      treats section stricter this test might fail due to that reason instead.
    */
    BOOST_CHECK_THROW( Parser{}.parseString(deck_string), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ParseRSConstT) {
    // Check that parsing RSCONSTT does not bleed into next keyword.

    const auto deck_string = std::string { R"(RUNSPEC
FIELD
TABDIMS
  1* 2
/

PROPS
RSCONSTT
  0.35  932 /
  0.40  945 /

DENSITY
  50.91  62.4 /
  51.90  64.2 /
)" };

    const auto deck = Parser{}.parseString( deck_string );

    BOOST_CHECK( deck.hasKeyword( "RSCONSTT" ) );

    const auto& rsconstt = deck.getKeyword( "RSCONSTT" );
    BOOST_CHECK_EQUAL( rsconstt.size( ), 2 );

    // First Record (ID = 0)
    {
        const auto& rec0 = rsconstt.getRecord( 0 );
        BOOST_CHECK_EQUAL( rec0.size( ), 2 );

        const auto& rs   = rec0.getItem( 0 );
        const auto& pbub = rec0.getItem( 1 );

        BOOST_CHECK_EQUAL( rs.name( ), "RS_CONSTT" );
        BOOST_CHECK_EQUAL( pbub.name( ), "PB_CONSTT" );

        BOOST_CHECK( ! rs.defaultApplied( 0 ) );
        BOOST_CHECK( ! pbub.defaultApplied( 0 ) );

        BOOST_CHECK_CLOSE( rs.get< double >( 0 ), 0.35, 1.0e-10 );
        BOOST_CHECK_CLOSE( pbub.get< double >( 0 ), 932.0, 1.0e-10 );

        BOOST_CHECK_CLOSE( rs.getSIDouble( 0 ), 62.337662337662323, 1.0e-10 );
        BOOST_CHECK_CLOSE( pbub.getSIDouble( 0 ), 6.425913797232911e+06, 1.0e-10 );
    }

    // Second Record (ID = 1)
    {
        const auto& rec1 = rsconstt.getRecord( 1 );
        BOOST_CHECK_EQUAL( rec1.size( ), 2 );

        const auto& rs   = rec1.getItem( 0 );
        const auto& pbub = rec1.getItem( 1 );

        BOOST_CHECK_EQUAL( rs.name( ), "RS_CONSTT" );
        BOOST_CHECK_EQUAL( pbub.name( ), "PB_CONSTT" );

        BOOST_CHECK( ! rs.defaultApplied( 0 ) );
        BOOST_CHECK( ! pbub.defaultApplied( 0 ) );

        BOOST_CHECK_CLOSE( rs.get< double >( 0 ), 0.40, 1.0e-10 );
        BOOST_CHECK_CLOSE( pbub.get< double >( 0 ), 945.0, 1.0e-10 );

        BOOST_CHECK_CLOSE( rs.getSIDouble( 0 ), 71.243042671614077, 1.0e-10 );
        BOOST_CHECK_CLOSE( pbub.getSIDouble( 0 ), 6.515545642044100e+06, 1.0e-10 );
    }
}

BOOST_AUTO_TEST_CASE(ParseDoubleRecords) {
        const auto deck_string = std::string { R"(
RUNSPEC
FIELD
TABDIMS
  1* 2
/

PROPS

DENSITY
  50.91  62.4 /
  51.90  64.2 /

SCHEDULE

CECONT
PROD1 4* CON NO /
TR1 1000.0 0.5 /
TR2 2* 1* 0.8 2* /
TR3 1500.0 0.1 /
/
PROD2 5 6 3 3 CON+ NO /
TR3 100.0 0.05 /
/
PROD3 6* /
TR4 200 5* /
PLY 1* 0.7 /
/
/

GCONSUMP
PLAT-A 20 50 /
PLAT-B 15 /
/

)" };
    Parser parser;
    auto deck = parser.parseString(deck_string);
    BOOST_CHECK( deck.hasKeyword("CECONT") );
    BOOST_CHECK( deck.hasKeyword("GCONSUMP") );

    auto kw_density = deck.getKeyword("DENSITY");

    auto kw = deck.getKeyword("CECONT");
    BOOST_CHECK( kw.isDoubleRecordKeyword() );

    auto record00 = kw.getRecord(0);
    BOOST_CHECK_EQUAL(record00.getItem(0).name(), "WELL");
    BOOST_CHECK_EQUAL(record00.getItem(0).get<std::string>(0), "PROD1");
    BOOST_CHECK(record00.getItem(1).getType() == type_tag::integer);
    BOOST_CHECK_EQUAL(record00.getItem(1).get<int>(0), 0);

    auto record01 = kw.getRecord(1);
    BOOST_CHECK_EQUAL(record01.getItem(0).name(), "TRACER");
    BOOST_CHECK_EQUAL(record01.getItem(0).get<std::string>(0), "TR1");
    BOOST_CHECK(record01.getItem(1).getType() == type_tag::fdouble);
    BOOST_CHECK_EQUAL(record01.getItem(1).get<double>(0), 1000);

    BOOST_CHECK_EQUAL( kw.getRecord(4).size(), 0 );

    auto record04 = kw.getRecord(5);
    BOOST_CHECK_EQUAL(record04.getItem(0).name(), "WELL");
    BOOST_CHECK_EQUAL(record04.getItem(0).get<std::string>(0), "PROD2");
    BOOST_CHECK(record04.getItem(1).getType() == type_tag::integer);
    BOOST_CHECK_EQUAL(record04.getItem(1).get<int>(0), 5);

    auto record08 = kw.getRecord(10);
    BOOST_CHECK_EQUAL(record08.getItem(0).name(), "TRACER");
    BOOST_CHECK_EQUAL(record08.getItem(0).get<std::string>(0), "PLY");
    BOOST_CHECK(record08.getItem(2).getType() == type_tag::fdouble);
    BOOST_CHECK_EQUAL(record08.getItem(2).get<double>(0), 0.7);
}

BOOST_AUTO_TEST_CASE(ParseSpecialKeywords) {
    const auto deck_string = std::string { R"(RUNSPEC
FIELD
TABDIMS
    2 /
PROPS

ADSORP
POLYMER /
LANGMUIR 0.0012  0.7  0.4  1.1  0.5  100 /
LANGMUIR 0.0009  0.7  0.5  1.2  0.3  50 /

ROCK
3600.00 .40E-05 /
3600.00 .40E-05 /
3000 0 /

SCHEDULE
UDT
-- here comes a comment
-- and another comment
-- comment


FIELD 1/
-- and anothther comment
NV 4.0E+06 5.0E+06 6.0E+06 7.0E+06 /
40 50 60 70 /
/
/
GCUTBACK
G1 0.6 3* 0.9 LIQ /
G2 1* 3.0 2* 0.9 RESV /
/
UDT
LANGMUIR 4 /
LC 2 /
This keyword will not be finished
)"};

Parser parser;
auto deck = parser.parseString(deck_string);
BOOST_CHECK( deck.hasKeyword("GCUTBACK") );
auto kw = deck.getKeyword("GCUTBACK");
BOOST_CHECK_EQUAL( kw.size(), 2 );
auto record = kw.getRecord(1);
BOOST_CHECK_EQUAL( record.getItem(5).get<double>(0), 0.9 );
BOOST_CHECK( !deck.hasKeyword("LANGMUIR") );
}
