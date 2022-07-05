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

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#define BOOST_TEST_MODULE WellTest
#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/ScheduleTypes.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/Well/Connection.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>

using namespace Opm;

namespace {
    double liquid_PI_unit()
    {
        return UnitSystem::newMETRIC().to_si(UnitSystem::measure::liquid_productivity_index, 1.0);
    }

    double cp_rm3_per_db()
    {
        return UnitSystem::newMETRIC().to_si(UnitSystem::measure::transmissibility, 1.0);
    }
}

BOOST_AUTO_TEST_CASE(WellCOMPDATtestTRACK) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
                "GRID\n"
                "PORO\n"
                "1000*0.1  /\n"
                "PERMX \n"
                "1000*1 /\n"
                "PERMY \n"
                "1000*0.1 /\n"
                "PERMZ \n"
                "1000*0.01 /\n"
                "SCHEDULE\n"
                "DATES             -- 1\n"
                " 10  OKT 2008 / \n"
                "/\n"
                "WELSPECS\n"
                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                "/\n"
                "COMPORD\n"
                " OP_1 TRACK / \n"
                "/\n"
                "COMPDAT\n"
                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   3   9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                "/\n"
                "DATES             -- 2\n"
                " 20  JAN 2010 / \n"
                "/\n";


    auto deck = parser.parseString(input);
    auto python = std::make_shared<Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    const auto& op_1 = schedule.getWell("OP_1", 2);

    const auto& completions = op_1.getConnections();
    BOOST_CHECK_EQUAL(9U, completions.size());

    //Verify TRACK completion ordering
    for (size_t k = 0; k < completions.size(); ++k) {
        BOOST_CHECK_EQUAL(completions.get( k ).getK(), int(k));
    }

    // Output / input ordering
    const auto& output_connections = completions.output(grid);
    std::vector<int> expected = {0,2,3,4,5,6,7,8,1};
    for (size_t k = 0; k < completions.size(); ++k)
        BOOST_CHECK_EQUAL( expected[k], output_connections[k]->getK());
}

BOOST_AUTO_TEST_CASE(WellCOMPDATtestDEPTH) {
    Opm::Parser parser;
    std::string input  = R"(
START             -- 0
19 JUN 2007 /
GRID
PORO
    1000*0.1 /
PERMX
    1000*1 /
PERMY
    1000*0.1 /
PERMZ
    1000*0.01 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
WELSPECS
    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPORD
 OP_1 DEPTH /
/
COMPDAT
 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_1'  9  9   3   9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 /
/
DATES             -- 2
 20  JAN 2010 /
/
)";


    auto deck = parser.parseString(input);
    auto python = std::make_shared<Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    const auto& op_1 = schedule.getWell("OP_1", 2);

    const auto& completions = op_1.getConnections();
    BOOST_CHECK_EQUAL(9U, completions.size());

    //Verify TRACK completion ordering
    for (size_t k = 0; k < completions.size() - 1; ++k) {
        BOOST_CHECK(completions[k].depth() <= completions[k+1].depth());
    }

    // Output / input ordering
    const auto& output_connections = completions.output(grid);
    std::vector<int> expected = {0,2,3,4,5,6,7,8,1};
    for (size_t k = 0; k < completions.size(); ++k)
        BOOST_CHECK_EQUAL( expected[k], output_connections[k]->getK());
}


BOOST_AUTO_TEST_CASE(WellCOMPDATtestDefaultTRACK) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
                "GRID\n"
                "PORO\n"
                "1000*0.1  /\n"
                "PERMX \n"
                "1000*1 /\n"
                "PERMY \n"
                "1000*0.1 /\n"
                "PERMZ \n"
                "1000*0.01 /\n"
                "SCHEDULE\n"
                "DATES             -- 1\n"
                " 10  OKT 2008 / \n"
                "/\n"
                "WELSPECS\n"
                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                "/\n"
                "COMPDAT\n"
                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   3   9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                "/\n"
                "DATES             -- 2\n"
                " 20  JAN 2010 / \n"
                "/\n";


    auto deck = parser.parseString(input);
    auto python = std::make_shared<Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    const auto& op_1 = schedule.getWell("OP_1", 2);

    const auto& completions = op_1.getConnections();
    BOOST_CHECK_EQUAL(9U, completions.size());

    //Verify TRACK completion ordering
    for (size_t k = 0; k < completions.size(); ++k) {
        BOOST_CHECK_EQUAL(completions.get( k ).getK(), int(k));
    }
}

BOOST_AUTO_TEST_CASE(WellCOMPDATtestINPUT) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
                "GRID\n"
                "PORO\n"
                "1000*0.1  /\n"
                "PERMX \n"
                "1000*1 /\n"
                "PERMY \n"
                "1000*0.1 /\n"
                "PERMZ \n"
                "1000*0.01 /\n"
                "SCHEDULE\n"
                "DATES             -- 1\n"
                " 10  OKT 2008 / \n"
                "/\n"
                "WELSPECS\n"
                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                "/\n"
                "COMPORD\n"
                " OP_1 INPUT / \n"
                "/\n"
                "COMPDAT\n"
                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   3   9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                "/\n"
                "DATES             -- 2\n"
                " 20  JAN 2010 / \n"
                "/\n";


    auto deck = parser.parseString(input);
    Opm::EclipseGrid grid(10,10,10);
    Opm::ErrorGuard errors;
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Opm::Schedule schedule(deck, grid , fp, runspec, Opm::ParseContext(), errors, python);
    const auto& op_1 = schedule.getWell("OP_1", 2);

    const auto& completions = op_1.getConnections();
    BOOST_CHECK_EQUAL(9U, completions.size());
    BOOST_CHECK_EQUAL(completions.get( 1 ).getK(), 2);
    BOOST_CHECK_EQUAL(completions.get( 2 ).getK(), 3);
    BOOST_CHECK_EQUAL(completions.get( 3 ).getK(), 4);
    BOOST_CHECK_EQUAL(completions.get( 4 ).getK(), 5);
    BOOST_CHECK_EQUAL(completions.get( 5 ).getK(), 6);
    BOOST_CHECK_EQUAL(completions.get( 6 ).getK(), 7);
    BOOST_CHECK_EQUAL(completions.get( 7 ).getK(), 8);
    BOOST_CHECK_EQUAL(completions.get( 8 ).getK(), 1);
}

