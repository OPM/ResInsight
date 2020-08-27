/*
Copyright 2016 Statoil ASA.

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

#define BOOST_TEST_MODULE RunspecTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(PhaseFromString) {
    BOOST_CHECK_THROW( get_phase("XXX") , std::invalid_argument );
    BOOST_CHECK_THROW( get_phase("WATE") , std::invalid_argument );
    BOOST_CHECK_THROW( get_phase("OI") , std::invalid_argument );
    BOOST_CHECK_THROW( get_phase("OILL") , std::invalid_argument );

    BOOST_CHECK_EQUAL( Phase::OIL  , get_phase("OIL") );
    BOOST_CHECK_EQUAL( Phase::WATER, get_phase("WATER") );
    BOOST_CHECK_EQUAL( Phase::WATER, get_phase("WAT") );
    BOOST_CHECK_EQUAL( Phase::GAS  , get_phase("GAS") );
    BOOST_CHECK_EQUAL( Phase::POLYMW  , get_phase("POLYMW") );
}

BOOST_AUTO_TEST_CASE(TwoPhase) {
    const std::string input = R"(
    RUNSPEC
    OIL
    WATER
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 2, phases.size() );
    BOOST_CHECK(  phases.active( Phase::OIL ) );
    BOOST_CHECK( !phases.active( Phase::GAS ) );
    BOOST_CHECK(  phases.active( Phase::WATER ) );
    BOOST_CHECK( !phases.active( Phase::POLYMW ) );

    const auto ECL_OIL_PHASE = 1 << 0;
    const auto ECL_WATER_PHASE = 1 << 2;

    BOOST_CHECK_EQUAL( ECL_OIL_PHASE + ECL_WATER_PHASE , runspec.eclPhaseMask( ));
}

BOOST_AUTO_TEST_CASE(ThreePhase) {
    const std::string input = R"(
    RUNSPEC
    OIL
    GAS
    WATER
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 3, phases.size() );
    BOOST_CHECK( phases.active( Phase::OIL ) );
    BOOST_CHECK( phases.active( Phase::GAS ) );
    BOOST_CHECK( phases.active( Phase::WATER ) );

}


BOOST_AUTO_TEST_CASE(TABDIMS) {
    const std::string input = R"(
    RUNSPEC
    TABDIMS
      1 * 3 * 5 * /
    OIL
    GAS
    WATER
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& tabdims = runspec.tabdims();
    BOOST_CHECK_EQUAL( tabdims.getNumSatTables( ) , 1 );
    BOOST_CHECK_EQUAL( tabdims.getNumPVTTables( ) , 1 );
    BOOST_CHECK_EQUAL( tabdims.getNumSatNodes( ) , 3 );
    BOOST_CHECK_EQUAL( tabdims.getNumPressureNodes( ) , 20 );
    BOOST_CHECK_EQUAL( tabdims.getNumFIPRegions( ) , 5 );
    BOOST_CHECK_EQUAL( tabdims.getNumRSNodes( ) , 20 );
}

BOOST_AUTO_TEST_CASE( EndpointScalingWithoutENDSCALE ) {
    const std::string input = R"(
    RUNSPEC
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( !endscale );
    BOOST_CHECK( !endscale.directional() );
    BOOST_CHECK( !endscale.nondirectional() );
    BOOST_CHECK( !endscale.reversible() );
    BOOST_CHECK( !endscale.irreversible() );
}



BOOST_AUTO_TEST_CASE( EndpointScalingDefaulted ) {
    const std::string input = R"(
    RUNSPEC
    ENDSCALE
        /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( endscale );
    BOOST_CHECK( !endscale.directional() );
    BOOST_CHECK( endscale.nondirectional() );
    BOOST_CHECK( endscale.reversible() );
    BOOST_CHECK( !endscale.irreversible() );
}

BOOST_AUTO_TEST_CASE( EndpointScalingDIRECT ) {
    const std::string input = R"(
    RUNSPEC
    ENDSCALE
        DIRECT /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( endscale );
    BOOST_CHECK( endscale.directional() );
    BOOST_CHECK( !endscale.nondirectional() );
    BOOST_CHECK( endscale.reversible() );
    BOOST_CHECK( !endscale.irreversible() );
}

BOOST_AUTO_TEST_CASE( EndpointScalingDIRECT_IRREVERS ) {
    const std::string input = R"(
    RUNSPEC
    ENDSCALE
        direct IRREVERS /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( endscale );
    BOOST_CHECK( endscale.directional() );
    BOOST_CHECK( !endscale.nondirectional() );
    BOOST_CHECK( !endscale.reversible() );
    BOOST_CHECK( endscale.irreversible() );
}

BOOST_AUTO_TEST_CASE( SCALECRS_without_ENDSCALE ) {
    const std::string input = R"(
    RUNSPEC
    SCALECRS
        /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( !endscale );
    BOOST_CHECK( !endscale.twopoint() );
    BOOST_CHECK( !endscale.threepoint() );
}

BOOST_AUTO_TEST_CASE( SCALECRS_N ) {
    const std::string N = R"(
    RUNSPEC
    ENDSCALE
        /
    SCALECRS
        N /
    )";

    const std::string defaulted = R"(
    RUNSPEC
    ENDSCALE
        /
    SCALECRS
        /
    )";

    for( const auto& input : { N, defaulted } ) {
        Runspec runspec( Parser{}.parseString( input ) );
        const auto& endscale = runspec.endpointScaling();

        BOOST_CHECK( endscale );
        BOOST_CHECK( endscale.twopoint() );
        BOOST_CHECK( !endscale.threepoint() );
    }
}

BOOST_AUTO_TEST_CASE( SCALECRS_Y ) {
    const std::string input = R"(
    RUNSPEC
    ENDSCALE
        /
    SCALECRS
        Y /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();

    BOOST_CHECK( endscale );
    BOOST_CHECK( !endscale.twopoint() );
    BOOST_CHECK( endscale.threepoint() );
}

BOOST_AUTO_TEST_CASE( endpoint_scaling_throw_invalid_argument ) {

    const std::string inputs[] = {
        R"(
            RUNSPEC
            ENDSCALE
            NODIR IRREVERSIBLE / -- irreversible requires direct
        )",
        R"(
            RUNSPEC
            ENDSCALE
                * IRREVERSIBLE / -- irreversible requires direct *specified*
        )",
        R"(
            RUNSPEC
            ENDSCALE -- ENDSCALE can't take arbitrary input (takes enumeration)
                broken /
        )",
        R"(
            RUNSPEC
            ENDSCALE
                /
            SCALECRS -- SCALECRS takes YES/NO
                broken /
        )",
    };

    for( auto&& input : inputs ) {
        auto deck = Parser{}.parseString( input );
        BOOST_CHECK_THROW( Runspec{ deck }, std::invalid_argument );
    }
}

BOOST_AUTO_TEST_CASE(WELLDIMS_FullSpec)
{
    const auto input = std::string {
        R"(
RUNSPEC

WELLDIMS
   130  36  15  84 /
)" };

    const auto wd = Welldims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wd.maxConnPerWell(), 36);   // WELLDIMS(2)
    BOOST_CHECK_EQUAL(wd.maxWellsPerGroup(), 84); // WELLDIMS(4)
    BOOST_CHECK_EQUAL(wd.maxGroupsInField(), 15); // WELLDIMS(3)
}

BOOST_AUTO_TEST_CASE(WELLDIMS_AllDefaulted)
{
    const auto input = std::string {
        R"(
RUNSPEC

WELLDIMS
/
)" };

    const auto wd = Welldims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wd.maxConnPerWell(), 0);   // WELLDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wd.maxWellsPerGroup(), 0); // WELLDIMS(4), defaulted
    BOOST_CHECK_EQUAL(wd.maxGroupsInField(), 0); // WELLDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WELLDIMS_MaxConn)
{
    const auto input = std::string {
        R"(
RUNSPEC

WELLDIMS
1* 36 /
)" };

    const auto wd = Welldims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wd.maxConnPerWell(), 36);  // WELLDIMS(2)
    BOOST_CHECK_EQUAL(wd.maxWellsPerGroup(), 0); // WELLDIMS(4), defaulted
    BOOST_CHECK_EQUAL(wd.maxGroupsInField(), 0); // WELLDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WELLDIMS_MaxGroups)
{
    const auto input = std::string {
        R"(
RUNSPEC

WELLDIMS
2* 4 /
)" };

    const auto wd = Welldims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wd.maxConnPerWell(), 0);   // WELLDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wd.maxWellsPerGroup(), 0); // WELLDIMS(4), defaulted
    BOOST_CHECK_EQUAL(wd.maxGroupsInField(), 4); // WELLDIMS(3)
}

BOOST_AUTO_TEST_CASE(WELLDIMS_MaxWellsInGrp)
{
    const auto input = std::string {
        R"(
RUNSPEC

WELLDIMS
3* 33 /
)" };

    const auto wd = Welldims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wd.maxConnPerWell(), 0);    // WELLDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wd.maxWellsPerGroup(), 33); // WELLDIMS(4)
    BOOST_CHECK_EQUAL(wd.maxGroupsInField(), 0);  // WELLDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WSEGDIMS_NotSpecified)
{
    const auto input = std::string {
        R"(
RUNSPEC
)" };

    const auto wsd = WellSegmentDims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wsd.maxSegmentedWells(), 0);         // WSEGDIMS1), defaulted
    BOOST_CHECK_EQUAL(wsd.maxSegmentsPerWell(), 1);        // WSEGDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wsd.maxLateralBranchesPerWell(), 1); // WSEGDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WSEGDIMS_AllDefaulted)
{
    const auto input = std::string {
        R"(
RUNSPEC

WSEGDIMS
/
)" };

    const auto wsd = WellSegmentDims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wsd.maxSegmentedWells(), 0);         // WSEGDIMS1), defaulted
    BOOST_CHECK_EQUAL(wsd.maxSegmentsPerWell(), 1);        // WSEGDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wsd.maxLateralBranchesPerWell(), 1); // WSEGDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WSEGDIMS_MaxSegmentedWells)
{
    const auto input = std::string {
        R"(
RUNSPEC

WSEGDIMS
  11
/
)" };

    const auto wsd = WellSegmentDims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wsd.maxSegmentedWells(), 11);        // WSEGDIMS(1)
    BOOST_CHECK_EQUAL(wsd.maxSegmentsPerWell(), 1);        // WSEGDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wsd.maxLateralBranchesPerWell(), 1); // WSEGDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WSEGDIMS_MaxSegmentsPerWell)
{
    const auto input = std::string {
        R"(
RUNSPEC

WSEGDIMS
  1* 22
/
)" };

    const auto wsd = WellSegmentDims {
        Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wsd.maxSegmentedWells(), 0);         // WSEGDIMS(1), defaulted
    BOOST_CHECK_EQUAL(wsd.maxSegmentsPerWell(), 22);       // WSEGDIMS(2)
    BOOST_CHECK_EQUAL(wsd.maxLateralBranchesPerWell(), 1); // WSEGDIMS(3), defaulted
}

BOOST_AUTO_TEST_CASE(WSEGDIMS_MaxLatBrPerWell)
{
    const auto input = std::string {
        R"(
RUNSPEC

WSEGDIMS
  2* 33
/
)" };

    const auto wsd = WellSegmentDims {
      Parser{}.parseString(input)
    };

    BOOST_CHECK_EQUAL(wsd.maxSegmentedWells(), 0);          // WSEGDIMS(1), defaulted
    BOOST_CHECK_EQUAL(wsd.maxSegmentsPerWell(), 1);         // WSEGDIMS(2), defaulted
    BOOST_CHECK_EQUAL(wsd.maxLateralBranchesPerWell(), 33); // WSEGDIMS(3)
}

BOOST_AUTO_TEST_CASE( SWATINIT ) {
    const std::string input = R"(
    SWATINIT
       1000*0.25 /
    )";

    Runspec runspec( Parser{}.parseString( input ) );
    const auto& endscale = runspec.endpointScaling();
    BOOST_CHECK( endscale );
    BOOST_CHECK( !endscale.directional() );
    BOOST_CHECK( endscale.nondirectional() );
    BOOST_CHECK( endscale.reversible() );
    BOOST_CHECK( !endscale.irreversible() );

}

BOOST_AUTO_TEST_CASE(TolCrit)
{
    {
        const auto sfctrl = ::Opm::SatFuncControls{};
        BOOST_CHECK_CLOSE(sfctrl.minimumRelpermMobilityThreshold(), 1.0e-6, 1.0e-10);
        BOOST_CHECK_MESSAGE(sfctrl == ::Opm::SatFuncControls{},
                            "Default-constructed SatFuncControl must equal itself");
    }

    {
        const auto sfctrl = ::Opm::SatFuncControls{ 5.0e-7, ::Opm::SatFuncControls::ThreePhaseOilKrModel::Default };
        BOOST_CHECK_CLOSE(sfctrl.minimumRelpermMobilityThreshold(), 5.0e-7, 1.0e-10);
        BOOST_CHECK_MESSAGE(!(sfctrl == ::Opm::SatFuncControls{}),
                            "Default-constructed SatFuncControl must NOT equal non-default");

        const auto deck = ::Opm::Parser{}.parseString(R"(
TOLCRIT
  5.0E-7 /
)");
        BOOST_CHECK_CLOSE(sfctrl.minimumRelpermMobilityThreshold(),
                          ::Opm::SatFuncControls{deck}.minimumRelpermMobilityThreshold(),
                          1.0e-10);
    }

    {
        const auto deck = ::Opm::Parser{}.parseString(R"(
RUNSPEC
END
)");

        const auto rspec = ::Opm::Runspec{deck};
        const auto sfctrl = rspec.saturationFunctionControls();

        BOOST_CHECK_CLOSE(sfctrl.minimumRelpermMobilityThreshold(), 1.0e-6, 1.0e-10);
    }

    {
        const auto deck = ::Opm::Parser{}.parseString(R"(
TOLCRIT
  5.0E-7 /
)");

        const auto rspec = ::Opm::Runspec{deck};
        const auto sfctrl = rspec.saturationFunctionControls();

        BOOST_CHECK_CLOSE(sfctrl.minimumRelpermMobilityThreshold(), 5.0e-7, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(Solvent) {
    const std::string input = R"(
    RUNSPEC
    OIL
    GAS
    WATER
    SOLVENT
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 4, phases.size() );
    BOOST_CHECK( phases.active( Phase::OIL ) );
    BOOST_CHECK( phases.active( Phase::GAS ) );
    BOOST_CHECK( phases.active( Phase::WATER ) );
    BOOST_CHECK( phases.active( Phase::SOLVENT ) );
    BOOST_CHECK( !phases.active( Phase::POLYMW) );


}

BOOST_AUTO_TEST_CASE(Polymer) {
    const std::string input = R"(
    RUNSPEC
    OIL
    GAS
    WATER
    POLYMER
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 4, phases.size() );
    BOOST_CHECK( phases.active( Phase::OIL ) );
    BOOST_CHECK( phases.active( Phase::GAS ) );
    BOOST_CHECK( phases.active( Phase::WATER ) );
    BOOST_CHECK( phases.active( Phase::POLYMER ) );
    BOOST_CHECK( !phases.active( Phase::POLYMW) );


}

BOOST_AUTO_TEST_CASE(PolymerMolecularWeight) {
    const std::string input = R"(
    RUNSPEC
    OIL
    WATER
    POLYMER
    POLYMW
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 4, phases.size() );
    BOOST_CHECK( phases.active( Phase::OIL ) );
    BOOST_CHECK( !phases.active( Phase::GAS ) );
    BOOST_CHECK( phases.active( Phase::WATER ) );
    BOOST_CHECK( phases.active( Phase::POLYMER ) );
    BOOST_CHECK( phases.active( Phase::POLYMW) );
}


BOOST_AUTO_TEST_CASE(Foam) {
    const std::string input = R"(
    RUNSPEC
    OIL
    GAS
    WATER
    FOAM
    )";

    Parser parser;

    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 4, phases.size() );
    BOOST_CHECK( phases.active( Phase::OIL ) );
    BOOST_CHECK( phases.active( Phase::GAS ) );
    BOOST_CHECK( phases.active( Phase::WATER ) );
    BOOST_CHECK( phases.active( Phase::FOAM) );

    // not in deck - default constructor.
    const auto& actdims = runspec.actdims();
    BOOST_CHECK_EQUAL(actdims.max_keywords(), 2);
}

BOOST_AUTO_TEST_CASE(ACTDIMS) {
    const std::string input = R"(
    RUNSPEC
    ACTDIMS
       2* 13 14 /
    )";

    Parser parser;
    auto deck = parser.parseString(input);

    Runspec runspec( deck );
    const auto& actdims = runspec.actdims();
    BOOST_CHECK_EQUAL(actdims.max_keywords(), 2);
    BOOST_CHECK_EQUAL(actdims.max_conditions(), 14);
}
