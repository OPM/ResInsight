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


#define BOOST_TEST_MODULE ParserEnumTests
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(TestCompletionStateEnum2String) {
    BOOST_CHECK_EQUAL( "AUTO" , WellCompletion::StateEnum2String(WellCompletion::AUTO));
    BOOST_CHECK_EQUAL( "OPEN" , WellCompletion::StateEnum2String(WellCompletion::OPEN));
    BOOST_CHECK_EQUAL( "SHUT" , WellCompletion::StateEnum2String(WellCompletion::SHUT));
}


BOOST_AUTO_TEST_CASE(TestCompletionStateEnumFromString) {
    BOOST_CHECK_THROW( WellCompletion::StateEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( WellCompletion::AUTO , WellCompletion::StateEnumFromString("AUTO"));
    BOOST_CHECK_EQUAL( WellCompletion::SHUT , WellCompletion::StateEnumFromString("SHUT"));
    BOOST_CHECK_EQUAL( WellCompletion::SHUT , WellCompletion::StateEnumFromString("STOP"));
    BOOST_CHECK_EQUAL( WellCompletion::OPEN , WellCompletion::StateEnumFromString("OPEN"));
}



BOOST_AUTO_TEST_CASE(TestCompletionStateEnumLoop) {
    BOOST_CHECK_EQUAL( WellCompletion::AUTO , WellCompletion::StateEnumFromString( WellCompletion::StateEnum2String( WellCompletion::AUTO ) ));
    BOOST_CHECK_EQUAL( WellCompletion::SHUT , WellCompletion::StateEnumFromString( WellCompletion::StateEnum2String( WellCompletion::SHUT ) ));
    BOOST_CHECK_EQUAL( WellCompletion::OPEN , WellCompletion::StateEnumFromString( WellCompletion::StateEnum2String( WellCompletion::OPEN ) ));

    BOOST_CHECK_EQUAL( "AUTO" , WellCompletion::StateEnum2String(WellCompletion::StateEnumFromString(  "AUTO" ) ));
    BOOST_CHECK_EQUAL( "OPEN" , WellCompletion::StateEnum2String(WellCompletion::StateEnumFromString(  "OPEN" ) ));
    BOOST_CHECK_EQUAL( "SHUT" , WellCompletion::StateEnum2String(WellCompletion::StateEnumFromString(  "SHUT" ) ));
}


/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestCompletionDirectionEnum2String)
{
    using namespace WellCompletion;

    BOOST_CHECK_EQUAL("X", DirectionEnum2String(DirectionEnum::X));
    BOOST_CHECK_EQUAL("Y", DirectionEnum2String(DirectionEnum::Y));
    BOOST_CHECK_EQUAL("Z", DirectionEnum2String(DirectionEnum::Z));
}

BOOST_AUTO_TEST_CASE(TestCompletionDirectionEnumFromString)
{
    using namespace WellCompletion;

    BOOST_CHECK_THROW(DirectionEnumFromString("XXX"), std::invalid_argument);

    BOOST_CHECK_EQUAL(DirectionEnum::X, DirectionEnumFromString("X"));
    BOOST_CHECK_EQUAL(DirectionEnum::Y, DirectionEnumFromString("Y"));
    BOOST_CHECK_EQUAL(DirectionEnum::Z, DirectionEnumFromString("Z"));
}

BOOST_AUTO_TEST_CASE(TestCompletionDirectionEnumLoop)
{
    using namespace WellCompletion;

    BOOST_CHECK_EQUAL(DirectionEnum::X, DirectionEnumFromString(DirectionEnum2String(DirectionEnum::X)));
    BOOST_CHECK_EQUAL(DirectionEnum::Y, DirectionEnumFromString(DirectionEnum2String(DirectionEnum::Y)));
    BOOST_CHECK_EQUAL(DirectionEnum::Z, DirectionEnumFromString(DirectionEnum2String(DirectionEnum::Z)));

    BOOST_CHECK_EQUAL("X", DirectionEnum2String(DirectionEnumFromString("X")));
    BOOST_CHECK_EQUAL("Y", DirectionEnum2String(DirectionEnumFromString("Y")));
    BOOST_CHECK_EQUAL("Z", DirectionEnum2String(DirectionEnumFromString("Z")));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestGroupInjectionControlEnum2String) {
    BOOST_CHECK_EQUAL( "NONE" , GroupInjection::ControlEnum2String(GroupInjection::NONE));
    BOOST_CHECK_EQUAL( "RATE" , GroupInjection::ControlEnum2String(GroupInjection::RATE));
    BOOST_CHECK_EQUAL( "RESV" , GroupInjection::ControlEnum2String(GroupInjection::RESV));
    BOOST_CHECK_EQUAL( "REIN" , GroupInjection::ControlEnum2String(GroupInjection::REIN));
    BOOST_CHECK_EQUAL( "VREP" , GroupInjection::ControlEnum2String(GroupInjection::VREP));
    BOOST_CHECK_EQUAL( "FLD"  , GroupInjection::ControlEnum2String(GroupInjection::FLD));
}


