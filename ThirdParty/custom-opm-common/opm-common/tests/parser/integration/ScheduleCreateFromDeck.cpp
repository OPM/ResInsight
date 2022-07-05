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

#define BOOST_TEST_MODULE ScheduleIntegrationTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/Events.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>

using namespace Opm;


inline std::string pathprefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE(CreateSchedule) {
    Parser parser;
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE1");
    auto deck1 =  parser.parseFile(scheduleFile);
    std::stringstream ss;
    ss << deck1;
    auto deck2 = parser.parseString( ss.str());
    for (const auto& deck : {deck1 , deck2}) {
        TableManager table ( deck );
        FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
        Runspec runspec (deck);
        Schedule sched(deck,  grid , fp, runspec, python);
        BOOST_CHECK_EQUAL(asTimeT(TimeStampUTC(2007 , 5 , 10)), sched.getStartTime());
        BOOST_CHECK_EQUAL(9U, sched.size());
        BOOST_CHECK( deck.hasKeyword("NETBALAN") );
    }
}


BOOST_AUTO_TEST_CASE(CreateSchedule_Comments_After_Keywords) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_COMMENTS_AFTER_KEYWORDS");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);
    BOOST_CHECK_EQUAL(asTimeT(TimeStampUTC(2007, 5 , 10)) , sched.getStartTime());
    BOOST_CHECK_EQUAL(9U, sched.size());
}


BOOST_AUTO_TEST_CASE(WCONPROD_MissingCmode) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_MISSING_CMODE");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    auto python = std::make_shared<Python>();
    Runspec runspec (deck);
    BOOST_CHECK_NO_THROW( Schedule(deck, grid , fp, runspec, python) );
}


BOOST_AUTO_TEST_CASE(WCONPROD_Missing_DATA) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_CMODE_MISSING_DATA");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    BOOST_CHECK_THROW( Schedule(deck, grid , fp, runspec, python) , std::exception );
}


BOOST_AUTO_TEST_CASE(WellTestRefDepth) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELLS2");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(40,60,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck , grid , fp, runspec, python);

    const auto& well1 = sched.getWellatEnd("W_1");
    const auto& well2 = sched.getWellatEnd("W_2");
    const auto& well4 = sched.getWellatEnd("W_4");
    BOOST_CHECK_EQUAL( well1.getRefDepth() , grid.getCellDepth( 29 , 36 , 0 ));
    BOOST_CHECK_EQUAL( well2.getRefDepth() , 100 );
    BOOST_CHECK_THROW( well4.getRefDepth() , std::exception );
}





