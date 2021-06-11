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

#define BOOST_TEST_MODULE WellTest
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleTypes.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellProductionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellInjectionProperties.hpp>

using namespace Opm;


namespace Opm {
inline std::ostream& operator<<( std::ostream& stream, const Connection& c ) {
    return stream << "(" << c.getI() << "," << c.getJ() << "," << c.getK() << ")";
}
inline std::ostream& operator<<( std::ostream& stream, const Well& well ) {
    return stream << "(" << well.name() << ")";
}
}



BOOST_AUTO_TEST_CASE(WellCOMPDATtestTRACK) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
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
        BOOST_CHECK_EQUAL(completions.get( k ).getK(), k);
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
        BOOST_CHECK_EQUAL(completions.get( k ).getK(), k);
    }
}

BOOST_AUTO_TEST_CASE(WellCOMPDATtestINPUT) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
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
        injectionProps1->surfaceInjectionRate = 100;
        well.updateInjection(injectionProps1);
        BOOST_CHECK_EQUAL( true  , well.isInjector());
        BOOST_CHECK_EQUAL( false , well.isProducer());
        BOOST_CHECK_EQUAL( 100 , well.getInjectionProperties().surfaceInjectionRate.get<double>());
    }


    {
        Opm::Well well("WELL1" , "GROUP", 0, 1, 0, 0, 0.0, Opm::WellType(Opm::Phase::OIL), Opm::Well::ProducerCMode::CMODE_UNDEFINED, Connection::Order::DEPTH, UnitSystem::newMETRIC(), 0, 1.0, false, false, 0, Opm::Well::GasInflowEquation::STD);

        /* Set a reservoir injection rate => Well becomes an Injector */
        auto injectionProps2 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
        injectionProps2->reservoirInjectionRate = 200;
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
        properties->OilRate = 100;
        properties->GasRate = 200;
        properties->WaterRate = 300;
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
    productionProps->BHPTarget = 100;
    productionProps->addProductionControl(Opm::Well::ProducerCMode::BHP);
    well.updateProduction(productionProps);
    BOOST_CHECK_EQUAL( 100 , well.getProductionProperties().BHPTarget.get<double>());
    BOOST_CHECK_EQUAL( true, well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::BHP ));

    auto injProps = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps->THPTarget = 200;
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
    properties1->OilRate = 100;
    properties1->addProductionControl(Opm::Well::ProducerCMode::ORAT);
    well.updateProduction(properties1);
    BOOST_CHECK(  well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::ORAT ));
    BOOST_CHECK( !well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::RESV ));

    auto properties2 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties2->ResVRate = 100;
    properties2->addProductionControl(Opm::Well::ProducerCMode::RESV);
    well.updateProduction(properties2);
    BOOST_CHECK( well.getProductionProperties().hasProductionControl( Opm::Well::ProducerCMode::RESV ));

    auto properties3 = std::make_shared<Opm::Well::WellProductionProperties>(well.getProductionProperties());
    properties3->OilRate = 100;
    properties3->WaterRate = 100;
    properties3->GasRate = 100;
    properties3->LiquidRate = 100;
    properties3->ResVRate = 100;
    properties3->BHPTarget = 100;
    properties3->THPTarget = 100;
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
    injProps1->surfaceInjectionRate = 100;
    injProps1->addInjectionControl(Opm::Well::InjectorCMode::RATE);
    well.updateInjection(injProps1);
    BOOST_CHECK(  well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RATE ));
    BOOST_CHECK( !well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));

    auto injProps2 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps2->reservoirInjectionRate = 100;
    injProps2->addInjectionControl(Opm::Well::InjectorCMode::RESV);
    well.updateInjection(injProps2);
    BOOST_CHECK( well.getInjectionProperties().hasInjectionControl( Opm::Well::InjectorCMode::RESV ));

    auto injProps3 = std::make_shared<Opm::Well::WellInjectionProperties>(well.getInjectionProperties());
    injProps3->BHPTarget = 100;
    injProps3->addInjectionControl(Opm::Well::InjectorCMode::BHP);
    injProps3->THPTarget = 100;
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


        Opm::Well::WellProductionProperties properties(const std::string& input) {
            Opm::Parser parser;
            Opm::UnitSystem unit_system(Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC);
            auto deck = parser.parseString(input);
            const auto& record = deck.getKeyword("WCONHIST").getRecord(0);
            Opm::Well::WellProductionProperties hist(unit_system, "W");
            hist.handleWCONHIST(record);


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
        Opm::Well::WellProductionProperties properties(const std::string& input)
        {
            Opm::Parser parser;
            auto deck = parser.parseString(input);
            const auto& kwd     = deck.getKeyword("WCONPROD");
            const auto&  record = kwd.getRecord(0);
            Opm::Well::WellProductionProperties pred(unit_system, "W");
            pred.handleWCONPROD("WELL", record);

            return pred;
        }
    }

} // namespace anonymous

