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

#define BOOST_TEST_MODULE CompletionSetTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>




#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>



BOOST_AUTO_TEST_CASE(CreateCompletionSetOK) {
    Opm::CompletionSet completionSet;
    BOOST_CHECK_EQUAL( 0U , completionSet.size() );
}



BOOST_AUTO_TEST_CASE(AddCompletionSizeCorrect) {
    Opm::CompletionSet completionSet;
    Opm::CompletionConstPtr completion1(new Opm::Completion(10,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    Opm::CompletionConstPtr completion2(new Opm::Completion(11,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    completionSet.add( completion1 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );

    completionSet.add( completion2 );
    BOOST_CHECK_EQUAL( 2U , completionSet.size() );

    BOOST_CHECK_EQUAL( completion1 , completionSet.get(0));
}


BOOST_AUTO_TEST_CASE(CompletionSetGetOutOfRangeThrows) {
    Opm::CompletionSet completionSet;
    Opm::CompletionConstPtr completion1(new Opm::Completion(10,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    Opm::CompletionConstPtr completion2(new Opm::Completion(11,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    completionSet.add( completion1 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );

    completionSet.add( completion2 );
    BOOST_CHECK_EQUAL( 2U , completionSet.size() );

    BOOST_CHECK_THROW( completionSet.get(10) , std::range_error );
}




BOOST_AUTO_TEST_CASE(AddCompletionSameCellUpdates) {
    Opm::CompletionSet completionSet;
    Opm::CompletionConstPtr completion1(new Opm::Completion(10,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    Opm::CompletionConstPtr completion2(new Opm::Completion(10,10,10,Opm::WellCompletion::SHUT , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));


    completionSet.add( completion1 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );

    completionSet.add( completion2 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );
}



BOOST_AUTO_TEST_CASE(AddCompletionShallowCopy) {
    Opm::CompletionSet completionSet;

    Opm::CompletionConstPtr completion1(new Opm::Completion(10,10,10,Opm::WellCompletion::OPEN , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    Opm::CompletionConstPtr completion2(new Opm::Completion(10,10,11,Opm::WellCompletion::SHUT , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));
    Opm::CompletionConstPtr completion3(new Opm::Completion(10,10,12,Opm::WellCompletion::SHUT , Opm::Value<double>("ConnectionTransmissibilityFactor",99.88), Opm::Value<double>("D",22.33), Opm::Value<double>("SKIN",33.22)));

    completionSet.add( completion1 );
    completionSet.add( completion2 );
    completionSet.add( completion3 );
    BOOST_CHECK_EQUAL( 3U , completionSet.size() );

    Opm::CompletionSetConstPtr copy = Opm::CompletionSetConstPtr( completionSet.shallowCopy() );
    BOOST_CHECK_EQUAL( 3U , copy->size() );

    BOOST_CHECK_EQUAL( completion1 , copy->get(0));
    BOOST_CHECK_EQUAL( completion2 , copy->get(1));
    BOOST_CHECK_EQUAL( completion3 , copy->get(2));
}