BOOST_AUTO_TEST_CASE(TestGroupInjectionControlEnumFromString) {
    BOOST_CHECK_THROW( GroupInjection::ControlEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( GroupInjection::NONE , GroupInjection::ControlEnumFromString("NONE"));
    BOOST_CHECK_EQUAL( GroupInjection::RATE , GroupInjection::ControlEnumFromString("RATE"));
    BOOST_CHECK_EQUAL( GroupInjection::RESV , GroupInjection::ControlEnumFromString("RESV"));
    BOOST_CHECK_EQUAL( GroupInjection::REIN , GroupInjection::ControlEnumFromString("REIN"));
    BOOST_CHECK_EQUAL( GroupInjection::VREP , GroupInjection::ControlEnumFromString("VREP"));
    BOOST_CHECK_EQUAL( GroupInjection::FLD  , GroupInjection::ControlEnumFromString("FLD"));
}



BOOST_AUTO_TEST_CASE(TestGroupInjectionControlEnumLoop) {
    BOOST_CHECK_EQUAL( GroupInjection::NONE , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::NONE ) ));
    BOOST_CHECK_EQUAL( GroupInjection::RATE , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::RATE ) ));
    BOOST_CHECK_EQUAL( GroupInjection::RESV , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::RESV ) ));
    BOOST_CHECK_EQUAL( GroupInjection::REIN , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::REIN ) ));
    BOOST_CHECK_EQUAL( GroupInjection::VREP , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::VREP ) ));
    BOOST_CHECK_EQUAL( GroupInjection::FLD  , GroupInjection::ControlEnumFromString( GroupInjection::ControlEnum2String( GroupInjection::FLD ) ));

    BOOST_CHECK_EQUAL( "NONE" , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "NONE" ) ));
    BOOST_CHECK_EQUAL( "RATE" , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "RATE" ) ));
    BOOST_CHECK_EQUAL( "RESV" , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "RESV" ) ));
    BOOST_CHECK_EQUAL( "REIN" , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "REIN" ) ));
    BOOST_CHECK_EQUAL( "VREP" , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "VREP" ) ));
    BOOST_CHECK_EQUAL( "FLD"  , GroupInjection::ControlEnum2String(GroupInjection::ControlEnumFromString( "FLD"  ) ));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestGroupProductionControlEnum2String) {
    BOOST_CHECK_EQUAL( "NONE" , GroupProduction::ControlEnum2String(GroupProduction::NONE));
    BOOST_CHECK_EQUAL( "ORAT" , GroupProduction::ControlEnum2String(GroupProduction::ORAT));
    BOOST_CHECK_EQUAL( "WRAT" , GroupProduction::ControlEnum2String(GroupProduction::WRAT));
    BOOST_CHECK_EQUAL( "GRAT" , GroupProduction::ControlEnum2String(GroupProduction::GRAT));
    BOOST_CHECK_EQUAL( "LRAT" , GroupProduction::ControlEnum2String(GroupProduction::LRAT));
    BOOST_CHECK_EQUAL( "CRAT" , GroupProduction::ControlEnum2String(GroupProduction::CRAT));
    BOOST_CHECK_EQUAL( "RESV" , GroupProduction::ControlEnum2String(GroupProduction::RESV));
    BOOST_CHECK_EQUAL( "PRBL" , GroupProduction::ControlEnum2String(GroupProduction::PRBL));
}