BOOST_AUTO_TEST_CASE(NewWellZeroCompletions) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED,  Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);
    BOOST_CHECK_EQUAL( 0U , well.getConnections( ).size() );
}


BOOST_AUTO_TEST_CASE(isProducerCorrectlySet) {
    // HACK: This test checks correctly setting of isProducer/isInjector. This property depends on which of
    //       WellProductionProperties/WellInjectionProperties is set last, independent of actual values.
    {
        Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);


        /* 1: Well is created as producer */
        BOOST_CHECK_EQUAL( false , well.isInjector());
        BOOST_CHECK_EQUAL( true , well.isProducer());

        /* Set a surface injection rate => Well becomes an Injector */
        auto injectionProps1 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
        injectionProps1->surfaceInjectionRate.update(100);
        well.updateInjection(injectionProps1);
        BOOST_CHECK_EQUAL( true  , well.isInjector());
        BOOST_CHECK_EQUAL( false , well.isProducer());
        BOOST_CHECK_EQUAL( 100 , well.getInjectionProperties().surfaceInjectionRate.get<double>());
    }


    {
        Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

        /* Set a reservoir injection rate => Well becomes an Injector */
        auto injectionProps2 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
        injectionProps2->reservoirInjectionRate.update(200);
        well.updateInjection(injectionProps2);
        BOOST_CHECK_EQUAL( false , well.isProducer());
        BOOST_CHECK_EQUAL( 200 , well.getInjectionProperties().reservoirInjectionRate.get<double>());
    }

    {
        Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

        /* Set rates => Well becomes a producer; injection rate should be set to 0. */
        auto injectionProps3 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
        well.updateInjection(injectionProps3);

        auto properties = std::make_shared<Opm::Well::WellProductionProperties>( well.getProductionProperties() );
        properties->OilRate.update(100);
        properties->GasRate.update(200);
        properties->WaterRate.update(300);
        well.updateProduction(properties);

        BOOST_CHECK_EQUAL( false , well.isInjector());
        BOOST_CHECK_EQUAL( true , well.isProducer());
        BOOST_CHECK_EQUAL( 0 , well.getInjectionProperties().surfaceInjectionRate.get<double>());
        BOOST_CHECK_EQUAL( 0 , well.getInjectionProperties().reservoirInjectionRate.get<double>());
        BOOST_CHECK_EQUAL( 100 , well.getProductionProperties().OilRate.get<double>());
        BOOST_CHECK_EQUAL( 200 , well.getProductionProperties().GasRate.get<double>());
        BOOST_CHECK_EQUAL( 300 , well.getProductionProperties().WaterRate.get<double>());
    }
}

BOOST_AUTO_TEST_CASE(GroupnameCorretlySet) {
    Opm::Well well("WELL1" , "G1", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    BOOST_CHECK_EQUAL("G1" , well.groupName());
    well.updateGroup( "GROUP2");
    BOOST_CHECK_EQUAL("GROUP2" , well.groupName());
}


BOOST_AUTO_TEST_CASE(addWELSPECS_setData_dataSet) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 23, 42, 2334.32, Opm::WellType(Opm::Phase::WATER), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    BOOST_CHECK_EQUAL(23, well.getHeadI());
    BOOST_CHECK_EQUAL(42, well.getHeadJ());
    BOOST_CHECK_EQUAL(2334.32, well.getRefDepth());
    BOOST_CHECK_EQUAL(Opm::Phase::WATER, well.getPreferredPhase());
}


BOOST_AUTO_TEST_CASE(XHPLimitDefault) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 23, 42, 2334.32, Opm::WellType(Opm::Phase::WATER), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);


    auto productionProps = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    productionProps->BHPTarget.update(100);
    productionProps->addProductionControl(Opm::Well::ProducerCMode::BHP);
    well.updateProduction(productionProps);
    BOOST_CHECK_EQUAL( 100 , well.getProductionProperties().BHPTarget.get<double>());
    BOOST_CHECK_EQUAL( true, well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::BHP ));

    auto injProps = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps->THPTarget.update(200);
    well.updateInjection(injProps);
    BOOST_CHECK_EQUAL( 200 , well.getInjectionProperties().THPTarget.get<double>());
    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::THP ));
}



BOOST_AUTO_TEST_CASE(ScheduleTypesInjectorType) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 23, 42, 2334.32, Opm::WellType(Opm::Phase::WATER), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);


    auto injectionProps = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injectionProps->injectorType = Opm::InjectorType::WATER;
    well.updateInjection(injectionProps);
    // TODO: Should we test for something other than wate here, as long as
    //       the default value for InjectorType is WellInjector::WATER?
    BOOST_CHECK( Opm::InjectorType::WATER == well.getInjectionProperties().injectorType);

}



/*****************************************************************/


