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

#define BOOST_TEST_MODULE TransMultTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Events.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/GroupTreeNode.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(MULTFLT_IN_SCHEDULE) {
    ParserPtr parser(new Parser());
    std::string scheduleFile("testdata/integration_tests/TRANS/Deck1");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(scheduleFile, parseContext);
    std::shared_ptr<EclipseState> eclState = std::make_shared<EclipseState>( deck , parseContext );
    std::shared_ptr<const TransMult> trans = eclState->getTransMult();
    std::shared_ptr<const Schedule> schedule = eclState->getSchedule();
    const Events& events = schedule->getEvents();

    BOOST_CHECK_EQUAL( 0.10 , trans->getMultiplier( 3,2,0,FaceDir::XPlus ));
    BOOST_CHECK_EQUAL( 0.10 , trans->getMultiplier( 2,2,0,FaceDir::XPlus ));
    BOOST_CHECK( events.hasEvent( ScheduleEvents::GEO_MODIFIER , 3 ) );
    {
        std::shared_ptr<const Deck> mini_deck = schedule->getModifierDeck(3);
        eclState->applyModifierDeck( *mini_deck );
    }
    BOOST_CHECK_EQUAL( 2.00 , trans->getMultiplier( 2,2,0,FaceDir::XPlus ));
    BOOST_CHECK_EQUAL( 0.10 , trans->getMultiplier( 3,2,0,FaceDir::XPlus ));
}
