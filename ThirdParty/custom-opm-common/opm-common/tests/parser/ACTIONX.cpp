/*
  Copyright 2018 Statoil ASA.

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
#include <algorithm>
#define BOOST_TEST_MODULE ACTIONX

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <opm/common/utility/TimeService.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Action/Actdims.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionAST.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/input/eclipse/Schedule/Action/Actions.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/Action/WGNames.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionResult.hpp>
#include <opm/input/eclipse/Schedule/Well/WList.hpp>
#include <opm/input/eclipse/Schedule/Well/WListManager.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

using namespace Opm;


Schedule make_schedule(const std::string& deck_string, const ParseContext& parseContext = {}) {
    ErrorGuard errors;
    Opm::Parser parser;
    auto deck = parser.parseString(deck_string);
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid1, table);
    auto python = std::make_shared<Python>();
    Runspec runspec (deck);
    return Schedule(deck, grid1, fp, runspec, parseContext, errors, python);
}


BOOST_AUTO_TEST_CASE(Create) {
    const auto action_kw = std::string{ R"(
ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/
)"};
    Action::ActionX action1("NAME", 10, 100, 0);
    BOOST_CHECK_EQUAL(action1.name(), "NAME");

    const auto deck = Parser{}.parseString( action_kw );
    const auto& kw = deck["ACTIONX"].back();

    Action::ActionX action2(kw, {}, 0);
    BOOST_CHECK_EQUAL(action2.name(), "ACTION");
}


BOOST_AUTO_TEST_CASE(SCAN) {
    const auto MISSING_END= std::string{ R"(
SCHEDULE

ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/

TSTEP
   10 /
)"};

    const auto WITH_WELSPECS = std::string{ R"(
SCHEDULE

WELSPECS
  'W0'  'OP'  1 1 3.33  'OIL' 7*/
  'W2'  'OP'  1 1 3.33  'OIL' 7*/
  'W3'  'OP'  1 1 3.33  'OIL' 7*/
/

ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/

WELSPECS
  'W1'  'OP'  1 1 3.33  'OIL' 7*/
/

WCONPROD
 'W0'      'OPEN'      'ORAT'      0.000      0.000      0.000  5* /
/

WCONINJE
 'W3' 'WATER'  'OPEN'  'RATE'  200  1*  450.0 /
/

ENDACTIO

TSTEP
   10 /
)"};


    const auto WITH_GRID = std::string{ R"(
SCHEDULE

WELSPECS
  'W2'  'OP'  1 1 3.33  'OIL' 7*/
/

ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/

PORO
  100*0.78 /

ENDACTIO

TSTEP
   10 /
)"};
    BOOST_CHECK_THROW(make_schedule(MISSING_END), OpmInputError);

    Schedule sched = make_schedule(WITH_WELSPECS);
    BOOST_CHECK( !sched.hasWell("W1") );
    BOOST_CHECK( sched.hasWell("W2"));

    Action::Result action_result(true);
    const auto& action1 = sched[0].actions.get()["ACTION"];
    auto sim_update = sched.applyAction(0, action1, action_result.wells(), {});
    const auto& affected_wells = sim_update.affected_wells;
    std::vector<std::string> expected_wells{"W0", "W1", "W3"};
    BOOST_CHECK( std::is_permutation(affected_wells.begin(), affected_wells.end(),
                                     expected_wells.begin(), expected_wells.end() ));

    {
        const auto& wg_events = sched[0].wellgroup_events();
        const auto& events = sched[0].events();
        BOOST_CHECK(events.hasEvent(ScheduleEvents::ACTIONX_WELL_EVENT));
        BOOST_CHECK(wg_events.hasEvent("W1", ScheduleEvents::ACTIONX_WELL_EVENT));
        BOOST_CHECK(!wg_events.hasEvent("W2", ScheduleEvents::ACTIONX_WELL_EVENT));
    }

    {
        const auto& wg_events = sched[1].wellgroup_events();
        const auto& events = sched[1].events();
        BOOST_CHECK(!events.hasEvent(ScheduleEvents::ACTIONX_WELL_EVENT));
        BOOST_CHECK(!wg_events.hasEvent("W1", ScheduleEvents::ACTIONX_WELL_EVENT));
        BOOST_CHECK(!wg_events.hasEvent("W2", ScheduleEvents::ACTIONX_WELL_EVENT));
    }

    // The deck3 contains the 'GRID' keyword in the ACTIONX block - that is not a whitelisted keyword.
    ParseContext parseContext( {{ParseContext::ACTIONX_ILLEGAL_KEYWORD, InputError::THROW_EXCEPTION}} );
    BOOST_CHECK_THROW( make_schedule(WITH_GRID, parseContext), OpmInputError );
}

BOOST_AUTO_TEST_CASE(COMPDAT) {

    const auto TRAILING_COMPDAT = std::string{ R"(
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
  'W2'  'OP'  1 1 3.33  'OIL' 7*/
/

ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/

ENDACTIO

TSTEP
   10 /

COMPDAT
 'W2'  1  1   1   1 'OPEN'  /
/

)"};

    Schedule sched = make_schedule(TRAILING_COMPDAT);
    Action::Result action_result(true);
    const auto& action1 = sched[0].actions.get()["ACTION"];
    BOOST_CHECK_NO_THROW( sched.applyAction(0, action1, {}, {}));
}

BOOST_AUTO_TEST_CASE(EMPTY) {

    const auto EMPTY_ACTION = std::string{ R"(
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

ACTIONX
   'ACTION' /
/

ENDACTIO
)"};

    Schedule sched = make_schedule(EMPTY_ACTION);
    Action::Result action_result(true);
    const auto& action1 = sched[0].actions.get()["ACTION"];
    Opm::SummaryState st(TimeService::now());
    Opm::WListManager wlm;
    Opm::Action::Context context(st, wlm);
    BOOST_CHECK( !action1.eval(context) );
}