BOOST_AUTO_TEST_CASE(WellHaveProductionControlLimit) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 23, 42, 2334.32, Opm::WellType(Opm::Phase::WATER), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);


    BOOST_CHECK( !well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::ORAT ));
    BOOST_CHECK( !well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::RESV ));

    auto properties1 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties1->OilRate.update(100);
    properties1->addProductionControl(Opm::Well::ProducerCMode::ORAT);
    well.updateProduction(properties1);
    BOOST_CHECK(  well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::ORAT ));
    BOOST_CHECK( !well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::RESV ));

    auto properties2 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties2->ResVRate.update(100);
    properties2->addProductionControl(Opm::Well::ProducerCMode::RESV);
    well.updateProduction(properties2);
    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::RESV ));

    auto properties3 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties3->OilRate.update(100);
    properties3->WaterRate.update(100);
    properties3->GasRate.update(100);
    properties3->LiquidRate.update(100);
    properties3->ResVRate.update(100);
    properties3->BHPTarget.update(100);
    properties3->THPTarget.update(100);
    properties3->addProductionControl(Opm::Well::ProducerCMode::ORAT);
    properties3->addProductionControl(Opm::Well::ProducerCMode::LRAT);
    properties3->addProductionControl(Opm::Well::ProducerCMode::BHP);
    well.updateProduction(properties3);

    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::ORAT ));
    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::LRAT ));
    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::BHP ));

    auto properties4 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties4->dropProductionControl( Opm::Well::ProducerCMode::LRAT );
    well.updateProduction(properties4);

    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::ORAT ));
    BOOST_CHECK(!well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::LRAT ));
    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::BHP ));
}


BOOST_AUTO_TEST_CASE(WellHaveInjectionControlLimit) {
    Opm::Well well("WELL1", "GROUP", 0, 1, 23, 42, 2334.32, Opm::WellType(Opm::Phase::WATER), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RATE ));
    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));

    auto injProps1 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps1->surfaceInjectionRate.update(100);
    injProps1->addInjectionControl(Opm::Well::InjectorCMode::RATE);
    well.updateInjection(injProps1);
    BOOST_CHECK(  well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RATE ));
    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));

    auto injProps2 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps2->reservoirInjectionRate.update(100);
    injProps2->addInjectionControl(Opm::Well::InjectorCMode::RESV);
    well.updateInjection(injProps2);
    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));

    auto injProps3 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps3->BHPTarget.update(100);
    injProps3->addInjectionControl(Opm::Well::InjectorCMode::BHP);
    injProps3->THPTarget.update(100);
    injProps3->addInjectionControl(Opm::Well::InjectorCMode::THP);
    well.updateInjection(injProps3);

    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RATE ));
    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));
    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::THP ));
    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::BHP ));


    auto injProps4 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps4->dropInjectionControl( Opm::Well::InjectorCMode::RESV );
    well.updateInjection(injProps4);
    BOOST_CHECK(  well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RATE ));
    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));
    BOOST_CHECK(  well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::THP ));
    BOOST_CHECK(  well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::BHP ));
}
/*********************************************************************/

BOOST_AUTO_TEST_CASE(WellGuideRatePhase_GuideRatePhaseSet) {
    Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    BOOST_CHECK(Opm::Well::GuideRateTarget::UNDEFINED == well.getGuideRatePhase());

    BOOST_CHECK(well.updateWellGuideRate(true, 100, Opm::Well::GuideRateTarget::RAT, 66.0));
    BOOST_CHECK(Opm::Well::GuideRateTarget::RAT == well.getGuideRatePhase());
    BOOST_CHECK_EQUAL(100, well.getGuideRate());
    BOOST_CHECK_EQUAL(66.0, well.getGuideRateScalingFactor());
}

BOOST_AUTO_TEST_CASE(WellEfficiencyFactorSet) {
    Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    BOOST_CHECK_EQUAL(1.0, well.getEfficiencyFactor());
    BOOST_CHECK( well.updateEfficiencyFactor(0.9));
    BOOST_CHECK_EQUAL(0.9, well.getEfficiencyFactor());
}


namespace {
    namespace WCONHIST {
        std::string all_specified_CMODE_THP() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'THP' 1 2 3/\n/\n";

            return input;
        }



        std::string all_specified() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'ORAT' 1 2 3/\n/\n";

            return input;
        }

        std::string orat_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'WRAT' 1* 2 3/\n/\n";

            return input;
        }

        std::string owrat_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'GRAT' 1* 1* 3/\n/\n";

            return input;
        }

        std::string all_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'LRAT'/\n/\n";

            return input;
        }

        std::string all_defaulted_with_bhp() {
            const std::string input =
                "WCONHIST\n"
                "-- 1    2     3      4-9 10\n"
                "   'P' 'OPEN' 'RESV' 6*  500/\n/\n";

            return input;
        }

        std::string bhp_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "-- 1    2     3    4-9 10\n"
                "  'P' 'OPEN' 'BHP' 6*  500/\n/\n";

            return input;
        }

        std::string all_defaulted_with_bhp_vfp_table() {
            const std::string input =
                "WCONHIST\n"
                "-- 1    2     3    4-6  7  8  9  10\n"
                "  'P' 'OPEN' 'RESV' 3*  3 10. 1* 500/\n/\n";

            return input;
        }


    Opm::Well::WellProductionProperties properties(const std::string& input, std::optional<VFPProdTable::ALQ_TYPE> alq_type = {}) {
            Opm::Parser parser;
            Opm::UnitSystem unit_system(Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC);
            auto deck = parser.parseString(input);
            const auto& record = deck["WCONHIST"].back().getRecord(0);
            Opm::Well::WellProductionProperties hist(unit_system, "W");
            hist.handleWCONHIST(alq_type, unit_system, record);


            return hist;
        }
    } // namespace WCONHIST

    namespace WCONPROD {
        std::string
        all_specified_CMODE_BHP()
        {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'BHP' 1 2 3/\n/\n";

            return input;
        }

        std::string orat_CMODE_other_defaulted()
        {
            const std::string input =
                "WCONPROD\n"
                "'P' 'OPEN' 'ORAT' 1 2 3/\n/\n";

            return input;
        }

        std::string thp_CMODE()
        {
            const std::string input =
                "WCONPROD\n"
                "'P' 'OPEN' 'THP' 1 2 3 3* 10. 8 13./\n/\n";

            return input;
        }

        std::string bhp_CMODE()
        {
            const std::string input =
                "WCONPROD\n"
                "'P' 'OPEN' 'BHP' 1 2 3 2* 20. 10. 8 13./\n/\n";

            return input;
        }

        Opm::UnitSystem unit_system(Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC);
        Opm::Well::WellProductionProperties properties(const std::string& input, std::optional<VFPProdTable::ALQ_TYPE> alq_type = {})
        {
            Opm::Parser parser;
            auto deck = parser.parseString(input);
            const auto& kwd     = deck["WCONPROD"].back();
            const auto&  record = kwd.getRecord(0);
            Opm::Well::WellProductionProperties pred(unit_system, "W");
            pred.handleWCONPROD(alq_type, unit_system, "WELL", record);

            return pred;
        }
    }

} // namespace anonymous

