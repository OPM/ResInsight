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
#define BOOST_TEST_MODULE GeoModifiersTests
#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Events.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CheckUnsoppertedInSCHEDULE ) {
    const std::string deckString = R"(
START
 10 'JAN' 2000 /
RUNSPEC
DIMENS
  10 10 10 /
GRID
DX
1000*0.25 /

DY
1000*0.25 /

DZ
1000*0.25 /

TOPS
100*0.25 /

SCHEDULE
TSTEP -- 1,2
   10 10/

MULTFLT
   'F1' 100 /
/

MULTFLT
   'F2' 77 /
/

TSTEP  -- 3,4
   10 10/
)";

    auto python = std::make_shared<Python>();
    Parser parser(true);
    ParseContext parseContext;
    ErrorGuard errors;
    auto deck = parser.parseString( deckString, parseContext, errors);
    EclipseGrid grid( deck );
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);

    {
        Runspec runspec ( deck );
        Schedule schedule( deck, grid , fp, runspec , parseContext, errors, python);
        BOOST_CHECK_EQUAL( false , schedule[1].events().hasEvent( ScheduleEvents::GEO_MODIFIER ));
        BOOST_CHECK_EQUAL( true  , schedule[2].events().hasEvent( ScheduleEvents::GEO_MODIFIER ));
        BOOST_CHECK_EQUAL( false , schedule[3].events().hasEvent( ScheduleEvents::GEO_MODIFIER ));


        BOOST_CHECK_EQUAL( 0U, schedule[1].geo_keywords().size() );
        BOOST_CHECK_EQUAL( 0U, schedule[3].geo_keywords().size() );

        const auto& multflt_deck = schedule[2].geo_keywords();
        BOOST_CHECK_EQUAL( 2U , multflt_deck.size());
        BOOST_CHECK_EQUAL( multflt_deck[0].name(), "MULTFLT");
        BOOST_CHECK_EQUAL( multflt_deck[1].name(), "MULTFLT");

        const auto& multflt1 = multflt_deck[0];
        BOOST_CHECK_EQUAL( 1U , multflt1.size( ) );

        const auto& record0 = multflt1.getRecord( 0 );
        BOOST_CHECK_EQUAL( 100.0  , record0.getItem<ParserKeywords::MULTFLT::factor>().get< double >(0));
        BOOST_CHECK_EQUAL( "F1" , record0.getItem<ParserKeywords::MULTFLT::fault>().get< std::string >(0));

        const auto& multflt2 = multflt_deck[1];
        BOOST_CHECK_EQUAL( 1U , multflt2.size( ) );

        const auto& record1 = multflt2.getRecord( 0 );
        BOOST_CHECK_EQUAL( 77.0  , record1.getItem<ParserKeywords::MULTFLT::factor>().get< double >(0));
        BOOST_CHECK_EQUAL( "F2" , record1.getItem<ParserKeywords::MULTFLT::fault>().get< std::string >(0));
    }
}
