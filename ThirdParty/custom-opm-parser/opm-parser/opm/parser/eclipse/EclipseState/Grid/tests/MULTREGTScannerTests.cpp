/*
  Copyright 2014 Statoil ASA.

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

#define BOOST_TEST_MODULE MULTREGTScannerTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>




BOOST_AUTO_TEST_CASE(TestRegionName) {
    BOOST_CHECK_EQUAL( "FLUXNUM" , Opm::MULTREGT::RegionNameFromDeckValue( "F"));
    BOOST_CHECK_EQUAL( "MULTNUM" , Opm::MULTREGT::RegionNameFromDeckValue( "M"));
    BOOST_CHECK_EQUAL( "OPERNUM" , Opm::MULTREGT::RegionNameFromDeckValue( "O"));

    BOOST_CHECK_THROW( Opm::MULTREGT::RegionNameFromDeckValue("o") , std::invalid_argument);
    BOOST_CHECK_THROW( Opm::MULTREGT::RegionNameFromDeckValue("X") , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(TestNNCBehaviourEnum) {
    BOOST_CHECK_EQUAL( Opm::MULTREGT::ALL      , Opm::MULTREGT::NNCBehaviourFromString( "ALL"));
    BOOST_CHECK_EQUAL( Opm::MULTREGT::NNC      , Opm::MULTREGT::NNCBehaviourFromString( "NNC"));
    BOOST_CHECK_EQUAL( Opm::MULTREGT::NONNC    , Opm::MULTREGT::NNCBehaviourFromString( "NONNC"));
    BOOST_CHECK_EQUAL( Opm::MULTREGT::NOAQUNNC , Opm::MULTREGT::NNCBehaviourFromString( "NOAQUNNC"));


    BOOST_CHECK_THROW(  Opm::MULTREGT::NNCBehaviourFromString( "Invalid") , std::invalid_argument);
}



static Opm::DeckPtr createInvalidMULTREGTDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 3 3 2 /\n"
        "GRID\n"
        "FLUXNUM\n"
        "1 1 2\n"
        "1 1 2\n"
        "1 1 2\n"
        "3 4 5\n"
        "3 4 5\n"
        "3 4 5\n"
        "/\n"
        "MULTREGT\n"
        "1  2   0.50   G   ALL    M / -- Invalid direction\n"
        "/\n"
        "MULTREGT\n"
        "1  2   0.50   X   ALL    G / -- Invalid region \n"
        "/\n"
        "MULTREGT\n"
        "1  2   0.50   X   ALL    M / -- Region not in deck \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(InvalidInput) {
    Opm::DeckPtr deck = createInvalidMULTREGTDeck();
    Opm::EclipseGrid grid( deck );
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);


    // Invalid direction
    std::vector<const Opm::DeckKeyword*> keywords0;
    const auto& multregtKeyword0 = deck->getKeyword( "MULTREGT", 0 );
    keywords0.push_back( &multregtKeyword0 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords0, "MULTNUM" ); , std::invalid_argument );

    // Not supported region
    std::vector<const Opm::DeckKeyword*> keywords1;
    const auto& multregtKeyword1 = deck->getKeyword( "MULTREGT", 1 );
    keywords1.push_back( &multregtKeyword1 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords1, "MULTNUM" ); , std::invalid_argument );

    // The keyword is ok; but it refers to a region which is not in the deck.
    std::vector<const Opm::DeckKeyword*> keywords2;
    const auto& multregtKeyword2 = deck->getKeyword( "MULTREGT", 2 );
    keywords2.push_back( &multregtKeyword2 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords2, "MULTNUM" ); , std::logic_error );
}


static Opm::DeckPtr createNotSupportedMULTREGTDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 3 3 2 /\n"
        "GRID\n"
        "FLUXNUM\n"
        "1 1 2\n"
        "1 1 2\n"
        "1 1 2\n"
        "3 4 5\n"
        "3 4 5\n"
        "3 4 5\n"
        "/\n"
        "MULTREGT\n"
        "1  2   0.50   X   NOAQUNNC  F / -- Not support NOAQUNNC behaviour \n"
        "/\n"
        "MULTREGT\n"
        "*  2   0.50   X   ALL    M / -- Defaulted from region value \n"
        "/\n"
        "MULTREGT\n"
        "2  *   0.50   X   ALL    M / -- Defaulted to region value \n"
        "/\n"
        "MULTREGT\n"
        "2  2   0.50   X   ALL    M / -- Region values equal \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}




BOOST_AUTO_TEST_CASE(NotSupported) {
    Opm::DeckPtr deck = createNotSupportedMULTREGTDeck();
    Opm::EclipseGrid grid( deck );
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);


    // Not support NOAQUNNC behaviour
    std::vector<const Opm::DeckKeyword*> keywords0;
    const auto& multregtKeyword0 = deck->getKeyword( "MULTREGT", 0 );
    keywords0.push_back( &multregtKeyword0 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords0, "MULTNUM" ); , std::invalid_argument );

    // Defaulted from value - not supported
    std::vector<const Opm::DeckKeyword*> keywords1;
    const auto& multregtKeyword1 = deck->getKeyword( "MULTREGT", 1 );
    keywords1.push_back( &multregtKeyword1 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords1, "MULTNUM" ); , std::invalid_argument );


    // Defaulted to value - not supported
    std::vector<const Opm::DeckKeyword*> keywords2;
    const auto& multregtKeyword2 = deck->getKeyword( "MULTREGT", 2 );
    keywords2.push_back( &multregtKeyword2 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords2, "MULTNUM" ); , std::invalid_argument );

    // srcValue == targetValue - not supported
    std::vector<const Opm::DeckKeyword*> keywords3;
    const Opm::DeckKeyword& multregtKeyword3 = deck->getKeyword( "MULTREGT", 3 );
    keywords3.push_back( &multregtKeyword3 );
    BOOST_CHECK_THROW( Opm::MULTREGTScanner scanner( props, keywords3, "MULTNUM" ); , std::invalid_argument );
}

static Opm::DeckPtr createCopyMULTNUMDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "FLUXNUM\n"
        "1 2\n"
        "1 2\n"
        "3 4\n"
        "3 4\n"
        "/\n"
        "COPY\n"
        " FLUXNUM  MULTNUM /\n"
        "/\n"
        "MULTREGT\n"
        "1  2   0.50/ \n"
        "/\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(MULTREGT_COPY_MULTNUM) {
    Opm::DeckPtr deck = createCopyMULTNUMDeck();
    Opm::TableManager tm(*deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(*deck, tm, eg);

    BOOST_CHECK_NO_THROW(props.hasDeckIntGridProperty("FLUXNUM"));
    BOOST_CHECK_NO_THROW(props.hasDeckIntGridProperty("MULTNUM"));
    const auto& fdata = props.getIntGridProperty("FLUXNUM").getData();
    const auto& mdata = props.getIntGridProperty("MULTNUM").getData();
    std::vector<int> data = { 1, 2, 1, 2, 3, 4, 3, 4 };

    for (auto i = 0; i < 2 * 2 * 2; i++) {
        BOOST_CHECK_EQUAL(fdata[i], mdata[i]);
        BOOST_CHECK_EQUAL(fdata[i], data[i]);
    }
}