BOOST_AUTO_TEST_CASE(WCH_All_Specified_BHP_Defaulted)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_specified());

    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::ORAT);

    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_ORAT_Defaulted_BHP_Defaulted)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::orat_defaulted());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::WRAT);

    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_OWRAT_Defaulted_BHP_Defaulted)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::owrat_defaulted());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::GRAT);

    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_Rates_Defaulted_BHP_Defaulted)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::LRAT);

    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_Rates_Defaulted_BHP_Specified)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted_with_bhp());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::RESV));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::RESV);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_Rates_NON_Defaulted_VFP)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted_with_bhp_vfp_table(), VFPProdTable::ALQ_TYPE::ALQ_UNDEF);

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::RESV));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::RESV);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    BOOST_CHECK_EQUAL(p.VFPTableNumber, 3);
    BOOST_CHECK_EQUAL(p.ALQValue.get<double>(), 10.);
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_BHP_Specified)
{
    Opm::SummaryState st(TimeService::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::bhp_defaulted());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::BHP);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 5e7);
}

BOOST_AUTO_TEST_CASE(WCONPROD_ORAT_CMode)
{
    const Opm::Well::WellProductionProperties& p = WCONPROD::properties(WCONPROD::orat_CMODE_other_defaulted());

    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::THP));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::ORAT);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    BOOST_CHECK_EQUAL(p.VFPTableNumber, 0);
    BOOST_CHECK_EQUAL(p.ALQValue.get<double>(), 0.);
}

BOOST_AUTO_TEST_CASE(WCONPROD_THP_CMode)
{
    const Opm::Well::WellProductionProperties& p =
        WCONPROD::properties(WCONPROD::thp_CMODE(), VFPProdTable::ALQ_TYPE::ALQ_UNDEF);

    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::THP));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::THP);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    BOOST_CHECK_EQUAL(p.VFPTableNumber, 8);
    BOOST_CHECK_EQUAL(p.ALQValue.get<double>(), 13.);
    BOOST_CHECK_EQUAL(p.THPTarget.getSI(), 1000000.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.getSI(), 101325.); // 1 atm.
}

BOOST_AUTO_TEST_CASE(WCONPROD_BHP_CMode)
{
    const Opm::Well::WellProductionProperties& p =
        WCONPROD::properties(WCONPROD::bhp_CMODE(), VFPProdTable::ALQ_TYPE::ALQ_UNDEF);

    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::THP));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::BHP);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    BOOST_CHECK_EQUAL(p.VFPTableNumber, 8);
    BOOST_CHECK_EQUAL(p.ALQValue.get<double>(), 13.);
    BOOST_CHECK_EQUAL(p.THPTarget.get<double>(), 10.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.get<double>(), 20.); // 20 barsa
    BOOST_CHECK_EQUAL(p.THPTarget.getSI(), 1000000.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.getSI(), 2000000.); // 20 barsa
}


BOOST_AUTO_TEST_CASE(BHP_CMODE)
{
    BOOST_CHECK_THROW( WCONHIST::properties(WCONHIST::all_specified_CMODE_THP()) , std::exception);
    BOOST_CHECK_THROW( WCONPROD::properties(WCONPROD::all_specified_CMODE_BHP()) , std::exception);
}




BOOST_AUTO_TEST_CASE(CMODE_DEFAULT) {
    auto unit_system = UnitSystem::newMETRIC();
    const Opm::Well::WellProductionProperties Pproperties(unit_system, "W");
    const Opm::Well::WellInjectionProperties Iproperties(unit_system, "W");

    BOOST_CHECK( Pproperties.controlMode == Opm::Well::ProducerCMode::CMODE_UNDEFINED );
    BOOST_CHECK( Iproperties.controlMode == Opm::Well::InjectorCMode::CMODE_UNDEFINED );
}




BOOST_AUTO_TEST_CASE(WELL_CONTROLS) {
    auto unit_system = UnitSystem::newMETRIC();
    Opm::Well well("WELL", "GROUP", 0, 0, 0, 0, 1000, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Opm::Connection::Order::DEPTH, unit_system, 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);
    Opm::Well::WellProductionProperties prod(unit_system, "OP1");
    Opm::SummaryState st(Opm::TimeService::now());
    well.productionControls(st);

    // Use a scalar FIELD variable - that should work; although it is a bit weird.
    st.update("FUX", 1);
    prod.OilRate = UDAValue("FUX");
    BOOST_CHECK_EQUAL(1, prod.controls(st, 0).oil_rate);


    // Use the wellrate WUX for well OP1; the well is now added with
    // SummaryState::update_well_var() and we should automatically fetch the
    // correct well value.
    prod.OilRate = UDAValue("WUX");
    st.update_well_var("OP1", "WUX", 10);
    BOOST_CHECK_EQUAL(10, prod.controls(st, 0).oil_rate);
}


