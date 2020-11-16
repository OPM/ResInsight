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

#include <cstdio>
#include <ctime>
#include <iostream>
#include <math.h>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <unistd.h>

#include <opm/common/utility/FileSystem.hpp>

#define BOOST_TEST_MODULE EclipseGridTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridDims.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/io/eclipse/EclFile.hpp>

BOOST_AUTO_TEST_CASE(CreateMissingDIMENS_throws) {
    Opm::Deck deck;
    Opm::Parser parser;
    deck.addKeyword( Opm::DeckKeyword( parser.getKeyword("RUNSPEC" )));
    deck.addKeyword( Opm::DeckKeyword( parser.getKeyword("GRID" )));
    deck.addKeyword( Opm::DeckKeyword( parser.getKeyword("EDIT" )));

    BOOST_CHECK_THROW(Opm::EclipseGrid{ deck } , std::invalid_argument);
}

static Opm::Deck createDeckHeaders() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "GRID\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck createDeckDIMENS() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 13 17 19/\n"
        "GRID\n"
        "EDIT\n"
        "\n";
    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck createDeckSPECGRID() {
    const char* deckData =
        "GRID\n"
        "SPECGRID \n"
        "  13 17 19 / \n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "EDIT\n"
        "\n";
    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck createDeckMissingDIMS() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "GRID\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(MissingDimsThrows) {
    Opm::Deck deck = createDeckMissingDIMS();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(HasGridKeywords) {
    Opm::Deck deck = createDeckHeaders();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( deck ));
}

BOOST_AUTO_TEST_CASE(CreateGridNoCells) {
    Opm::Deck deck = createDeckHeaders();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);

    const Opm::GridDims grid( deck);
    BOOST_CHECK_EQUAL( 10 , grid.getNX());
    BOOST_CHECK_EQUAL( 10 , grid.getNY());
    BOOST_CHECK_EQUAL( 10 , grid.getNZ());

    BOOST_CHECK_EQUAL(10, grid[0]);
    BOOST_CHECK_EQUAL(10, grid[2]);
    BOOST_CHECK_THROW( grid[10], std::invalid_argument);

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

static Opm::Deck createCPDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

static Opm::Deck createPinchedCPDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}


static Opm::Deck createMinpvDefaultCPDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}


static Opm::Deck createMinpvCPDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}



static Opm::Deck createCARTDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}


static Opm::Deck createCARTDeckDEPTHZ() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}


static Opm::Deck createCARTInvalidDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CREATE_SIMPLE) {
    Opm::EclipseGrid grid(10,20,30);

    BOOST_CHECK_EQUAL( grid.getNX() , 10 );
    BOOST_CHECK_EQUAL( grid.getNY() , 20 );
    BOOST_CHECK_EQUAL( grid.getNZ() , 30 );
    BOOST_CHECK_EQUAL( grid.getCartesianSize() , 6000 );
}

