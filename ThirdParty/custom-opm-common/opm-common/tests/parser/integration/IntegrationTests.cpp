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
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>

#include <opm/input/eclipse/Parser/ParserEnums.hpp>

#include <filesystem>

using namespace Opm;

inline std::string pathprefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

namespace {

ParserKeyword createFixedSized(const std::string& kw , size_t size) {
    ParserKeyword pkw(kw, KeywordSize(size));
    return pkw;
}

ParserKeyword createDynamicSized(const std::string& kw) {
    ParserKeyword  pkw( kw, KeywordSize(SLASH_TERMINATED) );
    return pkw;
}

Parser createWWCTParser() {
    auto parserKeyword = createDynamicSized("WWCT");

    ParserRecord record;
    ParserItem item("WELL", ParserItem::itype::STRING);
    item.setSizeType(ParserItem::item_size::ALL);
    record.addItem(item);
    parserKeyword.addRecord( record );

    auto summaryKeyword = createFixedSized("SUMMARY" , (size_t) 0);

    Parser parser;
    parser.addParserKeyword( std::move( parserKeyword ) );
    parser.addParserKeyword( std::move( summaryKeyword ) );
    return parser;
}

}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_deckReturned) {
    std::filesystem::path singleKeywordFile(pathprefix() + "wwct.data");
    auto parser = createWWCTParser();
    BOOST_CHECK( parser.isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser.isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW( parser.parseFile(singleKeywordFile.string()) );
}

BOOST_AUTO_TEST_CASE(parse_stringWithWWCTKeyword_deckReturned) {
    const char *wwctString =
        "SUMMARY\n"
        "\n"
        "-- Kommentar\n"
        "WWCT\n"
        "  'WELL-1' 'WELL-2' / -- Ehne mehne muh\n"
        "/\n";
    auto parser = createWWCTParser();
    BOOST_CHECK( parser.isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser.isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW(parser.parseString(wwctString));
}

BOOST_AUTO_TEST_CASE(parse_streamWithWWCTKeyword_deckReturned) {
    const char *wwctString =
        "SUMMARY\n"
        "\n"
        "-- Kommentar\n"
        "WWCT\n"
        "  'WELL-1' 'WELL-2' / -- Rumpelstilzchen\n"
        "/\n";
    auto parser = createWWCTParser();
    BOOST_CHECK( parser.isRecognizedKeyword("WWCT"));
    BOOST_CHECK( parser.isRecognizedKeyword("SUMMARY"));
    BOOST_CHECK_NO_THROW(parser.parseString( wwctString));
}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_deckHasWWCT) {
    std::filesystem::path singleKeywordFile(pathprefix() + "wwct.data");
    auto parser = createWWCTParser();
    auto deck = parser.parseFile(singleKeywordFile.string());
    BOOST_CHECK(deck.hasKeyword("SUMMARY"));
    BOOST_CHECK(deck.hasKeyword("WWCT"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_dataIsCorrect) {
    std::filesystem::path singleKeywordFile(pathprefix() + "wwct.data");
    auto parser = createWWCTParser();
    auto deck = parser.parseFile(singleKeywordFile.string());
    BOOST_CHECK_EQUAL("WELL-1", deck["WWCT"][0].getRecord(0).getItem(0).get< std::string >(0));
    BOOST_CHECK_EQUAL("WELL-2", deck["WWCT"][0].getRecord(0).getItem(0).get< std::string >(1));
}

BOOST_AUTO_TEST_CASE(parser_internal_name_vs_deck_name) {
    Parser parser;

    // internal names cannot appear in the deck if the deck names and/or deck regular
    // match expressions are given
    BOOST_CHECK(!parser.isRecognizedKeyword("WELL_PROBE"));

    // an existing deck name
    BOOST_CHECK(parser.isRecognizedKeyword("WWPR"));

    // a non-existing deck name
    BOOST_CHECK(!parser.isRecognizedKeyword("WWPRFOO"));

    // user defined quantity. (regex needs to be used.)
    BOOST_CHECK(parser.isRecognizedKeyword("WUFOO"));
}

static Parser createBPRParser() {
    auto parserKeyword = createDynamicSized("BPR");
    {
        ParserRecord bprRecord;
        bprRecord.addItem( ParserItem("I", ParserItem::itype::INT) );
        bprRecord.addItem( ParserItem("J", ParserItem::itype::INT) );
        bprRecord.addItem( ParserItem("K", ParserItem::itype::INT) );
        parserKeyword.addRecord( bprRecord );
    }
    auto summaryKeyword = createFixedSized("SUMMARY" , (size_t) 0);
    Parser parser;
    parser.addParserKeyword( std::move( parserKeyword ) );
    parser.addParserKeyword( std::move( summaryKeyword ) );
    return parser;
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_deckReturned) {
    std::filesystem::path singleKeywordFile(pathprefix() + "bpr.data");
    auto parser = createBPRParser();

    BOOST_CHECK_NO_THROW(parser.parseFile(singleKeywordFile.string()));
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_DeckhasBRP) {
    std::filesystem::path singleKeywordFile(pathprefix() + "bpr.data");

    auto parser = createBPRParser();
    auto deck =  parser.parseFile(singleKeywordFile.string());

    BOOST_CHECK_EQUAL(true, deck.hasKeyword("BPR"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithBPRKeyword_dataiscorrect) {
    std::filesystem::path singleKeywordFile(pathprefix() + "bpr.data");

    auto parser = createBPRParser();
    auto deck =  parser.parseFile(singleKeywordFile.string());

    const auto& keyword = deck["BPR"][0];
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
    Parser parser;
    std::string deck = R"(
-- Comment
OIL

GRIDUNIT
METRES                   /

GRUDINT -- A wrong, or unknown keyword
 "text" 3 5 /
  3 3 3 3 3 3 /
/



RADFIN4
 'NAME' 213 123 123 123 7 7 18 18 18 18 /
)";
    BOOST_CHECK_THROW( parser.parseString(deck), OpmInputError);
}

/*********************Testing truncated (default) records ***************************/


// Datafile contains 3 RADFIN4 keywords. One fully specified, one with 2 out of 11 items, and one with no items.
BOOST_AUTO_TEST_CASE(parse_truncatedrecords_deckFilledWithDefaults) {
    Parser parser;
    std::string deck_string = R"(
-- Comment
OIL

RADFIN4
'NAME' 213 123 123 123 7 7 18 18 18 18 /

RADFIN4
'NAME' 213  123 123 123 7 7 18 18 18 /
)";

    auto deck =  parser.parseString(deck_string);
    BOOST_CHECK_EQUAL(3U, deck.size());
    const auto& radfin4_0_full= deck["RADFIN4"][0];
    const auto& radfin4_1_partial= deck["RADFIN4"][1];

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

    const auto& parserKeyword = parser.getParserKeywordFromDeckName("RADFIN4");
    const auto& parserRecord = parserKeyword.getRecord(0);
    const auto& intItem = parserRecord.get("NWMAX");

    BOOST_CHECK_EQUAL(18, radfin4_0_full.getRecord(0).getItem(10).get< int >(0));
    BOOST_CHECK_EQUAL(intItem.getDefault< int >(), radfin4_1_partial.getRecord(0).getItem(10).get< int >(0));
}
