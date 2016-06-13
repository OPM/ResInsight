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

#define BOOST_TEST_MODULE DeckStringItemTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(InitializeString) {
    auto stringItem = DeckItem::make< std::string >("TEST");
    BOOST_CHECK_EQUAL("TEST", stringItem.name());
}

BOOST_AUTO_TEST_CASE(DummyDefaults) {
    auto deckStringItem = DeckItem::make< std::string >("TEST");
    BOOST_CHECK_EQUAL(deckStringItem.size(), 0);

    deckStringItem.push_backDummyDefault();
    BOOST_CHECK_EQUAL(deckStringItem.size(), 0);
    BOOST_CHECK_EQUAL(true, deckStringItem.defaultApplied(0));
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(GetStringAtIndex_NoData_ExceptionThrown) {
    auto deckStringItem = DeckItem::make< std::string >("TEST");
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(0), std::out_of_range);
    deckStringItem.push_back("SA");
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(size_variouspushes_sizecorrect) {
    auto deckStringItem = DeckItem::make< std::string >("TEST");

    BOOST_CHECK_EQUAL(0U, deckStringItem.size());
    deckStringItem.push_back("WELL-3");
    BOOST_CHECK_EQUAL(1U, deckStringItem.size());

    deckStringItem.push_back("WELL-4");
    deckStringItem.push_back("WELL-5");
    BOOST_CHECK_EQUAL(3U, deckStringItem.size());
}

BOOST_AUTO_TEST_CASE(DefaultNotApplied) {
    auto deckStringItem = DeckItem::make< std::string >("TEST");
    BOOST_CHECK( deckStringItem.size() == 0 );

    deckStringItem.push_back( "FOO") ;
    BOOST_CHECK( deckStringItem.size() == 1 );
    BOOST_CHECK( deckStringItem.get< std::string >(0) == "FOO" );
    BOOST_CHECK( !deckStringItem.defaultApplied(0) );
}

BOOST_AUTO_TEST_CASE(DefaultApplied) {
    auto deckStringItem = DeckItem::make< std::string >("TEST");
    BOOST_CHECK( deckStringItem.size() == 0 );

    deckStringItem.push_backDefault( "FOO" );
    BOOST_CHECK( deckStringItem.size() == 1 );
    BOOST_CHECK( deckStringItem.get< std::string >(0) == "FOO" );
    BOOST_CHECK( deckStringItem.defaultApplied(0) );
}


BOOST_AUTO_TEST_CASE(PushBackMultiple) {
    auto stringItem = DeckItem::make< std::string >("HEI");
    stringItem.push_back("Heisann ", 100U );
    BOOST_CHECK_EQUAL( 100U , stringItem.size() );
    for (size_t i=0; i < 100; i++)
        BOOST_CHECK_EQUAL("Heisann " , stringItem.get< std::string >(i));
}
