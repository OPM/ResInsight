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

#define BOOST_TEST_MODULE GroupTests
#include <boost/test/unit_test.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRateModel.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRate.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Group/GConSump.hpp>
#include <opm/input/eclipse/Schedule/Group/GConSale.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/common/utility/TimeService.hpp>

using namespace Opm;


Opm::Schedule create_schedule(const std::string& deck_string) {
    Opm::Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseString(deck_string);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck );
    return Opm::Schedule(deck,  grid, fp, runspec, python);
}


BOOST_AUTO_TEST_CASE(CreateGroup_CorrectNameAndDefaultValues) {
    Opm::Group group("G1" , 1, 0, UnitSystem::newMETRIC());
    BOOST_CHECK_EQUAL( "G1" , group.name() );
}



BOOST_AUTO_TEST_CASE(CreateGroup_SetInjectorProducer_CorrectStatusSet) {
    Opm::Group group1("IGROUP" , 1,  0, UnitSystem::newMETRIC());
    Opm::Group group2("PGROUP" , 2,  0, UnitSystem::newMETRIC());

    group1.setProductionGroup();
    BOOST_CHECK(group1.isProductionGroup());
    BOOST_CHECK(!group1.isInjectionGroup());

    group2.setInjectionGroup();
    BOOST_CHECK(!group2.isProductionGroup());
    BOOST_CHECK(group2.isInjectionGroup());
}








BOOST_AUTO_TEST_CASE(GroupDoesNotHaveWell) {
    Opm::Group group("G1" , 1, 0, UnitSystem::newMETRIC());

    BOOST_CHECK_EQUAL(false , group.hasWell("NO"));
    BOOST_CHECK_EQUAL(0U , group.numWells());
}




BOOST_AUTO_TEST_CASE(createDeckWithGEFAC) {
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

	    "WELSPECS\n"
     	     " 'B-37T2' 'PRODUC'  9  9   1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
	     " 'B-43A'  'PRODUC'  8  8   1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
	     "/\n"

	     "COMPDAT\n"
	     " 'B-37T2'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
             " 'B-43A'   8  8   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
	     "/\n"

            "GEFAC\n"
            " 'PRODUC' 0.85   / \n"
            "/\n";

    auto schedule = create_schedule(input);

    auto group_names = schedule.groupNames("PRODUC");
    BOOST_CHECK_EQUAL(group_names.size(), 1U);
    BOOST_CHECK_EQUAL(group_names[0], "PRODUC");

    const auto& group1 = schedule.getGroup("PRODUC", 0);
    BOOST_CHECK_EQUAL(group1.getGroupEfficiencyFactor(), 0.85);
    BOOST_CHECK(group1.getTransferGroupEfficiencyFactor());
}



BOOST_AUTO_TEST_CASE(createDeckWithWGRUPCONandWCONPROD) {

    /* Test deck with well guide rates for group control:
       GRUPCON (well guide rates for group control)
       WCONPROD (conrol data for production wells) with GRUP control mode */
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

	    "WELSPECS\n"
     	     " 'B-37T2' 'PRODUC'  9  9   1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
	     " 'B-43A'  'PRODUC'  8  8   1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
	     "/\n"

	     "COMPDAT\n"
	     " 'B-37T2'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
             " 'B-43A'   8  8   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
	     "/\n"


             "WGRUPCON\n"
             " 'B-37T2'  YES 30 OIL / \n"
             " 'B-43A'   YES 30 OIL / \n"
             "/\n"

             "WCONPROD\n"
             " 'B-37T2'    'OPEN'     'GRUP'  1000  2*   2000.000  2* 1*   0 200000.000  5* /  / \n"
             " 'B-43A'     'OPEN'     'GRUP'  1200  2*   3000.000  2* 1*   0  0.000      5* /  / \n"
             "/\n";



    auto schedule = create_schedule(input);
    const auto& currentWell = schedule.getWell("B-37T2", 0);
    const Opm::Well::WellProductionProperties& wellProductionProperties = currentWell.getProductionProperties();
    BOOST_CHECK(wellProductionProperties.controlMode == Opm::Well::ProducerCMode::GRUP);

    BOOST_CHECK_EQUAL(currentWell.isAvailableForGroupControl(), true);
    BOOST_CHECK_EQUAL(currentWell.getGuideRate(), 30);
    BOOST_CHECK(currentWell.getGuideRatePhase() == Opm::Well::GuideRateTarget::OIL);
    BOOST_CHECK_EQUAL(currentWell.getGuideRateScalingFactor(), 1.0);
}





