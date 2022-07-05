/*
  Copyright 2021 NORCE.

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

#define BOOST_TEST_MODULE MICPTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/MICPpara.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

BOOST_AUTO_TEST_CASE(TestMICP) {

    const char *data =
      "RUNSPEC\n"
      "WATER\n"
      "MICP\n";

    Opm::Parser parser;

    auto deck = parser.parseString(data);

    Opm::Runspec runspec( deck );
    const auto& phases = runspec.phases();
    BOOST_CHECK_EQUAL( 1U, phases.size() );
    BOOST_CHECK( phases.active( Opm::Phase::WATER ) );
    BOOST_CHECK( runspec.micp() );
}

BOOST_AUTO_TEST_CASE( TestMICPPARA ) {
    const char *data =
    "DIMENS\n"
    "10 10 10 /\n"
    "TABDIMS\n"
    "3 /\n"
    "GRID\n"
    "DX\n"
    "1000*0.25 /\n"
    "DY\n"
    "1000*0.25 /\n"
    "DZ\n"
    "1000*0.25 /\n"
    "TOPS\n"
    "100*0.25 /\n"
    "PROPS\n"
    "MICPPARA\n"
    " 1. 2. 3. 4. 5. 6. 7. 8. 9. 10. 11. 12. 13. 14. 15. 16. 17. /\n";

    Opm::Parser parser;

    Opm::UnitSystem unitSystem = Opm::UnitSystem( Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC );

    auto deck = parser.parseString(data);

    double siFactor1 = unitSystem.parse("Length/Viscosity").getSIScaling();
    double siFactor2 = unitSystem.parse("1/Time").getSIScaling();
    double siFactor3 = unitSystem.parse("Permeability").getSIScaling();

    Opm::EclipseState eclipsestate( deck );
    const auto& MICPpara = eclipsestate.getMICPpara();
    BOOST_CHECK_EQUAL( MICPpara.getDensityBiofilm()             , 1.  );
    BOOST_CHECK_EQUAL( MICPpara.getDensityCalcite()             , 2.  );
    BOOST_CHECK_EQUAL( MICPpara.getDetachmentRate()             , 3. * siFactor1  );
    BOOST_CHECK_EQUAL( MICPpara.getCriticalPorosity()           , 4.  );
    BOOST_CHECK_EQUAL( MICPpara.getFittingFactor()              , 5.  );
    BOOST_CHECK_EQUAL( MICPpara.getHalfVelocityOxygen()         , 6.  );
    BOOST_CHECK_EQUAL( MICPpara.getHalfVelocityUrea()           , 7.  );
    BOOST_CHECK_EQUAL( MICPpara.getMaximumGrowthRate()          , 8. * siFactor2  );
    BOOST_CHECK_EQUAL( MICPpara.getMaximumOxygenConcentration() , 9.  );
    BOOST_CHECK_EQUAL( MICPpara.getMaximumUreaConcentration()   , 10. );
    BOOST_CHECK_EQUAL( MICPpara.getMaximumUreaUtilization()     , 11. * siFactor2 );
    BOOST_CHECK_EQUAL( MICPpara.getMicrobialAttachmentRate()    , 12. * siFactor2 );
    BOOST_CHECK_EQUAL( MICPpara.getMicrobialDeathRate()         , 13. * siFactor2 );
    BOOST_CHECK_EQUAL( MICPpara.getMinimumPermeability()        , 14. * siFactor3 );
    BOOST_CHECK_EQUAL( MICPpara.getOxygenConsumptionFactor()    , 15. );
    BOOST_CHECK_EQUAL( MICPpara.getToleranceBeforeClogging()    , 16. );
    BOOST_CHECK_EQUAL( MICPpara.getYieldGrowthCoefficient()     , 17. );
}