BOOST_AUTO_TEST_CASE(DEPTHZ_EQUAL_TOPS) {
    Opm::Deck deck1 = createCARTDeck();
    Opm::Deck deck2 = createCARTDeckDEPTHZ();

    Opm::EclipseGrid grid1( deck1 );
    Opm::EclipseGrid grid2( deck2 );

    BOOST_CHECK( grid1.equal( grid2 ) );

    {
        BOOST_CHECK_THROW( grid1.getCellVolume(1000) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellVolume(10,0,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellVolume(0,10,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellVolume(0,0,10) , std::invalid_argument);

        for (size_t g=0; g < 1000; g++)
            BOOST_CHECK_CLOSE( grid1.getCellVolume(g) , 0.25*0.25*0.25 , 0.001);


        for (size_t k= 0; k < 10; k++)
            for (size_t j= 0; j < 10; j++)
                for (size_t i= 0; i < 10; i++)
                    BOOST_CHECK_CLOSE( grid1.getCellVolume(i, j, k) , 0.25*0.25*0.25 , 0.001 );

    }

    {
        BOOST_CHECK_THROW( grid1.getCellCenter(1000) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellCenter(10,0,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellCenter(0,10,0) , std::invalid_argument);
        BOOST_CHECK_THROW( grid1.getCellCenter(0,0,10) , std::invalid_argument);

        for (size_t k= 0; k < 10; k++) {
            for (size_t j= 0; j < 10; j++) {
                for (size_t i= 0; i < 10; i++) {
                    auto pos = grid1.getCellCenter(i, j, k);

                    BOOST_CHECK_CLOSE( std::get<0>(pos) , i*0.25 + 0.125, 0.001);
                    BOOST_CHECK_CLOSE( std::get<1>(pos) , j*0.25 + 0.125, 0.001);
                    BOOST_CHECK_CLOSE( std::get<2>(pos) , k*0.25 + 0.125 + 0.25, 0.001);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(HasCPKeywords) {
    Opm::Deck deck = createCPDeck();
    BOOST_CHECK(  Opm::EclipseGrid::hasCornerPointKeywords( deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( deck ));
}

BOOST_AUTO_TEST_CASE(HasCartKeywords) {
    Opm::Deck deck = createCARTDeck();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( deck ));
    BOOST_CHECK(  Opm::EclipseGrid::hasCartesianKeywords( deck ));
}

BOOST_AUTO_TEST_CASE(HasCartKeywordsDEPTHZ) {
    Opm::Deck deck = createCARTDeckDEPTHZ();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( deck ));
    BOOST_CHECK(  Opm::EclipseGrid::hasCartesianKeywords( deck ));
}

BOOST_AUTO_TEST_CASE(HasINVALIDCartKeywords) {
    Opm::Deck deck = createCARTInvalidDeck();
    BOOST_CHECK( !Opm::EclipseGrid::hasCornerPointKeywords( deck ));
    BOOST_CHECK( !Opm::EclipseGrid::hasCartesianKeywords( deck ));
}

BOOST_AUTO_TEST_CASE(CreateMissingGRID_throws) {
    auto deck= createDeckHeaders();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

static Opm::Deck createInvalidDXYZCARTDeck() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRID) {
    auto deck = createInvalidDXYZCARTDeck();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

static Opm::Deck createInvalidDXYZCARTDeckDEPTHZ() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRIDDEPTHZ) {
    auto deck = createInvalidDXYZCARTDeckDEPTHZ();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

static Opm::Deck createOnlyTopDZCartGrid() {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}


static Opm::Deck createInvalidDEPTHZDeck1 () {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRIDInvalidDEPTHZ1) {
    auto deck = createInvalidDEPTHZDeck1();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

static Opm::Deck createInvalidDEPTHZDeck2 () {
    const char* deckData =
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

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRIDInvalidDEPTHZ2) {
    auto deck = createInvalidDEPTHZDeck2();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(CreateCartesianGRIDOnlyTopLayerDZ) {
    Opm::Deck deck = createOnlyTopDZCartGrid();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK_EQUAL( 10 , grid.getNX( ));
    BOOST_CHECK_EQUAL(  5 , grid.getNY( ));
    BOOST_CHECK_EQUAL( 20 , grid.getNZ( ));
    BOOST_CHECK_EQUAL( 1000 , grid.getNumActive());
}

BOOST_AUTO_TEST_CASE(AllActiveExportActnum) {
    Opm::Deck deck = createOnlyTopDZCartGrid();
    Opm::EclipseGrid grid( deck );

    std::vector<int> actnum = grid.getACTNUM();

    BOOST_CHECK_EQUAL( 1000 , actnum.size());
}

BOOST_AUTO_TEST_CASE(CornerPointSizeMismatchCOORD) {
    const char* deckData =
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

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;
    const auto& zcorn = deck.getKeyword("ZCORN");
    BOOST_CHECK_EQUAL( 8000U , zcorn.getDataSize( ));

    BOOST_CHECK_THROW(Opm::EclipseGrid{ deck }, std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(CornerPointSizeMismatchZCORN) {
    const char* deckData =
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

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;
    BOOST_CHECK_THROW(Opm::EclipseGrid{ deck }, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(ResetACTNUM) {
    const char* deckData =
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

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    Opm::EclipseGrid grid( deck);
    BOOST_CHECK_EQUAL( 1000U , grid.getNumActive());
    std::vector<int> actnum(1000);
    actnum[0] = 1;
    actnum[2] = 1;
    actnum[4] = 1;
    actnum[6] = 1;

    grid.resetACTNUM( actnum );

    BOOST_CHECK_EQUAL( 4U , grid.getNumActive() );
    {
        std::vector<int> full(grid.getCartesianSize());
        std::iota(full.begin(), full.end(), 0);

        auto compressed = grid.compressedVector( full );

        BOOST_CHECK_EQUAL( compressed.size() , 4U );
        BOOST_CHECK_EQUAL( compressed[0] , 0 );
        BOOST_CHECK_EQUAL( compressed[1] , 2 );
        BOOST_CHECK_EQUAL( compressed[2] , 4 );
        BOOST_CHECK_EQUAL( compressed[3] , 6 );
    }

    {
        const auto& activeMap = grid.getActiveMap( );
        BOOST_CHECK_EQUAL( 4U , activeMap.size() );
        BOOST_CHECK_EQUAL( 0 , activeMap[0] );
        BOOST_CHECK_EQUAL( 2 , activeMap[1] );
        BOOST_CHECK_EQUAL( 4 , activeMap[2] );
        BOOST_CHECK_EQUAL( 6 , activeMap[3] );
    }

    grid.resetACTNUM();

    BOOST_CHECK_EQUAL( 1000U , grid.getNumActive() );
    {
        const auto&  activeMap = grid.getActiveMap( );
        BOOST_CHECK_EQUAL( 1000U , activeMap.size() );
        BOOST_CHECK_EQUAL( 0 , activeMap[0] );
        BOOST_CHECK_EQUAL( 1 , activeMap[1] );
        BOOST_CHECK_EQUAL( 2 , activeMap[2] );
        BOOST_CHECK_EQUAL( 999 , activeMap[999] );
    }

    actnum.assign(1000, 1);

    actnum[0] = 0;
    actnum[1] = 0;
    actnum[2] = 0;
    actnum[11] = 0;
    actnum[21] = 0;
    actnum[430] = 0;
    actnum[431] = 0;

    grid.resetACTNUM( actnum );

    std::vector<int> actMap = grid.getActiveMap();

    BOOST_CHECK_EQUAL(actMap.size(), 993);
    BOOST_CHECK_THROW(grid.getGlobalIndex(993), std::out_of_range);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(0), 3);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(33), 38);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(450), 457);
    BOOST_CHECK_EQUAL(grid.getGlobalIndex(1,2,3), 321);
}

BOOST_AUTO_TEST_CASE(TestCP_example) {
    const char* deckData =

    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    " 3 2 1 /\n"
    "GRID\n"
    "COORD\n"
    " 2000.0000  2000.0000  2000.0000   1999.9476  2000.0000  2002.9995\n"
    " 2049.9924  2000.0000  2000.8726   2049.9400  2000.0000  2003.8722 \n"
    " 2099.9848  2000.0000  2001.7452   2099.9324  2000.0000  2004.7448 \n"
    " 2149.9772  2000.0000  2002.6179   2149.9248  2000.0000  2005.6174 \n"
    " 2000.0000  2050.0000  2000.0000   1999.9476  2050.0000  2002.9995 \n"
    " 2049.9924  2050.0000  2000.8726   2049.9400  2050.0000  2003.8722 \n"
    " 2099.9848  2050.0000  2001.7452   2099.9324  2050.0000  2004.7448 \n"
    " 2149.9772  2050.0000  2002.6179   2149.9248  2050.0000  2005.6174 \n"
    " 2000.0000  2100.0000  2000.0000   1999.9476  2100.0000  2002.9995 \n"
    " 2049.9924  2100.0000  2000.8726   2049.9400  2100.0000  2003.8722 \n"
    " 2099.9848  2100.0000  2001.7452   2099.9324  2100.0000  2004.7448 \n"
    " 2149.9772  2100.0000  2002.6179   2149.9248  2100.0000  2005.6174 / \n"
    "ZCORN\n"
    " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
    " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
    " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
    " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
    " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
    " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
    " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
    " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 / \n"
    "EDIT\n"
    "\n";

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    Opm::EclipseGrid grid( deck);
    BOOST_CHECK_EQUAL( 6U , grid.getNumActive());

    std::vector<int> actnum(6, 0);
    actnum[0] = 1;
    actnum[2] = 1;
    actnum[4] = 1;

    grid.resetACTNUM( actnum );

    BOOST_CHECK_EQUAL( 3U , grid.getNumActive() );
}



BOOST_AUTO_TEST_CASE(ConstructorNORUNSPEC) {
    const char* deckData =
        "GRID\n"
        "SPECGRID \n"
        "  10 10 10 / \n"
        "COORD\n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "PORO\n"
        "  1000*0.15 /\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    auto deck1 = parser.parseString( deckData) ;
    auto deck2 = createCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(grid1.equal( grid2 ));
}

BOOST_AUTO_TEST_CASE(ConstructorNoSections) {
    const char* deckData =
        "DIMENS \n"
        "  10 10 10 / \n"
        "COORD \n"
        "  726*1 / \n"
        "ZCORN \n"
        "  8000*1 / \n"
        "ACTNUM \n"
        "  1000*1 / \n"
        "PORO\n"
        "  1000*0.15 /\n"
        "\n";

    Opm::Parser parser;
    auto deck1 = parser.parseString( deckData) ;
    auto deck2 = createCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(grid1.equal( grid2 ));
}

BOOST_AUTO_TEST_CASE(ConstructorNORUNSPEC_PINCH) {
    auto deck1 = createCPDeck();
    auto deck2 = createPinchedCPDeck();

    Opm::EclipseGrid grid1(deck1);
    Opm::EclipseGrid grid2(deck2);

    BOOST_CHECK(!grid1.equal( grid2 ));

    BOOST_CHECK(!grid1.isPinchActive());
    BOOST_CHECK_THROW(grid1.getPinchThresholdThickness(), std::logic_error);
    BOOST_CHECK(grid2.isPinchActive());
    BOOST_CHECK_EQUAL(grid2.getPinchThresholdThickness(), 0.2);
}

BOOST_AUTO_TEST_CASE(ConstructorMINPV) {
    auto deck1 = createCPDeck();
    auto deck2 = createMinpvDefaultCPDeck();
    auto deck3 = createMinpvCPDeck();

    Opm::EclipseGrid grid1(deck1);
    BOOST_CHECK_THROW(Opm::EclipseGrid grid2(deck2), std::invalid_argument);
    Opm::EclipseGrid grid3(deck3);

    BOOST_CHECK(!grid1.equal( grid3 ));
    BOOST_CHECK_EQUAL(grid1.getMinpvMode(), Opm::MinpvMode::ModeEnum::Inactive);
    BOOST_CHECK_EQUAL(grid3.getMinpvMode(), Opm::MinpvMode::ModeEnum::EclSTD);
    BOOST_CHECK_EQUAL(grid3.getMinpvVector()[0], 10.0);
}

static Opm::Deck createActnumDeck() {
    const char* deckData = "RUNSPEC\n"
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
            "PORO\n"
            "  8*0.15 /\n"
            "FLUXNUM\n"
            "8*0 /\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}


/// creates a deck where the top-layer has ACTNUM = 0 and two partially
/// overlapping 2*2*2 boxes in the center, one [5,7]^3 and one [6,8]^3
/// have ACTNUM = 0
static Opm::Deck createActnumBoxDeck() {
    const char* deckData = "RUNSPEC\n"
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
            "PORO \n"
            "  1000*0.15 /\n"
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

    Opm::Parser parser;
    return parser.parseString( deckData);
}

BOOST_AUTO_TEST_CASE(GridBoxActnum) {
    auto deck = createActnumBoxDeck();
    Opm::EclipseState es( deck);
    const auto& fp = es.fieldProps();
    const auto& grid = es.getInputGrid();

    BOOST_CHECK_NO_THROW(fp.get_int("ACTNUM"));

    size_t active = 10 * 10 * 10     // 1000
                    - (10 * 10 * 1)  // - top layer
                    - ( 3 *  3 * 3)  // - [5,7]^3 box
                    - ( 3 *  3 * 3)  // - [6,8]^3 box
                    + ( 2 *  2 * 2); // + inclusion/exclusion

    BOOST_CHECK_NO_THROW(grid.getNumActive());
    BOOST_CHECK_EQUAL(grid.getNumActive(), active);

    BOOST_CHECK_EQUAL(es.getInputGrid().getNumActive(), active);

    {
        size_t active_index = 0;
        // NB: The implementation of this test actually assumes that
        //     the loops are running with z as the outer and x as the
        //     inner direction.
        for (size_t z = 0; z < grid.getNZ(); z++) {
            for (size_t y = 0; y < grid.getNY(); y++) {
                for (size_t x = 0; x < grid.getNX(); x++) {
                    if (z == 0)
                        BOOST_CHECK(!grid.cellActive(x, y, z));
                    else if (x >= 4 && x <= 6 && y >= 4 && y <= 6 && z >= 4 && z <= 6)
                        BOOST_CHECK(!grid.cellActive(x, y, z));
                    else if (x >= 5 && x <= 7 && y >= 5 && y <= 7 && z >= 5 && z <= 7)
                        BOOST_CHECK(!grid.cellActive(x, y, z));
                    else {
                        size_t g = grid.getGlobalIndex( x,y,z );

                        BOOST_CHECK(grid.cellActive(x, y, z));
                        BOOST_CHECK_EQUAL( grid.activeIndex(x,y,z) , active_index );
                        BOOST_CHECK_EQUAL( grid.activeIndex(g) , active_index );

                        active_index++;
                    }
                }
            }
        }

        BOOST_CHECK_THROW( grid.activeIndex(0,0,0) , std::invalid_argument );
    }
}

BOOST_AUTO_TEST_CASE(GridActnumVia3D) {
    auto deck = createActnumDeck();

    Opm::EclipseState es( deck);
    const auto& fp = es.fieldProps();
    const auto& grid = es.getInputGrid();
    Opm::EclipseGrid grid2( grid );

    std::vector<int> actnum = {1, 1, 0, 1, 1, 0, 1, 1};
    Opm::EclipseGrid grid3( grid , actnum);

    BOOST_CHECK_NO_THROW(fp.get_int("ACTNUM"));
    BOOST_CHECK_NO_THROW(grid.getNumActive());
    BOOST_CHECK_EQUAL(grid.getNumActive(), 2 * 2 * 2 - 1);

    BOOST_CHECK_NO_THROW(grid2.getNumActive());
    BOOST_CHECK_EQUAL(grid2.getNumActive(), 2 * 2 * 2 - 1);

    BOOST_CHECK_EQUAL(grid3.getNumActive(), 6);
}


BOOST_AUTO_TEST_CASE(GridActnumViaState) {
    auto deck = createActnumDeck();

    BOOST_CHECK_NO_THROW( std::unique_ptr<Opm::EclipseState>(new Opm::EclipseState( deck)));
    Opm::EclipseState es( deck);
    BOOST_CHECK_EQUAL(es.getInputGrid().getNumActive(), 2 * 2 * 2 - 1);
}


BOOST_AUTO_TEST_CASE(GridDimsSPECGRID) {
    auto deck =  createDeckSPECGRID();
    auto gd = Opm::GridDims( deck );
    BOOST_CHECK_EQUAL(gd.getNX(), 13);
    BOOST_CHECK_EQUAL(gd.getNY(), 17);
    BOOST_CHECK_EQUAL(gd.getNZ(), 19);
}


BOOST_AUTO_TEST_CASE(GridDimsDIMENS) {
    auto deck =  createDeckDIMENS();
    auto gd = Opm::GridDims( deck );
    BOOST_CHECK_EQUAL(gd.getNX(), 13);
    BOOST_CHECK_EQUAL(gd.getNY(), 17);
    BOOST_CHECK_EQUAL(gd.getNZ(), 19);
}


BOOST_AUTO_TEST_CASE(ProcessedCopy) {
    Opm::EclipseGrid gd(10,10,10);
    std::vector<double> zcorn;
    std::vector<int> actnum;

    zcorn = gd.getZCORN();
    actnum = gd.getACTNUM();

    Opm::EclipseGrid gd1(gd , actnum );
    BOOST_CHECK( gd.equal( gd1 ));
    {
        Opm::EclipseGrid gd2(gd , zcorn.data() , actnum );
        BOOST_CHECK( gd.equal( gd2 ));
    }

    zcorn[0] -= 1.0;
    {
        Opm::EclipseGrid gd2(gd , zcorn.data() , actnum );
        BOOST_CHECK( !gd.equal( gd2 ));
    }

    {
        Opm::EclipseGrid gd2(gd , actnum );
        BOOST_CHECK( gd.equal( gd2 ));
    }

    actnum.assign( gd.getCartesianSize() , 1);
    actnum[0] = 0;
    {
        Opm::EclipseGrid gd2(gd , actnum );
        BOOST_CHECK( !gd.equal( gd2 ));
        BOOST_CHECK( !gd2.cellActive( 0 ));
    }
}

BOOST_AUTO_TEST_CASE(regularCartGrid) {

    int nx = 3;
    int ny = 4;
    int nz = 5;

    double dx = 25;
    double dy = 35;
    double dz = 2;

    double ref_volume = dx* dy* dz;

    Opm::EclipseGrid grid(nx, ny, nz, dx, dy, dz);

    std::array<int, 3> dims = grid.getNXYZ();

    int nCells = dims[0]*dims[1]*dims[2];

    for (int n=0; n<nCells; n++){
        BOOST_CHECK_CLOSE(grid.getCellVolume(n), ref_volume, 1e-12);
        BOOST_CHECK_CLOSE(grid.getCellThickness(n), 2.0, 1e-12);
    }

    for (int k=0; k< dims[2]; k++){
        double ref_depth = k*dz + dz/2.0;
        for (int j=0; j< dims[1]; j++){
            for (int i=0; i< dims[0]; i++){
                BOOST_CHECK_CLOSE(grid.getCellDepth(i,j,k), ref_depth, 1e-12);
            }
        }
    }

    for (int k=0; k< dims[2]; k++){
        double ref_z = k*dz + dz/2.0;
        for (int j=0; j< dims[1]; j++){
            double ref_y = j*dy + dy/2.0;
            for (int i=0; i< dims[0]; i++){
                double ref_x = i*dx + dx/2.0;
                std::array<double, 3> cc = grid.getCellCenter(i, j, k);
                BOOST_CHECK_CLOSE(cc[0], ref_x, 1e-12);
                BOOST_CHECK_CLOSE(cc[1], ref_y, 1e-12);
                BOOST_CHECK_CLOSE(cc[2], ref_z, 1e-12);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(ZcornMapper) {

    int nx = 3;
    int ny = 4;
    int nz = 5;

    Opm::EclipseGrid grid(nx, ny, nz);
    Opm::ZcornMapper zmp = grid.zcornMapper( );

    BOOST_CHECK_THROW(zmp.index(nx,1,1,0) , std::invalid_argument);
    BOOST_CHECK_THROW(zmp.index(0,ny,1,0) , std::invalid_argument);
    BOOST_CHECK_THROW(zmp.index(0,1,nz,0) , std::invalid_argument);
    BOOST_CHECK_THROW(zmp.index(0,1,2,8) , std::invalid_argument);

    auto points_adjusted = grid.fixupZCORN();

    std::vector<int> actnum = grid.getACTNUM();
    std::vector<double> zcorn = grid.getZCORN();

    zcorn[42] = zcorn[42] + 2.0;
    zcorn[96] = zcorn[96] + 2.0;

    Opm::EclipseGrid grid2(grid , zcorn.data() , actnum );
    points_adjusted = grid2.getZcornFixed();
    BOOST_CHECK_EQUAL( points_adjusted , 4 );

    points_adjusted = grid2.fixupZCORN();
    BOOST_CHECK_EQUAL( points_adjusted , 0 );

    zcorn = grid.getZCORN();

    BOOST_CHECK( zmp.validZCORN( zcorn ));

    // Manually destroy it - cell internal
    zcorn[ zmp.index(0,0,0,4) ] = zcorn[ zmp.index(0,0,0,0) ] - 0.1;
    BOOST_CHECK( !zmp.validZCORN( zcorn ));
    points_adjusted = zmp.fixupZCORN( zcorn );
    BOOST_CHECK_EQUAL( points_adjusted , 1 );
    BOOST_CHECK( zmp.validZCORN( zcorn ));

    // Manually destroy it - cell 2 cell
    zcorn[ zmp.index(0,0,0,4) ] = zcorn[ zmp.index(0,0,1,0) ] + 0.1;
    BOOST_CHECK( !zmp.validZCORN( zcorn ));
    points_adjusted = zmp.fixupZCORN( zcorn );
    BOOST_CHECK_EQUAL( points_adjusted , 1 );
    BOOST_CHECK( zmp.validZCORN( zcorn ));

    // Manually destroy it - cell 2 cell and cell internal
    zcorn[ zmp.index(0,0,0,4) ] = zcorn[ zmp.index(0,0,1,0) ] + 0.1;
    zcorn[ zmp.index(0,0,0,0) ] = zcorn[ zmp.index(0,0,0,4) ] + 0.1;
    BOOST_CHECK( !zmp.validZCORN( zcorn ));
    points_adjusted = zmp.fixupZCORN( zcorn );
    BOOST_CHECK_EQUAL( points_adjusted , 2 );
    BOOST_CHECK( zmp.validZCORN( zcorn ));
}

BOOST_AUTO_TEST_CASE(MoveTest) {
    int nx = 3;
    int ny = 4;
    int nz = 5;
    Opm::EclipseGrid grid1(nx,ny,nz);
    Opm::EclipseGrid grid2( std::move( grid1 )); // grid2 should be move constructed from grid1

    BOOST_CHECK( !grid1.circle( ));
}

static Opm::Deck radial_missing_INRAD() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 10 /\n"
        "RADIAL\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}



static Opm::Deck radial_keywords_OK() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 6 10 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "6*60 /\n"
        "DZV\n"
        "10*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  600*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck radial_keywords_OK_CIRCLE() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 6 10 /\n"
        "RADIAL\n"
        "GRID\n"
        "CIRCLE\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "6*60 /\n"
        "DZV\n"
        "10*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  600*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}



BOOST_AUTO_TEST_CASE(RadialTest) {
    Opm::Deck deck = radial_missing_INRAD();
    BOOST_CHECK_THROW( Opm::EclipseGrid{ deck }, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RadialKeywordsOK) {
    Opm::Deck deck = radial_keywords_OK();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK(!grid.circle());
}

BOOST_AUTO_TEST_CASE(RadialKeywordsOK_CIRCLE) {
    Opm::Deck deck = radial_keywords_OK_CIRCLE();
    Opm::EclipseGrid grid( deck );
    BOOST_CHECK(grid.circle());
}

static Opm::Deck radial_keywords_DRV_size_mismatch() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "10 6 12 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "9*1 /\n"
        "DTHETAV\n"
        "6*60 /\n"
        "DZV\n"
        "12*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  720*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}


static Opm::Deck radial_keywords_DZV_size_mismatch() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "10 6 12 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "6*60 /\n"
        "DZV\n"
        "11*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  720*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck radial_keywords_DTHETAV_size_mismatch() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "10 6 12 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "5*60 /\n"
        "DZV\n"
        "12*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  720*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

//  This is stricter than the ECLIPSE implementation; we assume that
//  *only* the top layer is explicitly given.

static Opm::Deck radial_keywords_TOPS_size_mismatch() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "10 6 12 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "6*60 /\n"
        "DZV\n"
        "12*0.25 /\n"
        "TOPS\n"
        "65*0.0 /\n"
        "PORO \n"
        "  720*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck radial_keywords_ANGLE_OVERFLOW() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "10 6 12 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "10*1 /\n"
        "DTHETAV\n"
        "6*70 /\n"
        "DZV\n"
        "12*0.25 /\n"
        "TOPS\n"
        "60*0.0 /\n"
        "PORO \n"
        "  720*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}


BOOST_AUTO_TEST_CASE(RadialKeywords_SIZE_ERROR) {
    BOOST_CHECK_THROW( Opm::EclipseGrid{ radial_keywords_DRV_size_mismatch() } , std::invalid_argument);
    BOOST_CHECK_THROW( Opm::EclipseGrid{ radial_keywords_DZV_size_mismatch() } , std::invalid_argument);
    BOOST_CHECK_THROW( Opm::EclipseGrid{ radial_keywords_TOPS_size_mismatch() } , std::invalid_argument);
    BOOST_CHECK_THROW( Opm::EclipseGrid{ radial_keywords_DTHETAV_size_mismatch() } , std::invalid_argument);
    BOOST_CHECK_THROW( Opm::EclipseGrid{ radial_keywords_ANGLE_OVERFLOW() } , std::invalid_argument);
}

static Opm::Deck radial_details() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "1 5 2 /\n"
        "RADIAL\n"
        "GRID\n"
        "INRAD\n"
        "1 /\n"
        "DRV\n"
        "1 /\n"
        "DTHETAV\n"
        "3*90 60 30/\n"
        "DZV\n"
        "2*1 /\n"
        "TOPS\n"
        "5*1.0 /\n"
        "PORO \n"
        "  10*0.15 /"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

BOOST_AUTO_TEST_CASE(RadialDetails) {
    Opm::Deck deck = radial_details();
    Opm::EclipseGrid grid( deck );

    BOOST_CHECK_CLOSE( grid.getCellVolume( 0 , 0 , 0 ) , 0.5*(2*2 - 1)*1, 0.0001);
    BOOST_CHECK_CLOSE( grid.getCellVolume( 0 , 3 , 0 ) , sqrt(3.0)*0.25*( 4 - 1 ) , 0.0001);
    auto pos0 = grid.getCellCenter(0,0,0);
    auto pos2 = grid.getCellCenter(0,2,0);

    BOOST_CHECK_CLOSE( std::get<0>(pos0) , 0.75 , 0.0001);
    BOOST_CHECK_CLOSE( std::get<1>(pos0) , 0.75 , 0.0001);
    BOOST_CHECK_CLOSE( std::get<2>(pos0) , 1.50 , 0.0001);

    BOOST_CHECK_CLOSE( std::get<0>(pos2) , -0.75 , 0.0001);
    BOOST_CHECK_CLOSE( std::get<1>(pos2) , -0.75 , 0.0001);
    BOOST_CHECK_CLOSE( std::get<2>(pos2) , 1.50 , 0.0001);

    {
        const auto& p0 = grid.getCornerPos( 0,0,0 , 0 );
        const auto& p6 = grid.getCornerPos( 0,0,0 , 6 );
        BOOST_CHECK_CLOSE( p0[0]*p0[0] + p0[1]*p0[1] , 1.0, 0.0001);
        BOOST_CHECK_CLOSE( p6[0]*p6[0] + p6[1]*p6[1] , 1.0, 0.0001);

        BOOST_CHECK_THROW( grid.getCornerPos( 0,0,0 , 8 ) , std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(CoordMapper) {
    size_t nx = 10;
    size_t ny = 7;
    Opm::CoordMapper cmp = Opm::CoordMapper( nx , ny );
    BOOST_CHECK_THROW( cmp.index(12,6,0,0), std::invalid_argument );
    BOOST_CHECK_THROW( cmp.index(10,8,0,0), std::invalid_argument );
    BOOST_CHECK_THROW( cmp.index(10,7,5,0), std::invalid_argument );
    BOOST_CHECK_THROW( cmp.index(10,5,1,2), std::invalid_argument );

    BOOST_CHECK_EQUAL( cmp.index(10,7,2,1) + 1 , cmp.size( ));
}

static Opm::Deck createCARTDeckTest3x4x2() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 3 4 2 /\n"
        "GRID\n"
        "DX\n"
        "100 120 110 100 120 110 100 120 110 100 120 110 /\n"
        "DY\n"
        "70  80  85  80  70  80  85  80  70  80  85  80 /\n"
        "DZ\n"
        "12*25 12*35 /\n"
        "TOPS\n"
        "2500 2510 2520  2520 2530 2540  2540 2550 2560  2560 2570 2580 /\n"
        "PORO \n"
        "  24*0.15 /"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CART_Deck_3x4x2) {

    Opm::Deck deck1 = createCARTDeckTest3x4x2();
    Opm::EclipseGrid grid1( deck1 );

    const std::vector<double> t_coord = grid1.getCOORD();
    const std::vector<double> t_zcorn = grid1.getZCORN();

    std::vector<float> ref_coord = {0, 0, 2500, 0, 0, 2560, 100, 0, 2510, 100, 0, 2570, 220, 0, 2520, 220, 0,
        2580, 330, 0, 2520, 330, 0, 2580, 0, 70, 2520, 0, 70, 2580, 100, 80, 2530, 100, 80, 2590, 220, 85,
        2540, 220, 85, 2600, 330, 85, 2540, 330, 85, 2600, 0, 150, 2540, 0, 150, 2600, 100, 150, 2550, 100,
        150, 2610, 220, 165, 2560, 220, 165, 2620, 330, 165, 2560, 330, 165, 2620, 0, 235, 2560, 0, 235,
        2620, 100, 230, 2570, 100, 230, 2630, 220, 235, 2580, 220, 235, 2640, 330, 235, 2580, 330, 235,
        2640, 0, 315, 2560, 0, 315, 2620, 100, 315, 2570, 100, 315, 2630, 220, 315, 2580, 220, 315, 2640,
        330, 315, 2580, 330, 315, 2640};

    std::vector<float> ref_zcorn = {2500, 2500, 2510, 2510, 2520, 2520, 2500, 2500, 2510, 2510, 2520, 2520,
        2520, 2520, 2530, 2530, 2540, 2540, 2520, 2520, 2530, 2530, 2540, 2540, 2540, 2540, 2550, 2550, 2560,
        2560, 2540, 2540, 2550, 2550, 2560, 2560, 2560, 2560, 2570, 2570, 2580, 2580, 2560, 2560, 2570, 2570,
        2580, 2580, 2525, 2525, 2535, 2535, 2545, 2545, 2525, 2525, 2535, 2535, 2545, 2545, 2545, 2545, 2555,
        2555, 2565, 2565, 2545, 2545, 2555, 2555, 2565, 2565, 2565, 2565, 2575, 2575, 2585, 2585, 2565, 2565,
        2575, 2575, 2585, 2585, 2585, 2585, 2595, 2595, 2605, 2605, 2585, 2585, 2595, 2595, 2605, 2605, 2525,
        2525, 2535, 2535, 2545, 2545, 2525, 2525, 2535, 2535, 2545, 2545, 2545, 2545, 2555, 2555, 2565, 2565,
        2545, 2545, 2555, 2555, 2565, 2565, 2565, 2565, 2575, 2575, 2585, 2585, 2565, 2565, 2575, 2575, 2585,
        2585, 2585, 2585, 2595, 2595, 2605, 2605, 2585, 2585, 2595, 2595, 2605, 2605, 2560, 2560, 2570, 2570,
        2580, 2580, 2560, 2560, 2570, 2570, 2580, 2580, 2580, 2580, 2590, 2590, 2600, 2600, 2580, 2580, 2590,
        2590, 2600, 2600, 2600, 2600, 2610, 2610, 2620, 2620, 2600, 2600, 2610, 2610, 2620, 2620, 2620, 2620,
        2630, 2630, 2640, 2640, 2620, 2620, 2630, 2630, 2640, 2640};

    BOOST_CHECK_EQUAL( t_coord.size() , ref_coord.size());

    for (size_t i=0; i< t_coord.size(); i++) {
        BOOST_CHECK_CLOSE( t_coord[i] , ref_coord[i], 1.0e-5);
    }

    BOOST_CHECK_EQUAL( t_zcorn.size() , ref_zcorn.size());

    for (size_t i=0; i< t_zcorn.size(); i++) {
        BOOST_CHECK_CLOSE( t_zcorn[i] , ref_zcorn[i], 1.0e-5);
    }
}

static Opm::Deck createCARTDeckDEPTHZ_2x3x2() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 2 3 2 /\n"
        "GRID\n"
        "DXV\n"
        "100 120 /\n"
        "DYV\n"
        "70  80  85 /\n"
        "DZV\n"
        "25 35 /\n"
        "DEPTHZ\n"
        "2500 2510 2520  2502 2512 2522  2504 2514 2524  2505 2515 2525 /\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    return parser.parseString( deckData) ;
}

BOOST_AUTO_TEST_CASE(CART_Deck_DEPTHZ_2x3x2) {

    Opm::Deck deck1 = createCARTDeckDEPTHZ_2x3x2();
    Opm::EclipseGrid grid1( deck1 );

    std::vector<double> ref_coord = { 0, 0, 2500, 0, 0, 2560, 100, 0, 2510, 100, 0, 2570, 220, 0, 2520, 220, 0,
        2580, 0, 70, 2502, 0, 70, 2562, 100, 70, 2512, 100, 70, 2572, 220, 70, 2522, 220, 70, 2582, 0, 150,
        2504, 0, 150, 2564, 100, 150, 2514, 100, 150, 2574, 220, 150, 2524, 220, 150, 2584, 0, 235, 2505, 0,
        235, 2565, 100, 235, 2515, 100, 235, 2575, 220, 235, 2525, 220, 235, 2585 };

    std::vector<double> ref_zcorn = { 2500, 2510, 2510, 2520, 2502, 2512, 2512, 2522, 2502, 2512, 2512, 2522,
        2504, 2514, 2514, 2524, 2504, 2514, 2514, 2524, 2505, 2515, 2515, 2525, 2525, 2535, 2535, 2545, 2527,
        2537, 2537, 2547, 2527, 2537, 2537, 2547, 2529, 2539, 2539, 2549, 2529, 2539, 2539, 2549, 2530, 2540,
        2540, 2550, 2525, 2535, 2535, 2545, 2527, 2537, 2537, 2547, 2527, 2537, 2537, 2547, 2529, 2539, 2539,
        2549, 2529, 2539, 2539, 2549, 2530, 2540, 2540, 2550, 2560, 2570, 2570, 2580, 2562, 2572, 2572, 2582,
        2562, 2572, 2572, 2582, 2564, 2574, 2574, 2584, 2564, 2574, 2574, 2584, 2565, 2575, 2575, 2585 };

    const std::vector<double> t_coord = grid1.getCOORD();
    const std::vector<double> t_zcorn = grid1.getZCORN();

    BOOST_CHECK_EQUAL( t_coord.size() , ref_coord.size());

    for (size_t i=0; i< t_coord.size(); i++) {
        BOOST_CHECK_CLOSE( t_coord[i] , ref_coord[i], 1.0e-5);
    }

    BOOST_CHECK_EQUAL( t_zcorn.size() , ref_zcorn.size());

    for (size_t i=0; i< t_zcorn.size(); i++) {
        BOOST_CHECK_CLOSE( t_zcorn[i] , ref_zcorn[i], 1.0e-5);
    }
}

static Opm::Deck BAD_CP_GRID() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "PORO\n"
        "  8*0.15 /\n"
        "EDIT\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

static Opm::Deck BAD_CP_GRID_MAPAXES() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "MAPAXES\n"
        " 0.  100.  0.  0.  100.  0.  /\n"
        "\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "PORO\n"
        "  8*0.15 /\n"
        "EDIT\n";

    Opm::Parser parser;
    return parser.parseString( deckData);
}

BOOST_AUTO_TEST_CASE(SAVE_FIELD_UNITS) {

    const char* deckData =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 4 4 3 /\n"
        "FIELD\n"
        "GRID\n"
        "DX\n"
        " 48*300 /\n"
        "DY\n"
        " 48*300 /\n"
        "DZ\n"
        " 16*20 16*30 16*50 / \n"
        "TOPS\n"
        " 16*8325 / \n"
        "PORO\n"
        "  48*0.15 /\n"
        "EDIT\n"
        "\n";

    const char* deckData2 =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 4 4 3 /\n"
        "FIELD\n"
        "GRID\n"
        "MAPUNITS\n"
        " METRES /\n"
        "MAPAXES\n"
        " 0.0  101.1  0.0  0.0  102.2  0.0  /\n"
        "DX\n"
        " 48*300 /\n"
        "DY\n"
        " 48*300 /\n"
        "DZ\n"
        " 16*20 16*30 16*50 / \n"
        "TOPS\n"
        " 16*8325 / \n"
        "PORO\n"
        "  48*0.15 /\n"
        "EDIT\n"
        "\n";

    const char* deckData3 =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 4 4 3 /\n"
        "FIELD\n"
        "GRID\n"
        "MAPUNITS\n"
        " FEET /\n"
        "MAPAXES\n"
        " 0.0  102.2  0.0  0.0  103.3  0.0  /\n"
        "DX\n"
        " 48*300 /\n"
        "DY\n"
        " 48*300 /\n"
        "DZ\n"
        " 16*20 16*30 16*50 / \n"
        "TOPS\n"
        " 16*8325 / \n"
        "PORO\n"
        "  48*0.15 /\n"
        "EDIT\n"
        "\n";

    std::vector<float> ref2_mapaxes = {0.0, 101.1, 0.0, 0.0, 102.2, 0.0 };
    std::vector<float> ref3_mapaxes = {0.0, 102.2, 0.0, 0.0, 103.3, 0.0 };

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    Opm::EclipseState es(deck);
    Opm::UnitSystem units = es.getDeckUnitSystem();
    const auto length = ::Opm::UnitSystem::measure::length;

    const auto& grid1 = es.getInputGrid();

    Opm::NNC nnc( deck );
    bool formatted = false;

    time_t timer;
    time(&timer);

    std::string cwd = Opm::filesystem::current_path().c_str();
    std::string testDir = cwd + "/tmp_dir_" + std::to_string(timer);

    if ( Opm::filesystem::exists( testDir )) {
        Opm::filesystem::remove_all(testDir);
    }

    Opm::filesystem::create_directory(testDir);

    std::string fileName = testDir + "/" + "TMP.EGRID";
    grid1.save(fileName, formatted, nnc, units);

    Opm::EclIO::EclFile file1(fileName);

    // Values getZCORNed from the grid needs to be converted from SI to Field units
    // and then converted from double to single precissions before comparing with values saved to
    // the EGRID file

    // check coord
    const std::vector<float> coord_egrid = file1.get<float>("COORD");
    std::vector<double> coord_input_si = grid1.getCOORD();

    BOOST_CHECK( coord_egrid.size() == coord_input_si.size());

    std::vector<float> coord_input_f;
    coord_input_f.reserve(coord_input_si.size());

    for (size_t n =0; n< coord_egrid.size(); n++) {
        coord_input_f.push_back( static_cast<float>(units.from_si(length, coord_input_si[n])));
        BOOST_CHECK_CLOSE( coord_input_f[n] , coord_egrid[n], 1e-6 );
    }

    // check zcorn
    const std::vector<float> zcorn_egrid = file1.get<float>("ZCORN");
    std::vector<double> zcorn_input_si = grid1.getZCORN();

    BOOST_CHECK( zcorn_egrid.size() == zcorn_input_si.size());

    std::vector<float> zcorn_input_f;
    zcorn_input_f.reserve(zcorn_input_si.size());

    for (size_t n =0; n< zcorn_egrid.size(); n++) {
        zcorn_input_f.push_back( static_cast<float>(units.from_si(length, zcorn_input_si[n])));
        BOOST_CHECK_CLOSE( zcorn_input_f[n] , zcorn_egrid[n], 1e-6 );
    }

    BOOST_CHECK( file1.hasKey("GRIDUNIT"));
    const std::vector<std::string> gridunits = file1.get<std::string>("GRIDUNIT");

    BOOST_CHECK( gridunits[0]=="FEET");

    // input deck do not hold MAPAXES or MAPUNITS entries. Below keywords should not be written to EGRID file
    BOOST_CHECK( !file1.hasKey("MAPAXES"));
    BOOST_CHECK( !file1.hasKey("MAPUNITS"));

    // this deck do not have any nnc. Below keywords should not be written to EGRID file
    BOOST_CHECK( !file1.hasKey("NNCHEAD"));
    BOOST_CHECK( !file1.hasKey("NNC1"));
    BOOST_CHECK( !file1.hasKey("NNC2"));

    // testing deck in field units and MAPUNITS in METRES
    auto deck2 = parser.parseString( deckData2) ;

    Opm::EclipseState es2(deck2);
    Opm::UnitSystem units2 = es.getDeckUnitSystem();
    Opm::NNC nnc2( deck2 );

    const auto& grid2 = es2.getInputGrid();

    std::string fileName2 = testDir + "/" + "TMP2.FEGRID";

    grid2.save(fileName2, true, nnc2, units);

    Opm::EclIO::EclFile file2(fileName2);

    const std::vector<std::string>& test_mapunits2 = file2.get<std::string>("MAPUNITS");
    BOOST_CHECK( test_mapunits2[0] == "METRES");

    const std::vector<float>& test_mapaxes2 = file2.get<float>("MAPAXES");

    BOOST_CHECK( test_mapaxes2.size() == ref2_mapaxes.size());

    for (size_t n =0; n< ref2_mapaxes.size(); n++) {
        BOOST_CHECK( ref2_mapaxes[n] == test_mapaxes2[n]);
    }

    // testing deck in field units and MAPUNITS in FEET
    auto deck3 = parser.parseString( deckData3) ;

    Opm::EclipseState es3(deck3);
    Opm::UnitSystem units3 = es.getDeckUnitSystem();
    Opm::NNC nnc3( deck3 );

    const auto& grid3 = es3.getInputGrid();

    std::string fileName3 = testDir + "/" + "TMP3.FEGRID";

    grid3.save(fileName3, true, nnc3, units3);

    Opm::EclIO::EclFile file3(fileName3);

    const std::vector<std::string>& test_mapunits3 = file3.get<std::string>("MAPUNITS");
    BOOST_CHECK( test_mapunits3[0] == "FEET");

    const std::vector<float>& test_mapaxes3 = file3.get<float>("MAPAXES");

    BOOST_CHECK( test_mapaxes3.size() == ref3_mapaxes.size());

    for (size_t n =0; n< ref3_mapaxes.size(); n++) {
        BOOST_CHECK( ref3_mapaxes[n] == test_mapaxes3[n]);
    }

    Opm::filesystem::remove_all(testDir);
}

BOOST_AUTO_TEST_CASE(SAVE_METRIC_UNITS) {

    const char* deckData1 =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 4 4 3 /\n"
        "GRID\n"
        "MAPAXES\n"
        " 0.0 45000.0 0.0 0.0 720000.0 0.0 / \n"
        "MAPUNITS\n"
        " METRES / \n"
        "DX\n"
        " 48*300 /\n"
        "DY\n"
        " 48*300 /\n"
        "DZ\n"
        " 16*20 16*30 16*50 / \n"
        "TOPS\n"
        " 16*8325 / \n"
        "NNC\n"
        " 2 2 1  2 3 2   0.95 / \n"
        " 3 2 1  3 3 2   1.05 / \n"
        " 4 2 1  4 3 2   1.15 / \n"
        "/ \n"
        "PORO\n"
        "  48*0.15 /\n"
        "EDIT\n"
        "\n";

    const char* deckData2 =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 4 4 3 /\n"
        "GRID\n"
        "MAPAXES\n"
        " 0.0 450.0 0.0 0.0 7200.0 0.0 / \n"
        "MAPUNITS\n"
        " FEET / \n"
        "DX\n"
        " 48*300 /\n"
        "DY\n"
        " 48*300 /\n"
        "DZ\n"
        " 16*20 16*30 16*50 / \n"
        "TOPS\n"
        " 16*8325 / \n"
        "NNC\n"
        " 2 2 1  2 3 2   0.95 / \n"
        " 3 2 1  3 3 2   1.05 / \n"
        " 4 2 1  4 3 2   1.15 / \n"
        "/ \n"
        "PORO\n"
        "  48*0.15 /\n"
        "EDIT\n"
        "\n";

    std::vector<float> ref_mapaxes1 = { 0.0, 45000.0, 0.0, 0.0, 720000.0, 0.0 };
    std::vector<float> ref_mapaxes2 = { 0.0, 450.0, 0.0, 0.0, 7200.0, 0.0 };

    Opm::Parser parser;
    auto deck1 = parser.parseString( deckData1) ;

    Opm::EclipseState es1(deck1);
    Opm::UnitSystem units1 = es1.getDeckUnitSystem();
    const auto length = ::Opm::UnitSystem::measure::length;

    const auto& grid1 = es1.getInputGrid();
    Opm::NNC nnc( deck1 );

    bool formatted = true;

    time_t timer;
    time(&timer);

    std::string cwd = Opm::filesystem::current_path().c_str();
    std::string testDir = cwd + "/tmp_dir_" + std::to_string(timer);

    if ( Opm::filesystem::exists( testDir )) {
        Opm::filesystem::remove_all(testDir);
    }

    Opm::filesystem::create_directory(testDir);

    std::string fileName = testDir + "/" + "TMP.FEGRID";
    grid1.save(fileName, formatted, nnc, units1);

    Opm::EclIO::EclFile file1(fileName);

    // Values getZCORNed from the grid have same units as input deck (metric), however these needs to be
    // converted from double to single precissions before comparing with values saved to the EGRID file

    // check coord
    const std::vector<float> coord_egrid = file1.get<float>("COORD");
    std::vector<double> coord_input_si = grid1.getCOORD();

    BOOST_CHECK( coord_egrid.size() == coord_input_si.size());

    std::vector<float> coord_input_f;
    coord_input_f.reserve(coord_input_si.size());

    for (size_t n =0; n< coord_egrid.size(); n++) {
        coord_input_f.push_back( static_cast<float>(units1.from_si(length, coord_input_si[n])));
        BOOST_CHECK_CLOSE( coord_input_f[n] , coord_egrid[n], 1e-6 );
    }

    // check zcorn
    const std::vector<float> zcorn_egrid = file1.get<float>("ZCORN");
    std::vector<double> zcorn_input_si = grid1.getZCORN();

    BOOST_CHECK( zcorn_egrid.size() == zcorn_input_si.size());

    std::vector<float> zcorn_input_f;
    zcorn_input_f.reserve(zcorn_input_si.size());

    for (size_t n =0; n< zcorn_egrid.size(); n++) {
        zcorn_input_f.push_back( static_cast<float>(units1.from_si(length, zcorn_input_si[n])));
        BOOST_CHECK_CLOSE( zcorn_input_f[n] , zcorn_egrid[n], 1e-6 );
    }

    BOOST_CHECK( file1.hasKey("GRIDUNIT"));
    const std::vector<std::string> gridunits = file1.get<std::string>("GRIDUNIT");

    BOOST_CHECK( gridunits[0]=="METRES");

    BOOST_CHECK( file1.hasKey("MAPAXES"));
    std::vector<float> mapaxes = file1.get<float>("MAPAXES");

    for (size_t n = 0; n < 6; n++) {
        BOOST_CHECK_CLOSE( mapaxes[n] , ref_mapaxes1[n], 1e-6 );
    }

    BOOST_CHECK( file1.hasKey("MAPUNITS"));
    const std::vector<std::string> mapunits = file1.get<std::string>("MAPUNITS");
    BOOST_CHECK( gridunits[0]=="METRES");

    BOOST_CHECK( file1.hasKey("NNCHEAD"));
    const std::vector<int> nnchead = file1.get<int>("NNCHEAD");

    BOOST_CHECK( nnchead[0] == static_cast<int>(nnc.numNNC()) );

    std::vector<int> ref_nnc1 = { 6, 7, 8 };
    std::vector<int> ref_nnc2 = { 26, 27, 28 };

    BOOST_CHECK( file1.hasKey("NNC1"));
    BOOST_CHECK( file1.hasKey("NNC2"));

    const std::vector<int> nnc1 = file1.get<int>("NNC1");
    const std::vector<int> nnc2 = file1.get<int>("NNC2");

    BOOST_CHECK( nnc1.size() == nnc2.size() );

    for (size_t n =0; n< nnc1.size(); n++) {
        BOOST_CHECK( nnc1[n] == ref_nnc1[n] );
    }

    for (size_t n =0; n< nnc2.size(); n++) {
        BOOST_CHECK( nnc2[n] == ref_nnc2[n] );
    }

    // testing deck in metric units with mapaxes in field units
    auto deck2 = parser.parseString( deckData2) ;

    Opm::EclipseState es2(deck2);
    Opm::UnitSystem units2 = es2.getDeckUnitSystem();

    const auto& grid2 = es2.getInputGrid();
    //Opm::NNC nnc( deck2 );

    std::string fileName2 = testDir + "/" + "TMP2.FEGRID";

    grid2.save(fileName2, true, nnc, units2);

    Opm::EclIO::EclFile file2(fileName2);

    const std::vector<std::string>& test_mapunits2 = file2.get<std::string>("MAPUNITS");
    BOOST_CHECK( test_mapunits2[0] == "FEET");

    const std::vector<float>& test_mapaxes2 = file2.get<float>("MAPAXES");

    BOOST_CHECK( test_mapaxes2.size() == ref_mapaxes2.size());

    for (size_t n =0; n< ref_mapaxes2.size(); n++) {
        BOOST_CHECK( ref_mapaxes2[n] == test_mapaxes2[n]);
    }


    Opm::filesystem::remove_all(testDir);
}

BOOST_AUTO_TEST_CASE(CalcCellDims) {

    Opm::Deck deck = BAD_CP_GRID();
    Opm::EclipseGrid grid( deck );

    std::array<int, 3> dims = grid.getNXYZ();

    size_t nCells = dims[0]*dims[1]*dims[2];

    std::vector<double> dz_ref = { 0.33223500E+01, 0.40973248E+01, 0.32474000E+01, 0.28723750E+01, 0.49961748E+01, 0.49961748E+01,
                                   0.49961748E+01, 0.49961748E+01
                                 };

    std::vector<double> dx_ref = { 0.10309320E+02, 0.70301223E+01, 0.84377403E+01, 0.85725355E+01, 0.10140956E+02, 0.89693098E+01,
                                   0.94102650E+01, 0.94102678E+01
                                 };

    std::vector<double> dy_ref = { 0.99226236E+01, 0.10826077E+02, 0.93370037E+01, 0.93144703E+01, 0.10008223E+02, 0.10302064E+02,
                                   0.97221985E+01, 0.97221985E+01
                                 };

    std::vector<double> depth_ref = { 0.10142293E+03, 0.10190942E+03, 0.10230995E+03, 0.10284644E+03, 0.10625719E+03,
                                      0.10660616E+03, 0.10643174E+03, 0.10678072E+03
                                    };

    for (size_t n=0; n<nCells; n++) {

        BOOST_CHECK_CLOSE( grid.getCellThickness(n) , dz_ref[n], 1e-5 );

        std::array<double, 3> cellDims = grid.getCellDims(n);

        BOOST_CHECK_CLOSE( cellDims[0] , dx_ref[n], 1e-5 );
        BOOST_CHECK_CLOSE( cellDims[1] , dy_ref[n], 1e-5 );
        BOOST_CHECK_CLOSE( cellDims[2] , dz_ref[n], 1e-5 );

        BOOST_CHECK_CLOSE( grid.getCellDepth(n) , depth_ref[n], 1e-5 );
    }

    for (int k = 0; k < dims[2]; k++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int i = 0; i < dims[0]; i++) {
                size_t globInd = i + j*dims[0] + k*dims[0]*dims[1];
                BOOST_CHECK_CLOSE( grid.getCellThickness(i, j, k) , dz_ref[globInd], 1e-5 );

                std::array<double, 3> cellDims = grid.getCellDims(i, j, k);

                BOOST_CHECK_CLOSE( cellDims[0] , dx_ref[globInd], 1e-5 );
                BOOST_CHECK_CLOSE( cellDims[1] , dy_ref[globInd], 1e-5 );
                BOOST_CHECK_CLOSE( cellDims[2] , dz_ref[globInd], 1e-5 );

                BOOST_CHECK_CLOSE( grid.getCellDepth(i, j, k) , depth_ref[globInd], 1e-5 );
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(ExportMAPAXES_TEST) {

    Opm::Deck deck1 = BAD_CP_GRID_MAPAXES();
    Opm::EclipseGrid grid1( deck1 );

    std::vector<double> ref_mapaxes = { 0.0, 100.0, 0.0, 0.0, 100.0, 0.0 };

    std::vector<double> mapaxes = grid1.getMAPAXES();

    for (size_t n=0; n< mapaxes.size(); n++ ) {
        BOOST_CHECK_EQUAL( ref_mapaxes[n] , mapaxes[n]);
    }

    Opm::Deck deck2 = BAD_CP_GRID();
    Opm::EclipseGrid grid2( deck2 );

    BOOST_CHECK( !grid1.equal( grid2 ));

    std::vector<double> coord = grid1.getCOORD();
    std::vector<double> zcorn = grid1.getZCORN();
    std::vector<int> actnum = grid1.getACTNUM();

    std::array<int, 3> dims = grid1.getNXYZ();

    Opm::EclipseGrid grid3(dims, coord, zcorn, actnum.data(), mapaxes.data());

    BOOST_CHECK( grid3.equal( grid1 ));

    mapaxes[1] = 101;
    Opm::EclipseGrid grid4(dims, coord, zcorn, actnum.data(), mapaxes.data());

    BOOST_CHECK( !grid4.equal( grid1 ));
}

BOOST_AUTO_TEST_CASE(TESTCP_ACTNUM_UPDATE) {
    const char* deckData =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 3 2 1 /\n"
        "GRID\n"
        "COORD\n"
        " 2000.0000  2000.0000  2000.0000   1999.9476  2000.0000  2002.9995\n"
        " 2049.9924  2000.0000  2000.8726   2049.9400  2000.0000  2003.8722 \n"
        " 2099.9848  2000.0000  2001.7452   2099.9324  2000.0000  2004.7448 \n"
        " 2149.9772  2000.0000  2002.6179   2149.9248  2000.0000  2005.6174 \n"
        " 2000.0000  2050.0000  2000.0000   1999.9476  2050.0000  2002.9995 \n"
        " 2049.9924  2050.0000  2000.8726   2049.9400  2050.0000  2003.8722 \n"
        " 2099.9848  2050.0000  2001.7452   2099.9324  2050.0000  2004.7448 \n"
        " 2149.9772  2050.0000  2002.6179   2149.9248  2050.0000  2005.6174 \n"
        " 2000.0000  2100.0000  2000.0000   1999.9476  2100.0000  2002.9995 \n"
        " 2049.9924  2100.0000  2000.8726   2049.9400  2100.0000  2003.8722 \n"
        " 2099.9848  2100.0000  2001.7452   2099.9324  2100.0000  2004.7448 \n"
        " 2149.9772  2100.0000  2002.6179   2149.9248  2100.0000  2005.6174 / \n"
        "ZCORN\n"
        " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
        " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
        " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
        " 2000.0000  2000.8726  2000.8726  2001.7452  2001.7452  2002.6179 \n"
        " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
        " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
        " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 \n"
        " 2002.9995  2003.8722  2003.8722  2004.7448  2004.7448  2005.6174 / \n"
        "ACTNUM\n"
        " 0 1 1 1 0 1 / \n"
        "PORO\n"
        "  6*0.15 /\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    std::vector<int> actInDeck = {0, 1, 1, 1, 0, 1};
    std::vector<int> newAct = {1, 0, 0, 0, 1, 0};

    Opm::EclipseGrid grid1( deck);
    Opm::EclipseGrid grid2( deck, newAct.data());

    std::vector<int> actGrid1 = grid1.getACTNUM();
    std::vector<int> actGrid2 = grid2.getACTNUM();

    BOOST_CHECK( actGrid1.size() == actGrid2.size());

    for (size_t n=0; n< actGrid1.size(); n++) {
        BOOST_CHECK_EQUAL( actGrid1[n], actInDeck[n]);
        BOOST_CHECK_EQUAL( actGrid2[n], newAct[n]);
    }
}


BOOST_AUTO_TEST_CASE(TEST_altGridConstructors) {

    const char* deckData =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 2 2 3 /\n"
        "GRID\n"
        "SPECGRID\n"
        " 2 2 3 1 F / \n"
        "\n"
        "COORD\n"
        "  2000.0000  2000.0000  1497.0000   1999.9127  2000.0000  1511.9977\n"
        "  2049.9924  2000.0000  1500.8726   2049.9051  2000.0000  1515.8703\n"
        "  2099.9848  2000.0000  1501.7452   2099.8975  2000.0000  1516.7430 \n"
        "  2000.0000  2050.0000  1497.0000   1999.9127  2050.0000  1511.9977\n"
        "  2049.9924  2050.0000  1500.8726   2049.9051  2050.0000  1515.8703\n"
        "  2099.9848  2050.0000  1501.7452   2099.8975  2050.0000  1516.7430\n"
        "  2000.0000  2100.0000  1497.0000   1999.9127  2100.0000  1511.9977\n"
        "  2049.9924  2100.0000  1500.8726   2049.9051  2100.0000  1515.8703\n"
        "  2099.9848  2100.0000  1501.7452   2099.8975  2100.0000  1516.7430 /\n"
        "\n"
        "ZCORN\n"
        "  1497.0000  1497.8726  1500.8726  1501.7452  1497.0000  1497.8726\n"
        "  1500.8726  1501.7452  1497.0000  1497.8726  1500.8726  1501.7452\n"
        "  1497.0000  1497.8726  1500.8726  1501.7452  1501.9992  1502.8719\n"
        "  1505.8719  1506.7445  1501.9992  1502.8719  1505.8719  1506.7445\n"
        "  1501.9992  1502.8719  1505.8719  1506.7445  1501.9992  1502.8719\n"
        "  1505.8719  1506.7445  1501.9992  1502.8719  1505.8719  1506.7445\n"
        "  1501.9992  1502.8719  1505.8719  1506.7445  1501.9992  1502.8719\n"
        "  1505.8719  1506.7445  1501.9992  1502.8719  1505.8719  1506.7445\n"
        "  1506.9985  1507.8711  1510.8711  1511.7437  1506.9985  1507.8711\n"
        "  1510.8711  1511.7437  1506.9985  1507.8711  1510.8711  1511.7437\n"
        "  1506.9985  1507.8711  1510.8711  1511.7437  1506.9985  1507.8711\n"
        "  1510.8711  1511.7437  1506.9985  1507.8711  1510.8711  1511.7437\n"
        "  1506.9985  1507.8711  1510.8711  1511.7437  1506.9985  1507.8711\n"
        "  1510.8711  1511.7437  1511.9977  1512.8703  1515.8703  1516.7430\n"
        "  1511.9977  1512.8703  1515.8703  1516.7430  1511.9977  1512.8703\n"
        "  1515.8703  1516.7430  1511.9977  1512.8703  1515.8703  1516.7430 /\n"
        "\n"
        "ACTNUM\n"
        " 1 1 1 1 1 0 1 1 1 0 1 1  /\n"
        "PORO\n"
        "  24*0.15 /\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    Opm::EclipseGrid grid1( deck);

    std::vector<int> actnum = grid1.getACTNUM();
    std::vector<double> coord = grid1.getCOORD();
    std::vector<double> zcorn = grid1.getZCORN();

    Opm::EclipseGrid grid2( grid1 , zcorn.data(), actnum);
    //Opm::EclipseGrid grid2( grid1 , zcorn, actnum);

    BOOST_CHECK( grid1.equal( grid2) );

    std::vector<double> emptyZcorn;

    Opm::EclipseGrid grid3( grid1 , emptyZcorn.data(), actnum);
    BOOST_CHECK( grid1.equal( grid3) );
}

static Opm::Deck BAD_CP_GRID_ACTNUM() {
    const char* deckData =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "ACTNUM\n"
        "   1 1 1 1 0 1 0 1  /\n"
        "PORO\n"
        "  8*0.15 /\n"
        "EDIT\n";

    Opm::Parser parser;
    return parser.parseString( deckData );
}

BOOST_AUTO_TEST_CASE(TEST_getCellCenters) {

    Opm::Deck deck1 = BAD_CP_GRID_ACTNUM();
    Opm::EclipseGrid grid1( deck1 );

    std::vector<std::array<double, 3>> ref_centers_prev = {
        { 2.006104082026e+03, 2.005869733884e+03, 1.014229250000e+02 },
        { 2.014871600974e+03, 2.005382958337e+03, 1.019094125000e+02 },
        { 2.006149627699e+03, 2.015375548018e+03, 1.023099500000e+02 },
        { 2.014617670881e+03, 2.015373620907e+03, 1.028464375000e+02 },
        { 2.014794142285e+03, 2.005084683036e+03, 1.066061625000e+02 },
        { 2.014720614821e+03, 2.015083182885e+03, 1.067807125000e+02 }
    };

    std::vector<std::array<double, 3>> ref_dims_prev = {
        { 1.030932076324e+01, 9.922624077310e+00, 3.322350000000e+00 },
        { 7.030122275667e+00, 1.082607766242e+01, 4.097325000000e+00 },
        { 8.437740561450e+00, 9.337003646693e+00, 3.247400000000e+00 },
        { 8.572535163904e+00, 9.314470420138e+00, 2.872375000000e+00 },
        { 8.969310273018e+00, 1.030206365401e+01, 4.996175000000e+00 },
        { 9.410267880374e+00, 9.722198202980e+00, 4.996175000000e+00 }
    };

    std::vector<int> actMap = grid1.getActiveMap();

    int n = 0;
    for (auto ind : actMap) {
        std::array<double, 3> cellC = grid1.getCellCenter(ind);
        std::array<double, 3> cellD = grid1.getCellDims(ind);

        for (size_t i = 0; i < 3; i++) {
            BOOST_CHECK_CLOSE( ref_centers_prev[n][i], cellC[i], 1e-10);
            BOOST_CHECK_CLOSE( ref_dims_prev[n][i], cellD[i], 1e-10);
        }

        n++;
    }
}

BOOST_AUTO_TEST_CASE(LoadFromBinary) {
    BOOST_CHECK_THROW(Opm::EclipseGrid( "No/does/not/exist" ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(TEST_constructFromEgrid) {

    const char* deckData =

        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        " 10 10 3 /\n"
        "FIELD\n"
        "GRID\n"
        "DX\n"
        " 300*1000 /\n"
        "DY\n"
        " 300*1000 /\n"
        "DZ\n"
        " 100*20 100*30 100*50 / \n"
        "TOPS\n"
        " 100*8325 / \n"
        "ACTNUM\n"
        " 44*1 3*0 7*1 3*0  243*1/\n"
        "PORO\n"
        "  300*0.15 /\n"
        "EDIT\n"
        "\n";

    Opm::Parser parser;
    auto deck = parser.parseString( deckData) ;

    Opm::EclipseGrid grid1( deck);
    Opm::EclipseGrid grid2( "SPE1CASE1.EGRID");

    // compare actnum
    std::vector<int> actGrid1 = grid1.getACTNUM();
    std::vector<int> actGrid2 = grid2.getACTNUM();

    BOOST_CHECK( actGrid1.size() == actGrid2.size() );

    for (size_t n= 0; n< actGrid1.size(); n++) {
        BOOST_CHECK( actGrid1[n] == actGrid2[n] );
    }

    // compare coord
    std::vector<double> coordGrid1 = grid1.getCOORD();
    std::vector<double> coordGrid2 = grid2.getCOORD();

    BOOST_CHECK( coordGrid1.size() == coordGrid2.size() );

    std::vector<double> zcornGrid1 = grid1.getZCORN();
    std::vector<double> zcornGrid2 = grid2.getZCORN();

    BOOST_CHECK( zcornGrid1.size() == zcornGrid2.size() );

    for (size_t n = 0; n < zcornGrid1.size(); n++){
        BOOST_CHECK_CLOSE( zcornGrid1[n], zcornGrid2[n], 1.0e-6 );
    }

    BOOST_CHECK( grid1.getCartesianSize() == grid2.getCartesianSize() );

    for (size_t n=0; n < grid1.getCartesianSize(); n++){
        BOOST_CHECK_CLOSE( grid1.getCellVolume(n), grid2.getCellVolume(n), 1e-6);

    }
}



BOOST_AUTO_TEST_CASE(TEST_GDFILE_1) {

    const char* deckData1 =
        "RUNSPEC\n"
        "DIMENS\n"
        "1 1 2 /\n"
        "GRID\n"
        "COORD\n"
        "10.0000    10.0000  2000.0000      9.8255    10.0000  2014.9977\n"
        "109.9848   10.0000  2001.7452    109.8102    10.0000  2016.7430\n"
        "10.0000   110.0000  2000.0000      9.8255   110.0000  2014.9977\n"
        "109.9848   110.0000  2001.7452    109.8102   110.0000  2016.7430 /\n"
        "ZCORN\n"
        "2000.0000  2001.7452  2000.0000  2001.7452  2004.9992  2006.7445\n"
        "2004.9992  2006.7445  2004.9992  2006.7445  2004.9992  2006.7445\n"
        "2014.9977  2016.7430  2014.9977  2016.7430 /\n"
        "PORO\n"
        "   2*0.15 /\n";

    const char* deckData2 =
        "RUNSPEC\n"
        "DIMENS\n"
        "1 1 2 /\n"
        "GRID\n"
        "GDFILE\n"
        " 'BAD_CP_M.EGRID' /\n"
        "COORD\n"
        "10.0000    10.0000  2000.0000      9.8255    10.0000  2014.9977\n"
        "109.9848   10.0000  2001.7452    109.8102    10.0000  2016.7430\n"
        "10.0000   110.0000  2000.0000      9.8255   110.0000  2014.9977\n"
        "109.9848   110.0000  2001.7452    109.8102   110.0000  2016.7430 /\n"
        "PORO\n"
        "   2*0.15 /\n";

    const char* deckData3 =
        "RUNSPEC\n"
        "DIMENS\n"
        "1 1 2 /\n"
        "GRID\n"
        "GDFILE\n"
        " 'BAD_CP_M.EGRID' /\n"
        "ZCORN\n"
        "2000.0000  2001.7452  2000.0000  2001.7452  2004.9992  2006.7445\n"
        "2004.9992  2006.7445  2004.9992  2006.7445  2004.9992  2006.7445\n"
        "2014.9977  2016.7430  2014.9977  2016.7430 /\n"
        "PORO\n"
        "   2*0.15 /\n";


    Opm::Parser parser;
    auto deck1 = parser.parseString( deckData1) ;
    auto deck2 = parser.parseString( deckData2) ;
    auto deck3 = parser.parseString( deckData3) ;

    BOOST_CHECK_NO_THROW( Opm::EclipseGrid grid1(deck1) );
    BOOST_CHECK_THROW(Opm::EclipseGrid grid2(deck2), std::invalid_argument);
    BOOST_CHECK_THROW(Opm::EclipseGrid grid3(deck3), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(TEST_GDFILE_2) {

    const char* deckData1 =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "PORO\n"
        "   8*0.15 /\n"
        "EDIT\n";

    const char* deckData1a =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "ACTNUM\n"
        " 1 1 1 1 0 1 0 1 /\n"
        "PORO\n"
        "   8*0.15 /\n"
        "EDIT\n";

    const char* deckData1b =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "FIELD\n"
        "GRID\n"
        "MAPUNITS\n"
        " METRES /\n"
        "MAPAXES\n"
        " 0.  100.  0.  0.  100.  0.  /\n"
        "SPECGRID\n"
        " 2 2 2 1 F /\n"
        "COORD\n"
        "  2002.0000  2002.0000   100.0000   1999.8255  1999.9127   108.4935\n"
        "  2011.9939  2000.0000   100.3490   2009.8194  1999.9127   108.8425\n"
        "  2015.9878  2000.0000   100.6980   2019.8133  1999.9127   109.1915\n"
        "  2000.0000  2009.9985   100.1745   1999.8255  2009.9112   108.6681 \n"
        "  2010.9939  2011.9985   100.5235   2009.8194  2009.9112   109.0170\n"
        "  2019.9878  2009.9985   100.8725   2019.8133  2009.9112   109.3660\n"
        "  2005.0000  2019.9970   100.3490   1999.8255  2019.9097   108.8426\n"
        "  2009.9939  2019.9970   100.6980   2009.8194  2019.9097   109.1916\n"
        "  2016.9878  2019.9970   101.0470   2019.8133  2019.9097   109.5406 /\n"
        "ZCORN\n"
        "    98.0000   100.3490    97.3490   100.6980   100.1745   100.5235\n"
        "   100.5235   100.8725   100.1745   100.5235   100.5235   100.8725\n"
        "   100.3490   101.6980   101.6980   102.5470   102.4973   102.1463\n"
        "   103.2463   104.1953   103.6719   104.0209   104.0209   104.3698\n"
        "   103.6719   104.0209   104.0209   104.3698   103.8464   104.1954\n"
        "   104.1954   104.5444   103.4973   103.8463   103.8463   104.1953\n"
        "   103.6719   104.0209   104.0209   104.3698   103.6719   104.0209\n"
        "   104.0209   104.3698   103.8464   104.1954   104.1954   104.5444\n"
        "   108.4935   108.8425   108.8425   109.1915   108.6681   109.0170\n"
        "   109.0170   109.3660   108.6681   109.0170   109.0170   109.3660\n"
        "   108.8426   109.1916   109.1916   109.5406  /\n"
        "\n"
        "ACTNUM\n"
        " 1 1 1 1 0 1 0 1 /\n"
        "PORO\n"
        "   8*0.15 /\n"
        "EDIT\n";

    const char* deckData2 =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "GDFILE\n"
        " 'BAD_CP_M.EGRID' /\n"
        "EDIT\n";

    const char* deckData3a =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "ACTNUM\n"
        " 1 0 1 0 1 1 1 1 /\n"
        "MAPUNITS\n"
        " FEET /\n"
        "MAPAXES\n"
        " 0.  200.  0.  0.  200.  0.  /\n"
        "GDFILE\n"
        " 'BAD_CP_M.EGRID' /\n"
        "EDIT\n";

    const char* deckData3b =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "MAPUNITS\n"
        " FEET /\n"
        "MAPAXES\n"
        " 0.  200.  0.  0.  200.  0.  /\n"
        "GDFILE\n"
        " 'BAD_CP_F.EGRID' /\n"
        "ACTNUM\n"
        " 1 0 1 0 1 1 1 1 /\n"
        "EDIT\n";

    const char* deckData3c =
        "RUNSPEC\n"
        "\n"
        "DIMENS\n"
        "2 2 2 /\n"
        "GRID\n"
        "GDFILE\n"
        " 'BAD_CP_F.EGRID' /\n"
        "MAPUNITS\n"
        " FEET /\n"
        "MAPAXES\n"
        " 0.  200.  0.  0.  200.  0.  /\n"
        "ACTNUM\n"
        " 1 0 1 0 1 1 1 1 /\n"
        "EDIT\n";

    Opm::Parser parser;

    std::vector<int> ref_act_egrid = {1, 1, 1, 1, 0, 1, 0, 1};
    std::vector<int> ref_act_deck3 = {1, 0, 1, 0, 1, 1, 1, 1};

    std::vector<double> ref_mapaxes_egrid = { 0.0, 100.0, 0.0, 0.0, 100.0, 0.0 };
    std::vector<double> ref_mapaxes_deck = { 0.0, 200.0, 0.0, 0.0, 200.0, 0.0 };


    // egrid file in si units, no conversion requied by grid constructor
    std::vector<double> refDepthGrid3a = {101.42292, 101.90941, 102.30995, 102.84644, 106.25719, 106.60616, 106.43174, 106.78071 };

    // egrid file in field units, depths converted to SI units when loaded by grid constructor
    std::vector<double> refDepthGrid3b = {30.913707, 31.061988, 31.184072, 31.347594, 32.38719, 32.493558, 32.440393 };

    auto deck1a = parser.parseString( deckData1a) ;

    Opm::EclipseState es1a( deck1a );
    Opm::UnitSystem units1a = es1a.getDeckUnitSystem();

    const auto& grid1a = es1a.getInputGrid();
    Opm::NNC nnc( deck1a );

    grid1a.save("BAD_CP_M.EGRID", false, nnc, units1a);

    auto deck1b = parser.parseString( deckData1b) ;
    Opm::EclipseState es1b( deck1b );
    Opm::UnitSystem units1b = es1b.getDeckUnitSystem();
    const auto& grid1b = es1b.getInputGrid();

    grid1b.save("BAD_CP_F.EGRID", false, nnc, units1b);

    auto deck1 = parser.parseString( deckData1) ;
    Opm::EclipseGrid grid1( deck1);

    Opm::EclIO::EclFile file1("BAD_CP_M.EGRID");

    // actnum not defined in deck. keyword GDFILE not present in the DECK
    // check that coord and zcorn from deck-grid identical to coord and zcorn
    // from egrid - grid

    std::vector<double> coordGrid1 = grid1.getCOORD();
    std::vector<double> zcornGrid1 = grid1.getZCORN();
    std::vector<float> coordGrid1_f(coordGrid1.begin(), coordGrid1.end() );
    std::vector<float> zcornGrid1_f(zcornGrid1.begin(), zcornGrid1.end() );

    const std::vector<float> coord_egrid_f = file1.get<float>("COORD");
    const std::vector<float> zcorn_egrid_f = file1.get<float>("ZCORN");

    BOOST_CHECK( coordGrid1.size() == coord_egrid_f.size() );
    BOOST_CHECK( zcornGrid1.size() == zcorn_egrid_f.size() );

    for (size_t n = 0; n < coordGrid1.size(); n++){
        BOOST_CHECK( coordGrid1_f[n] == coord_egrid_f[n] );
    }

    for (size_t n = 0; n < zcornGrid1.size(); n++){
        BOOST_CHECK( zcornGrid1_f[n] == zcorn_egrid_f[n] );
    }

    // all cells are active, since ACTNUM not present
    std::vector<int> actGrid1 = grid1.getACTNUM();
    for (size_t n = 0; n < actGrid1.size(); n++){
        BOOST_CHECK( actGrid1[n] == 1 );
    }

    BOOST_CHECK( grid1.getMAPUNITS() == "" );

    std::vector<double> mapaxes = grid1.getMAPAXES();
    BOOST_CHECK( mapaxes.size() == 0 );


    auto deck2 = parser.parseString( deckData2) ;
    Opm::EclipseGrid grid2( deck2);

    std::vector<int> actGrid2 = grid2.getACTNUM();

    // check that actnum is reset from gdfile

    for (size_t n = 0; n < actGrid2.size(); n++){
        BOOST_CHECK( actGrid2[n] == ref_act_egrid[n] );
    }

    BOOST_CHECK( grid2.getMAPUNITS() == "" );

    mapaxes = grid2.getMAPAXES();
    BOOST_CHECK( mapaxes.size() == 0 );


    auto deck3a = parser.parseString( deckData3a) ;
    Opm::EclipseGrid grid3a( deck3a);

    // mapunits and mapaxes define in deck (only)

    BOOST_CHECK( grid3a.getMAPUNITS() == "FEET" );

    mapaxes = grid3a.getMAPAXES();
    BOOST_CHECK( mapaxes.size() == 6 );

    for (size_t n = 0; n < mapaxes.size(); n++){
        BOOST_CHECK( mapaxes[n] == ref_mapaxes_deck[n] );
    }

    std::vector<int> actGrid3 = grid3a.getACTNUM();

    // check that actnum is reset from gdfile, ACTNUM input in deck
    // but before keyword GDFILE

    for (size_t n = 0; n < actGrid3.size(); n++){
        BOOST_CHECK( actGrid3[n] == ref_act_egrid[n] );
    }

    // check that depth values are in SI units
    for (size_t n = 0; n < refDepthGrid3a.size(); n++){
        BOOST_CHECK_CLOSE( grid3a.getCellDepth(n), refDepthGrid3a[n], 1e-3 );
    }

    auto deck3b = parser.parseString( deckData3b) ;
    Opm::EclipseGrid grid3b( deck3b);

    // mapunits and mapaxes both in egrid and deck. Uses properties
    // from the egrid keyword gdfile input after MAPUNITS and MAPAXES

    BOOST_CHECK( grid3b.getMAPUNITS() == "METRES" );

    mapaxes = grid3b.getMAPAXES();
    BOOST_CHECK( mapaxes.size() == 6 );

    for (size_t n = 0; n < mapaxes.size(); n++){
        BOOST_CHECK( mapaxes[n] == ref_mapaxes_egrid[n] );
    }

    actGrid3 = grid3b.getACTNUM();

    // check that actnum is reset from deck since input after keyword GDFILE
    for (size_t n = 0; n < actGrid3.size(); n++){
        BOOST_CHECK( actGrid3[n] == ref_act_deck3[n] );
    }

    // check that depth values are converted from Field to SI units
    for (size_t n = 0; n < refDepthGrid3b.size(); n++){
        BOOST_CHECK_CLOSE( grid3b.getCellDepth(n), refDepthGrid3b[n], 1e-3 );
    }

    // mapunits and mapaxes both in egrid and deck. Uses properties
    // from the deck sinze these are input after GDfile

    auto deck3c = parser.parseString( deckData3c) ;
    Opm::EclipseGrid grid3c( deck3c);

    BOOST_CHECK( grid3c.getMAPUNITS() == "FEET" );

    mapaxes = grid3c.getMAPAXES();
    BOOST_CHECK( mapaxes.size() == 6 );

    for (size_t n = 0; n < mapaxes.size(); n++){
        BOOST_CHECK( mapaxes[n] == ref_mapaxes_deck[n] );
    }
}

BOOST_AUTO_TEST_CASE(TEST_COLLAPSED_CELL) {
    Opm::EclipseGrid grid(2,2,2,1,1,0);
    for (std::size_t g = 0; g < grid.getCartesianSize(); g++)
        BOOST_CHECK_EQUAL(grid.getCellVolume(g), 0);
}