BOOST_AUTO_TEST_CASE(WCH_All_Specified_BHP_Defaulted)
{
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    Opm::SummaryState st(std::chrono::system_clock::now());
    const Opm::Well::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted_with_bhp_vfp_table());

    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::Well::ProducerCMode::RESV));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::RESV);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));
    BOOST_CHECK_EQUAL(p.VFPTableNumber, 3);
    BOOST_CHECK_EQUAL(p.ALQValue, 10.);
    const auto& controls = p.controls(st, 0);
    BOOST_CHECK_EQUAL(controls.bhp_limit, 101325.);
}

BOOST_AUTO_TEST_CASE(WCH_BHP_Specified)
{
    Opm::SummaryState st(std::chrono::system_clock::now());
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
    BOOST_CHECK_EQUAL(p.ALQValue, 0.);
}

BOOST_AUTO_TEST_CASE(WCONPROD_THP_CMode)
{
    const Opm::Well::WellProductionProperties& p =
        WCONPROD::properties(WCONPROD::thp_CMODE());

    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::THP));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::THP);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    BOOST_CHECK_EQUAL(p.VFPTableNumber, 8);
    BOOST_CHECK_EQUAL(p.ALQValue, 13.);
    BOOST_CHECK_EQUAL(p.THPTarget.getSI(), 1000000.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.getSI(), 101325.); // 1 atm.
}

BOOST_AUTO_TEST_CASE(WCONPROD_BHP_CMode)
{
    const Opm::Well::WellProductionProperties& p =
        WCONPROD::properties(WCONPROD::bhp_CMODE());

    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::ORAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::WRAT));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::GRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::LRAT));
    BOOST_CHECK( !p.hasProductionControl(Opm::Well::ProducerCMode::RESV));
    BOOST_CHECK( p.hasProductionControl(Opm::Well::ProducerCMode::THP));

    BOOST_CHECK(p.controlMode == Opm::Well::ProducerCMode::BHP);

    BOOST_CHECK_EQUAL(true, p.hasProductionControl(Opm::Well::ProducerCMode::BHP));

    BOOST_CHECK_EQUAL(p.VFPTableNumber, 8);
    BOOST_CHECK_EQUAL(p.ALQValue, 13.);
    BOOST_CHECK_EQUAL(p.THPTarget.get<double>(), 10.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.get<double>(), 20.); // 20 barsa
    BOOST_CHECK_EQUAL(p.THPTarget.getSI(), 1000000.); // 10 barsa
    BOOST_CHECK_EQUAL(p.BHPTarget.getSI(), 2000000.); // 20 barsa
}


BOOST_AUTO_TEST_CASE(BHP_CMODE)
{
    BOOST_CHECK_THROW( WCONHIST::properties(WCONHIST::all_specified_CMODE_THP()) , std::invalid_argument);
    BOOST_CHECK_THROW( WCONPROD::properties(WCONPROD::all_specified_CMODE_BHP()) , std::invalid_argument);
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
    Opm::SummaryState st(std::chrono::system_clock::now());
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

    BOOST_CHECK_THROW(inj.alq_value(), std::runtime_error);
    BOOST_CHECK_THROW(prod.temperature(), std::runtime_error);
    BOOST_CHECK_EQUAL(inj.vfp_table_number(), 100);
    BOOST_CHECK_EQUAL(prod.vfp_table_number(), 200);
}

BOOST_AUTO_TEST_CASE(WELOPEN) {
    Opm::Parser parser;
    std::string input =
                "START             -- 0 \n"
                "19 JUN 2007 / \n"
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
    BOOST_CHECK(wtp.preferred_phase() == Phase::WATER);
    BOOST_CHECK(wtp.injector_type() == InjectorType::GAS);
}

