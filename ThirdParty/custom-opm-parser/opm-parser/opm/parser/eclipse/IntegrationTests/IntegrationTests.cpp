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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/filesystem/path.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;

namespace {

std::unique_ptr< ParserKeyword > createFixedSized(const std::string& kw , size_t size) {
    std::unique_ptr< ParserKeyword > pkw( new ParserKeyword( kw ) );
    pkw->setFixedSize( size );
    return pkw;
}

std::unique_ptr< ParserKeyword > createDynamicSized(const std::string& kw) {
    std::unique_ptr< ParserKeyword > pkw( new ParserKeyword( kw ) );
    pkw->setSizeType(SLASH_TERMINATED);
    return pkw;
}

ParserPtr createWWCTParser() {
    auto parserKeyword = createDynamicSized("WWCT");
    {
        std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
        record->addItem( ParserStringItemConstPtr(new ParserStringItem("WELL", ALL)) );
        parserKeyword->addRecord( record );
    }
    auto summaryKeyword = createFixedSized("SUMMARY" , (size_t) 0);

    ParserPtr parser(new Parser());
    parser->addParserKeyword( std::move( parserKeyword ) );
    parser->addParserKeyword( std::move( summaryKeyword ) );
    return parser;
}

}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_deckReturned) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/wwct.data");
    ParserPtr parser = createWWCTParser();
    BOOST_CHECK( parser->isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser->isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW( parser->parseFile(singleKeywordFile.string(), ParseContext()) );
}