BOOST_AUTO_TEST_CASE(ExtraAccessors) {
    Opm::Well inj("WELL1" , "GROUP", 0, 1, 0, 0, 0.0,  Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);
    Opm::Well prod("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

    auto inj_props= std::make_shared<Opm::Well::WellInjectionProperties>(inj.getInjectionProperties());
    inj_props->VFPTableNumber = 100;
    inj.updateInjection(inj_props);

    auto prod_props = std::make_shared<Opm::Well::WellProductionProperties>( prod.getProductionProperties() );
    prod_props->VFPTableNumber = 200;
    prod.updateProduction(prod_props);

    BOOST_CHECK_THROW(prod.temperature(), std::runtime_error);
    BOOST_CHECK_EQUAL(inj.vfp_table_number(), 100);
    BOOST_CHECK_EQUAL(prod.vfp_table_number(), 200);
}

BOOST_AUTO_TEST_CASE(WELOPEN) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
                "GRID\n"
                "PORO\n"
                "1000*0.1  /\n"
                "PERMX \n"
                "1000*1 /\n"
                "PERMY \n"
                "1000*0.1 /\n"
                "PERMZ \n"
                "1000*0.01 /\n"
                "SCHEDULE\n"
                "DATES             -- 1\n"
                " 10  OKT 2008 / \n"
                "/\n"
                "WELSPECS\n"
                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                "/\n"
                "COMPDAT\n"
                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   3   9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                "/\n"
                "WELOPEN \n"
                " 'OP_1'  'OPEN' /\n"
                "/\n"
                "DATES             -- 2\n"
                " 20  JAN 2010 / \n"
                "/\n"
                "WELOPEN \n"
                " 'OP_1'  'SHUT' 0 0 0 2* /\n"
                "/\n";


    auto deck = parser.parseString(input);
    auto python = std::make_shared<Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    {
        const auto& op_1 = schedule.getWell("OP_1", 1);
        BOOST_CHECK(op_1.getStatus() == Well::Status::OPEN);
    }
    {
        const auto& op_1 = schedule.getWell("OP_1", 2);
        BOOST_CHECK(op_1.getStatus() == Well::Status::SHUT);
    }
}


BOOST_AUTO_TEST_CASE(WellTypeTest) {
    BOOST_CHECK_THROW(Opm::WellType(0, 3), std::invalid_argument);
    BOOST_CHECK_THROW(Opm::WellType(5, 3), std::invalid_argument);
    BOOST_CHECK_THROW(Opm::WellType(3, 0), std::invalid_argument);
    BOOST_CHECK_THROW(Opm::WellType(3, 5), std::invalid_argument);

    Opm::WellType wt1(1,1);
    BOOST_CHECK(wt1.producer());
    BOOST_CHECK(!wt1.injector());
    BOOST_CHECK_EQUAL(wt1.ecl_wtype(), 1);
    BOOST_CHECK_EQUAL(wt1.ecl_phase(), 1);
    BOOST_CHECK(wt1.preferred_phase() == Phase::OIL);
    BOOST_CHECK_THROW(wt1.injector_type(), std::invalid_argument);

    Opm::WellType wt4(4,3);
    BOOST_CHECK(!wt4.producer());
    BOOST_CHECK(wt4.injector());
    BOOST_CHECK_EQUAL(wt4.ecl_wtype(), 4);
    BOOST_CHECK_EQUAL(wt4.ecl_phase(), 3);
    BOOST_CHECK(wt4.preferred_phase() == Phase::GAS);
    BOOST_CHECK(wt4.injector_type() == InjectorType::GAS);

    BOOST_CHECK(wt4.update(true));
    BOOST_CHECK(!wt4.update(true));
    BOOST_CHECK(wt4.producer());
    BOOST_CHECK(!wt4.injector());
    BOOST_CHECK_EQUAL(wt4.ecl_wtype(), 1);
    BOOST_CHECK_EQUAL(wt4.ecl_phase(), 3);
    BOOST_CHECK(wt4.preferred_phase() == Phase::GAS);

    Opm::WellType wtp(false, Phase::WATER);
    BOOST_CHECK(!wtp.producer());
    BOOST_CHECK(wtp.injector());
    BOOST_CHECK_EQUAL(wtp.ecl_wtype(), 3);
    BOOST_CHECK_EQUAL(wtp.ecl_phase(), 2);
    BOOST_CHECK(wtp.preferred_phase() == Phase::WATER);
    BOOST_CHECK(wtp.injector_type() == InjectorType::WATER);

    wtp.update( InjectorType::GAS );
    BOOST_CHECK_EQUAL(wtp.ecl_wtype(), 4);
    BOOST_CHECK_EQUAL(wtp.ecl_phase(), 2);
    BOOST_CHECK(wtp.preferred_phase() == Phase::GAS);
    BOOST_CHECK(wtp.injector_type() == InjectorType::GAS);
}