BOOST_AUTO_TEST_CASE(TestActions) {
    Opm::SummaryState st(TimeService::now());
    Opm::WListManager wlm;
    Opm::Action::Context context(st, wlm);
    Opm::Action::Actions config;
    std::vector<std::string> matching_wells;
    auto python = std::make_shared<Opm::Python>();
    BOOST_CHECK_EQUAL(config.ecl_size(), 0U);
    BOOST_CHECK(config.empty());

    Opm::Action::ActionX action1("NAME", 10, 100, 0);
    config.add(action1);
    BOOST_CHECK_EQUAL(config.ecl_size(), 1U);
    BOOST_CHECK(!config.empty());

    double min_wait = 86400;
    size_t max_eval = 3;
    {
        Opm::Action::ActionX action("NAME", max_eval, min_wait, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 })) );
        config.add(action);
        BOOST_CHECK_EQUAL(config.ecl_size(), 1U);


        Opm::Action::ActionX action3("NAME3", 1000000, 0, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 })) );
        config.add(action3);

        Opm::Action::PyAction py_action1(python, "PYTHON1", Opm::Action::PyAction::RunCount::single, "act1.py");
        config.add(py_action1);

        Opm::Action::PyAction py_action2(python, "PYTHON2", Opm::Action::PyAction::RunCount::single, "act1.py");
        config.add(py_action2);
    }
    const Opm::Action::ActionX& action2 = config["NAME"];
    Opm::Action::State action_state;
    // The action2 instance has an empty condition, so it will never evaluate to true.
    BOOST_CHECK(action2.ready(  action_state, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 }))  ));
    BOOST_CHECK(!action2.ready(  action_state, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 6, 1 }))   ));
    BOOST_CHECK(!action2.eval(context));

    auto pending = config.pending( action_state, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 }))  );
    BOOST_CHECK_EQUAL( pending.size(), 2U);
    for (auto& ptr : pending) {
        BOOST_CHECK( ptr->ready( action_state, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 }))  ));
        BOOST_CHECK( !ptr->eval( context));
    }
    BOOST_CHECK(!action2.eval(context));


    const auto& python_actions = config.pending_python(action_state);
    BOOST_CHECK_EQUAL(python_actions.size(), 2U);
}



BOOST_AUTO_TEST_CASE(TestContext) {
    Opm::SummaryState st(TimeService::now());
    st.update_well_var("OP1", "WOPR", 100);
    Opm::WListManager wlm;
    Opm::Action::Context context(st, wlm);

    BOOST_REQUIRE_THROW(context.get("func", "arg"), std::out_of_range);

    context.add("FUNC", "ARG", 100);
    BOOST_CHECK_EQUAL(context.get("FUNC", "ARG"), 100);

    const auto& wopr_wells = context.wells("WOPR");
    BOOST_CHECK_EQUAL(wopr_wells.size(), 1U);
    BOOST_CHECK_EQUAL(wopr_wells[0], "OP1");

    const auto& wwct_wells = context.wells("WWCT");
    BOOST_CHECK_EQUAL(wwct_wells.size(), 0U);
}




