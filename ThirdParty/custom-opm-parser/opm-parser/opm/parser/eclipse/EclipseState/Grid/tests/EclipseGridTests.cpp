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
#include <cstdio>

#define BOOST_TEST_MODULE EclipseGridTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>


#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>


BOOST_AUTO_TEST_CASE(CreateMissingDIMENS_throws) {
    Opm::DeckPtr deck(new Opm::Deck());
    deck->addKeyword( Opm::DeckKeyword( "RUNSPEC" ) );
    deck->addKeyword( Opm::DeckKeyword( "GRID" ) );
    deck->addKeyword( Opm::DeckKeyword( "EDIT" ) );

    BOOST_CHECK_THROW(new Opm::EclipseGrid( deck ) , std::invalid_argument);
}



static Opm::DeckPtr createDeckHeaders() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}


static Opm::DeckPtr createDeckMissingDIMS() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "GRID\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(MissingDimsThrows) {
    Opm::DeckPtr deck = createDeckMissingDIMS();
    BOOST_CHECK_THROW( new Opm::EclipseGrid( deck ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(HasGridKeywords) {
    Opm::DeckPtr deck = createDeckHeaders();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( *deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( *deck ));
}


BOOST_AUTO_TEST_CASE(CreateGridNoCells) {
    Opm::DeckPtr deck = createDeckHeaders();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK_EQUAL( 10 , grid.getNX());
    BOOST_CHECK_EQUAL( 10 , grid.getNY());
    BOOST_CHECK_EQUAL( 10 , grid.getNZ());
    BOOST_CHECK_EQUAL( 1000 , grid.getCartesianSize());
}

BOOST_AUTO_TEST_CASE(CheckGridIndex) {
    Opm::EclipseGrid grid(17, 19, 41); // prime time

    auto v_start = grid.getIJK(0);
    BOOST_CHECK_EQUAL(v_start[0], 0);
    BOOST_CHECK_EQUAL(v_start[1], 0);
    BOOST_CHECK_EQUAL(v_start[2], 0);

    auto v_end = grid.getIJK(17*19*41 - 1);
    BOOST_CHECK_EQUAL(v_end[0], 16);
    BOOST_CHECK_EQUAL(v_end[1], 18);
    BOOST_CHECK_EQUAL(v_end[2], 40);

    auto v167 = grid.getIJK(167);
    BOOST_CHECK_EQUAL(v167[0], 14);
    BOOST_CHECK_EQUAL(v167[1], 9);
    BOOST_CHECK_EQUAL(v167[2], 0);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(14, 9, 0), 167);

    auto v5723 = grid.getIJK(5723);
    BOOST_CHECK_EQUAL(v5723[0], 11);
    BOOST_CHECK_EQUAL(v5723[1], 13);
    BOOST_CHECK_EQUAL(v5723[2], 17);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(11, 13, 17), 5723);

    BOOST_CHECK_EQUAL(17 * 19 * 41, grid.getCartesianSize());
}

static Opm::DeckPtr createCPDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createPinchedCPDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "PINCH \n"
        "  0.2 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createMinpvDefaultCPDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "MINPV \n"
        "  / \n"
        "MINPVFIL \n"
        "  / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createMinpvCPDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "MINPV \n"
        "  10 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}



static Opm::DeckPtr createMinpvFilCPDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "MINPVFIL \n"
        "  20 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createCARTDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "100*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createCARTDeckDEPTHZ() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DXV\n"
        "10*0.25 /\n"
        "DYV\n"
        "10*0.25 /\n"
        "DZV\n"
        "10*0.25 /\n"
        "DEPTHZ\n"
        "121*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createCARTInvalidDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DYV\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(CREATE_SIMPLE) {
    Opm::EclipseGrid grid(10,20,30);

    BOOST_CHECK_EQUAL( grid.getNX() , 10 );
    BOOST_CHECK_EQUAL( grid.getNY() , 20 );
    BOOST_CHECK_EQUAL( grid.getNZ() , 30 );
    BOOST_CHECK_EQUAL( grid.getCartesianSize() , 6000 );
    BOOST_CHECK_EQUAL( true , grid.hasCellInfo() );

}