BOOST_AUTO_TEST_CASE(WellTesting) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELLS2");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(40,60,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);

    BOOST_CHECK_EQUAL(4U, sched.numWells());
    BOOST_CHECK(sched.hasWell("W_1"));
    BOOST_CHECK(sched.hasWell("W_2"));
    BOOST_CHECK(sched.hasWell("W_3"));

    BOOST_CHECK_CLOSE( 777/Metric::Time , sched.getWell("W_2", 7).getProductionProperties().ResVRate.getSI() , 0.0001);
    BOOST_CHECK_CLOSE( 777 , sched.getWell("W_2", 7).getProductionProperties().ResVRate.get<double>() , 0.0001);
    BOOST_CHECK_EQUAL( 0 ,                sched.getWell("W_2", 8).getProductionProperties().ResVRate.get<double>());

    BOOST_CHECK( Well::Status::SHUT == sched.getWell("W_2", 3).getStatus());

    {
        const auto & prop3 = sched.getWell("W_2", 3).getProductionProperties();
        BOOST_CHECK( Well::ProducerCMode::ORAT == prop3.controlMode);
        BOOST_CHECK(  prop3.hasProductionControl(Well::ProducerCMode::ORAT));
        BOOST_CHECK( !prop3.hasProductionControl(Well::ProducerCMode::GRAT));
        BOOST_CHECK( !prop3.hasProductionControl(Well::ProducerCMode::WRAT));
    }


    BOOST_CHECK( Well::Status::AUTO == sched.getWell("W_3", 3).getStatus());
    {
        const auto& prop7 = sched.getWell("W_3", 7).getProductionProperties();
        BOOST_CHECK_CLOSE( 999/Metric::Time , prop7.LiquidRate.getSI() , 0.001);
        BOOST_CHECK_CLOSE( 999 , prop7.LiquidRate.get<double>() , 0.001);
        BOOST_CHECK( Well::ProducerCMode::RESV == prop7.controlMode);
    }
    BOOST_CHECK_CLOSE( 8000/Metric::Time , sched.getWell("W_3", 3).getProductionProperties().LiquidRate.getSI(), 1.e-12);
    BOOST_CHECK_CLOSE( 18000/Metric::Time, sched.getWell("W_3", 8).getProductionProperties().LiquidRate.getSI(), 1.e-12);
    BOOST_CHECK_CLOSE( 8000 , sched.getWell("W_3", 3).getProductionProperties().LiquidRate.get<double>(), 1.e-12);
    BOOST_CHECK_CLOSE( 18000, sched.getWell("W_3", 8).getProductionProperties().LiquidRate.get<double>(), 1.e-12);


    {
        BOOST_CHECK_EQUAL(sched.getWell("W_1", 3).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().WaterRate.get<double>() , 4, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().GasRate.get<double>()   , 12345, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().OilRate.get<double>() , 4000, 0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().OilRate.get<double>() , 4000, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().WaterRate.get<double>() , 4, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().GasRate.get<double>()   , 12345,0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().WaterRate.get<double>(), 4, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().GasRate.get<double>() , 12345, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().OilRate.get<double>() , 4000, 0.001);


        BOOST_CHECK_EQUAL(sched.getWell("W_1", 6).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 6).getProductionProperties().OilRate.get<double>() , 14000, 0.001);

        BOOST_CHECK_EQUAL(sched.getWell("W_1", 7).getProductionProperties().predictionMode, true);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 7).getProductionProperties().OilRate.get<double>() , 11000, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 7).getProductionProperties().WaterRate.get<double>() , 44, 0.001);


        BOOST_CHECK_EQUAL(sched.getWell("W_1", 8).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 8).getProductionProperties().OilRate.get<double>() , 13000, 0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 10).getInjectionProperties().BHPTarget.get<double>(), 123.00 , 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 10).getInjectionProperties().THPTarget.get<double>(), 678.00 , 0.001);

        //----

        BOOST_CHECK_EQUAL(sched.getWell("W_1", 3).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().WaterRate.getSI() , 4/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().GasRate.getSI()   , 12345/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 3).getProductionProperties().OilRate.getSI() , 4000/Metric::Time, 0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().OilRate.getSI() , 4000/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().WaterRate.getSI() , 4/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 4).getProductionProperties().GasRate.getSI()   , 12345/Metric::Time,0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().WaterRate.getSI(), 4/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().GasRate.getSI() , 12345/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 5).getProductionProperties().OilRate.getSI() , 4000/Metric::Time, 0.001);


        BOOST_CHECK_EQUAL(sched.getWell("W_1", 6).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 6).getProductionProperties().OilRate.getSI() , 14000/Metric::Time, 0.001);

        BOOST_CHECK_EQUAL(sched.getWell("W_1", 7).getProductionProperties().predictionMode, true);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 7).getProductionProperties().OilRate.getSI() , 11000/Metric::Time, 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 7).getProductionProperties().WaterRate.getSI() , 44/Metric::Time, 0.001);


        BOOST_CHECK_EQUAL(sched.getWell("W_1", 8).getProductionProperties().predictionMode, false);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 8).getProductionProperties().OilRate.getSI() , 13000/Metric::Time , 0.001);

        BOOST_CHECK_CLOSE(sched.getWell("W_1", 10).getInjectionProperties().BHPTarget.getSI(), 123.00 * Metric::Pressure , 0.001);
        BOOST_CHECK_CLOSE(sched.getWell("W_1", 10).getInjectionProperties().THPTarget.getSI(), 678.00 * Metric::Pressure , 0.001);




        BOOST_CHECK( sched.getWell("W_1", 9).isInjector());
        {
            SummaryState st(TimeService::now());
            const auto controls = sched.getWell("W_1", 9).injectionControls(st);
            BOOST_CHECK_CLOSE(20000/Metric::Time ,  controls.surface_rate  , 0.001);
            BOOST_CHECK_CLOSE(200000/Metric::Time , controls.reservoir_rate, 0.001);
            BOOST_CHECK_CLOSE(6895 * Metric::Pressure , controls.bhp_limit, 0.001);
            BOOST_CHECK_CLOSE(0 , controls.thp_limit , 0.001);
            BOOST_CHECK( Well::InjectorCMode::RESV == controls.cmode);
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::RATE ));
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::RESV ));
            BOOST_CHECK( !controls.hasControl(Well::InjectorCMode::THP));
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::BHP));
        }


        BOOST_CHECK( Well::Status::OPEN == sched.getWell("W_1", 11).getStatus( ));
        BOOST_CHECK( Well::Status::OPEN == sched.getWell("W_1", 12).getStatus( ));
        BOOST_CHECK( Well::Status::SHUT == sched.getWell("W_1", 13).getStatus( ));
        BOOST_CHECK( Well::Status::OPEN == sched.getWell("W_1", 14).getStatus( ));
        {
            SummaryState st(TimeService::now());
            const auto controls = sched.getWell("W_1", 12).injectionControls(st);
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::RATE ));
            BOOST_CHECK( !controls.hasControl(Well::InjectorCMode::RESV));
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::THP ));
            BOOST_CHECK(  controls.hasControl(Well::InjectorCMode::BHP ));
        }
    }
}


