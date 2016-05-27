/*
  Copyright (C) 2015 SINTEF ICT, Applied Mathematics

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

#define BOOST_TEST_MODULE ParseMULTSEGWELL
#include <math.h>

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE( PARSE_MULTISEGMENT_ABS ) {

    ParserPtr parser(new Parser());
    std::string deckFile("testdata/integration_tests/SCHEDULE/SCHEDULE_MULTISEGMENT_WELL");
    DeckPtr deck =  parser->parseFile(deckFile, ParseContext());
    // for WELSEGS keyword
    const auto& kw = deck->getKeyword("WELSEGS");

    // check the size of the keywords
    BOOST_CHECK_EQUAL( 6, kw.size() );

    // check the information for the top segment and the segment set
    {
        const auto& rec1 = kw.getRecord(0); // top segment

        const std::string well_name = rec1.getItem("WELL").getTrimmedString(0);
        const double depth_top = rec1.getItem("DEPTH").get< double >(0);
        const double length_top = rec1.getItem("LENGTH").get< double >(0);
        const double volume_top = rec1.getItem("WELLBORE_VOLUME").get< double >(0);
        const WellSegment::LengthDepthEnum length_depth_type = WellSegment::LengthDepthEnumFromString(rec1.getItem("INFO_TYPE").getTrimmedString(0));
        const WellSegment::CompPressureDropEnum comp_pressure_drop = WellSegment::CompPressureDropEnumFromString(rec1.getItem("PRESSURE_COMPONENTS").getTrimmedString(0));
        const WellSegment::MultiPhaseModelEnum multiphase_model = WellSegment::MultiPhaseModelEnumFromString(rec1.getItem("FLOW_MODEL").getTrimmedString(0));

        BOOST_CHECK_EQUAL( "PROD01", well_name );
        BOOST_CHECK_EQUAL( 2512.5, depth_top );
        BOOST_CHECK_EQUAL( 2512.5, length_top );
        BOOST_CHECK_EQUAL( 1.0e-5, volume_top );
        const std::string length_depth_type_string = WellSegment::LengthDepthEnumToString(length_depth_type);
        BOOST_CHECK_EQUAL( length_depth_type_string, "ABS" );
        const std::string comp_pressure_drop_string = WellSegment::CompPressureDropEnumToString(comp_pressure_drop);
        BOOST_CHECK_EQUAL( comp_pressure_drop_string, "H--" );
        const std::string multiphase_model_string = WellSegment::MultiPhaseModelEnumToString(multiphase_model);
        BOOST_CHECK_EQUAL( multiphase_model_string, "HO" );
    }

    // check the information for the other segments
    // Here, we check the information for the segment 2 and 6 as samples.
    {
        const auto& rec2 = kw.getRecord(1);
        const int segment1 = rec2.getItem("SEGMENT2").get< int >(0);
        const int segment2 = rec2.getItem("SEGMENT2").get< int >(0);
        BOOST_CHECK_EQUAL( 2, segment1 );
        BOOST_CHECK_EQUAL( 2, segment2 );
        const int branch = rec2.getItem("BRANCH").get< int >(0);
        const int outlet_segment = rec2.getItem("JOIN_SEGMENT").get< int >(0);
        const double segment_length = rec2.getItem("SEGMENT_LENGTH").get< double >(0);
        const double depth_change = rec2.getItem("DEPTH_CHANGE").get< double >(0);
        const double diameter = rec2.getItem("DIAMETER").get< double >(0);
        const double roughness = rec2.getItem("ROUGHNESS").get< double >(0);
        BOOST_CHECK_EQUAL( 1, branch );
        BOOST_CHECK_EQUAL( 1, outlet_segment );
        BOOST_CHECK_EQUAL( 2537.5, segment_length );
        BOOST_CHECK_EQUAL( 2537.5, depth_change );
        BOOST_CHECK_EQUAL( 0.3, diameter );
        BOOST_CHECK_EQUAL( 0.0001, roughness );
    }

    {
        const auto& rec6 = kw.getRecord(5);
        const int segment1 = rec6.getItem("SEGMENT2").get< int >(0);
        const int segment2 = rec6.getItem("SEGMENT2").get< int >(0);
        BOOST_CHECK_EQUAL( 6, segment1 );
        BOOST_CHECK_EQUAL( 6, segment2 );
        const int branch = rec6.getItem("BRANCH").get< int >(0);
        const int outlet_segment = rec6.getItem("JOIN_SEGMENT").get< int >(0);
        const double segment_length = rec6.getItem("SEGMENT_LENGTH").get< double >(0);
        const double depth_change = rec6.getItem("DEPTH_CHANGE").get< double >(0);
        const double diameter = rec6.getItem("DIAMETER").get< double >(0);
        const double roughness = rec6.getItem("ROUGHNESS").get< double >(0);
        BOOST_CHECK_EQUAL( 2, branch );
        BOOST_CHECK_EQUAL( 5, outlet_segment );
        BOOST_CHECK_EQUAL( 3137.5, segment_length );
        BOOST_CHECK_EQUAL( 2537.5, depth_change );
        BOOST_CHECK_EQUAL( 0.2, diameter );
        BOOST_CHECK_EQUAL( 0.0001, roughness );
    }

    // for COMPSEG keyword
    const auto& kw1 = deck->getKeyword("COMPSEGS");
    // check the size of the keywords
    BOOST_CHECK_EQUAL( 7, kw1.size() );
    // first record only contains the well name
    {
        const auto& rec1 = kw1.getRecord(0);
        const std::string well_name = rec1.getItem("WELL").getTrimmedString(0);
        BOOST_CHECK_EQUAL( "PROD01", well_name );
    }

    // check the third record and the seventh record
    {
        const auto& rec3 = kw1.getRecord(2);
        const int i = rec3.getItem("I").get< int >(0);
        const int j = rec3.getItem("J").get< int >(0);
        const int k = rec3.getItem("K").get< int >(0);
        const int branch = rec3.getItem("BRANCH").get< int >(0);
        const double distance_start = rec3.getItem("DISTANCE_START").get< double >(0);
        const double distance_end = rec3.getItem("DISTANCE_END").get< double >(0);

        BOOST_CHECK_EQUAL( 20, i );
        BOOST_CHECK_EQUAL(  1, j );
        BOOST_CHECK_EQUAL(  2, k );
        BOOST_CHECK_EQUAL(  1, branch );
        BOOST_CHECK_EQUAL(  2525.0, distance_start );
        BOOST_CHECK_EQUAL(  2550.0, distance_end );
    }

    {
        const auto& rec7 = kw1.getRecord(6);
        const int i = rec7.getItem("I").get< int >(0);
        const int j = rec7.getItem("J").get< int >(0);
        const int k = rec7.getItem("K").get< int >(0);
        const int branch = rec7.getItem("BRANCH").get< int >(0);
        const double distance_start = rec7.getItem("DISTANCE_START").get< double >(0);
        const double distance_end = rec7.getItem("DISTANCE_END").get< double >(0);

        BOOST_CHECK_EQUAL( 17, i );
        BOOST_CHECK_EQUAL(  1, j );
        BOOST_CHECK_EQUAL(  2, k );
        BOOST_CHECK_EQUAL(  2, branch );
        BOOST_CHECK_EQUAL(  3037.5, distance_start );
        BOOST_CHECK_EQUAL(  3237.5, distance_end );
    }


}
