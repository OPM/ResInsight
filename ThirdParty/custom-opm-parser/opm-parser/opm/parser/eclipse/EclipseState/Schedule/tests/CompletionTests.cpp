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

#define BOOST_TEST_MODULE CompletionTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>




#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>



BOOST_AUTO_TEST_CASE(CreateCompletionOK) {
    Opm::Completion completion(10,10,10,0.0,Opm::WellCompletion::OPEN,Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
}


BOOST_AUTO_TEST_CASE(testGetFunctions) {
    Opm::Completion completion(10,11,12,0.0, Opm::WellCompletion::OPEN,Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
    BOOST_CHECK_EQUAL( 10 , completion.getI() );
    BOOST_CHECK_EQUAL( 11 , completion.getJ() );
    BOOST_CHECK_EQUAL( 12 , completion.getK() );

    BOOST_CHECK_EQUAL( Opm::WellCompletion::OPEN , completion.getState());
    BOOST_CHECK_EQUAL( 99.88 , completion.getConnectionTransmissibilityFactor());
    BOOST_CHECK_EQUAL( 22.33 , completion.getDiameter());
    BOOST_CHECK_EQUAL( 33.22 , completion.getSkinFactor());
}


BOOST_AUTO_TEST_CASE(CompletionTestssameCoordinate) {
    Opm::Completion completion1(10,10,10,0.0, Opm::WellCompletion::OPEN, Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
    Opm::Completion completion2(10,10,10,0.0, Opm::WellCompletion::OPEN, Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
    Opm::Completion completion3(11,10,10,0.0, Opm::WellCompletion::OPEN, Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
    Opm::Completion completion4(10,11,10,0.0, Opm::WellCompletion::OPEN, Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));
    Opm::Completion completion5(10,10,11,0.0, Opm::WellCompletion::OPEN, Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22));

    BOOST_CHECK( completion1.sameCoordinate( completion2 ));
    BOOST_CHECK_EQUAL( false , completion1.sameCoordinate( completion3 ));
    BOOST_CHECK_EQUAL( false , completion1.sameCoordinate( completion4 ));
    BOOST_CHECK_EQUAL( false , completion1.sameCoordinate( completion5 ));
}