BOOST_AUTO_TEST_CASE(WellTestCOMPDAT_DEFAULTED_ITEMS) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_COMPDAT1");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);
}


BOOST_AUTO_TEST_CASE(WellTestCOMPDAT) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELLS2");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(40,60,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);

    BOOST_CHECK_EQUAL(4U, sched.numWells());
    BOOST_CHECK(sched.hasWell("W_1"));
    BOOST_CHECK(sched.hasWell("W_2"));
    BOOST_CHECK(sched.hasWell("W_3"));
    {
        BOOST_CHECK_CLOSE(13000/Metric::Time , sched.getWell("W_1", 8).getProductionProperties().OilRate.getSI() , 0.0001);
        BOOST_CHECK_CLOSE(13000 , sched.getWell("W_1", 8).getProductionProperties().OilRate.get<double>() , 0.0001);
        {
            const auto& connections = sched.getWell("W_1", 3).getConnections();
            BOOST_CHECK_EQUAL(4U, connections.size());

            BOOST_CHECK(Connection::State::OPEN == connections.get(3).state());
            BOOST_CHECK_EQUAL(2.2836805555555556e-12 , connections.get(3).CF());
        }
        {
            const auto& connections = sched.getWell("W_1", 7).getConnections();
            BOOST_CHECK_EQUAL(4U, connections.size() );
            BOOST_CHECK(Connection::State::SHUT == connections.get( 3 ).state() );
        }
    }
}




BOOST_AUTO_TEST_CASE(GroupTreeTest_GRUPTREE_correct) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELSPECS_GRUPTREE");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule schedule(deck,  grid , fp, runspec, python);

    BOOST_CHECK( schedule.back().groups.has( "FIELD" ));
    BOOST_CHECK( schedule.back().groups.has( "PROD" ));
    BOOST_CHECK( schedule.back().groups.has( "INJE" ));
    BOOST_CHECK( schedule.back().groups.has( "MANI-PROD" ));
    BOOST_CHECK( schedule.back().groups.has( "MANI-INJ" ));
    BOOST_CHECK( schedule.back().groups.has( "DUMMY-PROD" ));
    BOOST_CHECK( schedule.back().groups.has( "DUMMY-INJ" ));
}




BOOST_AUTO_TEST_CASE(GroupTreeTest_GRUPTREE_WITH_REPARENT_correct_tree) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_GROUPS_REPARENT");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);

    const auto& field_group = sched.getGroup("FIELD", 1);
    const auto& new_group = sched.getGroup("GROUP_NEW", 1);
    const auto& nils_group = sched.getGroup("GROUP_NILS", 1);
    BOOST_CHECK_EQUAL(field_group.groups().size(), 2);
    BOOST_CHECK( field_group.hasGroup("GROUP_NEW"));
    BOOST_CHECK( field_group.hasGroup("GROUP_BJARNE"));

    BOOST_CHECK_EQUAL( new_group.control_group().value_or("ERROR"), "FIELD");
    BOOST_CHECK_EQUAL( new_group.flow_group().value_or("ERROR"), "FIELD");
    BOOST_CHECK( new_group.hasGroup("GROUP_NILS"));
    BOOST_CHECK_EQUAL( nils_group.control_group().value_or("ERROR"), "GROUP_NEW");
    BOOST_CHECK_EQUAL( nils_group.flow_group().value_or("ERROR"), "GROUP_NEW");
}

