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

#define BOOST_TEST_MODULE ScheduleIntegrationTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <filesystem>

using namespace Opm;

inline std::string prefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE(CreateCPGrid) {
    Parser parser;
    std::filesystem::path scheduleFile(prefix() + "GRID/CORNERPOINT.DATA");
    auto deck =  parser.parseFile(scheduleFile.string());
    EclipseState es(deck);
    const auto& grid = es.getInputGrid();

    BOOST_CHECK_EQUAL( 10U  , grid.getNX( ));
    BOOST_CHECK_EQUAL( 10U  , grid.getNY( ));
    BOOST_CHECK_EQUAL(  5U  , grid.getNZ( ));
    BOOST_CHECK_EQUAL( 500U , grid.getNumActive() );
}


BOOST_AUTO_TEST_CASE(CreateCPActnumGrid) {
    Parser parser;
    std::filesystem::path scheduleFile(prefix() + "GRID/CORNERPOINT_ACTNUM.DATA");
    auto deck =  parser.parseFile(scheduleFile.string());
    EclipseState es(deck);
    const auto& grid = es.getInputGrid();

    BOOST_CHECK_EQUAL(  10U , grid.getNX( ));
    BOOST_CHECK_EQUAL(  10U , grid.getNY( ));
    BOOST_CHECK_EQUAL(   5U , grid.getNZ( ));
    BOOST_CHECK_EQUAL( 100U , grid.getNumActive() );
}


BOOST_AUTO_TEST_CASE(ExportFromCPGridAllActive) {
    Parser parser;
    std::filesystem::path scheduleFile(prefix() + "GRID/CORNERPOINT.DATA");
    auto deck =  parser.parseFile(scheduleFile.string());
    EclipseState es(deck);
    const auto& grid = es.getInputGrid();

    std::vector<int> actnum;

    actnum = grid.getACTNUM();
    BOOST_CHECK_EQUAL( actnum.size() , 500U );
}




BOOST_AUTO_TEST_CASE(ExportFromCPGridACTNUM) {
    Parser parser;
    std::filesystem::path scheduleFile(prefix() + "GRID/CORNERPOINT_ACTNUM.DATA");
    auto deck =  parser.parseFile(scheduleFile.string());
    EclipseState es(deck);
    auto& grid = es.getInputGrid();

    std::vector<double> coord;
    std::vector<double> zcorn;
    std::vector<int> actnum;
    size_t volume = grid.getNX()*grid.getNY()*grid.getNZ();

    coord = grid.getCOORD();
    BOOST_CHECK_EQUAL( coord.size() , (grid.getNX() + 1) * (grid.getNY() + 1) * 6);

    zcorn = grid.getZCORN();
    BOOST_CHECK_EQUAL( zcorn.size() , volume * 8);

    actnum = grid.getACTNUM();
    BOOST_CHECK_EQUAL( actnum.size() , volume );

    {
        const std::vector<int>& deckActnum = deck["ACTNUM"].back().getIntData();
        const std::vector<double>& deckZCORN = deck["ZCORN"].back().getSIDoubleData();

        for (size_t i = 0; i < volume; i++) {
            BOOST_CHECK_EQUAL( deckActnum[i] , actnum[i]);
            for (size_t j=0; j < 8; j++)
                BOOST_CHECK_CLOSE( zcorn[i*8 + j] , deckZCORN[i*8 + j] , 0.0001);
        }
    }
}

