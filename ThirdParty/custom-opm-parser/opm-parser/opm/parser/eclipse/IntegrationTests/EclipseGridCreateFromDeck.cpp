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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(CreateCPGrid) {
    ParserPtr parser(new Parser());
    boost::filesystem::path scheduleFile("testdata/integration_tests/GRID/CORNERPOINT.DATA");
    DeckPtr deck =  parser->parseFile(scheduleFile.string(), ParseContext());
    EclipseState es(deck, ParseContext());
    auto grid = es.getInputGrid();

    BOOST_CHECK_EQUAL( 10U  , grid->getNX( ));
    BOOST_CHECK_EQUAL( 10U  , grid->getNY( ));
    BOOST_CHECK_EQUAL(  5U  , grid->getNZ( ));
    BOOST_CHECK_EQUAL( 500U , grid->getNumActive() );
}


BOOST_AUTO_TEST_CASE(CreateCPActnumGrid) {
    ParserPtr parser(new Parser());
    boost::filesystem::path scheduleFile("testdata/integration_tests/GRID/CORNERPOINT_ACTNUM.DATA");
    DeckPtr deck =  parser->parseFile(scheduleFile.string(), ParseContext());
    EclipseState es(deck, ParseContext());
    auto grid = es.getInputGrid();

    BOOST_CHECK_EQUAL(  10U , grid->getNX( ));
    BOOST_CHECK_EQUAL(  10U , grid->getNY( ));
    BOOST_CHECK_EQUAL(   5U , grid->getNZ( ));
    BOOST_CHECK_EQUAL( 100U , grid->getNumActive() );
}


BOOST_AUTO_TEST_CASE(ExportFromCPGridAllActive) {
    ParserPtr parser(new Parser());
    boost::filesystem::path scheduleFile("testdata/integration_tests/GRID/CORNERPOINT.DATA");
    DeckPtr deck =  parser->parseFile(scheduleFile.string(), ParseContext());
    EclipseState es(deck, ParseContext());
    auto grid = es.getInputGrid();

    std::vector<int> actnum;

    actnum.push_back(100);
    grid->exportACTNUM( actnum );
    BOOST_CHECK_EQUAL( actnum.size() , 0U );
}




BOOST_AUTO_TEST_CASE(ExportFromCPGridACTNUM) {
    ParserPtr parser(new Parser());
    boost::filesystem::path scheduleFile("testdata/integration_tests/GRID/CORNERPOINT_ACTNUM.DATA");
    DeckPtr deck =  parser->parseFile(scheduleFile.string(), ParseContext());
    EclipseState es(deck, ParseContext());
    auto grid = es.getInputGrid();

    std::vector<double> coord;
    std::vector<double> zcorn;
    std::vector<int> actnum;
    size_t volume = grid->getNX()*grid->getNY()*grid->getNZ();

    grid->exportCOORD( coord );
    BOOST_CHECK_EQUAL( coord.size() , (grid->getNX() + 1) * (grid->getNY() + 1) * 6);

    grid->exportZCORN( zcorn );
    BOOST_CHECK_EQUAL( zcorn.size() , volume * 8);

    grid->exportACTNUM( actnum );
    BOOST_CHECK_EQUAL( actnum.size() , volume );

    {
        const std::vector<int>& deckActnum = deck->getKeyword("ACTNUM").getIntData();
        const std::vector<double>& deckZCORN = deck->getKeyword("ZCORN").getSIDoubleData();

        for (size_t i = 0; i < volume; i++) {
            BOOST_CHECK_EQUAL( deckActnum[i] , actnum[i]);
            for (size_t j=0; j < 8; j++)
                BOOST_CHECK_CLOSE( zcorn[i*8 + j] , deckZCORN[i*8 + j] , 0.0001);
        }
    }
}