BOOST_AUTO_TEST_CASE(TestGroupProductionControlEnumFromString) {
    BOOST_CHECK_THROW( GroupProduction::ControlEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL(GroupProduction::NONE  , GroupProduction::ControlEnumFromString("NONE"));
    BOOST_CHECK_EQUAL(GroupProduction::ORAT  , GroupProduction::ControlEnumFromString("ORAT"));
    BOOST_CHECK_EQUAL(GroupProduction::WRAT  , GroupProduction::ControlEnumFromString("WRAT"));
    BOOST_CHECK_EQUAL(GroupProduction::GRAT  , GroupProduction::ControlEnumFromString("GRAT"));
    BOOST_CHECK_EQUAL(GroupProduction::LRAT  , GroupProduction::ControlEnumFromString("LRAT"));
    BOOST_CHECK_EQUAL(GroupProduction::CRAT  , GroupProduction::ControlEnumFromString("CRAT"));
    BOOST_CHECK_EQUAL(GroupProduction::RESV  , GroupProduction::ControlEnumFromString("RESV"));
    BOOST_CHECK_EQUAL(GroupProduction::PRBL  , GroupProduction::ControlEnumFromString("PRBL"));
}



BOOST_AUTO_TEST_CASE(TestGroupProductionControlEnumLoop) {
    BOOST_CHECK_EQUAL( GroupProduction::NONE, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::NONE ) ));
    BOOST_CHECK_EQUAL( GroupProduction::ORAT, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::ORAT ) ));
    BOOST_CHECK_EQUAL( GroupProduction::WRAT, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::WRAT ) ));
    BOOST_CHECK_EQUAL( GroupProduction::GRAT, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::GRAT ) ));
    BOOST_CHECK_EQUAL( GroupProduction::LRAT, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::LRAT ) ));
    BOOST_CHECK_EQUAL( GroupProduction::CRAT, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::CRAT ) ));
    BOOST_CHECK_EQUAL( GroupProduction::RESV, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::RESV ) ));
    BOOST_CHECK_EQUAL( GroupProduction::PRBL, GroupProduction::ControlEnumFromString( GroupProduction::ControlEnum2String( GroupProduction::PRBL ) ));

    BOOST_CHECK_EQUAL( "NONE" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "NONE" ) ));
    BOOST_CHECK_EQUAL( "ORAT" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "ORAT" ) ));
    BOOST_CHECK_EQUAL( "WRAT" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "WRAT" ) ));
    BOOST_CHECK_EQUAL( "GRAT" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "GRAT" ) ));
    BOOST_CHECK_EQUAL( "LRAT" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "LRAT" ) ));
    BOOST_CHECK_EQUAL( "CRAT" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "CRAT" ) ));
    BOOST_CHECK_EQUAL( "RESV" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "RESV" ) ));
    BOOST_CHECK_EQUAL( "PRBL" , GroupProduction::ControlEnum2String(GroupProduction::ControlEnumFromString( "PRBL" ) ));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestGroupProductionExceedLimitControlEnum2String) {
    BOOST_CHECK_EQUAL( "NONE"     , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::NONE));
    BOOST_CHECK_EQUAL( "CON"      , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::CON));
    BOOST_CHECK_EQUAL( "+CON"     , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::CON_PLUS));
    BOOST_CHECK_EQUAL( "WELL"     , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::WELL));
    BOOST_CHECK_EQUAL( "PLUG"     , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::PLUG));
    BOOST_CHECK_EQUAL( "RATE"     , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::RATE));
}


BOOST_AUTO_TEST_CASE(TestGroupProductionExceedLimitActionEnumFromString) {
    BOOST_CHECK_THROW( GroupProductionExceedLimit::ActionEnumFromString("XXX") , std::invalid_argument );

    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::NONE     , GroupProductionExceedLimit::ActionEnumFromString("NONE"));
    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::CON      , GroupProductionExceedLimit::ActionEnumFromString("CON" ));
    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::CON_PLUS , GroupProductionExceedLimit::ActionEnumFromString("+CON"));
    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::WELL     , GroupProductionExceedLimit::ActionEnumFromString("WELL"));
    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::PLUG     , GroupProductionExceedLimit::ActionEnumFromString("PLUG"));
    BOOST_CHECK_EQUAL(GroupProductionExceedLimit::RATE     , GroupProductionExceedLimit::ActionEnumFromString("RATE"));
}