BOOST_AUTO_TEST_CASE(createDeckWithGRUPNET) {
    std::string input =
        "START             -- 0 \n"
        "31 AUG 1993 / \n"
        "SCHEDULE\n"

        "GRUPNET \n"
        " 'FIELD'     20.000  5* / \n"
        " 'PROD'     20.000  5* / \n"
        " 'MANI-B2'  1*    8  1*        'NO'  2* / \n"
        " 'MANI-B1'  1*    8  1*        'NO'  2* / \n"
        " 'MANI-K1'  1* 9999  4* / \n"
        " 'B1-DUMMY'  1* 9999  4* / \n"
        " 'MANI-D1'  1*    8  1*        'NO'  2* / \n"
        " 'MANI-D2'  1*    8  1*        'NO'  2* / \n"
        " 'MANI-K2'  1* 9999  4* / \n"
        " 'D2-DUMMY'  1* 9999  4* / \n"
        " 'MANI-E1'  1*    9  1*        'NO'  2* / \n"
        " 'MANI-E2'  1*    9  4* / \n"
        "/\n";


    auto schedule = create_schedule(input);

    const auto& group1 = schedule.getGroup("PROD", 0);
    const auto& group2 = schedule.getGroup("MANI-E2", 0);
    const auto& group3 = schedule.getGroup("MANI-K1", 0);
    BOOST_CHECK_EQUAL(group1.getGroupNetVFPTable(), 0);
    BOOST_CHECK_EQUAL(group2.getGroupNetVFPTable(), 9);
    BOOST_CHECK_EQUAL(group3.getGroupNetVFPTable(), 9999);
}


BOOST_AUTO_TEST_CASE(GroupCreate) {
    Opm::Group g1("NAME", 1, 0, UnitSystem::newMETRIC());
    Opm::Group g2("NAME", 1, 0, UnitSystem::newMETRIC());

    BOOST_CHECK( g1.addWell("W1") );
    BOOST_CHECK( !g1.addWell("W1") );
    BOOST_CHECK( g1.addWell("W2") );
    BOOST_CHECK( g1.hasWell("W1"));
    BOOST_CHECK( g1.hasWell("W2"));
    BOOST_CHECK( !g1.hasWell("W3"));
    BOOST_CHECK_EQUAL( g1.numWells(), 2U);
    BOOST_CHECK_THROW(g1.delWell("W3"), std::invalid_argument);
    BOOST_CHECK_NO_THROW(g1.delWell("W1"));
    BOOST_CHECK_EQUAL( g1.numWells(), 1U);


    BOOST_CHECK( g2.addGroup("G1") );
    BOOST_CHECK( !g2.addGroup("G1") );
    BOOST_CHECK( g2.addGroup("G2") );

    // The children must be either all wells - or all groups.
    BOOST_CHECK_THROW(g1.addGroup("G1"), std::logic_error);
    BOOST_CHECK_THROW(g2.addWell("W1"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(createDeckWithGCONPROD) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONPROD
            'G1' 'ORAT' 10000 3* 'CON' /
            'G2' 'RESV' 10000 3* 'CON' /
        /)";

    auto schedule = create_schedule(input);
    SummaryState st(TimeService::now());

    const auto& group1 = schedule.getGroup("G1", 0);
    const auto& group2 = schedule.getGroup("G2", 0);

    auto ctrl1 = group1.productionControls(st);
    auto ctrl2 = group2.productionControls(st);

    BOOST_CHECK(ctrl1.exceed_action == Group::ExceedAction::RATE);
    BOOST_CHECK(ctrl2.exceed_action == Group::ExceedAction::CON);
}


BOOST_AUTO_TEST_CASE(TESTGuideRateModel) {
    Opm::GuideRateModel grc_default;
    BOOST_CHECK_THROW(Opm::GuideRateModel(0.0,GuideRateModel::Target::OIL, -5,0,0,0,0,0,true,1,true), std::invalid_argument);
    BOOST_CHECK_THROW(grc_default.eval(1,0.50,0.50), std::invalid_argument);

    Opm::GuideRateModel grc_delay(10, GuideRateModel::Target::OIL, 1,1,0,0,0,0,true,1,true);
    BOOST_CHECK_NO_THROW(grc_delay.eval(1.0, 0.5, 0.5));
}

BOOST_AUTO_TEST_CASE(TESTGuideRateLINCOM) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONPROD
            'G1' 'ORAT' 10000 3* 'CON' /
            'G2' 'RESV' 10000 3* 'CON' /
        /

        GUIDERAT
             1*  'COMB'  1.0 1.0 /

        LINCOM
             1  2  'WWCT:OPX' /

        )";


    /* The 'COMB' target mode is not supported */
    BOOST_CHECK_THROW(create_schedule(input), std::exception);
}

