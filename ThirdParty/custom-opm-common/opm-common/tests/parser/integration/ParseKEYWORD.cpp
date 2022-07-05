/*
  Copyright 2017 Statoil ASA.

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

#define BOOST_TEST_MODULE ParserKeywordsIntegrationTests
#include <boost/test/unit_test.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SlgofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TlpmixpaTable.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>

using namespace Opm;

inline std::string pathprefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE( debug ) {
    Parser().parseFile( pathprefix() + "DEBUG/DEBUG.DATA" );
}

BOOST_AUTO_TEST_CASE( CECON ) {
    const std::string input = R"(
CECON
    PROD1 4*     0.7    /
        'P*'  2* 2 2 1* 3.5 /
/
        )";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( COORDSYS ) {
    const std::string input = R"(
RUNSPEC

BRINE

NUMRES
    1 /
GRID
COORDSYS
    1 1141 /
COORDSYS
    1 1141 'COMP' 'JOIN' 1 2 /
COORDSYS
    1 1141 'COMP' 'JOIN' /
COORDSYS
    1 1141 'INCOMP' /
)";
    const auto& deck = Parser().parseString( input );

    const auto& brine = deck["BRINE"].back();
    BOOST_CHECK_EQUAL(brine.size(), 0);
}

BOOST_AUTO_TEST_CASE( DENSITY ) {
    Parser parser;
    std::string file(pathprefix() + "DENSITY/DENSITY1");
    auto deck =  parser.parseFile(file);
    const auto& densityKw = deck["DENSITY"][0];


    BOOST_CHECK_EQUAL( 2U , densityKw.size());
    const auto& rec1 = densityKw.getRecord(0);

    const auto& oilDensity = rec1.getItem("OIL");
    const auto& waterDensity = rec1.getItem("WATER");
    const auto& gasDensity = rec1.getItem("GAS");

    const auto density = Field::Density;
    BOOST_CHECK_CLOSE(  500 * density, oilDensity.getSIDouble(0),   0.001 );
    BOOST_CHECK_CLOSE( 1000 * density, waterDensity.getSIDouble(0), 0.001 );
    BOOST_CHECK_CLOSE(    1 * density, gasDensity.getSIDouble(0),   0.001 );
}

BOOST_AUTO_TEST_CASE( END ) {
    Parser parser;
    std::string fileWithTitleKeyword = pathprefix() + "END/END1.txt";
    auto deck = parser.parseFile(fileWithTitleKeyword);

    BOOST_CHECK_EQUAL(size_t(1), deck.size());
    BOOST_CHECK_EQUAL(true,  deck.hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("GAS"));
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("END"));
}

BOOST_AUTO_TEST_CASE( ENDINC ) {
    Parser parser;
    std::string fileWithTitleKeyword(pathprefix() + "END/ENDINC1.txt");
    auto deck = parser.parseFile(fileWithTitleKeyword);

    BOOST_CHECK_EQUAL(size_t(1), deck.size());
    BOOST_CHECK_EQUAL(true,  deck.hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("GAS"));
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("ENDINC"));
}

BOOST_AUTO_TEST_CASE( EQUIL_MISSING_DIMS ) {
    Parser parser;
    ErrorGuard errors;
    ParseContext parseContext;
    parseContext.update(ParseContext::PARSE_MISSING_DIMS_KEYWORD, InputError::IGNORE);
    const std::string equil = "EQUIL\n"
        "2469   382.4   1705.0  0.0    500    0.0     1     1      20 /";
    auto deck = parser.parseString(equil, parseContext, errors);
    const auto& kw1 = deck["EQUIL"][0];
    BOOST_CHECK_EQUAL( 1U , kw1.size() );

    const auto& rec1 = kw1.getRecord(0);
    const auto& item1       = rec1.getItem("OWC");
    const auto& item1_index = rec1.getItem(2);

    BOOST_CHECK_EQUAL( &item1  , &item1_index );
    BOOST_CHECK( fabs(item1.getSIDouble(0) - 1705) < 0.001);
}


BOOST_AUTO_TEST_CASE( EQUIL ) {
    Parser parser;
    std::string pvtgFile(pathprefix() + "EQUIL/EQUIL1");
    auto deck =  parser.parseFile(pvtgFile);
    const auto& kw1 = deck["EQUIL"][0];
    BOOST_CHECK_EQUAL( 3U , kw1.size() );

    const auto& rec1 = kw1.getRecord(0);
    const auto& rec3 = kw1.getRecord(2);

    const auto& item1       = rec1.getItem("OWC");
    const auto& item1_index = rec1.getItem(2);

    BOOST_CHECK_EQUAL( &item1  , &item1_index );
    BOOST_CHECK( fabs(item1.getSIDouble(0) - 1705) < 0.001 );

    const auto& item3       = rec3.getItem("OWC");
    const auto& item3_index = rec3.getItem(2);

    BOOST_CHECK_EQUAL( &item3  , &item3_index );
    BOOST_CHECK( fabs(item3.getSIDouble(0) - 3000) < 0.001 );
}

BOOST_AUTO_TEST_CASE( GRUPRIG ) {
    const std::string input = R"(
--      GRUP    WORK    DRILL   RIG
--      NAME    RIG     RIG     OPTN
GRUPRIG
        'FIELD' 1       1       ADD /
        'FIELD' 2*              REMOVE /
/
        )";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( LGR ) {
    Parser().parseFile( pathprefix() + "LGR/LGR.DATA" );
}

const std::string miscibleData = R"(
MISCIBLE
    2  3 /
)";

const std::string miscibleTightData = R"(
MISCIBLE
    1  2 /
)";

const std::string sorwmisData = R"(
SORWMIS
    .00 .00
    .50 .00
    1.0 .00 /
    .00 .00
    .30 .20
    1.0 .80 /
)";

const std::string sgcwmisData = R"(
SGCWMIS
    .00 .00
    .20 .00
    1.0 .00 /
    .00 .00
    .80 .20
    1.0 .70 /
)";

BOOST_AUTO_TEST_CASE( SORWMIS ) {

    Parser parser;
    // missing miscible keyword
    BOOST_CHECK_THROW (parser.parseString(sorwmisData), OpmInputError );

    //too many tables
    BOOST_CHECK_THROW( parser.parseString(miscibleTightData + sorwmisData), OpmInputError);

    auto deck1 =  parser.parseString(miscibleData + sorwmisData);

    const auto& sorwmis = deck1["SORWMIS"].back();
    const auto& miscible = deck1["MISCIBLE"].back();

    const auto& miscible0 = miscible.getRecord(0);
    const auto& sorwmis0 = sorwmis.getRecord(0);
    const auto& sorwmis1 = sorwmis.getRecord(1);

    // test number of columns
    size_t ntmisc = miscible0.getItem(0).get< int >(0);
    Opm::SorwmisTable sorwmisTable0(sorwmis0.getItem(0), 0);
    BOOST_CHECK_EQUAL(sorwmisTable0.numColumns(),ntmisc);

    // test table input 1
    BOOST_CHECK_EQUAL(3U, sorwmisTable0.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(1.0, sorwmisTable0.getWaterSaturationColumn()[2]);
    BOOST_CHECK_EQUAL(0.0, sorwmisTable0.getMiscibleResidualOilColumn()[2]);

    // test table input 2
    Opm::SorwmisTable sorwmisTable1(sorwmis1.getItem(0), 1);
    BOOST_CHECK_EQUAL(sorwmisTable1.numColumns(),ntmisc);

    BOOST_CHECK_EQUAL(3U, sorwmisTable1.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(0.3, sorwmisTable1.getWaterSaturationColumn()[1]);
    BOOST_CHECK_EQUAL(0.8, sorwmisTable1.getMiscibleResidualOilColumn()[2]);
}

BOOST_AUTO_TEST_CASE( SGCWMIS ) {
    Parser parser;
    auto deck1 =  parser.parseString(miscibleData + sgcwmisData);

    const auto& sgcwmis = deck1["SGCWMIS"].back();
    const auto& miscible = deck1["MISCIBLE"].back();

    const auto& miscible0 = miscible.getRecord(0);
    const auto& sgcwmis0 = sgcwmis.getRecord(0);
    const auto& sgcwmis1 = sgcwmis.getRecord(1);

    // test number of columns
    size_t ntmisc = miscible0.getItem(0).get< int >(0);
    Opm::SgcwmisTable sgcwmisTable0(sgcwmis0.getItem(0), 0);
    BOOST_CHECK_EQUAL(sgcwmisTable0.numColumns(),ntmisc);

    // test table input 1
    BOOST_CHECK_EQUAL(3U, sgcwmisTable0.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(0.2, sgcwmisTable0.getWaterSaturationColumn()[1]);
    BOOST_CHECK_EQUAL(0.0, sgcwmisTable0.getMiscibleResidualGasColumn()[1]);

    // test table input 2
    Opm::SgcwmisTable sgcwmisTable1(sgcwmis1.getItem(0), 1);
    BOOST_CHECK_EQUAL(sgcwmisTable1.numColumns(),ntmisc);

    BOOST_CHECK_EQUAL(3U, sgcwmisTable1.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(0.8, sgcwmisTable1.getWaterSaturationColumn()[1]);
    BOOST_CHECK_EQUAL(0.2, sgcwmisTable1.getMiscibleResidualGasColumn()[1]);
}

const std::string miscData = R"(
MISCIBLE
    1  3 /

MISC
    0.0 0.0
    0.1 0.5
    1.0 1.0 /
)";

const std::string miscOutOfRangeData = R"(
MISCIBLE
    1  3 /

MISC
    0.0 0.0
    1.0 0.5
    2.0 1.0 /
)";

const std::string miscTooSmallRangeData = R"(
MISCIBLE
    1  3 /

MISC
    0.0 0.0
    1.0 0.5 /
)";

BOOST_AUTO_TEST_CASE( MISC ) {
    Parser parser;

    // out of range MISC keyword
    auto deck1 = parser.parseString(miscOutOfRangeData);
    const auto& item = deck1["MISC"].back().getRecord(0).getItem(0);
    Opm::MiscTable miscTable1(item, 0);

    // too litle range of MISC keyword
    auto deck2 = parser.parseString(miscTooSmallRangeData);
    const auto& item2 = deck2["MISC"].back().getRecord(0).getItem(0);
    Opm::MiscTable miscTable2(item2, 0);

    // test table input
    auto deck3 =  parser.parseString(miscData);
    const auto& item3 = deck3["MISC"].back().getRecord(0).getItem(0);
    Opm::MiscTable miscTable3(item3, 0);
    BOOST_CHECK_EQUAL(3U, miscTable3.getSolventFractionColumn().size());
    BOOST_CHECK_EQUAL(0.1, miscTable3.getSolventFractionColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, miscTable3.getMiscibilityColumn()[1]);
}

const std::string pmiscData = R"(
MISCIBLE
    1  3 /

PMISC
    100 0.0
    200 0.5
    500 1.0 /
)";

BOOST_AUTO_TEST_CASE( PMISC ) {
    Parser parser;
    auto deck =  parser.parseString(pmiscData);
    Opm::PmiscTable pmiscTable(deck["PMISC"].back().getRecord(0).getItem(0), 0);
    BOOST_CHECK_EQUAL(3U, pmiscTable.getOilPhasePressureColumn().size());
    BOOST_CHECK_EQUAL(200*1e5, pmiscTable.getOilPhasePressureColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, pmiscTable.getMiscibilityColumn()[1]);
}

const std::string msfnData = R"(
TABDIMS
    2 /

MSFN
    0.0 0.0 1.0
    1.0 1.0 0.0 /
    0.0 0.0 1.0
    0.5 0.3 0.7
    1.0 1.0 0.0 /
)";

BOOST_AUTO_TEST_CASE( MSFN ) {
    Parser parser;
    auto deck =  parser.parseString(msfnData);

    Opm::MsfnTable msfnTable1(deck["MSFN"].back().getRecord(0).getItem(0), 0);
    BOOST_CHECK_EQUAL(2U, msfnTable1.getGasPhaseFractionColumn().size());
    BOOST_CHECK_EQUAL(1.0, msfnTable1.getGasPhaseFractionColumn()[1]);
    BOOST_CHECK_EQUAL(1.0, msfnTable1.getGasSolventRelpermMultiplierColumn()[1]);
    BOOST_CHECK_EQUAL(0.0, msfnTable1.getOilRelpermMultiplierColumn()[1]);

    Opm::MsfnTable msfnTable2(deck["MSFN"].back().getRecord(1).getItem(0), 1);
    BOOST_CHECK_EQUAL(3U, msfnTable2.getGasPhaseFractionColumn().size());
    BOOST_CHECK_EQUAL(0.5, msfnTable2.getGasPhaseFractionColumn()[1]);
    BOOST_CHECK_EQUAL(0.3, msfnTable2.getGasSolventRelpermMultiplierColumn()[1]);
    BOOST_CHECK_EQUAL(0.7, msfnTable2.getOilRelpermMultiplierColumn()[1]);
}

BOOST_AUTO_TEST_CASE( TLPMIXPA ) {
    const std::string tlpmixpa = R"(
        MISCIBLE
        1  3 /

        TLPMIXPA
        100 0.0
        200 0.5
        500 1.0 /
    )";

    Parser parser;
    auto deck =  parser.parseString(tlpmixpa);
    Opm::TlpmixpaTable tlpmixpaTable(deck["TLPMIXPA"].back().getRecord(0).getItem(0), 0);
    BOOST_CHECK_EQUAL(3U, tlpmixpaTable.getOilPhasePressureColumn().size());
    BOOST_CHECK_EQUAL(200*1e5, tlpmixpaTable.getOilPhasePressureColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, tlpmixpaTable.getMiscibilityColumn()[1]);
}

BOOST_AUTO_TEST_CASE( MULTREGT_ECLIPSE_STATE ) {
    Parser parser;
    auto deck =  parser.parseFile(pathprefix() + "MULTREGT/MULTREGT.DATA");
    EclipseState state(deck);
    const auto& transMult = state.getTransMult();

    // Test NONNC
    // cell 0 and 1 are neigbour
    // The last occurence of multiplier between region 1 and 2 is 0.2.
    // Note that MULTREGT is not direction dependent
    BOOST_CHECK_EQUAL( 0.20 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::XPlus));
    // cell 0 and 3 are not neigbours ==> 1
    BOOST_CHECK_EQUAL( 0.20 , transMult.getRegionMultiplier( 0 , 3 , FaceDir::DirEnum::XPlus));

    // Test NNC
    // cell 4 and 5 are neigbours ==> 1
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 4 , 5 , FaceDir::DirEnum::XPlus));
    // cell 4 and 7 are not neigbours
    BOOST_CHECK_EQUAL( 0.50 , transMult.getRegionMultiplier( 4 , 7 , FaceDir::DirEnum::XPlus));

    // Test direction X, returns 1 for directions other than +-X
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::YPlus));
    BOOST_CHECK_EQUAL( 1.00 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::ZPlus));
    BOOST_CHECK_EQUAL( 0.20 , transMult.getRegionMultiplier( 0 , 1 , FaceDir::DirEnum::XMinus));
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

    // The first record is overwritten by the third since MULTREGT is direction dependent
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::YPlus));

    // The 2 4 0.75 Z input is overwritten by 2 4 2.5 XY, ==) that 2 4 Z returns the 4 2 value = 0.6
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 7 , 3 , FaceDir::DirEnum::XPlus));
    BOOST_CHECK_EQUAL( 0.60 , transMult.getRegionMultiplier( 3 , 7 , FaceDir::DirEnum::ZPlus));
}

BOOST_AUTO_TEST_CASE( MULTISEGMENT_ABS ) {
    Parser parser;
    const std::string deckFile(pathprefix() + "SCHEDULE/SCHEDULE_MULTISEGMENT_WELL");
    const auto deck =  parser.parseFile(deckFile);

    // for WELSEGS keyword
    const auto& kw = deck["WELSEGS"].back();

    BOOST_CHECK_EQUAL( 7, kw.size() );

    // check the information for the top segment and the segment set
    {
        const auto& rec1 = kw.getRecord(0); // top segment

        const std::string well_name = rec1.getItem("WELL").getTrimmedString(0);
        const double depth_top = rec1.getItem("DEPTH").get< double >(0);
        const double length_top = rec1.getItem("LENGTH").get< double >(0);
        const double volume_top = rec1.getItem("WELLBORE_VOLUME").get< double >(0);
        const WellSegments::LengthDepth length_depth_type = WellSegments::LengthDepthFromString(rec1.getItem("INFO_TYPE").getTrimmedString(0));
        const WellSegments::CompPressureDrop comp_pressure_drop = WellSegments::CompPressureDropFromString(rec1.getItem("PRESSURE_COMPONENTS").getTrimmedString(0));
        const WellSegments::MultiPhaseModel multiphase_model = WellSegments::MultiPhaseModelFromString(rec1.getItem("FLOW_MODEL").getTrimmedString(0));

        BOOST_CHECK_EQUAL( "PROD01", well_name );
        BOOST_CHECK_EQUAL( 2512.5, depth_top );
        BOOST_CHECK_EQUAL( 2512.5, length_top );
        BOOST_CHECK_EQUAL( 1.0e-5, volume_top );
        const std::string length_depth_type_string = WellSegments::LengthDepthToString(length_depth_type);
        BOOST_CHECK_EQUAL( length_depth_type_string, "ABS" );
        const std::string comp_pressure_drop_string = WellSegments::CompPressureDropToString(comp_pressure_drop);
        BOOST_CHECK_EQUAL( comp_pressure_drop_string, "HF-" );
        const std::string multiphase_model_string = WellSegments::MultiPhaseModelToString(multiphase_model);
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
        const auto& rec6 = kw.getRecord(4);
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
        BOOST_CHECK_EQUAL( 4, outlet_segment );
        BOOST_CHECK_EQUAL( 3037.5, segment_length );
        BOOST_CHECK_EQUAL( 2539.5, depth_change );
        BOOST_CHECK_EQUAL( 0.2, diameter );
        BOOST_CHECK_EQUAL( 0.0001, roughness );
    }

    {
        const auto& rec7 = kw.getRecord(6);
        const int segment1 = rec7.getItem("SEGMENT2").get< int >(0);
        const int segment2 = rec7.getItem("SEGMENT2").get< int >(0);
        BOOST_CHECK_EQUAL( 8, segment1 );
        BOOST_CHECK_EQUAL( 8, segment2 );
        const int branch = rec7.getItem("BRANCH").get< int >(0);
        const int outlet_segment = rec7.getItem("JOIN_SEGMENT").get< int >(0);
        const double segment_length = rec7.getItem("SEGMENT_LENGTH").get< double >(0);
        const double depth_change = rec7.getItem("DEPTH_CHANGE").get< double >(0);
        const double diameter = rec7.getItem("DIAMETER").get< double >(0);
        const double roughness = rec7.getItem("ROUGHNESS").get< double >(0);
        BOOST_CHECK_EQUAL( 3, branch );
        BOOST_CHECK_EQUAL( 7, outlet_segment );
        BOOST_CHECK_EQUAL( 3337.6, segment_length );
        BOOST_CHECK_EQUAL( 2534.5, depth_change );
        BOOST_CHECK_EQUAL( 0.2, diameter );
        BOOST_CHECK_EQUAL( 0.00015, roughness );
    }

    // for COMPSEG keyword
    const auto& kw1 = deck["COMPSEGS"].back();
    // check the size of the keywords
    BOOST_CHECK_EQUAL( 8, kw1.size() );
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


    const EclipseState state(deck);
    const TableManager table ( deck );
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    const Schedule sched(deck, state, python);

    // checking the relation between segments and completions
    // and also the depth of completions
    BOOST_CHECK(sched.hasWell("PROD01"));
    const auto& well = sched.getWell("PROD01", 0);
    const auto& connections = well.getConnections();
    BOOST_CHECK_EQUAL(7U, connections.size());

    const Connection& connection5 = connections.get(4);
    const int seg_number_connection5 = connection5.segment();
    const double connection5_depth = connection5.depth();
    BOOST_CHECK_EQUAL(seg_number_connection5, 6);
    BOOST_CHECK_CLOSE(connection5_depth, 2538.83, 0.001);

    const Connection& connection6 = connections.get(5);
    const int seg_number_connection6 = connection6.segment();
    const double connection6_depth = connection6.depth();
    BOOST_CHECK_EQUAL(seg_number_connection6, 6);
    BOOST_CHECK_CLOSE(connection6_depth, 2537.83, 0.001);

    const Connection& connection1 = connections.get(0);
    const int seg_number_connection1 = connection1.segment();
    const double connection1_depth = connection1.depth();
    BOOST_CHECK_EQUAL(seg_number_connection1, 1);
    BOOST_CHECK_EQUAL(connection1_depth, 2512.5);

    const Connection& connection3 = connections.get(2);
    const int seg_number_connection3 = connection3.segment();
    const double connection3_depth = connection3.depth();
    BOOST_CHECK_EQUAL(seg_number_connection3, 3);
    BOOST_CHECK_EQUAL(connection3_depth, 2562.5);

}

BOOST_AUTO_TEST_CASE( PLYADS ) {
    Parser parser;
    auto deck =  parser.parseFile(pathprefix() + "POLYMER/plyads.data");
    const auto& kw   = deck["PLYADS"].back();
    const auto& rec  = kw.getRecord(0);
    const auto& item = rec.getItem(0);

    BOOST_CHECK_EQUAL( 0.0 , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 0.25 , item.get< double >(2) );
}

BOOST_AUTO_TEST_CASE( PLYADSS ) {
    Parser parser;
    std::string deckFile(pathprefix() + "POLYMER/plyadss.data");
    auto deck =  parser.parseFile(deckFile);
    const auto& kw = deck["PLYADSS"].back();
    BOOST_CHECK_EQUAL( kw.size() , 11U );
}

BOOST_AUTO_TEST_CASE( PLYDHFLF ) {
    Parser parser;
    std::string deckFile(pathprefix() + "POLYMER/plydhflf.data");
    auto deck =  parser.parseFile(deckFile);
    const auto& kw = deck["PLYDHFLF"].back();
    const auto& rec = kw.getRecord(0);
    const auto& item = rec.getItem(0);

    BOOST_CHECK_EQUAL( 0.0 , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 365.0,  item.get< double >(1) );
    BOOST_CHECK_EQUAL( 200.0 , item.get< double >(5) );
}

BOOST_AUTO_TEST_CASE( PLYSHLOG ) {
    Parser parser;
    std::string deckFile(pathprefix() + "POLYMER/plyshlog.data");
    auto deck =  parser.parseFile(deckFile);
    const auto& kw = deck["PLYSHLOG"].back();
    const auto& rec1 = kw.getRecord(0); // reference conditions

    const auto& itemRefPolyConc = rec1.getItem("REF_POLYMER_CONCENTRATION");
    const auto& itemRefSali = rec1.getItem("REF_SALINITY");
    const auto& itemRefTemp = rec1.getItem("REF_TEMPERATURE");

    BOOST_CHECK_EQUAL( true, itemRefPolyConc.hasValue(0) );
    BOOST_CHECK_EQUAL( true, itemRefSali.hasValue(0) );
    BOOST_CHECK_EQUAL( false, itemRefTemp.hasValue(0) );

    BOOST_CHECK_EQUAL( 1.0, itemRefPolyConc.get< double >(0) );
    BOOST_CHECK_EQUAL( 3.0, itemRefSali.get< double >(0) );

    const auto& rec2 = kw.getRecord(1);
    const auto& itemData = rec2.getItem(0);

    BOOST_CHECK_EQUAL( 1.e-7 , itemData.get< double >(0) );
    BOOST_CHECK_EQUAL( 1.0 , itemData.get< double >(1) );
    BOOST_CHECK_EQUAL( 1.0 , itemData.get< double >(2) );
    BOOST_CHECK_EQUAL( 1.2 , itemData.get< double >(3) );
    BOOST_CHECK_EQUAL( 1.e3 , itemData.get< double >(4) );
    BOOST_CHECK_EQUAL( 2.4 , itemData.get< double >(5) );
}

BOOST_AUTO_TEST_CASE( PLYVISC ) {
    Parser parser;
    std::string deckFile(pathprefix() + "POLYMER/plyvisc.data");
    auto deck =  parser.parseFile(deckFile);
    const auto& kw = deck["PLYVISC"].back();
    const auto& rec = kw.getRecord(0);
    const auto& item = rec.getItem(0);

    BOOST_CHECK_EQUAL( 0.0 , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 1.25 , item.get< double >(2) );
}

BOOST_AUTO_TEST_CASE( PORO_PERMX ) {
    Parser parser;
    std::string poroFile(pathprefix() + "PORO/PORO1");
    auto deck =  parser.parseFile(poroFile);
    const auto& kw1 = deck["PORO"][0];
    const auto& kw2 = deck["PERMX"][0];

    BOOST_CHECK_THROW( kw1.getIntData() , std::logic_error );
    BOOST_CHECK_THROW( kw1.getStringData() , std::logic_error );

    const std::vector<double>& poro = kw1.getRawDoubleData();
    BOOST_CHECK_EQUAL( 440U , poro.size() );
    BOOST_CHECK_EQUAL( 0.233782813 , poro[0]);
    BOOST_CHECK_EQUAL( 0.251224369 , poro[1]);
    BOOST_CHECK_EQUAL( 0.155628711 , poro[439]);

    const auto& permx = kw2.getSIDoubleData();
    BOOST_CHECK_EQUAL( 1000U , permx.size() );

    BOOST_CHECK_CLOSE( Metric::Permeability * 1 , permx[0] , 0.001);
    BOOST_CHECK_CLOSE( Metric::Permeability * 2 , permx[1] , 0.001);
    BOOST_CHECK_CLOSE( Metric::Permeability * 3 , permx[2] , 0.001);
    BOOST_CHECK_CLOSE( Metric::Permeability * 10, permx[999] , 0.001);
}

BOOST_AUTO_TEST_CASE( PRORDER ) {
    const std::string input = R"(
--      PROD    NO 1 NO 2 NO 3 NO 4 NO 5
--      ORDER   OPTN OPTN OPTN OPTN OPTN
PRORDER
    DRILL   /
    NO      /

PRORDER
    DRILL   THP  REPREF /
    NO      /
        )";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( RSVD ) {
    Parser parser;
    std::string pvtgFile(pathprefix() + "RSVD/RSVD.txt");
    auto deck =  parser.parseFile(pvtgFile);
    const auto& kw1 = deck["RSVD"][0];
    BOOST_CHECK_EQUAL( 6U , kw1.size() );

    const auto& rec1 = kw1.getRecord(0);
    const auto& rec3 = kw1.getRecord(2);

    const auto& item1       = rec1.getItem("DATA");
    BOOST_CHECK( fabs(item1.getSIDouble(0) - 2382) < 0.001);

    const auto& item3       = rec3.getItem("DATA");
    BOOST_CHECK( fabs(item3.getSIDouble(7) - 106.77) < 0.001);
}

BOOST_AUTO_TEST_CASE( PVTG ) {
const std::string pvtgData = R"(
TABDIMS
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT
     1      2     30    24    10    20  /

PVTG
--
     20.00    0.00002448   0.061895     0.01299
              0.00001224   0.061810     0.01300
              0.00000000   0.061725     0.01300 /
     40.00    0.00000628   0.030252     0.01383
              0.00000314   0.030249     0.01383
              0.00000000   0.030245     0.01383 /
/
    197.66    0.00006327   1*           0.02160
              0.00003164   *            0.02122
              0.00000000   0.005860     0.02086 /
    231.13    0.00010861   0.005042     0.02477
              0.00005431   0.005061     0.02389
              0.00000000   0.005082     0.02306 /
/
)";

    Parser parser;
    auto deck =  parser.parseString(pvtgData);
    const auto& kw1 = deck["PVTG"][0];
    BOOST_CHECK_EQUAL(5U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    const auto& record1 = kw1.getRecord(1);
    const auto& record2 = kw1.getRecord(2);
    const auto& record3 = kw1.getRecord(3);
    const auto& record4 = kw1.getRecord(4);

    const auto& item0_0 = record0.getItem("GAS_PRESSURE");
    const auto& item0_1 = record0.getItem("DATA");
    BOOST_CHECK(item0_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item0_1.data_size());

    const auto& item1_0 = record1.getItem("GAS_PRESSURE");
    const auto& item1_1 = record1.getItem("DATA");
    BOOST_CHECK(item1_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item1_1.data_size());

    const auto& item2_0 = record2.getItem("GAS_PRESSURE");
    const auto& item2_1 = record2.getItem("DATA");
    BOOST_CHECK( item2_0.defaultApplied(0));
    BOOST_CHECK_EQUAL(0U , item2_1.data_size());


    const auto& item3_0 = record3.getItem("GAS_PRESSURE");
    const auto& item3_1 = record3.getItem("DATA");
    BOOST_CHECK( !item3_1.defaultApplied(0));
    BOOST_CHECK( item3_1.defaultApplied(1));
    BOOST_CHECK( !item3_1.defaultApplied(2));
    BOOST_CHECK( !item3_1.defaultApplied(3));
    BOOST_CHECK( item3_1.defaultApplied(4));
    BOOST_CHECK( !item3_1.defaultApplied(5));
    BOOST_CHECK(item3_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item3_1.data_size());


    const auto& item4_0 = record4.getItem("GAS_PRESSURE");
    const auto& item4_1 = record4.getItem("DATA");
    BOOST_CHECK(item4_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item4_1.data_size());

    /*
    {
        Opm::PvtgTable pvtgTable;
        pvtgTable.initFORUNITTESTONLY(kw1, 0);

        const auto &outerTable = *pvtgTable.getOuterTable();
        const auto &innerTable0 = *pvtgTable.getInnerTable(0);

        BOOST_CHECK_EQUAL(2U, outerTable.numRows());
        BOOST_CHECK_EQUAL(4U, outerTable.numColumns());
        BOOST_CHECK_EQUAL(3U, innerTable0.numRows());
        BOOST_CHECK_EQUAL(3U, innerTable0.numColumns());

        BOOST_CHECK_EQUAL(20.0e5, outerTable.getPressureColumn()[0]);
        BOOST_CHECK_EQUAL(0.00002448, outerTable.getOilSolubilityColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getOilSolubilityColumn()[0], innerTable0.getOilSolubilityColumn()[0]);
        BOOST_CHECK_EQUAL(0.061895, outerTable.getGasFormationFactorColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getGasFormationFactorColumn()[0], innerTable0.getGasFormationFactorColumn()[0]);
        BOOST_CHECK_EQUAL(1.299e-5, outerTable.getGasViscosityColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getGasViscosityColumn()[0], innerTable0.getGasViscosityColumn()[0]);
    }
    */
}