BOOST_AUTO_TEST_CASE(parse_stringWithWWCTKeyword_deckReturned) {
    const char *wwctString =
        "SUMMARY\n"
        "\n"
        "-- Kommentar\n"
        "WWCT\n"
        "  'WELL-1' 'WELL-2' / -- Ehne mehne muh\n"
        "/\n";
    ParserPtr parser = createWWCTParser();
    BOOST_CHECK( parser->isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser->isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW(DeckPtr deck =  parser->parseString(wwctString, ParseContext()));
}

BOOST_AUTO_TEST_CASE(parse_streamWithWWCTKeyword_deckReturned) {
    const char *wwctString =
        "SUMMARY\n"
        "\n"
        "-- Kommentar\n"
        "WWCT\n"
        "  'WELL-1' 'WELL-2' / -- Rumpelstilzchen\n"
        "/\n";
    ParserPtr parser = createWWCTParser();
    BOOST_CHECK( parser->isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser->isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW(DeckPtr deck =  parser->parseString( wwctString, ParseContext()));
}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_deckHasWWCT) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/wwct.data");
    ParserPtr parser = createWWCTParser();
    DeckPtr deck = parser->parseFile(singleKeywordFile.string(), ParseContext());
    BOOST_CHECK(deck->hasKeyword("SUMMARY"));
    BOOST_CHECK(deck->hasKeyword("WWCT"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_dataIsCorrect) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/wwct.data");
    ParserPtr parser = createWWCTParser();
    DeckPtr deck =  parser->parseFile(singleKeywordFile.string(), ParseContext());
    BOOST_CHECK_EQUAL("WELL-1", deck->getKeyword("WWCT" , 0).getRecord(0).getItem(0).get< std::string >(0));
    BOOST_CHECK_EQUAL("WELL-2", deck->getKeyword("WWCT" , 0).getRecord(0).getItem(0).get< std::string >(1));
}

BOOST_AUTO_TEST_CASE(parser_internal_name_vs_deck_name) {
    ParserPtr parser(new Opm::Parser());

    // internal names cannot appear in the deck if the deck names and/or deck regular
    // match expressions are given
    BOOST_CHECK(!parser->isRecognizedKeyword("WELL_PROBE"));

    // an existing deck name
    BOOST_CHECK(parser->isRecognizedKeyword("WWPR"));

    // a non-existing deck name
    BOOST_CHECK(!parser->isRecognizedKeyword("WWPRFOO"));

    // user defined quantity. (regex needs to be used.)
    BOOST_CHECK(parser->isRecognizedKeyword("WUFOO"));
}

static ParserPtr createBPRParser() {
    auto parserKeyword = createDynamicSized("BPR");
    {
        std::shared_ptr<ParserRecord> bprRecord = std::make_shared<ParserRecord>();
        bprRecord->addItem(ParserIntItemConstPtr(new ParserIntItem("I", SINGLE)));
        bprRecord->addItem(ParserIntItemConstPtr(new ParserIntItem("J", SINGLE)));
        bprRecord->addItem(ParserIntItemConstPtr(new ParserIntItem("K", SINGLE)));
        parserKeyword->addRecord( bprRecord );
    }
    auto summaryKeyword = createFixedSized("SUMMARY" , (size_t) 0);
    ParserPtr parser(new Parser());
    parser->addParserKeyword( std::move( parserKeyword ) );
    parser->addParserKeyword( std::move( summaryKeyword ) );
    return parser;
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_deckReturned) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/bpr.data");
    ParserPtr parser = createBPRParser();

    BOOST_CHECK_NO_THROW(parser->parseFile(singleKeywordFile.string(), ParseContext()));
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_DeckhasBRP) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/bpr.data");

    ParserPtr parser = createBPRParser();
    DeckPtr deck =  parser->parseFile(singleKeywordFile.string(), ParseContext());

    BOOST_CHECK_EQUAL(true, deck->hasKeyword("BPR"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_dataiscorrect) {
    boost::filesystem::path singleKeywordFile("testdata/integration_tests/bpr.data");

    ParserPtr parser = createBPRParser();
    DeckPtr deck =  parser->parseFile(singleKeywordFile.string(), ParseContext());

    const auto& keyword = deck->getKeyword("BPR" , 0);
    BOOST_CHECK_EQUAL(2U, keyword.size());

    const auto& record1 = keyword.getRecord(0);
    BOOST_CHECK_EQUAL(3U, record1.size());

    BOOST_CHECK_EQUAL(1, record1.getItem(0).get< int >(0));
    BOOST_CHECK_EQUAL(1, record1.getItem("I").get< int >(0));

    BOOST_CHECK_EQUAL(2, record1.getItem(1).get< int >(0));
    BOOST_CHECK_EQUAL(2, record1.getItem("J").get< int >(0));

    BOOST_CHECK_EQUAL(3, record1.getItem(2).get< int >(0));
    BOOST_CHECK_EQUAL(3, record1.getItem("K").get< int >(0));
}


/***************** Testing non-recognized keywords ********************/
BOOST_AUTO_TEST_CASE(parse_unknownkeyword_exceptionthrown) {
    ParserPtr parser(new Parser());
    BOOST_CHECK_THROW( parser->parseFile("testdata/integration_tests/someobscureelements.data", ParseContext()), std::invalid_argument);
}

/*********************Testing truncated (default) records ***************************/


// Datafile contains 3 RADFIN4 keywords. One fully specified, one with 2 out of 11 items, and one with no items.
BOOST_AUTO_TEST_CASE(parse_truncatedrecords_deckFilledWithDefaults) {
    ParserPtr parser(new Parser());
    DeckPtr deck =  parser->parseFile("testdata/integration_tests/truncated_records.data", ParseContext());
    BOOST_CHECK_EQUAL(3U, deck->size());
    const auto& radfin4_0_full= deck->getKeyword("RADFIN4", 0);
    const auto& radfin4_1_partial= deck->getKeyword("RADFIN4", 1);

    // Specified in datafile
    BOOST_CHECK_EQUAL("NAME", radfin4_0_full.getRecord(0).getItem(0).get< std::string >(0));
    BOOST_CHECK_EQUAL("NAME", radfin4_1_partial.getRecord(0).getItem(0).get< std::string >(0));

    // Specified in datafile
    BOOST_CHECK_EQUAL(213, radfin4_0_full.getRecord(0).getItem(1).get< int >(0));
    BOOST_CHECK_EQUAL(213, radfin4_1_partial.getRecord(0).getItem(1).get< int >(0));

    const auto& record_0 = radfin4_0_full.getRecord(0);
    const auto& lastItem_0 = record_0.getItem(record_0.size() - 1);
    BOOST_CHECK(!lastItem_0.defaultApplied(0));
    BOOST_CHECK_EQUAL(lastItem_0.get< int >(0), 18);

    const auto& record_1 = radfin4_1_partial.getRecord(0);
    const auto& lastItem_1 = record_1.getItem(record_1.size() - 1);
    BOOST_CHECK_EQUAL(213, radfin4_1_partial.getRecord(0).getItem(1).get< int >(0));
    BOOST_CHECK(lastItem_1.defaultApplied(0));
    BOOST_CHECK_EQUAL(lastItem_1.get< int >(0), 1);

    auto* parserKeyword = parser->getParserKeywordFromDeckName("RADFIN4");
    ParserRecordConstPtr parserRecord = parserKeyword->getRecord(0);
    ParserItemConstPtr nwmaxItem = parserRecord->get("NWMAX");
    ParserIntItemConstPtr intItem = std::static_pointer_cast<const ParserIntItem>(nwmaxItem);

    BOOST_CHECK_EQUAL(18, radfin4_0_full.getRecord(0).getItem(10).get< int >(0));
    BOOST_CHECK_EQUAL(intItem->getDefault(), radfin4_1_partial.getRecord(0).getItem(10).get< int >(0));
}