BOOST_AUTO_TEST_CASE(TestGroupProductionExceedLimitActionEnumLoop) {
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::NONE     , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::NONE     ) ));
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::CON      , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::CON      ) ));
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::CON_PLUS , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::CON_PLUS ) ));
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::WELL     , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::WELL     ) ));
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::PLUG     , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::PLUG     ) ));
    BOOST_CHECK_EQUAL( GroupProductionExceedLimit::RATE     , GroupProductionExceedLimit::ActionEnumFromString( GroupProductionExceedLimit::ActionEnum2String( GroupProductionExceedLimit::RATE     ) ));

    BOOST_CHECK_EQUAL("NONE" , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "NONE" ) ));
    BOOST_CHECK_EQUAL("CON"  , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "CON"  ) ));
    BOOST_CHECK_EQUAL("+CON" , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "+CON" ) ));
    BOOST_CHECK_EQUAL("WELL" , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "WELL" ) ));
    BOOST_CHECK_EQUAL("PLUG" , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "PLUG" ) ));
    BOOST_CHECK_EQUAL("RATE" , GroupProductionExceedLimit::ActionEnum2String(GroupProductionExceedLimit::ActionEnumFromString( "RATE" ) ));
}


/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestPhaseEnum2String) {
    BOOST_CHECK_EQUAL( "OIL"  ,  Phase::PhaseEnum2String(Phase::OIL));
    BOOST_CHECK_EQUAL( "GAS"  ,  Phase::PhaseEnum2String(Phase::GAS));
    BOOST_CHECK_EQUAL( "WATER" , Phase::PhaseEnum2String(Phase::WATER));
}


BOOST_AUTO_TEST_CASE(TestPhaseEnumFromString) {
    BOOST_CHECK_THROW( Phase::PhaseEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( Phase::OIL   , Phase::PhaseEnumFromString("OIL"));
    BOOST_CHECK_EQUAL( Phase::WATER , Phase::PhaseEnumFromString("WATER"));
    BOOST_CHECK_EQUAL( Phase::WATER , Phase::PhaseEnumFromString("WAT"));
    BOOST_CHECK_EQUAL( Phase::GAS   , Phase::PhaseEnumFromString("GAS"));
}



BOOST_AUTO_TEST_CASE(TestPhaseEnumLoop) {
    BOOST_CHECK_EQUAL( Phase::OIL   , Phase::PhaseEnumFromString( Phase::PhaseEnum2String( Phase::OIL ) ));
    BOOST_CHECK_EQUAL( Phase::WATER , Phase::PhaseEnumFromString( Phase::PhaseEnum2String( Phase::WATER ) ));
    BOOST_CHECK_EQUAL( Phase::GAS   , Phase::PhaseEnumFromString( Phase::PhaseEnum2String( Phase::GAS ) ));

    BOOST_CHECK_EQUAL( "OIL"    , Phase::PhaseEnum2String(Phase::PhaseEnumFromString(  "OIL" ) ));
    BOOST_CHECK_EQUAL( "GAS"    , Phase::PhaseEnum2String(Phase::PhaseEnumFromString(  "GAS" ) ));
    BOOST_CHECK_EQUAL( "WATER"  , Phase::PhaseEnum2String(Phase::PhaseEnumFromString(  "WATER" ) ));
}



BOOST_AUTO_TEST_CASE(TestPhaseEnumMask) {
    BOOST_CHECK_EQUAL( 0 , Phase::OIL   & Phase::GAS );
    BOOST_CHECK_EQUAL( 0 , Phase::OIL   & Phase::WATER );
    BOOST_CHECK_EQUAL( 0 , Phase::WATER & Phase::GAS );
}



/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestInjectorEnum2String) {
    BOOST_CHECK_EQUAL( "OIL"  ,  WellInjector::Type2String(WellInjector::OIL));
    BOOST_CHECK_EQUAL( "GAS"  ,  WellInjector::Type2String(WellInjector::GAS));
    BOOST_CHECK_EQUAL( "WATER" , WellInjector::Type2String(WellInjector::WATER));
    BOOST_CHECK_EQUAL( "MULTI" , WellInjector::Type2String(WellInjector::MULTI));
}