BOOST_AUTO_TEST_CASE( PVTO ) {
const std::string pvtoData = R"(
TABDIMS
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT
     1      2     30    24    10    20  /

PVTO
--   Rs       PO           BO           MUO
     1e-3     1            1.20        1.02
              250          1.15         0.95
              500          1.01        0.93 /
     1e-2     14.8         1.30        1.03
              251          1.25         0.98
              502          1.05         0.95 /
/
     1e-1     1.1          1.21         1.03
              253          1.16         0.96
              504          1.02         0.97 /
     1e00     15           1.31         1.04
              255          1.26         0.99
              506          1.06         0.96 /
/
)";

    Parser parser;
    auto deck =  parser.parseString(pvtoData);
    const auto& kw1 = deck["PVTO"][0];
    BOOST_CHECK_EQUAL(5U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    const auto& record1 = kw1.getRecord(1);
    const auto& record2 = kw1.getRecord(2);
    const auto& record3 = kw1.getRecord(3);
    const auto& record4 = kw1.getRecord(4);

    const auto& item0_0 = record0.getItem("RS");
    const auto& item0_1 = record0.getItem("DATA");
    BOOST_CHECK(item0_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item0_1.data_size());
    BOOST_CHECK_EQUAL(2U , record0.size());

    const auto& item1_0 = record1.getItem("RS");
    const auto& item1_1 = record1.getItem("DATA");
    BOOST_CHECK(item1_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item1_1.data_size());
    BOOST_CHECK_EQUAL(2U , record1.size());

    const auto& item2_0 = record2.getItem("RS");
    const auto& item2_1 = record2.getItem("DATA");
    BOOST_CHECK(item2_0.defaultApplied(0));
    BOOST_CHECK_EQUAL(0U , item2_1.data_size());
    BOOST_CHECK_EQUAL(2U , record2.size());

    const auto& item3_0 = record3.getItem("RS");
    const auto& item3_1 = record3.getItem("DATA");
    BOOST_CHECK(item3_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item3_1.data_size());
    BOOST_CHECK_EQUAL(2U , record3.size());

    const auto& item4_0 = record4.getItem("RS");
    const auto& item4_1 = record4.getItem("DATA");
    BOOST_CHECK(item4_0.hasValue(0));
    BOOST_CHECK_EQUAL(9U , item4_1.data_size());
    BOOST_CHECK_EQUAL(2U , record4.size());


    Opm::PvtoTable pvtoTable(kw1 , 0);
    BOOST_CHECK_EQUAL(2, pvtoTable.size());

    const auto &table0 = pvtoTable.getUnderSaturatedTable(0);
    const auto& BO = table0.getColumn( "BO" );

    BOOST_CHECK_EQUAL( 3, table0.numRows());
    BOOST_CHECK_EQUAL( 3, table0.numColumns());
    BOOST_CHECK_EQUAL( BO.front( ) , 1.20 );
    BOOST_CHECK_EQUAL( BO.back( ) , 1.01 );

    BOOST_CHECK_CLOSE(1.15 , table0.evaluate( "BO" , 250*1e5 ) , 1e-6);

    BOOST_CHECK_CLOSE( 1.15 , pvtoTable.evaluate( "BO" , 1e-3 , 250*1e5 ) , 1e-6 );
    BOOST_CHECK_CLOSE( 1.15 , pvtoTable.evaluate( "BO" , 0.0 , 250*1e5 ) , 1e-6 );
}