BOOST_AUTO_TEST_CASE(TESTGuideRate) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONPROD
            'G1' 'ORAT' 10000 3* 'CON' /
            'G2' 'RESV' 10000 3* 'CON' /
        /

        GUIDERAT
             1*  'OIL'  1.0 1.0 /

        LINCOM
             1  2  'WWCT:OPX' /

        TSTEP
           1 1 1 1 1 1 1 1 1 1 1 /
        )";

    auto schedule = create_schedule(input);
    GuideRate gr(schedule);
}

BOOST_AUTO_TEST_CASE(TESTGCONSALE) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONSALE
        'G1' 50000 55000 45000 WELL /
        /

        GCONSUMP
        'G1' 20 50 'a_node' /
        'G2' 30 60 /
        /

        )";

    auto schedule = create_schedule(input);
    double metric_to_si = 1.0 / (24.0 * 3600.0);  //cubic meters / day

    const auto& gconsale = schedule[0].gconsale.get();
    BOOST_CHECK_EQUAL(gconsale.size(), 1U);
    BOOST_CHECK(gconsale.has("G1"));
    BOOST_CHECK(!gconsale.has("G2"));
    const GConSale::GCONSALEGroup& group = gconsale.get("G1");
    BOOST_CHECK_EQUAL(group.sales_target.get<double>(),   50000);
    BOOST_CHECK_EQUAL(group.max_sales_rate.get<double>(), 55000);
    BOOST_CHECK_EQUAL(group.min_sales_rate.get<double>(), 45000);
    BOOST_CHECK_EQUAL(group.sales_target.getSI(),   50000 * metric_to_si);
    BOOST_CHECK_EQUAL(group.max_sales_rate.getSI(), 55000 * metric_to_si);
    BOOST_CHECK_EQUAL(group.min_sales_rate.getSI(), 45000 * metric_to_si);
    BOOST_CHECK(group.max_proc == GConSale::MaxProcedure::WELL);

    const auto& gconsump = schedule[0].gconsump.get();
    BOOST_CHECK_EQUAL(gconsump.size(), 2U);
    BOOST_CHECK(gconsump.has("G1"));
    BOOST_CHECK(gconsump.has("G2"));
    const GConSump::GCONSUMPGroup group1 = gconsump.get("G1");
    BOOST_CHECK_EQUAL(group1.consumption_rate.get<double>(), 20);
    BOOST_CHECK_EQUAL(group1.import_rate.get<double>(), 50);
    BOOST_CHECK_EQUAL(group1.consumption_rate.getSI(), 20 * metric_to_si);
    BOOST_CHECK_EQUAL(group1.import_rate.getSI(), 50 * metric_to_si);
    BOOST_CHECK( group1.network_node == "a_node" );

    const GConSump::GCONSUMPGroup group2 = gconsump.get("G2");
    BOOST_CHECK_EQUAL( group2.network_node.size(), 0U );



}