BOOST_AUTO_TEST_CASE(DEPTHZ_EQUAL_TOPS) {
    Opm::DeckPtr deck1 = createCARTDeck();
    Opm::DeckPtr deck2 = createCARTDeckDEPTHZ();

    std::shared_ptr<Opm::EclipseGrid> grid1(new Opm::EclipseGrid( deck1 ));
    std::shared_ptr<Opm::EclipseGrid> grid2(new Opm::EclipseGrid( deck2 ));

    BOOST_CHECK( grid1->equal( *(grid2.get()) ));

    {
        BOOST_CHECK_THROW( grid1->getCellVolume(1000) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellVolume(10,0,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellVolume(0,10,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellVolume(0,0,10) , std::invalid_argument);

        for (size_t g=0; g < 1000; g++)
            BOOST_CHECK_CLOSE( grid1->getCellVolume(g) , 0.25*0.25*0.25 , 0.001);


        for (size_t k= 0; k < 10; k++)
            for (size_t j= 0; j < 10; j++)
                for (size_t i= 0; i < 10; i++)
                    BOOST_CHECK_CLOSE( grid1->getCellVolume(i,j,k) , 0.25*0.25*0.25 , 0.001 );
    }
    {
        BOOST_CHECK_THROW( grid1->getCellCenter(1000) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellCenter(10,0,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellCenter(0,10,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1->getCellCenter(0,0,10) , std::invalid_argument);

        for (size_t k= 0; k < 10; k++)
            for (size_t j= 0; j < 10; j++)
                for (size_t i= 0; i < 10; i++) {
                    auto pos = grid1->getCellCenter(i,j,k);

                    BOOST_CHECK_CLOSE( std::get<0>(pos) , i*0.25 + 0.125, 0.001);
                    BOOST_CHECK_CLOSE( std::get<1>(pos) , j*0.25 + 0.125, 0.001);
                    BOOST_CHECK_CLOSE( std::get<2>(pos) , k*0.25 + 0.125 + 0.25, 0.001);

                }
    }
}



BOOST_AUTO_TEST_CASE(HasCPKeywords) {
    Opm::DeckPtr deck = createCPDeck();
    BOOST_CHECK(  Opm::EclipseGrid::hasCornerPointKeywords( *deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( *deck ));
}


BOOST_AUTO_TEST_CASE(HasCartKeywords) {
    Opm::DeckPtr deck = createCARTDeck();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( *deck ));
    BOOST_CHECK(  Opm::EclipseGrid::hasCartesianKeywords( *deck ));
}


BOOST_AUTO_TEST_CASE(HasCartKeywordsDEPTHZ) {
    Opm::DeckPtr deck = createCARTDeckDEPTHZ();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( *deck ));
    BOOST_CHECK(  Opm::EclipseGrid::hasCartesianKeywords( *deck ));
}


BOOST_AUTO_TEST_CASE(HasINVALIDCartKeywords) {
    Opm::DeckPtr deck = createCARTInvalidDeck();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( *deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( *deck ));
}





BOOST_AUTO_TEST_CASE(CreateMissingGRID_throws) {
    Opm::DeckPtr deck = createDeckHeaders();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK_EQUAL( false , grid.hasCellInfo() );
}


static Opm::DeckPtr createInvalidDXYZCARTDeck() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "99*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "TOPS\n"
        "1000*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}



BOOST_AUTO_TEST_CASE(CreateCartesianGRID) {
    Opm::DeckPtr deck = createInvalidDXYZCARTDeck();
    BOOST_CHECK_THROW(new Opm::EclipseGrid( deck ) , std::invalid_argument);
}


static Opm::DeckPtr createInvalidDXYZCARTDeckDEPTHZ() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "DX\n"
        "100*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "1000*0.25 /\n"
        "DEPTHZ\n"
        "101*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}



BOOST_AUTO_TEST_CASE(CreateCartesianGRIDDEPTHZ) {
    Opm::DeckPtr deck = createInvalidDXYZCARTDeckDEPTHZ();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK_EQUAL( false , grid.hasCellInfo() );
}


static Opm::DeckPtr createOnlyTopDZCartGrid() {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 5 20 /\n"
        "GRID\n"
        "DX\n"
        "1000*0.25 /\n"
        "DY\n"
        "1000*0.25 /\n"
        "DZ\n"
        "101*0.25 /\n"
        "TOPS\n"
        "110*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


static Opm::DeckPtr createInvalidDEPTHZDeck1 () {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 5 20 /\n"
        "GRID\n"
        "DXV\n"
        "1000*0.25 /\n"
        "DYV\n"
        "5*0.25 /\n"
        "DZV\n"
        "20*0.25 /\n"
        "DEPTHZ\n"
        "66*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}


BOOST_AUTO_TEST_CASE(CreateCartesianGRIDInvalidDEPTHZ1) {
    Opm::DeckPtr deck = createInvalidDEPTHZDeck1();
    BOOST_CHECK_THROW(new Opm::EclipseGrid( deck ) , std::invalid_argument);
}


static Opm::DeckPtr createInvalidDEPTHZDeck2 () {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 5 20 /\n"
        "GRID\n"
        "DXV\n"
        "10*0.25 /\n"
        "DYV\n"
        "5*0.25 /\n"
        "DZV\n"
        "20*0.25 /\n"
        "DEPTHZ\n"
        "67*0.25 /\n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext()) ;
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRIDInvalidDEPTHZ2) {
    Opm::DeckPtr deck = createInvalidDEPTHZDeck2();
    BOOST_CHECK_THROW(new Opm::EclipseGrid( deck ) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(CreateCartesianGRIDOnlyTopLayerDZ) {
    Opm::DeckPtr deck = createOnlyTopDZCartGrid();
    std::shared_ptr<Opm::EclipseGrid> grid(new Opm::EclipseGrid( deck ));

    BOOST_CHECK_EQUAL( 10 , grid->getNX( ));
    BOOST_CHECK_EQUAL(  5 , grid->getNY( ));
    BOOST_CHECK_EQUAL( 20 , grid->getNZ( ));
    BOOST_CHECK_EQUAL( 1000 , grid->getNumActive());
}



BOOST_AUTO_TEST_CASE(AllActiveExportActnum) {
    Opm::DeckPtr deck = createOnlyTopDZCartGrid();
    std::shared_ptr<Opm::EclipseGrid> grid(new Opm::EclipseGrid( deck ));

    std::vector<int> actnum;

    actnum.push_back(100);

    grid->exportACTNUM( actnum );
    BOOST_CHECK_EQUAL( 0U , actnum.size());
}


BOOST_AUTO_TEST_CASE(CornerPointSizeMismatchCOORD) {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  725*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseString(deckData, Opm::ParseContext()) ;
    const auto& zcorn = deck->getKeyword("ZCORN");
    BOOST_CHECK_EQUAL( 8000U , zcorn.getDataSize( ));

    BOOST_CHECK_THROW(Opm::EclipseGrid( std::move(deck) ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(CornerPointSizeMismatchZCORN) {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8001*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseString(deckData, Opm::ParseContext()) ;
    BOOST_CHECK_THROW((void)Opm::EclipseGrid(deck), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(ResetACTNUM) {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseString(deckData, Opm::ParseContext()) ;

    Opm::EclipseGrid grid(deck);
    BOOST_CHECK_EQUAL( 1000U , grid.getNumActive());
    std::vector<int> actnum(1000);
    actnum[0] = 1;
    grid.resetACTNUM( actnum.data() );
    BOOST_CHECK_EQUAL( 1U , grid.getNumActive() );

    grid.resetACTNUM( NULL );
    BOOST_CHECK_EQUAL( 1000U , grid.getNumActive() );
}


BOOST_AUTO_TEST_CASE(LoadFromBinary) {
    BOOST_CHECK_THROW(Opm::EclipseGrid( "No/does/not/exist" ) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(Fwrite) {
    const char *deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseString(deckData, Opm::ParseContext()) ;
    Opm::EclipseGrid grid1(deck );

    grid1.fwriteEGRID( "TEST.EGRID" , true);

    Opm::EclipseGrid grid2( "TEST.EGRID" );

    BOOST_CHECK( grid1.equal( grid2 ));
    remove("TEST.EGRID");
}




BOOST_AUTO_TEST_CASE(ConstructorNORUNSPEC) {
    const char *deckData =
        "GRID\n"
        "SPECGRID \n"
        "  10 10 10 / \n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "EDIT\n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck1 = parser->parseString(deckData, Opm::ParseContext()) ;
    Opm::DeckConstPtr deck2 = createCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(grid1.equal( grid2 ));
}



BOOST_AUTO_TEST_CASE(ConstructorNoSections) {
    const char *deckData =
        "DIMENS \n"
        "  10 10 10 / \n"
        "COORD \n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck1 = parser->parseString(deckData, Opm::ParseContext()) ;
    Opm::DeckConstPtr deck2 = createCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(grid1.equal( grid2 ));
}



BOOST_AUTO_TEST_CASE(ConstructorNORUNSPEC_PINCH) {
    Opm::DeckConstPtr deck1 = createCPDeck();
    Opm::DeckConstPtr deck2 = createPinchedCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(!grid1.equal( grid2 ));
    BOOST_CHECK(!grid1.isPinchActive());
    BOOST_CHECK_THROW(grid1.getPinchThresholdThickness(), std::logic_error);
    BOOST_CHECK(grid2.isPinchActive());
    BOOST_CHECK_EQUAL(grid2.getPinchThresholdThickness(), 0.2);
}




BOOST_AUTO_TEST_CASE(ConstructorMINPV) {
    Opm::DeckConstPtr deck1 = createCPDeck();
    Opm::DeckConstPtr deck2 = createMinpvDefaultCPDeck();
    Opm::DeckConstPtr deck3 = createMinpvCPDeck();
    Opm::DeckConstPtr deck4 = createMinpvFilCPDeck();

    Opm::EclipseGrid grid1(deck1);
    BOOST_CHECK_THROW(Opm::EclipseGrid grid2(deck2), std::invalid_argument);
    Opm::EclipseGrid grid3(deck3);
    Opm::EclipseGrid grid4(deck4);

    BOOST_CHECK(!grid1.equal( grid3 ));
    BOOST_CHECK_EQUAL(grid1.getMinpvMode(), Opm::MinpvMode::ModeEnum::Inactive);
    BOOST_CHECK_EQUAL(grid3.getMinpvMode(), Opm::MinpvMode::ModeEnum::EclSTD);
    BOOST_CHECK_EQUAL(grid3.getMinpvValue(), 10.0);
    BOOST_CHECK_EQUAL(grid4.getMinpvMode(), Opm::MinpvMode::ModeEnum::OpmFIL);
    BOOST_CHECK_EQUAL(grid4.getMinpvValue(), 20.0);
}


static Opm::DeckPtr createActnumDeck() {
    const char *deckData = "RUNSPEC\n"
            "\n"
            "DIMENS \n"
            "  2 2 2 / \n"
            "GRID\n"
            "DXV\n"
            "  2*0.25 /\n"
            "DYV\n"
            "  2*0.25 /\n"
            "DZV\n"
            "  2*0.25 /\n"
            "DEPTHZ\n"
            "  9*0.25 /\n"
            "EQUALS\n"
            " ACTNUM 0 1 1 1 1 1 1 /\n"
            "/ \n"
            "FLUXNUM\n"
            "8*0 /\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}


/// creates a deck where the top-layer has ACTNUM = 0 and two partially
/// overlapping 2*2*2 boxes in the center, one [5,7]^3 and one [6,8]^3
/// have ACTNUM = 0
static Opm::DeckPtr createActnumBoxDeck() {
    const char *deckData = "RUNSPEC\n"
            "\n"
            "DIMENS \n"
            "  10 10 10 / \n"
            "GRID\n"
            "DXV\n"
            "  10*0.25 /\n"
            "DYV\n"
            "  10*0.25 /\n"
            "DZV\n"
            "  10*0.25 /\n"
            "DEPTHZ\n"
            "  121*0.25 /\n"
            "EQUALS\n"
            " ACTNUM 0 1 10 1 10 1 1 /\n" // disable top layer
            "/ \n"
            // start box
            "BOX\n"
            "  5 7 5 7 5 7 /\n"
            "ACTNUM \n"
            "    0 0 0 0 0 0 0 0 0\n"
            "    0 0 0 0 0 0 0 0 0\n"
            "    0 0 0 0 0 0 0 0 0\n"
            "/\n"
            "BOX\n" // don't need ENDBOX
            "  6 8 6 8 6 8 /\n"
            "ACTNUM \n"
            "    27*0\n"
            "/\n"
            "ENDBOX\n"
            // end   box
            "FLUXNUM\n"
            "1000*0 /\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}

BOOST_AUTO_TEST_CASE(GridBoxActnum) {
    Opm::DeckConstPtr deck = createActnumBoxDeck();
    Opm::EclipseState es(deck, Opm::ParseContext());
    auto ep = es.get3DProperties();
    auto grid = es.getInputGrid();

    BOOST_CHECK_NO_THROW(ep.getIntGridProperty("ACTNUM"));

    size_t active = 10 * 10 * 10     // 1000
                    - (10 * 10 * 1)  // - top layer
                    - ( 3 *  3 * 3)  // - [5,7]^3 box
                    - ( 3 *  3 * 3)  // - [6,8]^3 box
                    + ( 2 *  2 * 2); // + inclusion/exclusion

    BOOST_CHECK_NO_THROW(grid->getNumActive());
    BOOST_CHECK_EQUAL(grid->getNumActive(), active);

    BOOST_CHECK_EQUAL(es.getInputGrid()->getNumActive(), active);

    for (size_t x = 0; x < 10; x++) {
        for (size_t y = 0; y < 10; y++) {
            for (size_t z = 0; z < 10; z++) {
                if (z == 0)
                    BOOST_CHECK(!grid->cellActive(x, y, z));
                else if (x >= 4 && x <= 6 && y >= 4 && y <= 6 && z >= 4 && z <= 6)
                    BOOST_CHECK(!grid->cellActive(x, y, z));
                else if (x >= 5 && x <= 7 && y >= 5 && y <= 7 && z >= 5 && z <= 7)
                    BOOST_CHECK(!grid->cellActive(x, y, z));
                else
                    BOOST_CHECK(grid->cellActive(x, y, z));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(GridActnumVia3D) {
    Opm::DeckConstPtr deck = createActnumDeck();

    Opm::EclipseState es(deck, Opm::ParseContext());
    auto ep = es.get3DProperties();
    auto grid = es.getInputGrid();

    BOOST_CHECK_NO_THROW(ep.getIntGridProperty("ACTNUM"));

    BOOST_CHECK_NO_THROW(grid->getNumActive());
    BOOST_CHECK(grid->hasCellInfo());
    BOOST_CHECK_EQUAL(grid->getNumActive(), 2 * 2 * 2 - 1);
}

BOOST_AUTO_TEST_CASE(GridActnumViaState) {
    Opm::DeckConstPtr deck = createActnumDeck();

    BOOST_CHECK_NO_THROW(Opm::EclipseState(deck, Opm::ParseContext()));
    Opm::EclipseState es(deck, Opm::ParseContext());
    BOOST_CHECK(es.getInputGrid()->hasCellInfo());
    BOOST_CHECK_EQUAL(es.getInputGrid()->getNumActive(), 2 * 2 * 2 - 1);
}