BOOST_AUTO_TEST_CASE( SGOF ) {
const std::string parserData = R"(
GAS
OIL

TABDIMS
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT
        1     1     30     1     1     1 /

--  S_g k_rg k_rog p_cog
SGOF
    0.1 0.0 1.0 0.0
    0.2 0.1 1.0 1.0
    0.3 0.2 0.9 2.0
    0.4 0.3 0.8 3.0
    0.5 0.5 0.5 4.0
    0.6 0.6 0.4 5.0
    0.7 0.8 0.3 6.0
    0.8 0.9 0.2 7.0
    0.9 0.5 0.1 8.0
    1.0 1.0 0.1 9.0 /;
)";
    Parser parser;
    auto deck =  parser.parseString(parserData);

    const auto& kw1 = deck["SGOF"].back();
    BOOST_CHECK_EQUAL(1U , kw1.size());
    const auto& record0 = kw1.getRecord(0);
    BOOST_CHECK_EQUAL(1U , record0.size());
    const auto& item0 = record0.getItem(0);
    BOOST_CHECK_EQUAL(10U * 4, item0.data_size());

    Opm::SgofTable sgofTable(deck["SGOF"].back().getRecord(0).getItem(0), false, 0);
    BOOST_CHECK_EQUAL(10U, sgofTable.getSgColumn().size());
    BOOST_CHECK_EQUAL(0.1, sgofTable.getSgColumn()[0]);
    BOOST_CHECK_EQUAL(0.0, sgofTable.getKrgColumn()[0]);
    BOOST_CHECK_EQUAL(1.0, sgofTable.getKrogColumn()[0]);
    BOOST_CHECK_EQUAL(0.0, sgofTable.getPcogColumn()[0]);
}