BOOST_AUTO_TEST_CASE(TestInjectorEnumFromString) {
    BOOST_CHECK_THROW( WellInjector::TypeFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( WellInjector::OIL   , WellInjector::TypeFromString("OIL"));
    BOOST_CHECK_EQUAL( WellInjector::WATER , WellInjector::TypeFromString("WATER"));
    BOOST_CHECK_EQUAL( WellInjector::WATER , WellInjector::TypeFromString("WAT"));
    BOOST_CHECK_EQUAL( WellInjector::GAS   , WellInjector::TypeFromString("GAS"));
    BOOST_CHECK_EQUAL( WellInjector::MULTI , WellInjector::TypeFromString("MULTI"));
}



BOOST_AUTO_TEST_CASE(TestInjectorEnumLoop) {
    BOOST_CHECK_EQUAL( WellInjector::OIL     , WellInjector::TypeFromString( WellInjector::Type2String( WellInjector::OIL ) ));
    BOOST_CHECK_EQUAL( WellInjector::WATER   , WellInjector::TypeFromString( WellInjector::Type2String( WellInjector::WATER ) ));
    BOOST_CHECK_EQUAL( WellInjector::GAS     , WellInjector::TypeFromString( WellInjector::Type2String( WellInjector::GAS ) ));
    BOOST_CHECK_EQUAL( WellInjector::MULTI   , WellInjector::TypeFromString( WellInjector::Type2String( WellInjector::MULTI ) ));

    BOOST_CHECK_EQUAL( "MULTI"    , WellInjector::Type2String(WellInjector::TypeFromString(  "MULTI" ) ));
    BOOST_CHECK_EQUAL( "OIL"      , WellInjector::Type2String(WellInjector::TypeFromString(  "OIL" ) ));
    BOOST_CHECK_EQUAL( "GAS"      , WellInjector::Type2String(WellInjector::TypeFromString(  "GAS" ) ));
    BOOST_CHECK_EQUAL( "WATER"    , WellInjector::Type2String(WellInjector::TypeFromString(  "WATER" ) ));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(InjectorCOntrolMopdeEnum2String) {
    BOOST_CHECK_EQUAL( "RATE"  ,  WellInjector::ControlMode2String(WellInjector::RATE));
    BOOST_CHECK_EQUAL( "RESV"  ,  WellInjector::ControlMode2String(WellInjector::RESV));
    BOOST_CHECK_EQUAL( "BHP" , WellInjector::ControlMode2String(WellInjector::BHP));
    BOOST_CHECK_EQUAL( "THP" , WellInjector::ControlMode2String(WellInjector::THP));
    BOOST_CHECK_EQUAL( "GRUP" , WellInjector::ControlMode2String(WellInjector::GRUP));
}


BOOST_AUTO_TEST_CASE(InjectorControlModeEnumFromString) {
    BOOST_CHECK_THROW( WellInjector::ControlModeFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( WellInjector::RATE   , WellInjector::ControlModeFromString("RATE"));
    BOOST_CHECK_EQUAL( WellInjector::BHP , WellInjector::ControlModeFromString("BHP"));
    BOOST_CHECK_EQUAL( WellInjector::RESV   , WellInjector::ControlModeFromString("RESV"));
    BOOST_CHECK_EQUAL( WellInjector::THP , WellInjector::ControlModeFromString("THP"));
    BOOST_CHECK_EQUAL( WellInjector::GRUP , WellInjector::ControlModeFromString("GRUP"));
}



BOOST_AUTO_TEST_CASE(InjectorControlModeEnumLoop) {
    BOOST_CHECK_EQUAL( WellInjector::RATE     , WellInjector::ControlModeFromString( WellInjector::ControlMode2String( WellInjector::RATE ) ));
    BOOST_CHECK_EQUAL( WellInjector::BHP   , WellInjector::ControlModeFromString( WellInjector::ControlMode2String( WellInjector::BHP ) ));
    BOOST_CHECK_EQUAL( WellInjector::RESV     , WellInjector::ControlModeFromString( WellInjector::ControlMode2String( WellInjector::RESV ) ));
    BOOST_CHECK_EQUAL( WellInjector::THP   , WellInjector::ControlModeFromString( WellInjector::ControlMode2String( WellInjector::THP ) ));
    BOOST_CHECK_EQUAL( WellInjector::GRUP   , WellInjector::ControlModeFromString( WellInjector::ControlMode2String( WellInjector::GRUP ) ));

    BOOST_CHECK_EQUAL( "THP"    , WellInjector::ControlMode2String(WellInjector::ControlModeFromString(  "THP" ) ));
    BOOST_CHECK_EQUAL( "RATE"      , WellInjector::ControlMode2String(WellInjector::ControlModeFromString(  "RATE" ) ));
    BOOST_CHECK_EQUAL( "RESV"      , WellInjector::ControlMode2String(WellInjector::ControlModeFromString(  "RESV" ) ));
    BOOST_CHECK_EQUAL( "BHP"    , WellInjector::ControlMode2String(WellInjector::ControlModeFromString(  "BHP" ) ));
    BOOST_CHECK_EQUAL( "GRUP"    , WellInjector::ControlMode2String(WellInjector::ControlModeFromString(  "GRUP" ) ));
}



/*****************************************************************/

BOOST_AUTO_TEST_CASE(InjectorStatusEnum2String) {
    BOOST_CHECK_EQUAL( "OPEN"  ,  WellCommon::Status2String(WellCommon::OPEN));
    BOOST_CHECK_EQUAL( "SHUT"  ,  WellCommon::Status2String(WellCommon::SHUT));
    BOOST_CHECK_EQUAL( "AUTO"   ,  WellCommon::Status2String(WellCommon::AUTO));
    BOOST_CHECK_EQUAL( "STOP"   ,  WellCommon::Status2String(WellCommon::STOP));
}


BOOST_AUTO_TEST_CASE(InjectorStatusEnumFromString) {
    BOOST_CHECK_THROW( WellCommon::StatusFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( WellCommon::OPEN   , WellCommon::StatusFromString("OPEN"));
    BOOST_CHECK_EQUAL( WellCommon::AUTO , WellCommon::StatusFromString("AUTO"));
    BOOST_CHECK_EQUAL( WellCommon::SHUT   , WellCommon::StatusFromString("SHUT"));
    BOOST_CHECK_EQUAL( WellCommon::STOP , WellCommon::StatusFromString("STOP"));
}



BOOST_AUTO_TEST_CASE(InjectorStatusEnumLoop) {
    BOOST_CHECK_EQUAL( WellCommon::OPEN     , WellCommon::StatusFromString( WellCommon::Status2String( WellCommon::OPEN ) ));
    BOOST_CHECK_EQUAL( WellCommon::AUTO   , WellCommon::StatusFromString( WellCommon::Status2String( WellCommon::AUTO ) ));
    BOOST_CHECK_EQUAL( WellCommon::SHUT     , WellCommon::StatusFromString( WellCommon::Status2String( WellCommon::SHUT ) ));
    BOOST_CHECK_EQUAL( WellCommon::STOP   , WellCommon::StatusFromString( WellCommon::Status2String( WellCommon::STOP ) ));

    BOOST_CHECK_EQUAL( "STOP"    , WellCommon::Status2String(WellCommon::StatusFromString(  "STOP" ) ));
    BOOST_CHECK_EQUAL( "OPEN"      , WellCommon::Status2String(WellCommon::StatusFromString(  "OPEN" ) ));
    BOOST_CHECK_EQUAL( "SHUT"      , WellCommon::Status2String(WellCommon::StatusFromString(  "SHUT" ) ));
    BOOST_CHECK_EQUAL( "AUTO"    , WellCommon::Status2String(WellCommon::StatusFromString(  "AUTO" ) ));
}



/*****************************************************************/

BOOST_AUTO_TEST_CASE(ProducerCOntrolMopdeEnum2String) {
    BOOST_CHECK_EQUAL( "ORAT"  ,  WellProducer::ControlMode2String(WellProducer::ORAT));
    BOOST_CHECK_EQUAL( "WRAT"  ,  WellProducer::ControlMode2String(WellProducer::WRAT));
    BOOST_CHECK_EQUAL( "GRAT"  , WellProducer::ControlMode2String(WellProducer::GRAT));
    BOOST_CHECK_EQUAL( "LRAT"  , WellProducer::ControlMode2String(WellProducer::LRAT));
    BOOST_CHECK_EQUAL( "CRAT"  , WellProducer::ControlMode2String(WellProducer::CRAT));
    BOOST_CHECK_EQUAL( "RESV"  ,  WellProducer::ControlMode2String(WellProducer::RESV));
    BOOST_CHECK_EQUAL( "BHP"   , WellProducer::ControlMode2String(WellProducer::BHP));
    BOOST_CHECK_EQUAL( "THP"   , WellProducer::ControlMode2String(WellProducer::THP));
    BOOST_CHECK_EQUAL( "GRUP"  , WellProducer::ControlMode2String(WellProducer::GRUP));
}


BOOST_AUTO_TEST_CASE(ProducerControlModeEnumFromString) {
    BOOST_CHECK_THROW( WellProducer::ControlModeFromString("XRAT") , std::invalid_argument );
    BOOST_CHECK_EQUAL( WellProducer::ORAT   , WellProducer::ControlModeFromString("ORAT"));
    BOOST_CHECK_EQUAL( WellProducer::WRAT   , WellProducer::ControlModeFromString("WRAT"));
    BOOST_CHECK_EQUAL( WellProducer::GRAT   , WellProducer::ControlModeFromString("GRAT"));
    BOOST_CHECK_EQUAL( WellProducer::LRAT   , WellProducer::ControlModeFromString("LRAT"));
    BOOST_CHECK_EQUAL( WellProducer::CRAT   , WellProducer::ControlModeFromString("CRAT"));
    BOOST_CHECK_EQUAL( WellProducer::RESV   , WellProducer::ControlModeFromString("RESV"));
    BOOST_CHECK_EQUAL( WellProducer::BHP    , WellProducer::ControlModeFromString("BHP" ));
    BOOST_CHECK_EQUAL( WellProducer::THP    , WellProducer::ControlModeFromString("THP" ));
    BOOST_CHECK_EQUAL( WellProducer::GRUP   , WellProducer::ControlModeFromString("GRUP"));
}



BOOST_AUTO_TEST_CASE(ProducerControlModeEnumLoop) {
    BOOST_CHECK_EQUAL( WellProducer::ORAT     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::ORAT ) ));
    BOOST_CHECK_EQUAL( WellProducer::WRAT     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::WRAT ) ));
    BOOST_CHECK_EQUAL( WellProducer::GRAT     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::GRAT ) ));
    BOOST_CHECK_EQUAL( WellProducer::LRAT     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::LRAT ) ));
    BOOST_CHECK_EQUAL( WellProducer::CRAT     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::CRAT ) ));
    BOOST_CHECK_EQUAL( WellProducer::RESV     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::RESV ) ));
    BOOST_CHECK_EQUAL( WellProducer::BHP      , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::BHP  ) ));
    BOOST_CHECK_EQUAL( WellProducer::THP      , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::THP  ) ));
    BOOST_CHECK_EQUAL( WellProducer::GRUP     , WellProducer::ControlModeFromString( WellProducer::ControlMode2String( WellProducer::GRUP ) ));

    BOOST_CHECK_EQUAL( "ORAT"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "ORAT"  ) ));
    BOOST_CHECK_EQUAL( "WRAT"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "WRAT"  ) ));
    BOOST_CHECK_EQUAL( "GRAT"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "GRAT"  ) ));
    BOOST_CHECK_EQUAL( "LRAT"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "LRAT"  ) ));
    BOOST_CHECK_EQUAL( "CRAT"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "CRAT"  ) ));
    BOOST_CHECK_EQUAL( "RESV"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "RESV"  ) ));
    BOOST_CHECK_EQUAL( "BHP"       , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "BHP"   ) ));
    BOOST_CHECK_EQUAL( "THP"       , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "THP"   ) ));
    BOOST_CHECK_EQUAL( "GRUP"      , WellProducer::ControlMode2String(WellProducer::ControlModeFromString( "GRUP"  ) ));
}