BOOST_AUTO_TEST_CASE(Injector_Control_Mode) {
    using IMode = ::Opm::Well::InjectorCMode;
    using IType = ::Opm::InjectorType;

    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::GRUP, IType::GAS), -1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::GRUP, IType::WATER), -1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::GRUP, IType::MULTI), -1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::GRUP, IType::OIL), -1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RATE, IType::OIL), 1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RATE, IType::WATER), 2);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RATE, IType::GAS), 3);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RATE, IType::MULTI), -10);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RESV, IType::GAS), 5);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RESV, IType::WATER), 5);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RESV, IType::MULTI), 5);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::RESV, IType::OIL), 5);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::THP, IType::GAS), 6);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::THP, IType::WATER), 6);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::THP, IType::MULTI), 6);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::THP, IType::OIL), 6);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::BHP, IType::GAS), 7);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::BHP, IType::WATER), 7);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::BHP, IType::MULTI), 7);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::BHP, IType::OIL), 7);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(IMode::CMODE_UNDEFINED, IType::WATER), -10);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(static_cast<IMode>(1729), IType::WATER), -10);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(static_cast<IMode>(1729), IType::WATER), -10); // Unknown combination
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(static_cast<IMode>(1729), IType::WATER), -10); // Unknown combination
}

BOOST_AUTO_TEST_CASE(Producer_Control_Mode) {
    using PMode = ::Opm::Well::ProducerCMode;

    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::GRUP), -1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::ORAT), 1);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::WRAT), 2);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::GRAT), 3);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::LRAT), 4);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::RESV), 5);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::THP ), 6);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::BHP ), 7);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::CRAT), 9);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::NONE), -10);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(PMode::CMODE_UNDEFINED), -10);
    BOOST_CHECK_EQUAL(Well::eclipseControlMode(static_cast<PMode>(271828)), -10);
}

BOOST_AUTO_TEST_CASE(WPIMULT) {
    Opm::Parser parser;
    std::string input = R"(
START             -- 0
19 JUN 2007 /

REGIONS

PVTNUM
 1000*77 /

GRID
PORO
 1000*0.1 /
PERMX
 1000*1 /
PERMY
 1000*0.1 /
PERMZ
 1000*0.01 /
SCHEDULE

WELSPECS
    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
    'OP_2'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  66 /
/
COMPDAT
 'OP_1'  9  9   1   1 'OPEN' 1*   1.0 0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_1'  9  9   2   2 'OPEN' 1*   2.0 0.311  3047.839 1*  1*  'X'  22.100 /
 'OP_1'  9  9   3   3 'OPEN' 1*   3.0 0.311  4332.346 1*  1*  'X'  22.123 /
/
DATES             -- 1
 20  JAN 2010 /
/

-- Should not hit any connections
WPIMULT
  'OP_1'  2  5  /
/

DATES             -- 2
 20  FEB 2010 /
/

--
WPIMULT
  'OP_1'  2  9  9 1 /
/

DATES             -- 3
 20  MAR 2010 /
/

--
WPIMULT
  'OP_1'  2  9  9 2 /
/

DATES             -- 4
 20  APR 2010 /
/

--
WPIMULT
  'OP_1'  2  9  9  3 /
/

DATES             -- 5
 20  JUN 2010 /
/

--
WPIMULT
  'OP_1'  0.5 /
/

)";


    auto deck = parser.parseString(input);
    const auto& units = deck.getActiveUnitSystem();
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    const auto CF0 = units.to_si(Opm::UnitSystem::measure::transmissibility, 1.0);
    {
        const auto& well = schedule.getWell("OP_1", 0);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 1.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 3.0 * CF0, 1e-6);
        BOOST_CHECK_EQUAL( well.pvt_table_number(), 77);

        const auto& well2 = schedule.getWell("OP_2", 0);
        BOOST_CHECK_EQUAL( well2.pvt_table_number(), 66);
    }
    {
        const auto& well = schedule.getWell("OP_1", 1);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 1.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 3.0 * CF0, 1e-6);
    }
    {
        const auto& well = schedule.getWell("OP_1", 2);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 3.0 * CF0, 1e-6);
    }
    {
        const auto& well = schedule.getWell("OP_1", 3);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 4.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 3.0 * CF0, 1e-6);
    }
    {
        const auto& well = schedule.getWell("OP_1", 4);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 4.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 6.0 * CF0, 1e-6);
    }
    {
        const auto& well = schedule.getWell("OP_1", 5);
        const auto& connections = well.getConnections();
        BOOST_CHECK_CLOSE( connections[0].CF(), 1.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[1].CF(), 2.0 * CF0, 1e-6);
        BOOST_CHECK_CLOSE( connections[2].CF(), 3.0 * CF0, 1e-6);
    }
}


BOOST_AUTO_TEST_CASE(FIRST_OPEN) {
    Opm::Parser parser;
    std::string input = R"(
START             -- 0
19 JUN 2007 /

REGIONS

PVTNUM
 1000*77 /

GRID
PORO
 1000*0.1 /
PERMX
 1000*1 /
PERMY
 1000*0.1 /
PERMZ
 1000*0.01 /

SCHEDULE

WELSPECS
    'P'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
    'I'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  66 /
/

COMPDAT
 'P'  9  9   2   2 'OPEN' 1*   2.0 0.311  3047.839 1*  1*  'X'  22.100 /
 'I'  9  9   3   3 'OPEN' 1*   3.0 0.311  4332.346 1*  1*  'X'  22.123 /
/

DATES             -- 1
 20  JAN 2010 /
/

DATES             -- 2
 20  FEB 2010 /
/

WCONPROD
 'P' 'OPEN' 'BHP' 1 2 3 2* 20. 10. 0 13 /
/

WCONINJE
 'I' 'GAS' 'OPEN' 'RATE'  1000 /
/


)";


    auto deck = parser.parseString(input);
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    Opm::Schedule schedule(deck, grid , fp, runspec, python);
    {
        const auto& iwell = schedule.getWell("I", 0);
        const auto& pwell = schedule.getWell("P", 0);

        BOOST_CHECK( iwell.getStatus() == Well::Status::SHUT );
        BOOST_CHECK( pwell.getStatus() == Well::Status::SHUT );
        BOOST_CHECK( !iwell.hasProduced() );
        BOOST_CHECK( !pwell.hasProduced() );
    }
    {
        const auto& iwell = schedule.getWell("I", 2);
        const auto& pwell = schedule.getWell("P", 2);

        BOOST_CHECK( iwell.getStatus() == Well::Status::OPEN );
        BOOST_CHECK( pwell.getStatus() == Well::Status::OPEN );
        BOOST_CHECK( !iwell.hasProduced() );
        BOOST_CHECK( pwell.hasProduced() );
    }
}