BOOST_AUTO_TEST_CASE( SWOF ) {

    const std::string parserData = R"(
OIL
WATER

TABDIMS
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT
        1     1     30     1     1     1 /

--  S_w k_rw k_row p_cow
SWOF
    0.1  2*     0.0
    0.2 0.1 1.0 1.0
    0.3  1* 0.9 2.0
    0.4 0.3  1* 3.0
    0.5 0.5 0.5 4.0
    0.6 0.6 0.4  1*
    0.7 0.8 0.3 6.0
    0.8 0.9 0.2 7.0
    0.9 0.5 0.1 8.0
    1.0  1* 0.1 9.0 /
)";
    Parser parser;
    auto deck =  parser.parseString(parserData);

    const auto& kw1 = deck["SWOF"].back();
    const auto& record0 = kw1.getRecord(0);
    const auto& item0 = record0.getItem(0);
    BOOST_CHECK_EQUAL(1U , kw1.size());
    BOOST_CHECK_EQUAL(1U , record0.size());
    BOOST_CHECK_EQUAL(10U * 4, item0.data_size());

    Opm::SwofTable swofTable(deck["SWOF"].back().getRecord(0).getItem(0), false, 0);
    BOOST_CHECK_EQUAL(10U, swofTable.getSwColumn().size());
    BOOST_CHECK_CLOSE(0.1, swofTable.getSwColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(1.0, swofTable.getSwColumn().back(), 1e-8);

    BOOST_CHECK_CLOSE(0.1, swofTable.getKrwColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(0.1, swofTable.getKrwColumn()[1], 1e-8);
    BOOST_CHECK_CLOSE(0.2, swofTable.getKrwColumn()[2], 1e-8);
    BOOST_CHECK_CLOSE(0.3, swofTable.getKrwColumn()[3], 1e-8);
    BOOST_CHECK_CLOSE(0.5, swofTable.getKrwColumn().back(), 1e-8);

    BOOST_CHECK_CLOSE(1.0, swofTable.getKrowColumn()[0], 1e-8);
    BOOST_CHECK_CLOSE(0.9, swofTable.getKrowColumn()[2], 1e-8);
    BOOST_CHECK_CLOSE(0.7, swofTable.getKrowColumn()[3], 1e-8);
    BOOST_CHECK_CLOSE(0.5, swofTable.getKrowColumn()[4], 1e-8);

    BOOST_CHECK_CLOSE(4.0e5, swofTable.getPcowColumn()[4], 1e-8);
    BOOST_CHECK_CLOSE(5.0e5, swofTable.getPcowColumn()[5], 1e-8);
    BOOST_CHECK_CLOSE(6.0e5, swofTable.getPcowColumn()[6], 1e-8);

    BOOST_CHECK_CLOSE(0.10, swofTable.evaluate("KRW", -0.1), 1e-8);
    BOOST_CHECK_CLOSE(0.15, swofTable.evaluate("KRW", 0.25), 1e-8);
    BOOST_CHECK_CLOSE(0.50, swofTable.evaluate("KRW", 1.1), 1e-8);
}

BOOST_AUTO_TEST_CASE( SLGOF ) {

const std::string parserData = R"(
GAS

TABDIMS
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT
        1     1     30     1     1     1 /

--  S_l k_rg k_rog p_cog
SLGOF
    0.1 1.0 0.0 9.0
    0.2 0.9 0.2 8.0
    0.3 0.8 0.3 7.0
    0.4 0.7 0.3 6.0
    0.5 0.6 0.4 5.0
    0.6 0.5 0.5 4.0
    0.7 0.3 0.8 3.0
    0.8 0.2 0.9 2.0
    0.9 0.1 1.0 1.0
    1.0 0.0 1.0 0.0 /;
)";

    Parser parser;
    auto deck =  parser.parseString(parserData);

    const auto& kw1 = deck["SLGOF"].back();
    const auto& record0 = kw1.getRecord(0);
    const auto& item0 = record0.getItem(0);
    BOOST_CHECK_EQUAL(1U , kw1.size());
    BOOST_CHECK_EQUAL(1U , record0.size());
    BOOST_CHECK_EQUAL(10U * 4, item0.data_size());

    Opm::SlgofTable slgofTable( deck["SLGOF"].back().getRecord(0).getItem(0), false, 0 );
    BOOST_CHECK_EQUAL(10U, slgofTable.getSlColumn().size());
    BOOST_CHECK_EQUAL(0.1, slgofTable.getSlColumn()[0]);
    BOOST_CHECK_EQUAL(1.0, slgofTable.getSlColumn()[9]);
    BOOST_CHECK_EQUAL(0.0, slgofTable.getKrgColumn()[9]);
    BOOST_CHECK_EQUAL(1.0, slgofTable.getKrogColumn()[9]);
    BOOST_CHECK_EQUAL(0.0, slgofTable.getPcogColumn()[9]);
}