/*******************************************************************/
/*****************************************************************/

BOOST_AUTO_TEST_CASE(GuideRatePhaseEnum2String) {
    BOOST_CHECK_EQUAL( "OIL"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::OIL));
    BOOST_CHECK_EQUAL( "WAT"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::WAT));
    BOOST_CHECK_EQUAL( "GAS"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::GAS));
    BOOST_CHECK_EQUAL( "LIQ"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::LIQ));
    BOOST_CHECK_EQUAL( "COMB"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::COMB));
    BOOST_CHECK_EQUAL( "WGA"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::WGA));
    BOOST_CHECK_EQUAL( "CVAL"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::CVAL));
    BOOST_CHECK_EQUAL( "RAT"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::RAT));
    BOOST_CHECK_EQUAL( "RES"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::RES));
    BOOST_CHECK_EQUAL( "UNDEFINED"  ,  GuideRate::GuideRatePhaseEnum2String(GuideRate::UNDEFINED));
}


BOOST_AUTO_TEST_CASE(GuideRatePhaseEnumFromString) {
    BOOST_CHECK_THROW( GuideRate::GuideRatePhaseEnumFromString("XRAT") , std::invalid_argument );
    BOOST_CHECK_EQUAL( GuideRate::OIL   , GuideRate::GuideRatePhaseEnumFromString("OIL"));
    BOOST_CHECK_EQUAL( GuideRate::WAT   , GuideRate::GuideRatePhaseEnumFromString("WAT"));
    BOOST_CHECK_EQUAL( GuideRate::GAS   , GuideRate::GuideRatePhaseEnumFromString("GAS"));
    BOOST_CHECK_EQUAL( GuideRate::LIQ   , GuideRate::GuideRatePhaseEnumFromString("LIQ"));
    BOOST_CHECK_EQUAL( GuideRate::COMB   , GuideRate::GuideRatePhaseEnumFromString("COMB"));
    BOOST_CHECK_EQUAL( GuideRate::WGA   , GuideRate::GuideRatePhaseEnumFromString("WGA"));
    BOOST_CHECK_EQUAL( GuideRate::CVAL   , GuideRate::GuideRatePhaseEnumFromString("CVAL"));
    BOOST_CHECK_EQUAL( GuideRate::RAT   , GuideRate::GuideRatePhaseEnumFromString("RAT"));
    BOOST_CHECK_EQUAL( GuideRate::RES   , GuideRate::GuideRatePhaseEnumFromString("RES"));
    BOOST_CHECK_EQUAL( GuideRate::UNDEFINED, GuideRate::GuideRatePhaseEnumFromString("UNDEFINED"));
}