BOOST_AUTO_TEST_CASE( WellTestGroups ) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_GROUPS");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);
    SummaryState st(TimeService::now());

    BOOST_CHECK_EQUAL( 3U , sched.back().groups.size() );
    BOOST_CHECK( sched.back().groups.has( "INJ" ));
    BOOST_CHECK( sched.back().groups.has( "OP" ));

    {
        auto& group = sched.getGroup("INJ", 3);
        const auto& injection = group.injectionControls(Phase::WATER, st);
        BOOST_CHECK_EQUAL( Phase::WATER , injection.phase);
        BOOST_CHECK( Group::InjectionCMode::VREP == injection.cmode);
        BOOST_CHECK_CLOSE( 10/Metric::Time , injection.surface_max_rate, 0.001);
        BOOST_CHECK_CLOSE( 20/Metric::Time , injection.resv_max_rate, 0.001);
        BOOST_CHECK_EQUAL( 0.75 , injection.target_reinj_fraction);
        BOOST_CHECK_EQUAL( 0.95 , injection.target_void_fraction);
        BOOST_CHECK_EQUAL("INJ" , injection.reinj_group);
        BOOST_CHECK_EQUAL("INJ" , injection.voidage_group);
        BOOST_CHECK(group.isInjectionGroup());
    }
    {
        auto& group = sched.getGroup("INJ", 6);
        const auto& injection = group.injectionControls(Phase::OIL, st);
        BOOST_CHECK_EQUAL( Phase::OIL , injection.phase);
        BOOST_CHECK( Group::InjectionCMode::RATE == injection.cmode);
        BOOST_CHECK_CLOSE( 1000/Metric::Time , injection.surface_max_rate, 0.0001);
        BOOST_CHECK(group.isInjectionGroup());
    }

    {
        auto& group = sched.getGroup("OP", 3);
        const auto& production = group.productionControls(st);
        BOOST_CHECK( Group::ProductionCMode::ORAT == production.cmode);
        BOOST_CHECK_CLOSE( 10/Metric::Time , production.oil_target , 0.001);
        BOOST_CHECK_CLOSE( 20/Metric::Time , production.water_target , 0.001);
        BOOST_CHECK_CLOSE( 30/Metric::Time , production.gas_target , 0.001);
        BOOST_CHECK_CLOSE( 40/Metric::Time , production.liquid_target , 0.001);
        BOOST_CHECK(group.isProductionGroup());
    }

}


BOOST_AUTO_TEST_CASE( WellTestGroupAndWellRelation ) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELLS_AND_GROUPS");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(10,10,3);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);

    {
        auto& group1 = sched.getGroup("GROUP1", 0);

        BOOST_CHECK(  group1.hasWell("W_1"));
        BOOST_CHECK(  group1.hasWell("W_2"));
    }

    {
        auto& group1 = sched.getGroup("GROUP1", 1);
        auto& group2 = sched.getGroup("GROUP2", 1);

        BOOST_CHECK(  group1.hasWell("W_1"));
        BOOST_CHECK( !group1.hasWell("W_2"));
        BOOST_CHECK( !group2.hasWell("W_1"));
        BOOST_CHECK(  group2.hasWell("W_2"));

        BOOST_CHECK_THROW( sched.getGroup("GROUP2", 0), std::exception);
        BOOST_CHECK_NO_THROW( sched.getGroup("GROUP2", 1));
    }
}


/*
BOOST_AUTO_TEST_CASE(WellTestWELOPEN_ConfigWithIndexes_Throws) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELOPEN_INVALID");
    auto deck =  parser.parseFile(scheduleFile);
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,3);
    BOOST_CHECK_THROW(Schedule(grid , deck), std::logic_error);
}


BOOST_AUTO_TEST_CASE(WellTestWELOPENControlsSet) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WELOPEN");
    auto deck =  parser.parseFile(scheduleFile);
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>( 10,10,10 );
    Schedule sched(grid , deck);

    const auto* well1 = sched.getWell("W_1");
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, sched.getWell("W_1")->getStatus(0));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, sched.getWell("W_1")->getStatus(1));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, sched.getWell("W_1")->getStatus(2));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::STOP, sched.getWell("W_1")->getStatus(3));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::AUTO, sched.getWell("W_1")->getStatus(4));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::STOP, sched.getWell("W_1")->getStatus(5));
}
*/



BOOST_AUTO_TEST_CASE(WellTestWGRUPCONWellPropertiesSet) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WGRUPCON");
    auto deck =  parser.parseFile(scheduleFile);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule sched(deck,  grid , fp, runspec, python);

    const auto& well1 = sched.getWell("W_1", 0);
    BOOST_CHECK(well1.isAvailableForGroupControl( ));
    BOOST_CHECK_EQUAL(-1, well1.getGuideRate( ));
    BOOST_CHECK(Well::GuideRateTarget::OIL == well1.getGuideRatePhase( ));
    BOOST_CHECK_EQUAL(1.0, well1.getGuideRateScalingFactor( ));

    const auto& well2 = sched.getWell("W_2", 0);
    BOOST_CHECK(!well2.isAvailableForGroupControl( ));
    BOOST_CHECK_EQUAL(-1, well2.getGuideRate( ));
    BOOST_CHECK(Well::GuideRateTarget::UNDEFINED == well2.getGuideRatePhase( ));
    BOOST_CHECK_EQUAL(1.0, well2.getGuideRateScalingFactor( ));

    const auto& well3 = sched.getWell("W_3", 0);
    BOOST_CHECK(well3.isAvailableForGroupControl( ));
    BOOST_CHECK_EQUAL(100, well3.getGuideRate( ));
    BOOST_CHECK(Well::GuideRateTarget::RAT == well3.getGuideRatePhase( ));
    BOOST_CHECK_EQUAL(0.5, well3.getGuideRateScalingFactor( ));
}


