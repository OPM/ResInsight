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

#define BOOST_TEST_MODULE ParseMULTREGT
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
using namespace Opm;




BOOST_AUTO_TEST_CASE( parse_MULTREGT_OK ) {
    ParserPtr parser(new Parser());
    DeckPtr deck =  parser->parseFile("testdata/integration_tests/MULTREGT/MULTREGT", ParseContext());
    BOOST_CHECK_NO_THROW( deck->getKeyword("MULTREGT" , 0); );
}



BOOST_AUTO_TEST_CASE( MULTREGT_ECLIPSE_STATE ) {
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckPtr deck =  parser->parseFile("testdata/integration_tests/MULTREGT/MULTREGT.DATA", parseContext);
    EclipseState state(*deck , parseContext);
    auto transMult = state.getTransMult();

    // Test NONNC
    // cell 0 and 1 are neigbours
    BOOST_CHECK_EQUAL( 0.10 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::XPlus));
    // cell 0 and 3 are not neigbours ==> 1
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 3 , FaceDir::DirEnum::XPlus));

    // Test NNC
    // cell 4 and 5 are neigbours ==> 1
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 4 , 5 , FaceDir::DirEnum::XPlus));
    // cell 4 and 7 are not neigbours
    BOOST_CHECK_EQUAL( 0.50 , transMult.getRegionMultiplier( 4 , 7 , FaceDir::DirEnum::XPlus));

    // Test direction X, returns 1 for directions other than +-X
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::YPlus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::ZPlus));
    BOOST_CHECK_EQUAL( 0.10 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::XMinus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::YMinus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::ZMinus));
    BOOST_CHECK_EQUAL( 0.20 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::YPlus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::ZPlus));
    BOOST_CHECK_EQUAL( 0.20 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::XMinus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::YMinus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 1 , 0 , FaceDir::DirEnum::ZMinus));

    // Multipliers between cells of the same region should return 1
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 2 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 2 , 0 , FaceDir::DirEnum::XPlus));

    // Test direcion XYZ, returns values for all directions
    BOOST_CHECK_EQUAL( 1.50 , transMult.getRegionMultiplier( 0 , 4 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 1.50 , transMult.getRegionMultiplier( 0 , 4 , FaceDir::DirEnum::YPlus));
    BOOST_CHECK_EQUAL( 1.50 , transMult.getRegionMultiplier( 0 , 4 , FaceDir::DirEnum::ZPlus));
    BOOST_CHECK_EQUAL( 1.50 , transMult.getRegionMultiplier( 4 , 0 , FaceDir::DirEnum::XPlus));

    // The first record is overwritten by the second
    BOOST_CHECK_EQUAL( 2.50 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 2.50 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::YPlus));

    // The 2 4 0.75 Z input is overwritten by 2 4 2.5 XY, ==) that 2 4 Z returns the 4 2 value = 0.6
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 7 , 3 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::ZPlus));

}
