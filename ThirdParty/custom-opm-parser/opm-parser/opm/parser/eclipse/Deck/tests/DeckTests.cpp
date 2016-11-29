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

#define BOOST_TEST_MODULE DeckTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(Initialize) {
    BOOST_REQUIRE_NO_THROW(Deck deck);
    BOOST_REQUIRE_NO_THROW(DeckPtr deckPtr(new Deck()));
    BOOST_REQUIRE_NO_THROW(DeckConstPtr deckConstPtr(new Deck()));
}

BOOST_AUTO_TEST_CASE(Initializer_lists) {
    DeckKeyword foo( "foo" );
    DeckKeyword bar( "bar" );

    std::string foostr( "foo" );
    std::string barstr( "bar" );

    BOOST_REQUIRE_NO_THROW( Deck( { foo, bar } ) );
    BOOST_REQUIRE_NO_THROW( Deck( { foostr, barstr } ) );
    BOOST_REQUIRE_NO_THROW( Deck( { "Kappa", "Phi" } ) );
}

BOOST_AUTO_TEST_CASE(hasKeyword_empty_returnFalse) {
    Deck deck;
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("Bjarne"));
    BOOST_CHECK_THROW( deck.getKeyword("Bjarne") , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(addKeyword_singlekeyword_keywordAdded) {
    Deck deck;
    BOOST_CHECK_NO_THROW(deck.addKeyword( DeckKeyword( "BJARNE" ) ) );
}


BOOST_AUTO_TEST_CASE(getKeywordList_empty_list) {
    Deck deck;
    auto kw_list = deck.getKeywordList("TRULS");
    BOOST_CHECK_EQUAL( kw_list.size() , 0 );
}

BOOST_AUTO_TEST_CASE(getKeyword_singlekeyword_outRange_throws) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "BJARNE" ) );
    BOOST_CHECK_THROW(deck.getKeyword("BJARNE" , 10) , std::out_of_range);
}


BOOST_AUTO_TEST_CASE(getKeywordList_returnOK) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "BJARNE" ) );
    BOOST_CHECK_NO_THROW( deck.getKeywordList("BJARNE") );
}


BOOST_AUTO_TEST_CASE(getKeyword_indexok_returnskeyword) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "BJARNE" ) );
    BOOST_CHECK_NO_THROW(deck.getKeyword(0));
}

BOOST_AUTO_TEST_CASE(numKeyword_singlekeyword_return1) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "BJARNE" ) );
    BOOST_CHECK_EQUAL(1U , deck.count("BJARNE"));
}


BOOST_AUTO_TEST_CASE(numKeyword_twokeyword_return2) {
    Deck deck;
    DeckKeyword keyword("BJARNE");
    deck.addKeyword(keyword);
    deck.addKeyword(keyword);
    BOOST_CHECK_EQUAL(2U , deck.count("BJARNE"));
}


BOOST_AUTO_TEST_CASE(numKeyword_nokeyword_return0) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "BJARNE" ) );
    BOOST_CHECK_EQUAL(0U , deck.count("BJARNEX"));
}


BOOST_AUTO_TEST_CASE(size_twokeyword_return2) {
    Deck deck;
    DeckKeyword keyword ("BJARNE");
    deck.addKeyword(keyword);
    deck.addKeyword(keyword);
    BOOST_CHECK_EQUAL(2U , deck.size());
}

BOOST_AUTO_TEST_CASE(getKeyword_outOfRange_throws) {
    Deck deck;
    deck.addKeyword(DeckKeyword( "TRULS" ) );
    BOOST_CHECK_THROW( deck.getKeyword("TRULS" , 3) , std::out_of_range);
}

BOOST_AUTO_TEST_CASE(getKeywordList_OK) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULS" ) );

    const auto& keywordList = deck.getKeywordList("TRULS");
    BOOST_CHECK_EQUAL( 3U , keywordList.size() );
}

BOOST_AUTO_TEST_CASE(keywordList_getnum_OK) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULSX" ) );

    BOOST_CHECK_EQUAL( 0U , deck.count( "TRULSY" ));
    BOOST_CHECK_EQUAL( 2U , deck.count( "TRULS" ));
    BOOST_CHECK_EQUAL( 1U , deck.count( "TRULSX" ));
}

BOOST_AUTO_TEST_CASE(keywordList_getbyindexoutofbounds_exceptionthrown) {
    Deck deck;
    BOOST_CHECK_THROW(deck.getKeyword(0), std::out_of_range);
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULSX" ) );
    BOOST_CHECK_NO_THROW(deck.getKeyword(2));
    BOOST_CHECK_THROW(deck.getKeyword(3), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(keywordList_getbyindex_correctkeywordreturned) {
    Deck deck;
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULS" ) );
    deck.addKeyword( DeckKeyword( "TRULSX" ) );
    BOOST_CHECK_EQUAL("TRULS",  deck.getKeyword(0).name());
    BOOST_CHECK_EQUAL("TRULS",  deck.getKeyword(1).name());
    BOOST_CHECK_EQUAL("TRULSX", deck.getKeyword(2).name());
}

BOOST_AUTO_TEST_CASE(set_and_get_data_file) {
    Deck deck;
    BOOST_CHECK_EQUAL("", deck.getDataFile());
    std::string file("/path/to/file.DATA");
    deck.setDataFile( file );
    BOOST_CHECK_EQUAL(file, deck.getDataFile());
}