BOOST_AUTO_TEST_CASE(TestDefaultedCOMPDATIJ) {
    Parser parser;
    const char * deckString = "\n\
START\n\
\n\
10 MAI 2007 /\n\
\n\
GRID\n\
PERMX\n\
   9000*0.25 /\n\
COPY \n\
   PERMX PERMY /\n\
   PERMX PERMZ /\n\
/\n\
SCHEDULE\n\
WELSPECS \n\
     'W1'        'OP'   11   21  3.33       'OIL'  7* /   \n\
/\n\
COMPDAT \n\
     'W1'   2*    1    1      'OPEN'  1*     32.948      0.311   3047.839  2*         'X'     22.100 /\n\
/\n";
    auto deck =  parser.parseString(deckString);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(30,30,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule sched(deck,  grid , fp, runspec, python);
    const auto& connections = sched.getWell("W1", 0).getConnections();
    BOOST_CHECK_EQUAL( 10 , connections.get(0).getI() );
    BOOST_CHECK_EQUAL( 20 , connections.get(0).getJ() );
}


/**
   This is a deck used in the opm-core wellsManager testing; just be
   certain we can parse it.
*/
BOOST_AUTO_TEST_CASE(OpmCode) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/wells_group.data");
    auto deck =  parser.parseFile(scheduleFile);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,5);
    TableManager table ( deck );
    Runspec runspec (deck);
    FieldPropsManager fp(deck, runspec.phases(), grid, table);
    BOOST_CHECK_NO_THROW( Schedule(deck , grid , fp, runspec, python) );
}



BOOST_AUTO_TEST_CASE(WELLS_SHUT) {
    Parser parser;
    auto python = std::make_shared<Python>();
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_SHUT_WELL");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(20,40,1);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule sched(deck,  grid , fp, runspec, python);


    {
        const auto& well1 = sched.getWell("W1", 1);
        const auto& well2 = sched.getWell("W2", 1);
        const auto& well3 = sched.getWell("W3", 1);
        BOOST_CHECK( Well::Status::OPEN == well1.getStatus());
        BOOST_CHECK( Well::Status::OPEN == well2.getStatus());
        BOOST_CHECK( Well::Status::OPEN == well3.getStatus());
    }
    {
        const auto& well1 = sched.getWell("W1", 2);
        const auto& well2 = sched.getWell("W2", 2);
        const auto& well3 = sched.getWell("W3", 2);
        BOOST_CHECK( Well::Status::SHUT == well1.getStatus());
        BOOST_CHECK( Well::Status::SHUT == well2.getStatus());
        BOOST_CHECK( Well::Status::SHUT == well3.getStatus());
    }
}


