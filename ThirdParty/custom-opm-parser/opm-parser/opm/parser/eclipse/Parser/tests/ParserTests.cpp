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

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>
#define BOOST_TEST_MODULE ParserTests
#include <boost/test/unit_test.hpp>

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>

#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>


using namespace Opm;

inline std::unique_ptr< ParserKeyword > createDynamicSized(const std::string& kw) {
    std::unique_ptr< ParserKeyword > pkw( new ParserKeyword( kw ) );
    pkw->setSizeType(SLASH_TERMINATED);
    return pkw;
}


/************************Basic structural tests**********************'*/

BOOST_AUTO_TEST_CASE(Initializing) {
    BOOST_CHECK_NO_THROW(Parser parser);
    BOOST_CHECK_NO_THROW(Parser parser);
    BOOST_CHECK_NO_THROW(ParserPtr parserPtr(new Parser()));
    BOOST_CHECK_NO_THROW(ParserConstPtr parserConstPtr(new Parser()));
}

BOOST_AUTO_TEST_CASE(addKeyword_keyword_doesntfail) {
    Parser parser;
    parser.addParserKeyword( createDynamicSized( "EQUIL" ) );
}


BOOST_AUTO_TEST_CASE(canParseDeckKeyword_returnstrue) {
    ParserPtr parser(new Parser());
    parser->addParserKeyword(createDynamicSized("FJAS"));
    BOOST_CHECK(parser->isRecognizedKeyword("FJAS"));
}


BOOST_AUTO_TEST_CASE(getKeyword_haskeyword_returnskeyword) {
    ParserPtr parser(new Parser());
    parser->addParserKeyword( createDynamicSized( "FJAS" ) );
    BOOST_CHECK_EQUAL("FJAS", parser->getParserKeywordFromDeckName("FJAS")->getName());
}