BOOST_AUTO_TEST_CASE(GuideRatePhaseEnum2Loop) {
    BOOST_CHECK_EQUAL( GuideRate::OIL     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::OIL ) ));
    BOOST_CHECK_EQUAL( GuideRate::WAT     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::WAT ) ));
    BOOST_CHECK_EQUAL( GuideRate::GAS     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::GAS ) ));
    BOOST_CHECK_EQUAL( GuideRate::LIQ     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::LIQ ) ));
    BOOST_CHECK_EQUAL( GuideRate::COMB     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::COMB ) ));
    BOOST_CHECK_EQUAL( GuideRate::WGA     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::WGA ) ));
    BOOST_CHECK_EQUAL( GuideRate::CVAL     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::CVAL ) ));
    BOOST_CHECK_EQUAL( GuideRate::RAT     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::RAT ) ));
    BOOST_CHECK_EQUAL( GuideRate::RES     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::RES ) ));
    BOOST_CHECK_EQUAL( GuideRate::UNDEFINED     , GuideRate::GuideRatePhaseEnumFromString( GuideRate::GuideRatePhaseEnum2String( GuideRate::UNDEFINED ) ));

    BOOST_CHECK_EQUAL( "OIL"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "OIL"  ) ));
    BOOST_CHECK_EQUAL( "WAT"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "WAT"  ) ));
    BOOST_CHECK_EQUAL( "GAS"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "GAS"  ) ));
    BOOST_CHECK_EQUAL( "LIQ"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "LIQ"  ) ));
    BOOST_CHECK_EQUAL( "COMB"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "COMB"  ) ));
    BOOST_CHECK_EQUAL( "WGA"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "WGA"  ) ));
    BOOST_CHECK_EQUAL( "CVAL"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "CVAL"  ) ));
    BOOST_CHECK_EQUAL( "RAT"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "RAT"  ) ));
    BOOST_CHECK_EQUAL( "RES"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "RES"  ) ));
    BOOST_CHECK_EQUAL( "UNDEFINED"      , GuideRate::GuideRatePhaseEnum2String(GuideRate::GuideRatePhaseEnumFromString( "UNDEFINED"  ) ));

}