BOOST_AUTO_TEST_CASE(WellTestWPOLYMER) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_POLYMER");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(30,30,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);


    BOOST_CHECK_EQUAL(4U, sched.numWells());
    BOOST_CHECK(sched.hasWell("INJE01"));
    BOOST_CHECK(sched.hasWell("PROD01"));

    {
        const auto& well1 = sched.getWell("INJE01", 0);
        BOOST_CHECK( well1.isInjector());
        const WellPolymerProperties& props_well10 = well1.getPolymerProperties();
        BOOST_CHECK_CLOSE(1.5*Metric::PolymerDensity, props_well10.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well1 = sched.getWell("INJE01", 1);
        const WellPolymerProperties& props_well11 = well1.getPolymerProperties();
        BOOST_CHECK_CLOSE(1.0*Metric::PolymerDensity, props_well11.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well1 = sched.getWell("INJE01", 2);
        const WellPolymerProperties& props_well12 = well1.getPolymerProperties();
        BOOST_CHECK_CLOSE(0.1*Metric::PolymerDensity, props_well12.m_polymerConcentration, 0.0001);
    }

    {
        const auto& well2 = sched.getWell("INJE02", 0);
        BOOST_CHECK( well2.isInjector());
        const WellPolymerProperties& props_well20 = well2.getPolymerProperties();
        BOOST_CHECK_CLOSE(2.0*Metric::PolymerDensity, props_well20.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well2 = sched.getWell("INJE02", 1);
        const WellPolymerProperties& props_well21 = well2.getPolymerProperties();
        BOOST_CHECK_CLOSE(1.5*Metric::PolymerDensity, props_well21.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well2 = sched.getWell("INJE02", 2);
        const WellPolymerProperties& props_well22 = well2.getPolymerProperties();
        BOOST_CHECK_CLOSE(0.2*Metric::PolymerDensity, props_well22.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 0);
        BOOST_CHECK( well3.isInjector());
        const WellPolymerProperties& props_well30 = well3.getPolymerProperties();
        BOOST_CHECK_CLOSE(2.5*Metric::PolymerDensity, props_well30.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 1);
        const WellPolymerProperties& props_well31 = well3.getPolymerProperties();
        BOOST_CHECK_CLOSE(2.0*Metric::PolymerDensity, props_well31.m_polymerConcentration, 0.0001);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 2);
        const WellPolymerProperties& props_well32 = well3.getPolymerProperties();
        BOOST_CHECK_CLOSE(0.3*Metric::PolymerDensity, props_well32.m_polymerConcentration, 0.0001);
    }
}


BOOST_AUTO_TEST_CASE(WellTestWFOAM) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_FOAM");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(30,30,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);


    BOOST_CHECK_EQUAL(4U, sched.numWells());
    BOOST_CHECK(sched.hasWell("INJE01"));
    BOOST_CHECK(sched.hasWell("PROD01"));

    {
        const auto& well1 = sched.getWell("INJE01", 0);
        BOOST_CHECK( well1.isInjector());
        const WellFoamProperties& props_well10 = well1.getFoamProperties();
        BOOST_CHECK_EQUAL(0.11, props_well10.m_foamConcentration);
    }
    {
        const auto& well1 = sched.getWell("INJE01", 1);
        const WellFoamProperties& props_well11 = well1.getFoamProperties();
        BOOST_CHECK_EQUAL(0.12, props_well11.m_foamConcentration);
    }
    {
        const auto& well1 = sched.getWell("INJE01", 2);
        const WellFoamProperties& props_well12 = well1.getFoamProperties();
        BOOST_CHECK_EQUAL(0.13, props_well12.m_foamConcentration);
    }

    {
        const auto& well2 = sched.getWell("INJE02", 0);
        BOOST_CHECK( well2.isInjector());
        const WellFoamProperties& props_well20 = well2.getFoamProperties();
        BOOST_CHECK_EQUAL(0.0, props_well20.m_foamConcentration);
    }
    {
        const auto& well2 = sched.getWell("INJE02", 1);
        const WellFoamProperties& props_well21 = well2.getFoamProperties();
        BOOST_CHECK_EQUAL(0.22, props_well21.m_foamConcentration);
    }
    {
        const auto& well2 = sched.getWell("INJE02", 2);
        const WellFoamProperties& props_well22 = well2.getFoamProperties();
        BOOST_CHECK_EQUAL(0.0, props_well22.m_foamConcentration);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 0);
        BOOST_CHECK( well3.isInjector());
        const WellFoamProperties& props_well30 = well3.getFoamProperties();
        BOOST_CHECK_EQUAL(0.31, props_well30.m_foamConcentration);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 1);
        const WellFoamProperties& props_well31 = well3.getFoamProperties();
        BOOST_CHECK_EQUAL(0.0, props_well31.m_foamConcentration);
    }
    {
        const auto& well3 = sched.getWell("INJE03", 2);
        const WellFoamProperties& props_well32 = well3.getFoamProperties();
        BOOST_CHECK_EQUAL(0.33, props_well32.m_foamConcentration);
    }
}