BOOST_AUTO_TEST_CASE(getKeyword_hasnotkeyword_getKeywordThrowsException) {
    ParserPtr parser(new Parser());
    parser->addParserKeyword( createDynamicSized( "FJAS" ) );
    BOOST_CHECK_THROW(parser->getParserKeywordFromDeckName("FJASS"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(getAllDeckNames_hasTwoKeywords_returnsCompleteList) {
    ParserPtr parser(new Parser(false));
    std::cout << parser->getAllDeckNames().size() << std::endl;
    parser->addParserKeyword( createDynamicSized( "FJAS" ) );
    parser->addParserKeyword( createDynamicSized( "SAJF" ) );
    BOOST_CHECK_EQUAL(2U, parser->getAllDeckNames().size());
}

BOOST_AUTO_TEST_CASE(getAllDeckNames_hasNoKeywords_returnsEmptyList) {
    ParserPtr parser(new Parser(false));
    BOOST_CHECK_EQUAL(0U, parser->getAllDeckNames().size());
}



/************************ JSON config related tests **********************'*/


BOOST_AUTO_TEST_CASE(addParserKeywordJSON_isRecognizedKeyword_returnstrue) {
    ParserPtr parser(new Parser());
    Json::JsonObject jsonConfig("{\"name\": \"BPR\", \"sections\":[\"SUMMARY\"], \"size\" : 100 ,  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");
    parser->addParserKeyword( jsonConfig );
    BOOST_CHECK(parser->isRecognizedKeyword("BPR"));
}


BOOST_AUTO_TEST_CASE(addParserKeywordJSON_size_isObject_allGood) {
    ParserPtr parser(new Parser());
    Json::JsonObject jsonConfig("{\"name\": \"EQUIXL\", \"sections\":[], \"size\" : {\"keyword\":\"EQLDIMS\" , \"item\" : \"NTEQUL\"},  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}");
    parser->addParserKeyword( jsonConfig );
    BOOST_CHECK(parser->isRecognizedKeyword("EQUIXL"));
}



BOOST_AUTO_TEST_CASE(loadKeywordsJSON_notArray_throw) {
    ParserPtr parser(new Parser());
    Json::JsonObject jsonConfig( "{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"]}");

    BOOST_CHECK_THROW(parser->loadKeywords( jsonConfig ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(loadKeywordsJSON_noSectionsItem_throw) {
    ParserPtr parser(new Parser());
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}]");

    BOOST_CHECK_THROW(parser->loadKeywords( jsonConfig ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(loadKeywordsJSON_isRecognizedKeyword_returnstrue) {
    ParserPtr parser(new Parser());
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"], \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}]");

    parser->loadKeywords( jsonConfig );
    BOOST_CHECK(parser->isRecognizedKeyword("BPR"));
}


BOOST_AUTO_TEST_CASE(empty_sizeReturns0) {
    ParserPtr parser(new Parser( false ));
    BOOST_CHECK_EQUAL( 0U , parser->size());
}



BOOST_AUTO_TEST_CASE(loadKeywordsJSON_manyKeywords_returnstrue) {
    ParserPtr parser(new Parser( false ));
    Json::JsonObject jsonConfig( "[{\"name\" : \"BPR\" , \"size\" : 100, \"sections\":[\"SUMMARY\"] ,  \"items\" :[{\"name\":\"ItemX\" , \"size_type\":\"SINGLE\" , \"value_type\" : \"DOUBLE\"}]}, {\"name\" : \"WWCT\", \"sections\":[\"SUMMARY\"], \"size\" : 0} , {\"name\" : \"EQUIL\", \"sections\":[\"PROPS\"], \"size\" : 0}]");

    parser->loadKeywords( jsonConfig );
    BOOST_CHECK(parser->isRecognizedKeyword("BPR"));
    BOOST_CHECK(parser->isRecognizedKeyword("WWCT"));
    BOOST_CHECK(parser->isRecognizedKeyword("EQUIL"));
    BOOST_CHECK_EQUAL( 3U , parser->size() );
}




/*****************************************************************/


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_fileDoesNotExist_returnsFalse) {
    ParserPtr parser(new Parser());
    boost::filesystem::path configFile("File/does/not/exist");
    BOOST_CHECK_EQUAL( false , parser->loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_invalidJson_returnsFalse) {
    ParserPtr parser(new Parser());
    boost::filesystem::path configFile("testdata/json/example_invalid_json");
    BOOST_CHECK_EQUAL( false , parser->loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_invalidConfig_returnsFalse) {
    ParserPtr parser(new Parser());
    boost::filesystem::path configFile("testdata/json/example_missing_name.json");
    BOOST_CHECK_EQUAL( false , parser->loadKeywordFromFile( configFile ));
}


BOOST_AUTO_TEST_CASE(loadKeywordFromFile_validKeyword_returnsTrueHasKeyword) {
    ParserPtr parser(new Parser( false ));
    boost::filesystem::path configFile("testdata/json/BPR");
    BOOST_CHECK_EQUAL( true , parser->loadKeywordFromFile( configFile ));
    BOOST_CHECK_EQUAL( 1U , parser->size() );
    BOOST_CHECK_EQUAL( true , parser->isRecognizedKeyword("BPR") );
}



BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_directoryDoesNotexist_throws) {
        ParserPtr parser(new Parser(false));
        boost::filesystem::path configPath("path/does/not/exist");
        BOOST_CHECK_THROW(parser->loadKeywordsFromDirectory( configPath), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_notRecursive_allNames) {
        ParserPtr parser(new Parser(false));
        BOOST_CHECK_EQUAL(false , parser->isRecognizedKeyword("BPR"));
        boost::filesystem::path configPath("testdata/config/directory1");
        BOOST_CHECK_NO_THROW(parser->loadKeywordsFromDirectory( configPath, false));
        BOOST_CHECK(parser->isRecognizedKeyword("WWCT"));
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(false , parser->isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_notRecursive_strictNames) {
        ParserPtr parser(new Parser(false));
        boost::filesystem::path configPath("testdata/config/directory1");
        BOOST_CHECK_NO_THROW(parser->loadKeywordsFromDirectory( configPath, false));
        BOOST_CHECK(parser->isRecognizedKeyword("WWCT"));
        // the file name for the following keyword is "Bpr", but that
        // does not matter
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(false , parser->isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_Recursive_allNames) {
        ParserPtr parser(new Parser(false));
        BOOST_CHECK_EQUAL(false , parser->isRecognizedKeyword("BPR"));
        boost::filesystem::path configPath("testdata/config/directory1");
        BOOST_CHECK_NO_THROW(parser->loadKeywordsFromDirectory( configPath, true));
        BOOST_CHECK(parser->isRecognizedKeyword("WWCT"));
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("DIMENS"));
}


BOOST_AUTO_TEST_CASE(loadConfigFromDirectory_default) {
        ParserPtr parser(new Parser(false));
        BOOST_CHECK_EQUAL(false , parser->isRecognizedKeyword("BPR"));
        boost::filesystem::path configPath("testdata/config/directory1");
        BOOST_CHECK_NO_THROW(parser->loadKeywordsFromDirectory( configPath ));
        BOOST_CHECK(parser->isRecognizedKeyword("WWCT"));
        // the file name for the following keyword is "Bpr", but that
        // does not matter
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("BPR"));
        BOOST_CHECK_EQUAL(true , parser->isRecognizedKeyword("DIMENS"));
}

BOOST_AUTO_TEST_CASE(ReplaceKeyword) {
    ParserPtr parser(new Parser());
    const auto* eqldims = parser->getParserKeywordFromDeckName("EQLDIMS");

    BOOST_CHECK( parser->loadKeywordFromFile( "testdata/parser/EQLDIMS2" ) );

    eqldims = parser->getParserKeywordFromDeckName("EQLDIMS");
    auto record = eqldims->getRecord(0);
    BOOST_CHECK(record->hasItem("NEW"));
}


BOOST_AUTO_TEST_CASE(WildCardTest) {
    ParserPtr parser(new Parser());
    BOOST_CHECK(!parser->isRecognizedKeyword("TVDP*"));
    BOOST_CHECK(!parser->isRecognizedKeyword("TVDP"));
    BOOST_CHECK(parser->isRecognizedKeyword("TVDPXXX"));
    BOOST_CHECK(!parser->isRecognizedKeyword("TVDPIAMTOOLONG"));
    BOOST_CHECK(!parser->isRecognizedKeyword("TVD"));

    BOOST_CHECK(!parser->isRecognizedKeyword("TVDP"));

    const auto* keyword1 = parser->getParserKeywordFromDeckName("TVDPA");
    const auto* keyword2 = parser->getParserKeywordFromDeckName("TVDPBC");
    const auto* keyword3 = parser->getParserKeywordFromDeckName("TVDPXXX");

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

    parseContext.update( ParseContext::PARSE_MISSING_INCLUDE , Opm::InputError::THROW_EXCEPTION);
    parser.newDeckFromFile( "testdata/parser/PATHSInInclude.data", parseContext );
    BOOST_CHECK_THROW( parser.newDeckFromFile( "testdata/parser/PATHSInIncludeInvalid.data", ParseContext() ), std::invalid_argument );
}

BOOST_AUTO_TEST_CASE( handle_empty_title ) {
    const auto* input_deck = "RUNSPEC\n\n"
                             "TITLE\n\n"
                             "DIMENS\n10 10 10/\n"
                             "EQLDIMS\n/\n";

    Parser parser;
    const auto deck = parser.newDeckFromString( input_deck, ParseContext() );
    BOOST_CHECK_EQUAL( "untitled", deck->getKeyword( "TITLE" ).getStringData().front() );
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

    BOOST_CHECK_NO_THROW( Parser().newDeckFromString( deck, ParseContext() ) );
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

    Opm::ParseContext parseContext;
    Opm::Parser parser;
    auto deck = parser.parseString( deck1 , parseContext );
    BOOST_CHECK( deck->hasKeyword("TNUMFSGS"));
    BOOST_CHECK( deck->hasKeyword("TNUMFXXX"));
}