BOOST_AUTO_TEST_CASE(WellPI) {
    const auto deck = Parser{}.parseString(R"(RUNSPEC
START
7 OCT 2020 /

DIMENS
  10 10 3 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  300*100.0 /
PERMY
  300*100.0 /
PERMZ
  300*10.0 /
PORO
  300*0.3 /

SCHEDULE
WELSPECS
  'P' 'G' 10 10 2005 'LIQ' /
/
COMPDAT
  'P' 0 0 1 3 OPEN 1 100 /
/

END
)");

    const auto es    = EclipseState{ deck };
    const auto sched = Schedule{ deck, es };

    const auto expectCF = 100.0*cp_rm3_per_db();

    auto wellP = sched.getWell("P", 0);

    for (const auto& conn : wellP.getConnections()) {
        BOOST_CHECK_CLOSE(conn.CF(), expectCF, 1.0e-10);
    }

    // Simulate applying WELPI before WELPI keyword.  No effect.
    {
        std::vector<bool> scalingApplicable;
        wellP.applyWellProdIndexScaling(2.7182818, scalingApplicable);
        for (const auto& conn : wellP.getConnections()) {
            BOOST_CHECK_CLOSE(conn.CF(), expectCF, 1.0e-10);
        }

        for (const bool applicable : scalingApplicable) {
            BOOST_CHECK_MESSAGE(! applicable, "No connection must be eligible for WELPI scaling");
        }
    }

    // Simulate applying WELPI after seeing
    //
    //   WELPI
    //     P 2 /
    //   /
    //
    // (ignoring units of measure)
    BOOST_CHECK_MESSAGE(wellP.updateWellProductivityIndex(),
                        "First call to updateWellProductivityIndex() must be a state change");
    BOOST_CHECK_MESSAGE(!wellP.updateWellProductivityIndex(),
                        "Second call to updateWellProductivityIndex() must NOT be a state change");

    // Want PI=2, but actual/effective PI=1 => scale CF by 2.0/1.0.
    {
        const auto scalingFactor = wellP.convertDeckPI(2.0) /  liquid_PI_unit();
        BOOST_CHECK_CLOSE(scalingFactor, 2.0, 1.0e-10);

        std::vector<bool> scalingApplicable;
        wellP.applyWellProdIndexScaling(scalingFactor, scalingApplicable);
        for (const auto& conn : wellP.getConnections()) {
            BOOST_CHECK_CLOSE(conn.CF(), 2.0*expectCF, 1.0e-10);
        }

        for (const bool applicable : scalingApplicable) {
            BOOST_CHECK_MESSAGE(applicable, "All connections must be eligible for WELPI scaling");
        }
    }

    // Repeated application of WELPI multiplies scaling factors.
    {
        const auto scalingFactor = wellP.convertDeckPI(2.0) /  liquid_PI_unit();
        BOOST_CHECK_CLOSE(scalingFactor, 2.0, 1.0e-10);

        std::vector<bool> scalingApplicable;
        wellP.applyWellProdIndexScaling(scalingFactor, scalingApplicable);
        for (const auto& conn : wellP.getConnections()) {
            BOOST_CHECK_CLOSE(conn.CF(), 4.0*expectCF, 1.0e-10);
        }

        for (const bool applicable : scalingApplicable) {
            BOOST_CHECK_MESSAGE(applicable, "All connections must be eligible for WELPI scaling");
        }
    }

    // New WELPI record does not reset the scaling factors
    wellP.updateWellProductivityIndex();
    for (const auto& conn : wellP.getConnections()) {
        BOOST_CHECK_CLOSE(conn.CF(), 4.0*expectCF, 1.0e-10);
    }

    // Effective PI=desired PI => no scaling change
    {
        const auto scalingFactor = wellP.convertDeckPI(3.0) /  (3.0*liquid_PI_unit());
        BOOST_CHECK_CLOSE(scalingFactor, 1.0, 1.0e-10);

        std::vector<bool> scalingApplicable;
        wellP.applyWellProdIndexScaling(scalingFactor, scalingApplicable);
        for (const auto& conn : wellP.getConnections()) {
            BOOST_CHECK_CLOSE(conn.CF(), 4.0*expectCF, 1.0e-10);
        }

        for (const bool applicable : scalingApplicable) {
            BOOST_CHECK_MESSAGE(applicable, "All connections must be eligible for WELPI scaling");
        }
    }
}