BOOST_AUTO_TEST_CASE(WellTestWECON) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_WECON");
    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(30,30,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck,  grid , fp, runspec, python);

    BOOST_CHECK_EQUAL(3U, sched.numWells());
    BOOST_CHECK(sched.hasWell("INJE01"));
    BOOST_CHECK(sched.hasWell("PROD01"));
    BOOST_CHECK(sched.hasWell("PROD02"));

    {
        const WellEconProductionLimits& econ_limit1 = sched.getWell("PROD01", 0).getEconLimits();
        BOOST_CHECK(econ_limit1.onMinOilRate());
        BOOST_CHECK(econ_limit1.onMaxWaterCut());
        BOOST_CHECK(!(econ_limit1.onMinGasRate()));
        BOOST_CHECK(!(econ_limit1.onMaxGasOilRatio()));
        BOOST_CHECK_EQUAL(econ_limit1.maxWaterCut(), 0.95);
        BOOST_CHECK_EQUAL(econ_limit1.minOilRate(), 50.0/86400.);
        BOOST_CHECK_EQUAL(econ_limit1.minGasRate(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.maxGasOilRatio(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.endRun(), false);
        BOOST_CHECK_EQUAL(econ_limit1.followonWell(), "'");
        BOOST_CHECK(econ_limit1.quantityLimit() == WellEconProductionLimits::QuantityLimit::RATE);
        BOOST_CHECK(econ_limit1.workover() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit1.workoverSecondary() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit1.requireWorkover());
        BOOST_CHECK(econ_limit1.requireSecondaryWorkover());
        BOOST_CHECK(!(econ_limit1.validFollowonWell()));
        BOOST_CHECK(!(econ_limit1.endRun()));
        BOOST_CHECK(econ_limit1.onAnyRatioLimit());
        BOOST_CHECK(econ_limit1.onAnyRateLimit());
        BOOST_CHECK(econ_limit1.onAnyEffectiveLimit());

        const WellEconProductionLimits& econ_limit2 = sched.getWell("PROD01", 1).getEconLimits();
        BOOST_CHECK(!(econ_limit2.onMinOilRate()));
        BOOST_CHECK(econ_limit2.onMaxWaterCut());
        BOOST_CHECK(econ_limit2.onMinGasRate());
        BOOST_CHECK(!(econ_limit2.onMaxGasOilRatio()));
        BOOST_CHECK_EQUAL(econ_limit2.maxWaterCut(), 0.95);
        BOOST_CHECK_EQUAL(econ_limit2.minOilRate(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit2.minGasRate(), 1000./86400.);
        BOOST_CHECK_EQUAL(econ_limit2.maxGasOilRatio(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit2.endRun(), false);
        BOOST_CHECK_EQUAL(econ_limit2.followonWell(), "'");
        BOOST_CHECK(econ_limit2.quantityLimit() == WellEconProductionLimits::QuantityLimit::RATE);
        BOOST_CHECK(econ_limit2.workover() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit2.workoverSecondary() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit2.requireWorkover());
        BOOST_CHECK(econ_limit2.requireSecondaryWorkover());
        BOOST_CHECK(!(econ_limit2.validFollowonWell()));
        BOOST_CHECK(!(econ_limit2.endRun()));
        BOOST_CHECK(econ_limit2.onAnyRatioLimit());
        BOOST_CHECK(econ_limit2.onAnyRateLimit());
        BOOST_CHECK(econ_limit2.onAnyEffectiveLimit());
    }

    {
        const WellEconProductionLimits& econ_limit1 = sched.getWell("PROD02", 0).getEconLimits();
        BOOST_CHECK(!(econ_limit1.onMinOilRate()));
        BOOST_CHECK(!(econ_limit1.onMaxWaterCut()));
        BOOST_CHECK(!(econ_limit1.onMinGasRate()));
        BOOST_CHECK(!(econ_limit1.onMaxGasOilRatio()));
        BOOST_CHECK_EQUAL(econ_limit1.maxWaterCut(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.minOilRate(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.minGasRate(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.maxGasOilRatio(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit1.endRun(), false);
        BOOST_CHECK_EQUAL(econ_limit1.followonWell(), "'");
        BOOST_CHECK(econ_limit1.quantityLimit() == WellEconProductionLimits::QuantityLimit::RATE);
        BOOST_CHECK(econ_limit1.workover() == WellEconProductionLimits::EconWorkover::NONE);
        BOOST_CHECK(econ_limit1.workoverSecondary() == WellEconProductionLimits::EconWorkover::NONE);
        BOOST_CHECK(!(econ_limit1.requireWorkover()));
        BOOST_CHECK(!(econ_limit1.requireSecondaryWorkover()));
        BOOST_CHECK(!(econ_limit1.validFollowonWell()));
        BOOST_CHECK(!(econ_limit1.endRun()));
        BOOST_CHECK(!(econ_limit1.onAnyRatioLimit()));
        BOOST_CHECK(!(econ_limit1.onAnyRateLimit()));
        BOOST_CHECK(!(econ_limit1.onAnyEffectiveLimit()));

        const WellEconProductionLimits& econ_limit2 = sched.getWell("PROD02", 1).getEconLimits();
        BOOST_CHECK(!(econ_limit2.onMinOilRate()));
        BOOST_CHECK(econ_limit2.onMaxWaterCut());
        BOOST_CHECK(econ_limit2.onMinGasRate());
        BOOST_CHECK(!(econ_limit2.onMaxGasOilRatio()));
        BOOST_CHECK_EQUAL(econ_limit2.maxWaterCut(), 0.95);
        BOOST_CHECK_EQUAL(econ_limit2.minOilRate(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit2.minGasRate(), 1000.0/86400.);
        BOOST_CHECK_EQUAL(econ_limit2.maxGasOilRatio(), 0.0);
        BOOST_CHECK_EQUAL(econ_limit2.endRun(), false);
        BOOST_CHECK_EQUAL(econ_limit2.followonWell(), "'");
        BOOST_CHECK(econ_limit2.quantityLimit() == WellEconProductionLimits::QuantityLimit::RATE);
        BOOST_CHECK(econ_limit2.workover() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit2.workoverSecondary() == WellEconProductionLimits::EconWorkover::CON);
        BOOST_CHECK(econ_limit2.requireWorkover());
        BOOST_CHECK(econ_limit2.requireSecondaryWorkover());
        BOOST_CHECK(!(econ_limit2.validFollowonWell()));
        BOOST_CHECK(!(econ_limit2.endRun()));
        BOOST_CHECK(econ_limit2.onAnyRatioLimit());
        BOOST_CHECK(econ_limit2.onAnyRateLimit());
        BOOST_CHECK(econ_limit2.onAnyEffectiveLimit());
    }
}


BOOST_AUTO_TEST_CASE(TestEvents) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_EVENTS");

    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(40,40,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck , grid , fp, runspec, python);

    BOOST_CHECK(  sched[0].events().hasEvent(ScheduleEvents::NEW_WELL) );
    BOOST_CHECK( !sched[1].events().hasEvent(ScheduleEvents::NEW_WELL) );
    BOOST_CHECK(  sched[2].events().hasEvent(ScheduleEvents::NEW_WELL) );
    BOOST_CHECK( !sched[3].events().hasEvent(ScheduleEvents::NEW_WELL) );

    BOOST_CHECK(  sched[0].events().hasEvent(ScheduleEvents::COMPLETION_CHANGE) );
    BOOST_CHECK( !sched[1].events().hasEvent(ScheduleEvents::COMPLETION_CHANGE) );
    BOOST_CHECK(  sched[5].events().hasEvent(ScheduleEvents::COMPLETION_CHANGE) );

    BOOST_CHECK(  sched[1].events().hasEvent(ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK( !sched[2].events().hasEvent(ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK(  sched[3].events().hasEvent(ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK(  sched[5].events().hasEvent(ScheduleEvents::COMPLETION_CHANGE) );

    BOOST_CHECK(  sched[0].events().hasEvent(ScheduleEvents::GROUP_CHANGE));
    BOOST_CHECK( !sched[1].events().hasEvent(ScheduleEvents::GROUP_CHANGE));
    BOOST_CHECK(  sched[3].events().hasEvent(ScheduleEvents::GROUP_CHANGE));
    BOOST_CHECK( !sched[2].events().hasEvent(ScheduleEvents::NEW_GROUP));
    BOOST_CHECK(  sched[3].events().hasEvent(ScheduleEvents::NEW_GROUP));
}


BOOST_AUTO_TEST_CASE(TestWellEvents) {
    Parser parser;
    std::string scheduleFile(pathprefix() + "SCHEDULE/SCHEDULE_EVENTS");

    auto deck =  parser.parseFile(scheduleFile);
    EclipseGrid grid(40,40,30);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck , grid , fp, runspec, python);

    BOOST_CHECK(  sched[0].wellgroup_events().hasEvent( "W_1", ScheduleEvents::NEW_WELL));
    BOOST_CHECK(  sched[2].wellgroup_events().hasEvent( "W_2", ScheduleEvents::NEW_WELL));
    BOOST_CHECK( !sched[3].wellgroup_events().hasEvent( "W_2", ScheduleEvents::NEW_WELL));
    BOOST_CHECK(  sched[3].wellgroup_events().hasEvent( "W_2", ScheduleEvents::WELL_WELSPECS_UPDATE));

    BOOST_CHECK( sched[0].wellgroup_events().hasEvent( "W_1", ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK( sched[1].wellgroup_events().hasEvent( "W_1", ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK( sched[3].wellgroup_events().hasEvent( "W_1", ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK( sched[4].wellgroup_events().hasEvent( "W_1", ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK(!sched[5].wellgroup_events().hasEvent( "W_1", ScheduleEvents::WELL_STATUS_CHANGE));

    BOOST_CHECK( sched[0].wellgroup_events().hasEvent( "W_1", ScheduleEvents::COMPLETION_CHANGE));
    BOOST_CHECK( sched[5].wellgroup_events().hasEvent( "W_1", ScheduleEvents::COMPLETION_CHANGE));
}