BOOST_AUTO_TEST_CASE(GCONINJE_MULTIPLE_PHASES) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONINJE
           'G1'   'WATER'   1*  1000      /
           'G1'   'GAS'     1*  1*   2000 /
           'G2'   'WATER'   1*  1000      /
        /

        TSTEP
           10 /

        GCONINJE
           'G2'   'WATER'   1*  1000  /
           'G2'   'GAS'     1*  1*   2000  2*   'NO' /
           'G1'   'GAS'     1*  1000      /
        /

        )";

    auto schedule = create_schedule(input);
    SummaryState st(TimeService::now());
    // Step 0
    {
        const auto& g1 = schedule.getGroup("G1", 0);
        BOOST_CHECK(  g1.hasInjectionControl(Phase::WATER));
        BOOST_CHECK(  g1.hasInjectionControl(Phase::GAS));
        BOOST_CHECK( !g1.hasInjectionControl(Phase::OIL));

        BOOST_CHECK(  g1.injectionGroupControlAvailable(Phase::WATER) );
        BOOST_CHECK(  g1.injectionGroupControlAvailable(Phase::GAS) );
        BOOST_CHECK(  g1.productionGroupControlAvailable() );

        g1.injectionControls(Phase::WATER, st);
        g1.injectionControls(Phase::GAS, st);
        BOOST_CHECK_THROW(g1.injectionControls(Phase::OIL, st), std::out_of_range);

        BOOST_CHECK(Phase::GAS == g1.topup_phase().value());
    }
    {
        const auto& g2 = schedule.getGroup("G2", 0);
        BOOST_CHECK( !g2.topup_phase().has_value());
        BOOST_CHECK(  g2.injectionGroupControlAvailable(Phase::WATER) );
    }
    // Step 1
    {
        const auto& g2 = schedule.getGroup("G2", 1);
        BOOST_CHECK(  g2.hasInjectionControl(Phase::WATER));
        BOOST_CHECK(  g2.hasInjectionControl(Phase::GAS));
        BOOST_CHECK( !g2.hasInjectionControl(Phase::OIL));
        BOOST_CHECK( !g2.injectionGroupControlAvailable(Phase::GAS) );

        g2.injectionControls(Phase::WATER, st);
        g2.injectionControls(Phase::GAS, st);
        BOOST_CHECK_THROW(g2.injectionControls(Phase::OIL, st), std::out_of_range);

        BOOST_CHECK(g2.topup_phase().has_value());
        BOOST_CHECK(Phase::GAS == g2.topup_phase().value());
    }
    {
        const auto& g1 = schedule.getGroup("G1", 1);
        BOOST_CHECK(!g1.topup_phase().has_value());
    }
}

BOOST_AUTO_TEST_CASE(GCONINJE_GUIDERATE) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONINJE
           'G1'   'WATER'   1*  1000 /
           'G1'   'GAS'     1*  1000 /
           'G2'   'WATER'   1*  1000 /
        /

        TSTEP
           10 /

        GCONINJE
           'G1'   'WATER'   1*  1000 3* 'YES' 1 'RATE'/
           'G1'   'GAS'     1*  1000 3* 'YES' 1 'RATE'/
           'G2'   'WATER'   1*  1000 3* 'YES' 1 'RATE'/
        /

        TSTEP
            10 /

        GCONINJE
            'G1'   'WATER'   1*  1000 /
            'G1'   'GAS'     1*  1000 3* 'YES' 1 'RATE'/
            'G2'   'WATER'   1*  1000 3* 'YES' 1 'RATE'/
        /

        )";

    auto schedule = create_schedule(input);
    // Step 0
    {
        GuideRate gr = GuideRate(schedule);
        const auto& g1 = schedule.getGroup("G1", 0);
        const auto& g2 = schedule.getGroup("G2", 0);
        gr.compute(g1.name(), Phase::WATER, 0, 0.0);
        gr.compute(g1.name(), Phase::GAS, 0, 0.0);
        gr.compute(g2.name(), Phase::WATER, 0, 0.0);
        gr.compute(g2.name(), Phase::GAS, 0, 0.0);
        BOOST_CHECK( !gr.has(g1.name(), Phase::WATER));
        BOOST_CHECK( !gr.has(g1.name(), Phase::GAS));
        BOOST_CHECK( !gr.has(g2.name(), Phase::WATER));
        BOOST_CHECK( !gr.has(g2.name(), Phase::GAS));
    }
    // Step 1
    {
        GuideRate gr = GuideRate(schedule);
        const auto& g1 = schedule.getGroup("G1", 1);
        const auto& g2 = schedule.getGroup("G2", 1);
        gr.compute(g1.name(), Phase::WATER, 1, 0.0);
        gr.compute(g1.name(), Phase::GAS, 1, 0.0);
        gr.compute(g2.name(), Phase::WATER, 1, 0.0);
        gr.compute(g2.name(), Phase::GAS, 1, 0.0);

        BOOST_CHECK( gr.has(g1.name(), Phase::WATER));
        BOOST_CHECK( gr.has(g1.name(), Phase::GAS));
        BOOST_CHECK( gr.has(g2.name(), Phase::WATER));
        BOOST_CHECK( !gr.has(g2.name(), Phase::GAS));

        BOOST_CHECK_EQUAL(1.0, gr.get(g1.name(), Phase::WATER));
        BOOST_CHECK_EQUAL(1.0, gr.get(g1.name(), Phase::GAS));
        BOOST_CHECK_EQUAL(1.0, gr.get(g2.name(), Phase::WATER));
        BOOST_CHECK_THROW(gr.get(g2.name(), Phase::GAS), std::logic_error);
    }
    // Step 2
    {
        GuideRate gr = GuideRate(schedule);
        const auto& g1 = schedule.getGroup("G1", 2);
        const auto& g2 = schedule.getGroup("G2", 2);
        gr.compute(g1.name(), Phase::WATER, 2, 0.0);
        gr.compute(g1.name(), Phase::GAS, 2, 0.0);
        gr.compute(g2.name(), Phase::WATER, 2, 0.0);
        gr.compute(g2.name(), Phase::GAS, 2, 0.0);
        BOOST_CHECK( !gr.has(g1.name(), Phase::WATER));
        BOOST_CHECK( gr.has(g1.name(), Phase::GAS));
        BOOST_CHECK( gr.has(g2.name(), Phase::WATER));
        BOOST_CHECK( !gr.has(g2.name(), Phase::GAS));
    }

}