BOOST_AUTO_TEST_CASE(Injector_Control_Mode) {
    using IMode = ::Opm::Well::InjectorCMode;
    using IType = ::Opm::InjectorType;
    using WStat = ::Opm::Well::Status;

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::GAS, WStat::OPEN), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::GAS, WStat::SHUT),  0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::GAS, WStat::STOP), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::GAS, WStat::AUTO), -1);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::WATER, WStat::OPEN), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::WATER, WStat::SHUT),  0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::WATER, WStat::STOP), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::WATER, WStat::AUTO), -1);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::MULTI, WStat::OPEN), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::MULTI, WStat::SHUT),  0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::MULTI, WStat::STOP), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::MULTI, WStat::AUTO), -1);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::OIL, WStat::OPEN), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::OIL, WStat::SHUT),  0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::OIL, WStat::STOP), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::GRUP, IType::OIL, WStat::AUTO), -1);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::OIL, WStat::OPEN), 1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::OIL, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::OIL, WStat::STOP), 1);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::OIL, WStat::AUTO), 1);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::WATER, WStat::OPEN), 2);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::WATER, WStat::STOP), 2);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::WATER, WStat::AUTO), 2);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::GAS, WStat::OPEN), 3);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::GAS, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::GAS, WStat::STOP), 3);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::GAS, WStat::AUTO), 3);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::MULTI, WStat::OPEN), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::MULTI, WStat::SHUT),   0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::MULTI, WStat::STOP), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RATE, IType::MULTI, WStat::AUTO), -10);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::GAS, WStat::OPEN), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::GAS, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::GAS, WStat::STOP), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::GAS, WStat::AUTO), 5);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::WATER, WStat::OPEN), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::WATER, WStat::STOP), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::WATER, WStat::AUTO), 5);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::MULTI, WStat::OPEN), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::MULTI, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::MULTI, WStat::STOP), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::MULTI, WStat::AUTO), 5);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::OIL, WStat::OPEN), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::OIL, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::OIL, WStat::STOP), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::RESV, IType::OIL, WStat::AUTO), 5);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::GAS, WStat::OPEN), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::GAS, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::GAS, WStat::STOP), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::GAS, WStat::AUTO), 6);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::WATER, WStat::OPEN), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::WATER, WStat::STOP), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::WATER, WStat::AUTO), 6);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::MULTI, WStat::OPEN), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::MULTI, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::MULTI, WStat::STOP), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::MULTI, WStat::AUTO), 6);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::OIL, WStat::OPEN), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::OIL, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::OIL, WStat::STOP), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::THP, IType::OIL, WStat::AUTO), 6);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::GAS, WStat::OPEN), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::GAS, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::GAS, WStat::STOP), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::GAS, WStat::AUTO), 7);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::WATER, WStat::OPEN), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::WATER, WStat::STOP), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::WATER, WStat::AUTO), 7);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::MULTI, WStat::OPEN), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::MULTI, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::MULTI, WStat::STOP), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::MULTI, WStat::AUTO), 7);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::OIL, WStat::OPEN), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::OIL, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::OIL, WStat::STOP), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::BHP, IType::OIL, WStat::AUTO), 7);

    BOOST_CHECK_EQUAL(eclipseControlMode(IMode::CMODE_UNDEFINED, IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<IMode>(1729), IType::WATER, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<IMode>(1729), IType::WATER, WStat::STOP), -10); // Unknown combination
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<IMode>(1729), IType::WATER, WStat::OPEN), -10); // Unknown combination
}

BOOST_AUTO_TEST_CASE(Producer_Control_Mode) {
    using PMode = ::Opm::Well::ProducerCMode;
    using WStat = ::Opm::Well::Status;

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRUP, WStat::OPEN), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRUP, WStat::STOP), -1);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRUP, WStat::SHUT),  0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRUP, WStat::AUTO), -1);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::ORAT, WStat::OPEN), 1);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::ORAT, WStat::STOP), 1);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::ORAT, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::ORAT, WStat::AUTO), 1);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::WRAT, WStat::OPEN), 2);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::WRAT, WStat::STOP), 2);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::WRAT, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::WRAT, WStat::AUTO), 2);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRAT, WStat::OPEN), 3);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRAT, WStat::STOP), 3);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRAT, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::GRAT, WStat::AUTO), 3);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::LRAT, WStat::OPEN), 4);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::LRAT, WStat::STOP), 4);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::LRAT, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::LRAT, WStat::AUTO), 4);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::RESV, WStat::OPEN), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::RESV, WStat::STOP), 5);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::RESV, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::RESV, WStat::AUTO), 5);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::THP, WStat::OPEN), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::THP, WStat::STOP), 6);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::THP, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::THP, WStat::AUTO), 6);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::BHP, WStat::OPEN), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::BHP, WStat::STOP), 7);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::BHP, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::BHP, WStat::AUTO), 7);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CRAT, WStat::OPEN), 9);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CRAT, WStat::STOP), 9);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CRAT, WStat::SHUT), 0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CRAT, WStat::AUTO), 9);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::NONE, WStat::OPEN), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::NONE, WStat::STOP), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::NONE, WStat::SHUT),   0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::NONE, WStat::AUTO), -10);

    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CMODE_UNDEFINED, WStat::OPEN), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CMODE_UNDEFINED, WStat::STOP), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CMODE_UNDEFINED, WStat::SHUT),   0);
    BOOST_CHECK_EQUAL(eclipseControlMode(PMode::CMODE_UNDEFINED, WStat::AUTO), -10);

    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<PMode>(271828), WStat::OPEN), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<PMode>(271828), WStat::STOP), -10);
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<PMode>(271828), WStat::SHUT),   0);
    BOOST_CHECK_EQUAL(eclipseControlMode(static_cast<PMode>(271828), WStat::AUTO), -10);
}


BOOST_AUTO_TEST_CASE(WPIMULT) {
    Opm::Parser parser;
    std::string input = R"(
START             -- 0
19 JUN 2007 /

REGIONS

PVTNUM
 1000*77 /

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