BOOST_AUTO_TEST_CASE(Has_Same_Connections_Pointers) {
    const auto deck = Parser{}.parseString(R"(RUNSPEC
START
7 OCT 2020 /

DIMENS
  10 10 3 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  300*100.0 /
PERMY
  300*100.0 /
PERMZ
  300*10.0 /
PORO
  300*0.3 /

SCHEDULE
WELSPECS
  'P' 'G' 10 10 2005 'LIQ' /
/
COMPDAT
  'P' 0 0 1 3 OPEN 1 100 /
/

END
)");

    const auto es    = EclipseState{ deck };
    const auto sched = Schedule{ deck, es };

    const auto wellP = sched.getWell("P", 0);
    auto wellQ = wellP;

    BOOST_CHECK_MESSAGE(wellP.hasSameConnectionsPointers(wellQ),
                        "P and Q must have the same internal connections pointers");

    auto connQ = std::make_shared<WellConnections>(wellP.getConnections());
    wellQ.updateConnections(std::move(connQ), true);
    BOOST_CHECK_MESSAGE(! wellP.hasSameConnectionsPointers(wellQ),
                        "P and Q must NOT have the same internal connections pointers "
                        "after forcibly updating the connections structure");

    BOOST_CHECK_MESSAGE(wellP.getConnections() == wellQ.getConnections(),
                        "P and Q must have same WellConnections VALUE");
}



BOOST_AUTO_TEST_CASE(REPERF) {
      const auto deck = Parser{}.parseString(R"(RUNSPEC
START
7 OCT 2020 /

DIMENS
  10 10 4 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  4*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  400*100.0 /
PERMY
  400*100.0 /
PERMZ
  400*10.0 /
PORO
  400*0.3 /

SCHEDULE

WELSPECS
     'W1'   'G' 1  1  1*       'OIL'  2*      'STOP'  4* /
/

COMPDAT
     'W1'   1 1 4 4      'OPEN'  1*     34.720      0.216   3095.832  2*         'Y'     12.828 /
     'W1'   1 1 3 3      'OPEN'  1*     34.720      0.216   3095.832  2*         'Y'     12.828 /
/
-- W0

TSTEP
  1 /

COMPDAT
     'W1'     1    1     2    2      'OPEN'  1*     25.620      0.216   2086.842  2*         'Y'      8.486 /
/

-- W1
TSTEP
  1 /



WELSPECS
  'W1' 'G' 1 1 2005 'LIQ' /
/

-- W2

TSTEP
1 /

WELSPECS
  'W1' 'G' 1 1 1* 'LIQ' /
/
-- W3

WPAVEDEP
  'W1'  0 /
/

TSTEP
1  /


COMPDAT
     'W1'     1    1     1    1      'OPEN'  1*     25.620      0.216   2086.842  2*         'Y'      8.486 /
/
-- W4
TSTEP
1 /

WELSPECS
  'W1' 'G' 1 1 1* 'LIQ' /
/
-- W5

END
)");

    const auto es    = EclipseState{ deck };
    const auto& grid = es.getInputGrid();
    const auto sched = Schedule{ deck, es };

    const auto& w0 = sched.getWell("W1", 0);
    const auto& w1 = sched.getWell("W1", 1);
    const auto& w2 = sched.getWell("W1", 2);
    const auto& w3 = sched.getWell("W1", 3);
    const auto& w4 = sched.getWell("W1", 4);
    const auto& w5 = sched.getWell("W1", 5);


    BOOST_CHECK_EQUAL(w0.getRefDepth(), grid.getCellDepth(0,0,2));
    BOOST_CHECK_EQUAL(w0.getRefDepth(), w0.getWPaveRefDepth());
    BOOST_CHECK_EQUAL(w1.getRefDepth(), w0.getRefDepth());
    BOOST_CHECK_EQUAL(w2.getRefDepth(), 2005 );
    BOOST_CHECK_EQUAL(w3.getRefDepth(), grid.getCellDepth(0,0,1));
    BOOST_CHECK_EQUAL(w4.getRefDepth(), w3.getRefDepth());
    BOOST_CHECK_EQUAL(w5.getRefDepth(), grid.getCellDepth(0,0,0));
    BOOST_CHECK_EQUAL(w5.getWPaveRefDepth(), 0);
}

BOOST_AUTO_TEST_CASE(Missing_RefDepth) {
      const auto deck = Parser{}.parseString(R"(RUNSPEC
START
17 DEC 2021 /

DIMENS
  10 10 4 /
GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  4*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  400*100.0 /
PERMY
  400*100.0 /
PERMZ
  400*10.0 /
PORO
  400*0.3 /

-- Deactivate Cells (1,1,3) And (1,1,4)
ACTNUM
  1 99*1
  1 99*1
  0 99*1
  0 99*1
/

SCHEDULE

WELSPECS
     'W1'   'G' 1  1  1*       'OIL'  2*      'STOP'  4* /
/

COMPDAT
     'W1'   1 1 4 4      'OPEN'  1*     34.720      0.216   3095.832  2*         'Y'     12.828 /
     'W1'   1 1 3 3      'OPEN'  1*     34.720      0.216   3095.832  2*         'Y'     12.828 /
/

TSTEP
  1 /

COMPDAT
     'W1'   1 1 2 2      'OPEN'  1*     25.620      0.216   2086.842  2*         'Y'      8.486 /
/

TSTEP
  1 /

END
)");

    const auto es    = EclipseState{ deck };
    const auto sched = Schedule{ deck, es };

    const auto& w0 = sched[0].wells("W1");
    BOOST_CHECK_MESSAGE(! w0.hasRefDepth(),
                        R"(Well "W1" must NOT have a BHP reference depth at report=1)");
    BOOST_CHECK_THROW(w0.getRefDepth(), std::logic_error);

    const auto& w1 = sched[1].wells("W1");
    BOOST_CHECK_MESSAGE(w1.hasRefDepth(),
                        R"(Well "W1" must have a BHP reference depth at report=2)");
    BOOST_CHECK_CLOSE(w1.getRefDepth(), es.getInputGrid().getCellDepth(0, 0, 1), 1.0e-8);
}