BOOST_AUTO_TEST_CASE(GCONINJE_GCONPROD) {
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        SCHEDULE

        GRUPTREE
           'G1'  'FIELD' /
           'G2'  'FIELD' /
        /

        GCONPROD
            'G1' 'ORAT' 10000 3* 'CON' 'NO'/
            'G2' 'ORAT' 10000 3* 'CON' /
        /

        GCONINJE
           'G1'   'WATER'     1*  1000      /
           'G2'   'WATER'     1*  1*   2000 1*  1*  'NO'/
        /


        TSTEP
           1 /

        GCONPROD
            'G1' 'ORAT' 10000 3* 'CON' /
            'G2' 'ORAT' 10000 3* 'CON' 'NO'/
        /

        GCONINJE
           'G1'   'WATER'     1*  1000 3* 'NO'     /
           'G2'   'WATER'     1*  1*   2000 /
        /

        )";

    auto schedule = create_schedule(input);
    {
        const auto& f  = schedule.getGroup("FIELD", 0);
        const auto& g1 = schedule.getGroup("G1", 0);
        const auto& g2 = schedule.getGroup("G2", 0);

        BOOST_CHECK(!f.productionGroupControlAvailable() );
        BOOST_CHECK(!f.injectionGroupControlAvailable(Phase::WATER));
        BOOST_CHECK(!f.injectionGroupControlAvailable(Phase::GAS));

        BOOST_CHECK(!g1.productionGroupControlAvailable() );
        BOOST_CHECK( g2.productionGroupControlAvailable() );
        BOOST_CHECK( g1.injectionGroupControlAvailable(Phase::WATER));
        BOOST_CHECK(!g2.injectionGroupControlAvailable(Phase::WATER));
        BOOST_CHECK( g1.injectionGroupControlAvailable(Phase::GAS));
        BOOST_CHECK( g2.injectionGroupControlAvailable(Phase::GAS));

        BOOST_CHECK(f.is_field());
        BOOST_CHECK(!g1.is_field());
    }
    {
        const auto& g1 = schedule.getGroup("G1", 1);
        const auto& g2 = schedule.getGroup("G2", 1);

        BOOST_CHECK( g1.productionGroupControlAvailable() );
        BOOST_CHECK(!g2.productionGroupControlAvailable() );
        BOOST_CHECK(!g1.injectionGroupControlAvailable(Phase::WATER));
        BOOST_CHECK( g2.injectionGroupControlAvailable(Phase::WATER));
        BOOST_CHECK( g1.injectionGroupControlAvailable(Phase::GAS));
        BOOST_CHECK( g2.injectionGroupControlAvailable(Phase::GAS));
    }
}