BOOST_AUTO_TEST_CASE( TITLE ) {
    Parser parser;
    std::string fileWithTitleKeyword(pathprefix() + "TITLE/TITLE1.txt");

    auto deck = parser.parseFile(fileWithTitleKeyword);

    BOOST_CHECK_EQUAL(size_t(2), deck.size());
    BOOST_CHECK_EQUAL (true, deck.hasKeyword("TITLE"));

    const auto& titleKeyword = deck["TITLE"].back();
    const auto& record = titleKeyword.getRecord(0);
    const auto& item = record.getItem(0);

    std::vector<std::string> itemValue = item.getData< std::string >();
    std::vector<std::string> expected = {"This", "is", "the", "title", "of", "the", "model."};
    BOOST_CHECK(itemValue == expected);
    BOOST_CHECK_EQUAL(true, deck.hasKeyword("START"));
}

BOOST_AUTO_TEST_CASE( TOPS ) {
    Parser parser;
    std::string deckFile(pathprefix() + "GRID/TOPS.DATA");
    auto deck =  parser.parseFile(deckFile);
    EclipseState state(deck);
    const auto& grid = state.getInputGrid();

    BOOST_CHECK_EQUAL( grid.getNX() , 9 );
    BOOST_CHECK_EQUAL( grid.getNY() , 9 );
    BOOST_CHECK_EQUAL( grid.getNZ() , 2 );

    for (size_t g=0; g < 9*9*2; g++)
        BOOST_CHECK_CLOSE( grid.getCellVolume( g ) , 400*300*10 , 0.1);

    for (size_t k=0; k < grid.getNZ(); k++) {
        for (size_t j=0; j < grid.getNY(); j++) {
            for (size_t i=0; i < grid.getNX(); i++) {
                auto pos = grid.getCellCenter( i,j,k );
                BOOST_CHECK_CLOSE( std::get<0>(pos) , i*400 + 200 , 0.10 );
                BOOST_CHECK_CLOSE( std::get<1>(pos) , j*300 + 150 , 0.10 );
                BOOST_CHECK_CLOSE( std::get<2>(pos) , k*10  + 5 + 2202 , 0.10 );
            }
        }
    }
}

