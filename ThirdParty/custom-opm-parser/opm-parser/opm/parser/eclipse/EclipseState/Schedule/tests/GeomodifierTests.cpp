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
#define BOOST_TEST_MODULE GeoModifiersTests
#include <boost/test/unit_test.hpp>


#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/parser/eclipse/Parser/InputErrorAction.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Events.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CheckUnsoppertedInSCHEDULE ) {
    const char * deckString =
        "START\n"
        " 10 'JAN' 2000 /\n"
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 / \n"
        "SCHEDULE\n"
        "TSTEP -- 1,2\n"
        "   10 10/\n"
        "MULTFLT\n"
        "   'F1' 100 /\n"
        "/\n"
        "MULTFLT\n"
        "   'F2' 77 /\n"
        "/\n"
        "TSTEP  -- 3,4\n"
        "   10 10/\n"
        "\n";

    ParseContext parseContext;
    Parser parser(true);

    auto deck = parser.parseString( deckString , parseContext );
    std::shared_ptr<EclipseGrid> grid = std::make_shared<EclipseGrid>( deck );
    std::shared_ptr<IOConfig> ioconfig = std::make_shared<IOConfig>();

    parseContext.update( ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER , InputError::IGNORE );
    {
        Schedule schedule( parseContext , grid , deck , ioconfig );
        auto events = schedule.getEvents( );
        BOOST_CHECK_EQUAL( false , events.hasEvent( ScheduleEvents::GEO_MODIFIER , 1 ));
        BOOST_CHECK_EQUAL( true  , events.hasEvent( ScheduleEvents::GEO_MODIFIER , 2 ));
        BOOST_CHECK_EQUAL( false , events.hasEvent( ScheduleEvents::GEO_MODIFIER , 3 ));


        BOOST_CHECK( !schedule.getModifierDeck(1) );
        BOOST_CHECK( !schedule.getModifierDeck(3) );

        std::shared_ptr<const Deck> multflt_deck = schedule.getModifierDeck(2);
        BOOST_CHECK_EQUAL( 2U , multflt_deck->size());
        BOOST_CHECK( multflt_deck->hasKeyword<ParserKeywords::MULTFLT>() );

        const auto& multflt1 = multflt_deck->getKeyword(0);
        BOOST_CHECK_EQUAL( 1U , multflt1.size( ) );

        const auto& record0 = multflt1.getRecord( 0 );
        BOOST_CHECK_EQUAL( 100.0  , record0.getItem<ParserKeywords::MULTFLT::factor>().get< double >(0));
        BOOST_CHECK_EQUAL( "F1" , record0.getItem<ParserKeywords::MULTFLT::fault>().get< std::string >(0));

        const auto& multflt2 = multflt_deck->getKeyword(1);
        BOOST_CHECK_EQUAL( 1U , multflt2.size( ) );

        const auto& record1 = multflt2.getRecord( 0 );
        BOOST_CHECK_EQUAL( 77.0  , record1.getItem<ParserKeywords::MULTFLT::factor>().get< double >(0));
        BOOST_CHECK_EQUAL( "F2" , record1.getItem<ParserKeywords::MULTFLT::fault>().get< std::string >(0));
    }
}