BOOST_AUTO_TEST_CASE(TestAction_AST_BASIC) {
    // Missing comparator
    BOOST_REQUIRE_THROW( Action::AST( std::vector<std::string>{"WWCT", "OPX", "0.75"} ), std::invalid_argument);

    // Left hand side must be function expression
    BOOST_REQUIRE_THROW( Action::AST(std::vector<std::string>{"0.75", "<", "1.0"}), std::invalid_argument);

    //Extra data
    BOOST_REQUIRE_THROW(Action::AST(std::vector<std::string>{"0.75", "<", "1.0", "EXTRA"}), std::invalid_argument);

    Action::AST ast1({"WWCT", "OPX", ">", "0.75"});
    Action::AST ast2({"WWCT", "OPX", "=", "WWCT", "OPX"});
    Action::AST ast3({"WWCT", "OPY", ">", "0.75"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);
    std::vector<std::string> matching_wells;

    context.add("WWCT", "OPX", 100);
    BOOST_CHECK(ast1.eval(context));

    context.add("WWCT", "OPX", -100);
    BOOST_CHECK(!ast1.eval(context));

    BOOST_CHECK(ast2.eval(context));
    BOOST_REQUIRE_THROW(ast3.eval(context), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(TestAction_AST_OR_AND) {
    Action::AST ast_or({"WWCT", "OPX", ">", "0.75", "OR", "WWCT", "OPY", ">", "0.75"});
    Action::AST ast_and({"WWCT", "OPX", ">", "0.75", "AND", "WWCT", "OPY", ">", "0.75"});
    Action::AST par({"WWCT", "OPX", ">", "0.75", "AND", "(", "WWCT", "OPY", ">", "0.75", "OR", "WWCT", "OPZ", ">", "0.75", ")"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("WWCT", "OPX", 100);
    context.add("WWCT", "OPY", -100);
    context.add("WWCT", "OPZ", 100);
    BOOST_CHECK( ast_or.eval(context) );
    BOOST_CHECK( !ast_and.eval(context) );
    BOOST_CHECK( par.eval(context));


    context.add("WWCT", "OPX", -100);
    context.add("WWCT", "OPY", 100);
    context.add("WWCT", "OPZ", 100);
    BOOST_CHECK( ast_or.eval(context));
    BOOST_CHECK( !ast_and.eval(context) );
    BOOST_CHECK( !par.eval(context));


    context.add("WWCT", "OPX", 100);
    context.add("WWCT", "OPY", 100);
    context.add("WWCT", "OPZ", -100);
    BOOST_CHECK( ast_or.eval(context));
    BOOST_CHECK( ast_and.eval(context) );
    BOOST_CHECK( par.eval(context));

    context.add("WWCT", "OPX", -100);
    context.add("WWCT", "OPY", -100);
    context.add("WWCT", "OPZ", -100);
    BOOST_CHECK( !ast_or.eval(context) );
    BOOST_CHECK( !ast_and.eval(context) );
    BOOST_CHECK( !par.eval(context));
}

BOOST_AUTO_TEST_CASE(DATE) {
    Action::AST ast(std::vector<std::string>{"MNTH", ">=", "JUN"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("MNTH", 6);
    BOOST_CHECK( ast.eval(context));

    context.add("MNTH", 8);
    BOOST_CHECK( ast.eval(context) );

    context.add("MNTH", 5);
    BOOST_CHECK( !ast.eval(context));
}

BOOST_AUTO_TEST_CASE(MNTH_NUMERIC) {
    Action::AST ast(std::vector<std::string>{"MNTH", ">=", "6.3"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("MNTH", 5);
    BOOST_CHECK( !ast.eval(context));

    context.add("MNTH", 6);
    BOOST_CHECK( ast.eval(context) );
}


BOOST_AUTO_TEST_CASE(MANUAL1) {
    Action::AST ast({"GGPR", "FIELD", ">", "50000", "AND", "WGOR", "PR", ">" ,"GGOR", "FIELD"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("GGPR", "FIELD", 60000 );
    context.add("WGOR", "PR" , 300 );
    context.add("GGOR", "FIELD", 200);
    BOOST_CHECK( ast.eval(context));

    context.add("GGPR", "FIELD", 0 );
    context.add("WGOR", "PR" , 300 );
    context.add("GGOR", "FIELD", 200);
    BOOST_CHECK( !ast.eval(context) );

    context.add("GGPR", "FIELD", 60000 );
    context.add("WGOR", "PR" , 100 );
    context.add("GGOR", "FIELD", 200);
    BOOST_CHECK( !ast.eval(context) );
}

BOOST_AUTO_TEST_CASE(MANUAL2) {
    Action::AST ast({"GWCT", "LIST1", ">", "0.70", "AND", "(", "GWPR", "LIST1", ">", "GWPR", "LIST2", "OR", "GWPR", "LIST1", ">", "GWPR", "LIST3", ")"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("GWCT", "LIST1", 1.0);
    context.add("GWPR", "LIST1", 1 );
    context.add("GWPR", "LIST2", 2 );
    context.add("GWPR", "LIST3", 3 );
    BOOST_CHECK( !ast.eval(context));

    context.add("GWCT", "LIST1", 1.0);
    context.add("GWPR", "LIST1", 1 );
    context.add("GWPR", "LIST2", 2 );
    context.add("GWPR", "LIST3", 0 );
    BOOST_CHECK( ast.eval(context));

    context.add("GWCT", "LIST1", 1.0);
    context.add("GWPR", "LIST1", 1 );
    context.add("GWPR", "LIST2", 0 );
    context.add("GWPR", "LIST3", 3 );
    BOOST_CHECK( ast.eval(context));

    context.add("GWCT", "LIST1", 1.0);
    context.add("GWPR", "LIST1", 1 );
    context.add("GWPR", "LIST2", 0 );
    context.add("GWPR", "LIST3", 0 );
    BOOST_CHECK( ast.eval(context));

    context.add("GWCT", "LIST1", 0.0);
    context.add("GWPR", "LIST1", 1 );
    context.add("GWPR", "LIST2", 0 );
    context.add("GWPR", "LIST3", 3 );
    BOOST_CHECK( !ast.eval(context));
}

BOOST_AUTO_TEST_CASE(MANUAL3) {
    Action::AST ast({"MNTH", ".GE.", "MAR", "AND", "MNTH", ".LE.", "OCT", "AND", "GMWL", "HIGH", ".GE.", "4"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("MNTH", 4);
    context.add("GMWL", "HIGH", 4);
    BOOST_CHECK( ast.eval(context));

    context.add("MNTH", 3);
    context.add("GMWL", "HIGH", 4);
    BOOST_CHECK( ast.eval(context));

    context.add("MNTH", 11);
    context.add("GMWL", "HIGH", 4);
    BOOST_CHECK( !ast.eval(context));

    context.add("MNTH", 3);
    context.add("GMWL", "HIGH", 3);
    BOOST_CHECK( !ast.eval(context));
}


BOOST_AUTO_TEST_CASE(MANUAL4) {
    Action::AST ast({"GWCT", "FIELD", ">", "0.8", "AND", "DAY", ">", "1", "AND", "MNTH", ">", "JUN", "AND", "YEAR", ">=", "2021"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("MNTH", 7);
    context.add("DAY", 2);
    context.add("YEAR", 2030);
    context.add("GWCT", "FIELD", 1.0);
    BOOST_CHECK( ast.eval(context) );

    context.add("MNTH", 7);
    context.add("DAY", 2);
    context.add("YEAR", 2019);
    context.add("GWCT", "FIELD", 1.0);
    BOOST_CHECK( !ast.eval(context) );
}



BOOST_AUTO_TEST_CASE(MANUAL5) {
    Action::AST ast({"WCG2", "PROD1", ">", "WCG5", "PROD2", "AND", "GCG3", "G1", ">", "GCG7", "G2", "OR", "FCG1", ">", "FCG7"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("WCG2", "PROD1", 100);
    context.add("WCG5", "PROD2",  50);
    context.add("GCG3", "G1", 200);
    context.add("GCG7", "G2", 100);
    context.add("FCG1", 100);
    context.add("FCG7",  50);
    BOOST_CHECK(ast.eval(context));

    context.add("WCG2", "PROD1", 100);
    context.add("WCG5", "PROD2",  50);
    context.add("GCG3", "G1", 200);
    context.add("GCG7", "G2", 100);
    context.add("FCG1", 100);
    context.add("FCG7", 150);
    BOOST_CHECK(ast.eval(context));

    context.add("WCG2", "PROD1", 100);
    context.add("WCG5", "PROD2",  50);
    context.add("GCG3", "G1", 20);
    context.add("GCG7", "G2", 100);
    context.add("FCG1", 100);
    context.add("FCG7", 150);
    BOOST_CHECK(!ast.eval(context));

    context.add("WCG2", "PROD1", 100);
    context.add("WCG5", "PROD2",  50);
    context.add("GCG3", "G1", 20);
    context.add("GCG7", "G2", 100);
    context.add("FCG1", 200);
    context.add("FCG7", 150);
    BOOST_CHECK(ast.eval(context));
}



BOOST_AUTO_TEST_CASE(LGR) {
    Action::AST ast({"LWCC" , "OPX", "LOCAL", "1", "2", "3", ">", "100"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("LWCC", "OPX:LOCAL:1:2:3", 200);
    BOOST_CHECK(ast.eval(context));

    context.add("LWCC", "OPX:LOCAL:1:2:3", 20);
    BOOST_CHECK(!ast.eval(context));
}


BOOST_AUTO_TEST_CASE(Action_ContextTest) {
    SummaryState st(TimeService::now());
    st.update("WWCT:OP1", 100);
    WListManager wlm;
    Action::Context context(st, wlm);


    BOOST_CHECK_EQUAL(context.get("WWCT", "OP1"), 100);
    BOOST_REQUIRE_THROW(context.get("WGOR", "B37"), std::out_of_range);
    context.add("WWCT", "OP1", 200);

    BOOST_CHECK_EQUAL(context.get("WWCT", "OP1"), 200);
    BOOST_REQUIRE_THROW(context.get("WGOR", "B37"), std::out_of_range);
}

//Note: that this is only temporary test.
//Groupnames w/ astirisks wil eventually work with ACTIONX
BOOST_AUTO_TEST_CASE(TestGroupList) {
    Action::AST ast({"GWPR", "*", ">", "1.0"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);
    BOOST_CHECK_THROW( ast.eval(context), std::logic_error );
}

BOOST_AUTO_TEST_CASE(TestMatchingWells) {
    Action::AST ast({"WOPR", "*", ">", "1.0"});
    SummaryState st(TimeService::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);

    WListManager wlm;
    Action::Context context(st, wlm);
    auto res = ast.eval(context);
    auto wells = res.wells();
    BOOST_CHECK( res);

    BOOST_CHECK_EQUAL( wells.size(), 1U);
    BOOST_CHECK_EQUAL( wells[0], "OPZ" );
}


BOOST_AUTO_TEST_CASE(TestMatchingWells2) {
  Action::AST ast1({"WOPR", "P*", ">", "1.0"});
  Action::AST ast2({"WOPR", "*", ">", "1.0"});
  SummaryState st(TimeService::now());

  st.update_well_var("PX", "WOPR", 0);
  st.update_well_var("PY", "WOPR", 0.50);
  st.update_well_var("PZ", "WOPR", 2.0);

  st.update_well_var("IX", "WOPR", 0);
  st.update_well_var("IY", "WOPR", 0.50);
  st.update_well_var("IZ", "WOPR", 2.0);

  WListManager wlm;
  Action::Context context(st, wlm);
  auto res1 = ast1.eval(context);
  auto res2 = ast2.eval(context);
  auto wells1 = res1.wells();
  auto wells2 = res2.wells();
  BOOST_CHECK(res1);
  BOOST_CHECK_EQUAL( wells1.size(), 1U);
  BOOST_CHECK_EQUAL( wells1[0], "PZ" );

  BOOST_CHECK(res2);
  BOOST_CHECK_EQUAL( wells2.size(), 2U);
  BOOST_CHECK_EQUAL( std::count(wells2.begin(), wells2.end(), "PZ") , 1);
  BOOST_CHECK_EQUAL( std::count(wells2.begin(), wells2.end(), "IZ") , 1);
}



BOOST_AUTO_TEST_CASE(TestMatchingWells_AND) {
    Action::AST ast({"WOPR", "*", ">", "1.0", "AND", "WWCT", "*", "<", "0.50"});
    SummaryState st(TimeService::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);      // The WOPR check matches this well.

    st.update_well_var("OPX", "WWCT", 1.0);
    st.update_well_var("OPY", "WWCT", 0.0);     // The WWCT check matches this well.
    st.update_well_var("OPZ", "WWCT", 1.0);

    WListManager wlm;
    Action::Context context(st, wlm);
    auto res = ast.eval(context);
    BOOST_CHECK(res);

    // Even though condition as a whole matches, there is no finite set of wells
    // which mathes both conditions when combined with AND - i.e. the matching_wells
    // variable should be empty.
    BOOST_CHECK( res.wells().empty() );
}

BOOST_AUTO_TEST_CASE(TestMatchingWells_OR) {
    Action::AST ast({"WOPR", "*", ">", "1.0", "OR", "WWCT", "*", "<", "0.50"});
    SummaryState st(TimeService::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);      // The WOPR check matches this well.

    st.update_well_var("OPX", "WWCT", 1.0);
    st.update_well_var("OPY", "WWCT", 0.0);     // The WWCT check matches this well.
    st.update_well_var("OPZ", "WWCT", 1.0);

    WListManager wlm;
    Action::Context context(st, wlm);
    auto res = ast.eval(context);
    auto wells = res.wells();
    BOOST_CHECK(res);

    // The well 'OPZ' matches the first condition and the well 'OPY' matches the
    // second condition, since the two conditions are combined with || the
    // resulting mathcing_wells variable should contain both these wells.
    BOOST_CHECK_EQUAL( wells.size(), 2U);
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "OPZ") != wells.end());
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "OPY") != wells.end());
}

BOOST_AUTO_TEST_CASE(TestWLIST) {
    WListManager wlm;
    Action::AST ast({"WOPR", "*LIST1", ">", "1.0"});
    SummaryState st(TimeService::now());

    st.update_well_var("W1", "WOPR", 2.0);
    st.update_well_var("W2", "WOPR", 2.50);
    st.update_well_var("W3", "WOPR", 2.0);
    st.update_well_var("W4", "WOPR", 2.0);
    st.update_well_var("W5", "WOPR", 2.0);


    Action::Context context(st, wlm);
    wlm.newList("*LIST1", {"W1", "W3", "W5"});
    auto res = ast.eval(context);
    auto wells = res.wells();
    BOOST_CHECK(res);
    BOOST_CHECK_EQUAL( wells.size(), 3U);
    for (const auto& w : {"W1", "W3", "W5"}) {
        auto find_iter = std::find(wells.begin(), wells.end(), w);
        BOOST_CHECK( find_iter != wells.end() );
    }
}

BOOST_AUTO_TEST_CASE(TestFieldAND) {
    Action::AST ast({"FMWPR", ">=", "4", "AND", "WUPR3", "OP*", "=", "1"});
    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    st.update_well_var("OP1", "WUPR3", 3);
    st.update_well_var("OP2", "WUPR3", 2);
    st.update_well_var("OP3", "WUPR3", 1);
    st.update_well_var("OP4", "WUPR3", 4);

    st.update("FMWPR", 1);
    {
        auto res = ast.eval(context);
        BOOST_CHECK(!res);
        BOOST_CHECK_THROW(res.wells(), std::logic_error);
        BOOST_CHECK_THROW(res.has_well("ABC"), std::logic_error);
    }

    st.update("FMWPR", 4);
    {
        auto res = ast.eval(context);
        auto wells = res.wells();
        BOOST_CHECK(res);
        BOOST_CHECK_EQUAL(wells.size(), 1U);
        BOOST_CHECK_EQUAL(wells[0], "OP3");
    }
}


BOOST_AUTO_TEST_CASE(Conditions) {
    auto location = KeywordLocation("Keyword", "File", 100);

    // Missing comparator
    BOOST_CHECK_THROW(Action::Condition cond({"WWCT", "OPX"}, location), std::invalid_argument);

    // Missing right hand side
    BOOST_CHECK_THROW(Action::Condition cond({"WWCT", "OPX", ">"}, location), std::invalid_argument);

    Action::Condition cond({"WWCT", "OPX", ">", "0.75",  "AND"}, location);
    BOOST_CHECK(cond.cmp == Action::Comparator::GREATER);
    BOOST_CHECK(cond.cmp_string == ">" );
    BOOST_CHECK_EQUAL(cond.lhs.quantity, "WWCT");
    BOOST_CHECK_EQUAL(cond.lhs.args.size(), 1U);
    BOOST_CHECK_EQUAL(cond.lhs.args[0], "OPX");
    BOOST_CHECK( !cond.open_paren() );
    BOOST_CHECK( !cond.close_paren() );

    BOOST_CHECK_EQUAL(cond.rhs.quantity, "0.75");
    BOOST_CHECK_EQUAL(cond.rhs.args.size(), 0U);
    BOOST_CHECK(cond.logic == Action::Logical::AND);

    Action::Condition cond2({"WWCT", "OPX", "<=", "WSOPR", "OPX", "235"}, location);
    BOOST_CHECK(cond2.cmp == Action::Comparator::LESS_EQUAL);
    BOOST_CHECK(cond2.cmp_string == "<=" );
    BOOST_CHECK_EQUAL(cond2.lhs.quantity, "WWCT");
    BOOST_CHECK_EQUAL(cond2.lhs.args.size(), 1U);
    BOOST_CHECK_EQUAL(cond2.lhs.args[0], "OPX");

    BOOST_CHECK_EQUAL(cond2.rhs.quantity, "WSOPR");
    BOOST_CHECK_EQUAL(cond2.rhs.args.size(), 2U);
    BOOST_CHECK_EQUAL(cond2.rhs.args[0], "OPX");
    BOOST_CHECK_EQUAL(cond2.rhs.args[1], "235");
    BOOST_CHECK(cond2.logic == Action::Logical::END);
}


BOOST_AUTO_TEST_CASE(SCAN2) {
    const auto deck_string = std::string{ R"(
SCHEDULE

TSTEP
10 /

ACTIONX
   'B' /
   WWCT 'OPX'     > 0.75    AND /
   FPR < 100 /
/

WELSPECS
  'W1'  'OP'  1 1 3.33  'OIL' 7*/
/

ENDACTIO

TSTEP
   10 /


ACTIONX
   'A' /
   WOPR 'OPX'  = 1000 /
/

ENDACTIO

ACTIONX
   'B' /
   FWCT <= 0.50 /
/



ENDACTIO

TSTEP
10 /

)"};

    Opm::Parser parser;
    auto deck = parser.parseString(deck_string);
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid1, table);
    auto python = std::make_shared<Python>();

    Runspec runspec (deck);
    Schedule sched(deck, grid1, fp, runspec, python);
    const auto& actions0 = sched[0].actions.get();
    BOOST_CHECK_EQUAL(actions0.ecl_size(), 0U);

    const auto& actions1 = sched[1].actions.get();
    BOOST_CHECK_EQUAL(actions1.ecl_size(), 1U);


    const auto& act1 = actions1["B"];
    const auto& strings = act1.keyword_strings();
    BOOST_CHECK_EQUAL(strings.size(), 4U);
    BOOST_CHECK_EQUAL(strings.back(), "ENDACTIO");


    std::string rdeck_string = "";
    for (std::size_t i = 0; i < strings.size(); i++)
        rdeck_string += strings[i] + "\n";

    auto deck2 = parser.parseString(rdeck_string);
    BOOST_CHECK(deck2["WELSPECS"].back() == deck["WELSPECS"].back());


    const auto& conditions = act1.conditions();
    BOOST_CHECK_EQUAL(conditions.size() , 2U);

    const auto& cond0 = conditions[0];
    BOOST_CHECK_EQUAL(cond0.lhs.quantity, "WWCT");
    BOOST_CHECK(cond0.cmp == Action::Comparator::GREATER);
    BOOST_CHECK(cond0.logic == Action::Logical::AND);
    BOOST_CHECK_EQUAL(cond0.lhs.args.size(), 1U);
    BOOST_CHECK_EQUAL(cond0.lhs.args[0], "OPX");
    BOOST_CHECK_EQUAL(cond0.rhs.args.size(), 0U);
    BOOST_CHECK_EQUAL(cond0.rhs.quantity, "0.75");

    const auto& cond1 = conditions[1];
    BOOST_CHECK_EQUAL(cond1.lhs.quantity, "FPR");
    BOOST_CHECK(cond1.cmp == Action::Comparator::LESS);
    BOOST_CHECK(cond1.logic == Action::Logical::END);

    /*****************************************************************/

    const auto& actions2 = sched[2].actions.get();
    BOOST_CHECK_EQUAL(actions2.ecl_size(), 2U);

    const auto& actB = actions2["B"];
    const auto& condB = actB.conditions();
    BOOST_CHECK_EQUAL(condB.size() , 1U);
    BOOST_CHECK_EQUAL(condB[0].lhs.quantity, "FWCT");
    BOOST_CHECK(condB[0].cmp == Action::Comparator::LESS_EQUAL);
    BOOST_CHECK(condB[0].logic == Action::Logical::END);
    BOOST_CHECK_EQUAL(condB[0].cmp_string, "<=");

    const auto& actA = actions2["A"];
    const auto& condA = actA.conditions();
    BOOST_CHECK_EQUAL(condA.size() , 1U);
    BOOST_CHECK_EQUAL(condA[0].lhs.quantity, "WOPR");
    BOOST_CHECK(condA[0].cmp == Action::Comparator::EQUAL);
    BOOST_CHECK(condA[0].logic == Action::Logical::END);
    BOOST_CHECK_EQUAL(condA[0].cmp_string , "=");

    std::size_t index = 0;
    for (const auto& act : actions2) {
        if (index == 0)
            BOOST_CHECK_EQUAL("B", act.name());

        if (index == 1)
            BOOST_CHECK_EQUAL("A", act.name());
        index++;
    }
}



BOOST_AUTO_TEST_CASE(ACTIONRESULT_COPY_WELLS) {
    Action::Result res1(true, {"W1", "W2", "W3"});
    auto res2 = res1;

    BOOST_CHECK(res1);
    BOOST_CHECK(res2);
    BOOST_CHECK(!res1.has_well("NO"));
    BOOST_CHECK(!res2.has_well("NO"));
    for (const auto& w : {"W1", "W2", "W3"}) {
        BOOST_CHECK(res1.has_well(w));
        BOOST_CHECK(res2.has_well(w));
    }
}


BOOST_AUTO_TEST_CASE(ActionState) {
    Action::State st;
    Action::ActionX action1("NAME", 100, 100, 100); action1.update_id(100);
    Action::ActionX action2("NAME", 100, 100, 100); action1.update_id(200);
    Action::Result res1(true, {"W1"});
    Action::Result res2(true, {"W2"});
    Action::Result res3(true, {"W3"});

    BOOST_CHECK_EQUAL(0U, st.run_count(action1));
    BOOST_CHECK_THROW( st.run_time(action1), std::out_of_range);

    st.add_run(action1, 100, res1);
    BOOST_CHECK_EQUAL(1U, st.run_count(action1));
    BOOST_CHECK_EQUAL(100, st.run_time(action1));
    auto r1 = st.result("NAME");
    BOOST_CHECK(r1.value() == res1);

    st.add_run(action1, 1000, res2);
    BOOST_CHECK_EQUAL(2U, st.run_count(action1));
    BOOST_CHECK_EQUAL(1000, st.run_time(action1));
    auto r2 = st.result("NAME");
    BOOST_CHECK(r2.value() == res2);

    BOOST_CHECK_EQUAL(0U, st.run_count(action2));
    BOOST_CHECK_THROW( st.run_time(action2), std::out_of_range);

    st.add_run(action2, 100, res3);
    BOOST_CHECK_EQUAL(1U, st.run_count(action2));
    BOOST_CHECK_EQUAL(100, st.run_time(action2));
    auto r3 = st.result("NAME");
    BOOST_CHECK(r3.value() == res3);

    st.add_run(action2, 1000, res1);
    BOOST_CHECK_EQUAL(2U, st.run_count(action2));
    BOOST_CHECK_EQUAL(1000, st.run_time(action2));


    auto res = st.result("NAME-HIDDEN");
    BOOST_CHECK(!res.has_value());

}

BOOST_AUTO_TEST_CASE(MANUAL4_QUOTE) {
    const auto deck_string = std::string{ R"(
RUNSPEC
ACTDIMS
   3* 4 /

SCHEDULE

ACTIONX
'A' /
GWCT FIELD > 0.8 AND /
DAY > 1 AND /
MNTH > 'JUN' AND /
YEAR >= 2021 /
/

ENDACTIO

        )"};

    Opm::Parser parser;
    auto deck = parser.parseString(deck_string);
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid1, table);
    auto python = std::make_shared<Python>();

    Runspec runspec (deck);
    Schedule sched(deck, grid1, fp, runspec, python);
    const auto& action1 = sched[0].actions.get()["A"];

    SummaryState st(TimeService::now());
    WListManager wlm;
    Action::Context context(st, wlm);

    context.add("MNTH", 7);
    context.add("DAY", 2);
    context.add("YEAR", 2030);
    context.add("GWCT", "FIELD", 1.0);
    BOOST_CHECK( action1.eval(context) );

    context.add("MNTH", 7);
    context.add("DAY", 2);
    context.add("YEAR", 2019);
    context.add("GWCT", "FIELD", 1.0);
    BOOST_CHECK( !action1.eval(context) );
}


BOOST_AUTO_TEST_CASE(ActionID) {
    const auto deck_string = std::string{ R"(
SCHEDULE

TSTEP
10 /

ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

WELSPECS
'W1'  'OP'  1 1 3.33  'OIL' 7*/
/

ENDACTIO

TSTEP
10 /


ACTIONX
'A' /
WOPR 'OPX'  = 1000 /
/

ENDACTIO

        )"};

    Opm::Parser parser;
    auto deck = parser.parseString(deck_string);
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid1, table);
    auto python = std::make_shared<Python>();

    Runspec runspec (deck);
    Schedule sched(deck, grid1, fp, runspec, python);
    const auto& action1 = sched[1].actions.get()["A"];
    const auto& action2 = sched[2].actions.get()["A"];

    BOOST_CHECK(action1.id() != action2.id());

    Action::State st;
    st.add_run(action1, 1000, Action::Result{true});
    BOOST_CHECK_EQUAL( st.run_count(action1), 1U);
    BOOST_CHECK_EQUAL( st.run_count(action2), 0U);
}




BOOST_AUTO_TEST_CASE(Action_GCON) {
    const auto deck_string = std::string{ R"(
SCHEDULE


WELSPECS
    'PROD1' 'G1'  1 1 10 'OIL' /
    'INJ1'  'G1'  1 1 10 'WAT' /
/

GCONPROD
'G1' 'ORAT' 100  /
/

GCONINJE
'G1' 'WATER' 'RATE' 1000 /
/

ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

GCONPROD
   'G1'  'ORAT' 200 /
/

GCONINJE
'G1' 'WATER' 'RATE' 5000 /
/

ENDACTIO

TSTEP
10 /

        )"};

    auto unit_system =  UnitSystem::newMETRIC();
    const auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    const auto& action1 = sched[0].actions.get()["A"];
    {
        const auto& group = sched.getGroup("G1", 0);
        const auto& prod = group.productionControls(st);
        BOOST_CHECK_CLOSE( prod.oil_target , unit_system.to_si(UnitSystem::measure::liquid_surface_rate, 100), 1e-5 );

        const auto& inj = group.injectionControls(Phase::WATER, st);
        BOOST_CHECK_CLOSE( inj.surface_max_rate, unit_system.to_si(UnitSystem::measure::liquid_surface_rate, 1000), 1e-5 );
    }


    Action::Result action_result(true);
    sched.applyAction(0, action1, action_result.wells(), {});

    {
        const auto& group = sched.getGroup("G1", 1);
        const auto& prod = group.productionControls(st);
        BOOST_CHECK_CLOSE( prod.oil_target , unit_system.to_si(UnitSystem::measure::liquid_surface_rate, 200), 1e-5 );

        const auto& inj = group.injectionControls(Phase::WATER, st);
        BOOST_CHECK_CLOSE( inj.surface_max_rate, unit_system.to_si(UnitSystem::measure::liquid_surface_rate, 5000), 1e-5 );
    }


    auto wellpi = action1.wellpi_wells(WellMatcher(sched[0].well_order()), {});
    BOOST_CHECK( wellpi.empty() );
}


bool has_well(const std::vector<std::string>& wells, const std::string& well) {
    auto find_well = std::find(wells.begin(), wells.end(), well);
    return (find_well != wells.end());
}


BOOST_AUTO_TEST_CASE(WELPI_TEST1) {
    std::string deck_string = R"(
WELPI
   'W1'  10 /
   'W2'  20 /
/

WELPI
    'P*' 10 /
/

)";
    Parser parser;
    auto deck = parser.parseString(deck_string);
    Action::ActionX action("NAME", 1, 1, 0);
    NameOrder well_order({"W1", "W2", "P1", "P2", "P3"});
    WellMatcher well_matcher( well_order );
    action.addKeyword(deck["WELPI"][0]);
    {
        auto wells = action.wellpi_wells(well_matcher, {});
        BOOST_CHECK_EQUAL( wells.size(), 2 );
        has_well(wells, "W1");
        has_well(wells, "W2");
    }
    action.addKeyword(deck["WELPI"][1]);
    {
        auto wells = action.wellpi_wells(well_matcher, {});
        BOOST_CHECK_EQUAL( wells.size(), 5 );
        has_well(wells, "W1");
        has_well(wells, "W2");
        has_well(wells, "P1");
        has_well(wells, "P2");
        has_well(wells, "P3");
    }
}

BOOST_AUTO_TEST_CASE(GASLIFT_OPT_DECK) {
    const auto input = R"(-- Turns on gas lift optimization
RUNSPEC
LIFTOPT
/

SCHEDULE

WELSPECS
    'OPX' 'G1'  1 1 10 'OIL' /
/

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
 /

ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

GLIFTOPT
 'PLAT-A'  200000 /  --
/

ENDACTIO

TSTEP
10 /


)";

    Opm::UnitSystem unitSystem = UnitSystem( UnitSystem::UnitType::UNIT_TYPE_METRIC );
    auto sched = make_schedule(input);
    const auto& action1 = sched[0].actions.get()["A"];
    {
        const auto& glo = sched.glo(0);
        BOOST_CHECK(!glo.has_group("PLAT-A"));
    }
    std::unordered_set<std::string> required_summary;
    action1.required_summary(required_summary);
    BOOST_CHECK_EQUAL( required_summary.count("WWCT"), 1);
    BOOST_CHECK_EQUAL( required_summary.count("FPR"), 1);


    Action::Result action_result(true);
    const auto& sim_update = sched.applyAction(0, action1, action_result.wells(), {});
    BOOST_CHECK( sim_update.affected_wells.empty() );
    {
        const auto& glo = sched.glo(0);
        BOOST_CHECK(glo.has_group("PLAT-A"));
        const auto& plat_group = glo.group("PLAT-A");
        BOOST_CHECK_CLOSE( *plat_group.max_lift_gas(), unitSystem.to_si( UnitSystem::measure::gas_surface_rate, 200000), 1e-13);
        BOOST_CHECK(!plat_group.max_total_gas().has_value());
    }

}

BOOST_AUTO_TEST_CASE(ACTIONX_WGNAME) {
    Action::WGNames wgnames;

    wgnames.add_well("W1");
    BOOST_CHECK(wgnames.has_well("W1"));
    BOOST_CHECK(!wgnames.has_well("W2"));

    wgnames.add_group("G1");
    BOOST_CHECK(wgnames.has_group("G1"));
    BOOST_CHECK(!wgnames.has_group("G2"));
}

BOOST_AUTO_TEST_CASE(Action_COMPDAT_ACTION) {
    const auto deck_string = std::string{ R"(
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


ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

WELSPECS
    'PROD1' 'G1'  1 1 10 'OIL' /
/

COMPDAT
 'PROD1'  1  1   1   3 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

ENDACTIO

TSTEP
10 /

WELOPEN
  'PROD1' 'OPEN' 5* /
/

TSTEP
10/


        )"};

    const auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    const auto& action1 = sched[0].actions.get()["A"];

    BOOST_CHECK(!sched.hasWell("PROD1"));

    Action::Result action_result(true);
    sched.applyAction(0, action1, action_result.wells(), {});

    const auto& well = sched.getWell("PROD1", 1);
    const auto& connections = well.getConnections();
    BOOST_CHECK_EQUAL(connections.size(), 3);
}



BOOST_AUTO_TEST_CASE(Action_WELPI) {
    const auto deck_string = std::string{ R"(
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
    'PROD1' 'G1'  1 1 10 'OIL' /
/

COMPDAT
 'PROD1'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

WELPI
  'PROD1' 1000 /
/

ENDACTIO

TSTEP
10 /

        )"};

    const auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    const auto& action1 = sched[0].actions.get()["A"];
    double CF0;
    {
        const auto& target_wellpi = sched[0].target_wellpi;
        BOOST_CHECK_EQUAL( target_wellpi.count("PROD1"), 0);

        const auto& well = sched.getWell("PROD1", 0);
        CF0 = well.getConnections()[0].CF();
    }


    Action::Result action_result(true);
    BOOST_CHECK_THROW( sched.applyAction(0, action1, action_result.wells(), {}), std::exception);
    {
        const auto& well = sched.getWell("PROD1", 0);
        const auto& sim_update = sched.applyAction(0, action1, action_result.wells(), {{"PROD1", well.convertDeckPI(500)}});
        BOOST_CHECK_EQUAL( sim_update.affected_wells.count("PROD1"), 1);
        BOOST_CHECK_EQUAL( sim_update.affected_wells.size(), 1);
    }
    {
        const auto& target_wellpi = sched[0].target_wellpi;
        BOOST_CHECK_EQUAL( target_wellpi.at("PROD1"), 1000);

        const auto& well = sched.getWell("PROD1", 0);
        auto CF1 = well.getConnections()[0].CF();
        BOOST_CHECK_CLOSE(CF1 / CF0, 2.0, 1e-4 );
    }

    {
        std::unordered_set<std::string> required_summary;
        action1.required_summary(required_summary);
        BOOST_CHECK_EQUAL( required_summary.count("WWCT"), 1);
    }
}

BOOST_AUTO_TEST_CASE(Action_MULTZ) {
    const auto deck_string = std::string{ R"(
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
    'PROD1' 'G1'  1 1 10 'OIL' /
/

COMPDAT
 'PROD1'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

ACTIONX
'A' /
WWCT 'OPX'     > 0.75    AND /
FPR < 100 /
/

BOX
  1 10 1 10 5 5 /

MULTZ
   100*0.0 /

ENDBOX

ENDACTIO

TSTEP
10 /

        )"};

    const auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    const auto& action1 = sched[0].actions.get()["A"];

    BOOST_CHECK(sched[0].geo_keywords().empty());

    Action::Result action_result(true);
    auto sim_update = sched.applyAction(0, action1, action_result.wells(), {});
    BOOST_CHECK( sim_update.tran_update );
    BOOST_CHECK_EQUAL(sched[0].geo_keywords().size(), 3);
}



BOOST_AUTO_TEST_CASE(COMBINED_OR) {
    const auto deck_string = std::string{ R"(
RUNSPEC

ACTDIMS
   3* 4 /

SCHEDULE

ACTIONX
ACT1 1 /
FU1 < 10 AND   /
( FU2 < FU3 )  AND /
( FU2 > 1 OR /
  FU2 < -1 ) /
/

ENDACTIO

        )"};

    auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    Opm::WListManager wlm;
    Opm::Action::Context context(st, wlm);

    const auto& config = sched[0].actions.get();
    const Opm::Action::ActionX& action = config["ACT1"];

    /*
    FU1 < 10 |  FU2 < FU3 ||   FU2 > 1 |  FU2 < -1  | Result
    ----------------------||------------------------|-------
    T        |  T         || T         | T          | T
    T        |  T         || T         | F          | T
    T        |  T         || F         | T          | T
    T        |  T         || F         | F          | F
    ----------------------||------------------------|-------
    T        |  F         || T         | T          | F
    T        |  F         || T         | T          | F
    T        |  F         || T         | T          | F
    T        |  F         || T         | T          | F
    ----------------------||------------------------|-------
    F        |  T         || T         | T          | F
    F        |  T         || T         | T          | F
    F        |  T         || T         | T          | F
    F        |  T         || T         | T          | F
    ----------------------||------------------------|-------
    F        |  F         || T         | T          | F
    F        |  F         || T         | T          | F
    F        |  F         || T         | T          | F
    F        |  F         || T         | T          | F
    */

    std::vector<double> FU1_values = {1, 100};
    std::vector<double> FU2_values = {-5,0,5};


    for (const auto& FU1 : FU1_values) {
        for (const auto& FU2 : FU2_values) {
            std::vector FU3_values = { FU2 + 1, FU2 - 1 };
            for (const auto& FU3 : FU3_values) {
                bool expected = ((FU1 < 10) && (FU2 < FU3) && ((FU2 > 1) || (FU2 < -1)));

                st.update("FU1", FU1);
                st.update("FU2", FU2);
                st.update("FU3", FU3);

                auto result = action.eval(context);
                BOOST_CHECK_EQUAL(bool(result), expected);
            }
        }
    }
    const auto& conditions = action.conditions();
    BOOST_CHECK_EQUAL( conditions.size(), 4);
    {
        auto cond0 = conditions[0];
        BOOST_CHECK_EQUAL(cond0.lhs.quantity, "FU1");
        BOOST_CHECK(cond0.lhs.args.empty());
        BOOST_CHECK(!cond0.left_paren);
        BOOST_CHECK(!cond0.right_paren);
        BOOST_CHECK(!cond0.open_paren());
        BOOST_CHECK(!cond0.close_paren());
    }
    {
        auto cond1 = conditions[1];
        BOOST_CHECK_EQUAL(cond1.lhs.quantity, "FU2");
        BOOST_CHECK(cond1.lhs.args.empty());
        BOOST_CHECK(cond1.left_paren);
        BOOST_CHECK(cond1.right_paren);
        BOOST_CHECK(!cond1.open_paren());
        BOOST_CHECK(!cond1.close_paren());
    }
    {
        auto cond2 = conditions[2];
        BOOST_CHECK_EQUAL(cond2.lhs.quantity, "FU2");
        BOOST_CHECK(cond2.lhs.args.empty());
        BOOST_CHECK(cond2.left_paren);
        BOOST_CHECK(!cond2.right_paren);
        BOOST_CHECK(cond2.open_paren());
        BOOST_CHECK(!cond2.close_paren());
    }
    {
        auto cond3 = conditions[3];
        BOOST_CHECK_EQUAL(cond3.lhs.quantity, "FU2");
        BOOST_CHECK(cond3.lhs.args.empty());
        BOOST_CHECK(!cond3.left_paren);
        BOOST_CHECK(cond3.right_paren);
        BOOST_CHECK(!cond3.open_paren());
        BOOST_CHECK(cond3.close_paren());

        BOOST_CHECK(cond3.rhs.args.empty());
    }

}

BOOST_AUTO_TEST_CASE(MatchingWellsSpecified1) {
    Action::AST ast({"WBHP", "P1", "<", "200"});
    auto st = SummaryState{ TimeService::now() };
    Opm::WListManager wlm;

    st.update_well_var("P1", "WBHP", 150);
    Opm::Action::Context context(st, wlm);
    auto result = ast.eval(context);
    BOOST_CHECK(result);
    BOOST_CHECK(result.wells() == std::vector<std::string>{"P1"});
}


BOOST_AUTO_TEST_CASE(MatchingWellsSpecified2) {

    const auto deck_string = std::string{ R"(
SCHEDULE

WELSPECS
  'P1'  'OP'  1 1 3.33  'OIL' 7*/
/

ACTIONX
INJECTION 10 /
WBHP P1 < 200.0 /
/

WELOPEN
  'WI1' 'OPEN' 5* /
/

ENDACTIO

        )"};

    auto st = SummaryState{ TimeService::now() };
    Schedule sched = make_schedule(deck_string);
    Opm::WListManager wlm;

    st.update_well_var("P1", "WBHP", 150);
    Opm::Action::Context context(st, wlm);
    const auto& action = sched[0].actions.get()["INJECTION"];
    auto result = action.eval(context);
    BOOST_CHECK(result);
    BOOST_CHECK(result.wells() == std::vector<std::string>{"P1"});
}



BOOST_AUTO_TEST_CASE(MaxConditions) {

    const auto deck_string = std::string{ R"(
RUNSPEC

ACTDIMS
  3*  2 /

SCHEDULE

ACTIONX
INJECTION 10 /
WBHP P1 < 200.0 AND /
MNTH = JAN AND /
YEAR = 2020 /
/

EXIT
  1 /

ENDACTIO

        )"};

    auto st = SummaryState{ TimeService::now() };
    BOOST_CHECK_THROW( make_schedule(deck_string), std::exception);
}
