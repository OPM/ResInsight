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
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <array>

using namespace Opm;



BOOST_AUTO_TEST_CASE( PARSE_TOPS_OK) {
    ParserPtr parser(new Parser());
    std::string deckFile("testdata/integration_tests/GRID/TOPS.DATA");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(deckFile, parseContext);
    EclipseState state(*deck , parseContext);
    EclipseGridConstPtr grid = state.getInputGrid();

    BOOST_CHECK_EQUAL( grid->getNX() , 9 );
    BOOST_CHECK_EQUAL( grid->getNY() , 9 );
    BOOST_CHECK_EQUAL( grid->getNZ() , 2 );

    for (size_t g=0; g < 9*9*2; g++)
        BOOST_CHECK_CLOSE( grid->getCellVolume( g ) , 400*300*10 , 0.1);

    for (size_t k=0; k < grid->getNZ(); k++) {
        for (size_t j=0; j < grid->getNY(); j++) {
            for (size_t i=0; i < grid->getNX(); i++) {

                auto pos = grid->getCellCenter( i,j,k );
                BOOST_CHECK_CLOSE( std::get<0>(pos) , i*400 + 200 , 0.10 );
                BOOST_CHECK_CLOSE( std::get<1>(pos) , j*300 + 150 , 0.10 );
                BOOST_CHECK_CLOSE( std::get<2>(pos) , k*10  + 5 + 2202 , 0.10 );

            }
        }
    }
}