BOOST_AUTO_TEST_CASE( TRACERS ) {
    const std::string input = R"(
RUNSPEC
--      NO OIL  NO WAT  NO GAS  NO ENV  DIFF     MAX    MIN    TRACER
--      TRACERS TRACERS TRACERS TRACERS CONTL    NONLIN NONLIN NONLIN
TRACERS
        0       0       1       0       'NODIFF' 1*     1*      1*    /

)";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( TUNINGDP ) {
    const std::string input = R"(
TUNINGDP
/
TUNINGDP
    1.0
/
TUNINGDP
    1.0 2.0
/
TUNINGDP
    1.0 2.0 3.0
/
TUNINGDP
    1.0 2.0 3.0 4.0
/

)";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( TVDP ) {
    Parser parser;
    std::string poroFile(pathprefix() + "TVDP/TVDP1");
    auto deck =  parser.parseFile(poroFile);

    BOOST_CHECK(!deck.hasKeyword("TVDP*"));
    BOOST_CHECK( deck.hasKeyword("TVDPA"));
    BOOST_CHECK( deck.hasKeyword("TVDP1"));
    BOOST_CHECK( deck.hasKeyword("TVDPXX"));
    BOOST_CHECK( deck.hasKeyword("TVDPYY"));
}

BOOST_AUTO_TEST_CASE( VFPPROD ) {
    Parser parser;
    std::string file(pathprefix() + "VFPPROD/VFPPROD1");
    BOOST_CHECK( parser.isRecognizedKeyword("VFPPROD"));

    auto deck =  parser.parseFile(file);
    const auto& VFPPROD1 = deck["VFPPROD"][0];
    const auto& BPR = deck["BPR"][0];
    const auto& VFPPROD2 = deck["VFPPROD"][1];

    BOOST_CHECK_EQUAL( 573U  , VFPPROD1.size() );
    BOOST_CHECK_EQUAL( 1U    , BPR.size());
    BOOST_CHECK_EQUAL( 573U  , VFPPROD2.size());

    {
        const auto& record = VFPPROD1.getRecord(0);

        BOOST_CHECK_EQUAL( record.getItem("TABLE").get< int >(0) , 32 );
        BOOST_CHECK_EQUAL( record.getItem("DATUM_DEPTH").getSIDouble(0) , 394);
        BOOST_CHECK_EQUAL( record.getItem("RATE_TYPE").get< std::string >(0) , "LIQ");
        BOOST_CHECK_EQUAL( record.getItem("WFR").get< std::string >(0) , "WCT");
        BOOST_CHECK_EQUAL( record.getItem("GFR").get< std::string >(0) , "GOR");
    }

    {
        const auto& record = VFPPROD1.getRecord(1);
        const auto& item = record.getItem("FLOW_VALUES");

        BOOST_CHECK_EQUAL( item.data_size() , 12 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   100 );
        BOOST_CHECK_EQUAL( item.get< double >(11) , 20000 );
    }

    {
        const auto& record = VFPPROD1.getRecord(2);
        const auto& item = record.getItem("THP_VALUES");

        BOOST_CHECK_EQUAL( item.data_size() , 7 );
        BOOST_CHECK_CLOSE( item.get< double >(0)  , 16.01 , 0.0001 );
        BOOST_CHECK_CLOSE( item.get< double >(6) ,  61.01 , 0.0001 );
    }

    {
        const auto& record = VFPPROD1.getRecord(3);
        const auto& item = record.getItem("WFR_VALUES");

        BOOST_CHECK_EQUAL( item.data_size() , 9 );
        BOOST_CHECK_CLOSE( item.get< double >(1)  , 0.1 , 0.0001 );
        BOOST_CHECK_CLOSE( item.get< double >(7) ,  0.9 , 0.0001 );
    }

    {
        const auto& record = VFPPROD1.getRecord(4);
        const auto& item = record.getItem("GFR_VALUES");

        BOOST_CHECK_EQUAL( item.data_size() , 9 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   90 );
        BOOST_CHECK_EQUAL( item.get< double >(8) , 10000 );
    }

    {
        const auto& record = VFPPROD1.getRecord(5);
        const auto& item = record.getItem("ALQ_VALUES");

        BOOST_CHECK_EQUAL( item.data_size() , 1 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   0 );
    }

    {
        const auto& record = VFPPROD1.getRecord(6);

        {
            const auto& item = record.getItem("THP_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }

        {
            const auto& item = record.getItem("WFR_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("GFR_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("ALQ_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("VALUES");
            BOOST_CHECK_EQUAL( item.data_size() , 12 );
            BOOST_CHECK_EQUAL( item.get< double >(0) , 44.85 );
            BOOST_CHECK_EQUAL( item.get< double >(11) , 115.14 );
        }
    }

    {
        const auto& record = VFPPROD1.getRecord(572);
        {
            const auto& item = record.getItem("THP_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 7 );
        }
        {
            const auto& item = record.getItem("WFR_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 9 );
        }
        {
            const auto& item = record.getItem("GFR_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 9 );
        }
        {
            const auto& item = record.getItem("ALQ_INDEX");
            BOOST_CHECK( item.hasValue(0));
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("VALUES");
            BOOST_CHECK_EQUAL( item.data_size() , 12 );
            BOOST_CHECK_EQUAL( item.get< double >(0) , 100.80 );
            BOOST_CHECK_EQUAL( item.get< double >(11) , 147.79 );
        }
    }
}

BOOST_AUTO_TEST_CASE( WCHONHIST ) {
    Parser parser;
    std::string wconhistFile(pathprefix() + "WCONHIST/WCONHIST1");
    auto deck =  parser.parseFile(wconhistFile);
    const auto& kw1 = deck["WCONHIST"][0];
    BOOST_CHECK_EQUAL( 3U , kw1.size() );

    const auto& rec1 = kw1.getRecord(0);
    BOOST_CHECK_EQUAL( 12U , rec1.size() );

    const auto& rec3 = kw1.getRecord(2);
    BOOST_CHECK_EQUAL( 12U , rec3.size() );

    const auto& item1       = rec1.getItem("WELL");
    const auto& item1_index = rec1.getItem(0);

    BOOST_CHECK_EQUAL( &item1  , &item1_index );
    BOOST_CHECK_EQUAL( "OP_1" , item1.get< std::string >(0));

    const auto& kw2 = deck["WCONHIST"][1];
    BOOST_CHECK_EQUAL( "OP_3" , rec3.getItem("WELL").get< std::string >(0));
    BOOST_CHECK_EQUAL( 2U , deck.count("WCONHIST"));
    BOOST_CHECK_EQUAL( "OP_3_B" , kw2.getRecord( 2 ).getItem("WELL").get< std::string >(0));
    BOOST_CHECK( !deck.hasKeyword( "DIMENS" ) );
}

BOOST_AUTO_TEST_CASE( WDFACCOR ) {
    const std::string input = R"(
--          WELL    COEFF       POWER       POWER
--          NAME    A           B           C
WDFACCOR
            '*'     1.200E-3    -1.045      0.0 /
/
)";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( WEFAC ) {
    const std::string input = R"(
--      WELL        EFF         NETWK
--      NAME        FACT        OPTN

WEFAC
        '*     '    0.950   /
/
)";
    Parser().parseString( input );
}

BOOST_AUTO_TEST_CASE( WELL_PROBE ) {
    const std::string validDeckString = R"(
WBHP
    /
WOPR
    /;
)";
    Parser parser;

    // TODO: for some reason, the parser does not seem to throw here. Investigate
/*
    const char *invalidDeckString =
        "WELL_PROBE\n"
        "/\n";
    BOOST_CHECK_THROW(parser->parseString(invalidDeckString), std::invalid_argument);
*/

    auto deck = parser.parseString(validDeckString);
    BOOST_CHECK( !deck.hasKeyword("WELL_PROBE"));
    BOOST_CHECK(  deck.hasKeyword("WBHP"));
    BOOST_CHECK(  deck.hasKeyword("WOPR"));
    BOOST_CHECK( !deck.hasKeyword("WWPR"));
}

BOOST_AUTO_TEST_CASE( WCONPROD ) {
    Parser parser;
    std::string wconprodFile(pathprefix() + "WellWithWildcards/WCONPROD1");
    auto deck =  parser.parseFile(wconprodFile);
    EclipseGrid grid(30,30,30);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck, grid, fp, runspec, python);

    BOOST_CHECK_EQUAL(5U, sched.numWells());
    BOOST_CHECK(sched.hasWell("INJE1"));
    BOOST_CHECK(sched.hasWell("PROD2"));
    BOOST_CHECK(sched.hasWell("PROD3"));
    BOOST_CHECK(sched.hasWell("PROD4"));
    BOOST_CHECK(sched.hasWell("PROX5"));

    {
        const auto& well0 = sched.getWell("PROD2", 0 );
        const auto& well1 = sched.getWell("PROD2", 1 );
        BOOST_CHECK_CLOSE(1000, well0.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(1500, well1.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(1000/Metric::Time, well0.getProductionProperties().OilRate.getSI(), 0.001);
        BOOST_CHECK_CLOSE(1500/Metric::Time, well1.getProductionProperties().OilRate.getSI(), 0.001);
    }

    {
        const auto& well0 = sched.getWell("PROD3", 0 );
        const auto& well1 = sched.getWell("PROD3", 1 );
        BOOST_CHECK_CLOSE(0   , well0.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(1500, well1.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(0/Metric::Time   , well0.getProductionProperties().OilRate.getSI(), 0.001);
        BOOST_CHECK_CLOSE(1500/Metric::Time, well1.getProductionProperties().OilRate.getSI(), 0.001);
    }

    {
        const auto& well0 = sched.getWell("PROX5", 0);
        const auto& well1 = sched.getWell("PROX5", 1);
        BOOST_CHECK_CLOSE(2000, well0.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(2000, well1.getProductionProperties().OilRate.get<double>(), 0.001);
        BOOST_CHECK_CLOSE(2000/Metric::Time, well0.getProductionProperties().OilRate.getSI(), 0.001);
        BOOST_CHECK_CLOSE(2000/Metric::Time, well1.getProductionProperties().OilRate.getSI(), 0.001);
    }
}


BOOST_AUTO_TEST_CASE( WCONINJE ) {
    Parser parser;
    std::string wconprodFile(pathprefix() + "WellWithWildcards/WCONINJE1");
    auto deck = parser.parseFile(wconprodFile);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(30,30,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule sched( deck, grid, fp, runspec, python);
    SummaryState st(TimeService::now());

    BOOST_CHECK_EQUAL(5U, sched.numWells());
    BOOST_CHECK(sched.hasWell("PROD1"));
    BOOST_CHECK(sched.hasWell("INJE2"));
    BOOST_CHECK(sched.hasWell("INJE3"));
    BOOST_CHECK(sched.hasWell("PROD4"));
    BOOST_CHECK(sched.hasWell("INJX5"));

    {
        const auto& well0 = sched.getWell("INJE2", 0);
        const auto& well1 = sched.getWell("INJE2", 1);
        const auto controls0 = well0.injectionControls(st);
        const auto controls1 = well1.injectionControls(st);
        BOOST_CHECK_CLOSE(1000/Metric::Time, controls0.surface_rate, 0.001);
        BOOST_CHECK_CLOSE(1500/Metric::Time, controls1.surface_rate, 0.001);
    }

    {
        const auto& well1 = sched.getWell("INJE3", 1);
        const auto controls1 = well1.injectionControls(st);
        BOOST_CHECK_CLOSE(1500/Metric::Time, controls1.surface_rate, 0.001);
    }

    {
        const auto& well0 = sched.getWell("INJX5", 0);
        const auto& well1 = sched.getWell("INJX5", 1);
        const auto controls0 = well0.injectionControls(st);
        const auto controls1 = well1.injectionControls(st);
        BOOST_CHECK_CLOSE(2000/Metric::Time, controls0.surface_rate, 0.001);
        BOOST_CHECK_CLOSE(2000/Metric::Time, controls1.surface_rate, 0.001);
    }
}

BOOST_AUTO_TEST_CASE( WORKLIM ) {
    const std::string input = R"(
WORKLIM
        10.0
/
)";
    Parser().parseString( input );
}



BOOST_AUTO_TEST_CASE(PVTWSALT) {
    const std::string input = R"(
RUNSPEC

OIL
GAS

BRINE
NACL KCL /

TABDIMS
 1 2 /

PROPS

PVTWSALT
   1000  0 /
   1  2  3  4  5
   6  7  8  9 10 /
   2000  0.50 /
   1.5  2.5  3.5  4.5  5.5
   6.5  7.5  8.5  9.5 10.5/

--  S_g k_rg k_rog p_cog
SGOF
    0.1 0.0 1.0 0.0
    0.2 0.1 1.0 1.0
    0.3 0.2 0.9 2.0
    0.4 0.3 0.8 3.0
    0.5 0.5 0.5 4.0
    0.6 0.6 0.4 5.0
    0.7 0.8 0.3 6.0
    0.8 0.9 0.2 7.0
    0.9 0.5 0.1 8.0
    1.0 1.0 0.1 9.0 /
)";
    Parser parser;

    const auto& keyword = parser.getParserKeywordFromDeckName("BRINE");
    BOOST_CHECK_EQUAL(keyword.getFixedSize(), 1);
    BOOST_CHECK_EQUAL(keyword.min_size().value(), 0);


    auto deck = parser.parseString(input);
    const auto& pvtwsalt = deck["PVTWSALT"].back();
    BOOST_CHECK_EQUAL(pvtwsalt.size(), 4);

    const auto& sgof = deck["SGOF"].back();
    BOOST_CHECK_EQUAL(sgof.size(), 1);

    const auto& brine = deck["BRINE"].back();
    const auto& salts = brine.getRecord(0).getItem(0);
    BOOST_CHECK_EQUAL( salts.data_size(), 2);
}