BOOST_AUTO_TEST_CASE(GPMAINT) {
    const auto input = R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
 /

GPMAINT
  'PROD'  'WINJ'   2  1*  100  0.25  1.0 /
  'C1'    'GINJ'   0  1*  100  0.25  1.0 /
  'F1'    'PROD'  1 1 1 1 1 /
/

TSTEP
   10 /

TSTEP
  10 /

GPMAINT
  'PROD'  'WINJ'   2  1*  100  0.25  1.0 /
/

TSTEP
 10 /

GPMAINT
  'PROD'  'NONE' /
/

TSTEP
10 /

GCONPROD
   PROD        ORAT  0     0     1*    0     RATE  YES   1*    '   '     1*    1*    1*    1*    1*    /
   FIELD       ORAT  71500 1*    1*    1*    RATE  YES   1*    '   '     1*    1*    1*    1*    1*    /
/


)";
    Opm::UnitSystem unitSystem = UnitSystem( UnitSystem::UnitType::UNIT_TYPE_METRIC );
    const auto sched = create_schedule(input);
    GPMaint::State gpm_state;
    const auto T = 86400;
    const auto K = 0.25 / (86400 * 1e5);
    const double error = 100000;
    const double dt = 100000;
    const double current_rate = 65;
    {
        const auto& prod_group = sched.getGroup("PROD", 0);
        const auto& plat_group = sched.getGroup("PLAT-A", 0);
        const auto& c1_group = sched.getGroup("C1", 0);
        const auto& f1_group = sched.getGroup("F1", 0);

        const auto& gpm_prod = prod_group.gpmaint();
        BOOST_CHECK( gpm_prod );
        BOOST_CHECK(gpm_prod->flow_target() == GPMaint::FlowTarget::RESV_WINJ);
        {
            auto rate1 = gpm_prod->rate(gpm_state, current_rate, error, dt);
            BOOST_CHECK_EQUAL( rate1, current_rate + K * error );

            auto rate2 = gpm_prod->rate(gpm_state, current_rate, error, dt);
            BOOST_CHECK_EQUAL( rate2, (error + error*dt / T) * K + current_rate );

            auto rate3 = gpm_prod->rate(gpm_state, current_rate, error, dt);
            BOOST_CHECK_EQUAL( rate3, (error + 2*error*dt / T) * K + current_rate );
        }

        // This should be flagged as an injection group because the group is
        // under GPMAINT control with WINJ target.
        BOOST_CHECK( prod_group.isInjectionGroup() );
        BOOST_CHECK( f1_group.isProductionGroup() );
        BOOST_CHECK( prod_group.has_control(Phase::WATER, Group::InjectionCMode::RESV) );
        BOOST_CHECK( !prod_group.has_control(Phase::GAS, Group::InjectionCMode::RESV) );
        BOOST_CHECK( f1_group.has_control(Group::ProductionCMode::RESV) );

        auto [name, number] = *gpm_prod->region();
        BOOST_CHECK_EQUAL(number, 2);
        BOOST_CHECK_EQUAL(name, "FIPNUM");

        const auto& gpm_c1 = c1_group.gpmaint();
        BOOST_CHECK(!gpm_c1->region());

        const auto& plat_prod = plat_group.gpmaint();
        BOOST_CHECK( !plat_prod );
    }
    {
        const auto& prod_group = sched.getGroup("PROD", 1);
        const auto& gpm_prod = prod_group.gpmaint();

        auto rate4 = gpm_prod->rate(gpm_state, current_rate, error, dt);
        BOOST_CHECK_EQUAL( rate4, (error + 3*error*dt / T) * K + current_rate );
    }
    {
        const auto& prod_group = sched.getGroup("PROD", 2);
        const auto& gpm_prod = prod_group.gpmaint();

        auto rate1 = gpm_prod->rate(gpm_state, current_rate, error, dt);
        BOOST_CHECK_EQUAL( rate1, current_rate + K*error);
    }
    {
        const auto& prod_group = sched.getGroup("PROD", 4);
        const auto& gpm_prod = prod_group.gpmaint();
        BOOST_CHECK( !gpm_prod );
    }

    BOOST_CHECK(sched[0].has_gpmaint());
}

