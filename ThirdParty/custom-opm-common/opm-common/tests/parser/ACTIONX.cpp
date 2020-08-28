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
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE ACTIONX

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <opm/common/OpmLog/Location.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionAST.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionContext.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/Actions.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionX.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

using namespace Opm;




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
    const auto& kw = deck.getKeyword("ACTIONX");

    Action::ActionX action2(kw, 0);
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
  'W2'  'OP'  1 1 3.33  'OIL' 7*/
/

ACTIONX
   'ACTION' /
   WWCT OPX  > 0.75 /
/

WELSPECS
  'W1'  'OP'  1 1 3.33  'OIL' 7*/
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
    auto python = std::make_shared<Python>();
    Opm::Parser parser;
    auto deck1 = parser.parseString(MISSING_END);
    auto deck2 = parser.parseString(WITH_WELSPECS);
    auto deck3 = parser.parseString(WITH_GRID);
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck1 );
    FieldPropsManager fp( deck1, Phases{true, true, true}, grid1, table);
    Runspec runspec (deck1);

    // The ACTIONX keyword has no matching 'ENDACTIO' -> exception
    BOOST_CHECK_THROW(Schedule(deck1, grid1, fp, runspec, python), std::invalid_argument);

    Schedule sched(deck2, grid1, fp, runspec, python);
    BOOST_CHECK( !sched.hasWell("W1") );
    BOOST_CHECK( sched.hasWell("W2"));

    // The deck3 contains the 'GRID' keyword in the ACTIONX block - that is not a whitelisted keyword.
    ParseContext parseContext( {{ParseContext::ACTIONX_ILLEGAL_KEYWORD, InputError::THROW_EXCEPTION}} );
    ErrorGuard errors;
    BOOST_CHECK_THROW(Schedule(deck3, grid1, fp, runspec, parseContext, errors, python), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(TestActions) {
    Opm::SummaryState st(std::chrono::system_clock::now());
    Opm::Action::Context context(st);
    Opm::Action::Actions config;
    std::vector<std::string> matching_wells;
    auto python = std::make_shared<Opm::Python>();
    BOOST_CHECK_EQUAL(config.size(), 0);
    BOOST_CHECK(config.empty());

    Opm::Action::ActionX action1("NAME", 10, 100, 0);
    config.add(action1);
    BOOST_CHECK_EQUAL(config.size(), 1);
    BOOST_CHECK(!config.empty());

    double min_wait = 86400;
    size_t max_eval = 3;
    {
        Opm::Action::ActionX action("NAME", max_eval, min_wait, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 })) );
        config.add(action);
        BOOST_CHECK_EQUAL(config.size(), 1);


        Opm::Action::ActionX action3("NAME3", 1000000, 0, asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 })) );
        config.add(action3);

        Opm::Action::PyAction py_action1(python, "PYTHON1", Opm::Action::PyAction::RunCount::single, "act1.py");
        config.add(py_action1);

        Opm::Action::PyAction py_action2(python, "PYTHON2", Opm::Action::PyAction::RunCount::single, "act1.py");
        config.add(py_action2);
    }
    const Opm::Action::ActionX& action2 = config.get("NAME");
    // The action2 instance has an empty condition, so it will never evaluate to true.
    BOOST_CHECK(action2.ready(  asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 7, 1 }))  ));
    BOOST_CHECK(!action2.ready(  asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 6, 1 }))   ));
    BOOST_CHECK(!action2.eval(asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 6, 1 })), context));

    auto pending = config.pending( asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 }))  );
    BOOST_CHECK_EQUAL( pending.size(), 2);
    for (auto& ptr : pending) {
        BOOST_CHECK( ptr->ready(  asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 }))  ));
        BOOST_CHECK( !ptr->eval(asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 })), context));
    }
    BOOST_CHECK(!action2.eval(asTimeT(TimeStampUTC(TimeStampUTC::YMD{ 2000, 8, 7 })), context));


    const auto& python_actions = config.pending_python();
    BOOST_CHECK_EQUAL(python_actions.size(), 2);
}



BOOST_AUTO_TEST_CASE(TestContext) {
    Opm::SummaryState st(std::chrono::system_clock::now());
    st.update_well_var("OP1", "WOPR", 100);
    Opm::Action::Context context(st);

    BOOST_REQUIRE_THROW(context.get("func", "arg"), std::out_of_range);

    context.add("FUNC", "ARG", 100);
    BOOST_CHECK_EQUAL(context.get("FUNC", "ARG"), 100);

    const auto& wopr_wells = context.wells("WOPR");
    BOOST_CHECK_EQUAL(wopr_wells.size(), 1);
    BOOST_CHECK_EQUAL(wopr_wells[0], "OP1");

    const auto& wwct_wells = context.wells("WWCT");
    BOOST_CHECK_EQUAL(wwct_wells.size(), 0);
}



Opm::Schedule make_action(const std::string& action_string) {
    std::string start = std::string{ R"(
SCHEDULE
)"};
    std::string end = std::string{ R"(
ENDACTIO

TSTEP
   10 /
)"};

    std::string deck_string = start + action_string + end;
    Opm::Parser parser;
    auto deck = parser.parseString(deck_string);
    auto python = std::make_shared<Python>();
    EclipseGrid grid1(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid1, table);
    Runspec runspec(deck);

    return Schedule(deck, grid1, fp, runspec, python);
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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);
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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

    context.add("MNTH", 6);
    BOOST_CHECK( ast.eval(context));

    context.add("MNTH", 8);
    BOOST_CHECK( ast.eval(context) );

    context.add("MNTH", 5);
    BOOST_CHECK( !ast.eval(context));
}


BOOST_AUTO_TEST_CASE(MANUAL1) {
    Action::AST ast({"GGPR", "FIELD", ">", "50000", "AND", "WGOR", "PR", ">" ,"GGOR", "FIELD"});
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

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
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

    context.add("LWCC", "OPX:LOCAL:1:2:3", 200);
    BOOST_CHECK(ast.eval(context));

    context.add("LWCC", "OPX:LOCAL:1:2:3", 20);
    BOOST_CHECK(!ast.eval(context));
}


BOOST_AUTO_TEST_CASE(Action_ContextTest) {
    SummaryState st(std::chrono::system_clock::now());
    st.update("WWCT:OP1", 100);
    Action::Context context(st);


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
    SummaryState st(std::chrono::system_clock::now());

    Action::Context context(st);
    BOOST_CHECK_THROW( ast.eval(context), std::logic_error );
}

BOOST_AUTO_TEST_CASE(TestMatchingWells) {
    Action::AST ast({"WOPR", "*", ">", "1.0"});
    SummaryState st(std::chrono::system_clock::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);

    Action::Context context(st);
    auto res = ast.eval(context);
    auto wells = res.wells();
    BOOST_CHECK( res);

    BOOST_CHECK_EQUAL( wells.size(), 1);
    BOOST_CHECK_EQUAL( wells[0], "OPZ" );
}

BOOST_AUTO_TEST_CASE(TestMatchingWells2) {
  Action::AST ast1({"WOPR", "P*", ">", "1.0"});
  Action::AST ast2({"WOPR", "*", ">", "1.0"});
  SummaryState st(std::chrono::system_clock::now());

  st.update_well_var("PX", "WOPR", 0);
  st.update_well_var("PY", "WOPR", 0.50);
  st.update_well_var("PZ", "WOPR", 2.0);

  st.update_well_var("IX", "WOPR", 0);
  st.update_well_var("IY", "WOPR", 0.50);
  st.update_well_var("IZ", "WOPR", 2.0);

  Action::Context context(st);
  auto res1 = ast1.eval(context);
  auto res2 = ast2.eval(context);
  auto wells1 = res1.wells();
  auto wells2 = res2.wells();
  BOOST_CHECK(res1);
  BOOST_CHECK_EQUAL( wells1.size(), 1);
  BOOST_CHECK_EQUAL( wells1[0], "PZ" );

  BOOST_CHECK(res2);
  BOOST_CHECK_EQUAL( wells2.size(), 2);
  BOOST_CHECK_EQUAL( std::count(wells2.begin(), wells2.end(), "PZ") , 1);
  BOOST_CHECK_EQUAL( std::count(wells2.begin(), wells2.end(), "IZ") , 1);
}



BOOST_AUTO_TEST_CASE(TestMatchingWells_AND) {
    Action::AST ast({"WOPR", "*", ">", "1.0", "AND", "WWCT", "*", "<", "0.50"});
    SummaryState st(std::chrono::system_clock::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);      // The WOPR check matches this well.

    st.update_well_var("OPX", "WWCT", 1.0);
    st.update_well_var("OPY", "WWCT", 0.0);     // The WWCT check matches this well.
    st.update_well_var("OPZ", "WWCT", 1.0);

    Action::Context context(st);
    auto res = ast.eval(context);
    BOOST_CHECK(res);

    // Even though condition as a whole matches, there is no finite set of wells
    // which mathes both conditions when combined with AND - i.e. the matching_wells
    // variable should be empty.
    BOOST_CHECK( res.wells().empty() );
}

BOOST_AUTO_TEST_CASE(TestMatchingWells_OR) {
    Action::AST ast({"WOPR", "*", ">", "1.0", "OR", "WWCT", "*", "<", "0.50"});
    SummaryState st(std::chrono::system_clock::now());

    st.update_well_var("OPX", "WOPR", 0);
    st.update_well_var("OPY", "WOPR", 0.50);
    st.update_well_var("OPZ", "WOPR", 2.0);      // The WOPR check matches this well.

    st.update_well_var("OPX", "WWCT", 1.0);
    st.update_well_var("OPY", "WWCT", 0.0);     // The WWCT check matches this well.
    st.update_well_var("OPZ", "WWCT", 1.0);

    Action::Context context(st);
    auto res = ast.eval(context);
    auto wells = res.wells();
    BOOST_CHECK(res);

    // The well 'OPZ' matches the first condition and the well 'OPY' matches the
    // second condition, since the two conditions are combined with || the
    // resulting mathcing_wells variable should contain both these wells.
    BOOST_CHECK_EQUAL( wells.size(), 2);
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "OPZ") != wells.end());
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "OPY") != wells.end());
}

BOOST_AUTO_TEST_CASE(TestFieldAND) {
    Action::AST ast({"FMWPR", ">=", "4", "AND", "WUPR3", "OP*", "=", "1"});
    SummaryState st(std::chrono::system_clock::now());
    Action::Context context(st);

    st.update_well_var("OP1", "WUPR3", 3);
    st.update_well_var("OP2", "WUPR3", 2);
    st.update_well_var("OP3", "WUPR3", 1);
    st.update_well_var("OP4", "WUPR3", 4);

    st.update("FMWPR", 1);
    {
        auto res = ast.eval(context);
        auto wells = res.wells();
        BOOST_CHECK(!res);
    }

    st.update("FMWPR", 4);
    {
        auto res = ast.eval(context);
        auto wells = res.wells();
        BOOST_CHECK(res);
        BOOST_CHECK_EQUAL(wells.size(), 1);
        BOOST_CHECK_EQUAL(wells[0], "OP3");
    }
}


BOOST_AUTO_TEST_CASE(Conditions) {
    auto location = Location("File", 100);

    // Missing comparator
    BOOST_CHECK_THROW(Action::Condition cond({"WWCT", "OPX"}, location), std::invalid_argument);

    // Missing right hand side
    BOOST_CHECK_THROW(Action::Condition cond({"WWCT", "OPX", ">"}, location), std::invalid_argument);

    Action::Condition cond({"WWCT", "OPX", ">", "0.75",  "AND"}, location);
    BOOST_CHECK(cond.cmp == Action::Condition::Comparator::GREATER);
    BOOST_CHECK(cond.cmp_string == ">" );
    BOOST_CHECK_EQUAL(cond.lhs.quantity, "WWCT");
    BOOST_CHECK_EQUAL(cond.lhs.args.size(), 1);
    BOOST_CHECK_EQUAL(cond.lhs.args[0], "OPX");

    BOOST_CHECK_EQUAL(cond.rhs.quantity, "0.75");
    BOOST_CHECK_EQUAL(cond.rhs.args.size(), 0);
    BOOST_CHECK(cond.logic == Action::Condition::Logical::AND);

    Action::Condition cond2({"WWCT", "OPX", "<=", "WSOPR", "OPX", "235"}, location);
    BOOST_CHECK(cond2.cmp == Action::Condition::Comparator::LESS_EQUAL);
    BOOST_CHECK(cond2.cmp_string == "<=" );
    BOOST_CHECK_EQUAL(cond2.lhs.quantity, "WWCT");
    BOOST_CHECK_EQUAL(cond2.lhs.args.size(), 1);
    BOOST_CHECK_EQUAL(cond2.lhs.args[0], "OPX");

    BOOST_CHECK_EQUAL(cond2.rhs.quantity, "WSOPR");
    BOOST_CHECK_EQUAL(cond2.rhs.args.size(), 2);
    BOOST_CHECK_EQUAL(cond2.rhs.args[0], "OPX");
    BOOST_CHECK_EQUAL(cond2.rhs.args[1], "235");
    BOOST_CHECK(cond2.logic == Action::Condition::Logical::END);
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
    const auto& actions0 = sched.actions(0);
    BOOST_CHECK_EQUAL(actions0.size(), 0);

    const auto& actions1 = sched.actions(1);
    BOOST_CHECK_EQUAL(actions1.size(), 1);


    const auto& act1 = actions1.get("B");
    const auto& strings = act1.keyword_strings();
    BOOST_CHECK_EQUAL(strings.size(), 4);
    BOOST_CHECK_EQUAL(strings.back(), "ENDACTIO");


    std::string rdeck_string = "";
    for (std::size_t i = 0; i < strings.size(); i++)
        rdeck_string += strings[i] + "\n";

    auto deck2 = parser.parseString(rdeck_string);
    BOOST_CHECK(deck2.getKeyword("WELSPECS") == deck.getKeyword("WELSPECS"));


    const auto& conditions = act1.conditions();
    BOOST_CHECK_EQUAL(conditions.size() , 2);

    const auto& cond0 = conditions[0];
    BOOST_CHECK_EQUAL(cond0.lhs.quantity, "WWCT");
    BOOST_CHECK(cond0.cmp == Action::Condition::Comparator::GREATER);
    BOOST_CHECK(cond0.logic == Action::Condition::Logical::AND);
    BOOST_CHECK_EQUAL(cond0.lhs.args.size(), 1);
    BOOST_CHECK_EQUAL(cond0.lhs.args[0], "OPX");
    BOOST_CHECK_EQUAL(cond0.rhs.args.size(), 0);
    BOOST_CHECK_EQUAL(cond0.rhs.quantity, "0.75");

    const auto& cond1 = conditions[1];
    BOOST_CHECK_EQUAL(cond1.lhs.quantity, "FPR");
    BOOST_CHECK(cond1.cmp == Action::Condition::Comparator::LESS);
    BOOST_CHECK(cond1.logic == Action::Condition::Logical::END);

    /*****************************************************************/

    const auto& actions2 = sched.actions(2);
    BOOST_CHECK_EQUAL(actions2.size(), 2);

    const auto& actB = actions2.get("B");
    const auto& condB = actB.conditions();
    BOOST_CHECK_EQUAL(condB.size() , 1);
    BOOST_CHECK_EQUAL(condB[0].lhs.quantity, "FWCT");
    BOOST_CHECK(condB[0].cmp == Action::Condition::Comparator::LESS_EQUAL);
    BOOST_CHECK(condB[0].logic == Action::Condition::Logical::END);
    BOOST_CHECK_EQUAL(condB[0].cmp_string, "<=");

    const auto& actA = actions2.get("A");
    const auto& condA = actA.conditions();
    BOOST_CHECK_EQUAL(condA.size() , 1);
    BOOST_CHECK_EQUAL(condA[0].lhs.quantity, "WOPR");
    BOOST_CHECK(condA[0].cmp == Action::Condition::Comparator::EQUAL);
    BOOST_CHECK(condA[0].logic == Action::Condition::Logical::END);
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



BOOST_AUTO_TEST_CASE(ACTIONRESULT_COPY_EMPTY) {
    Action::Result res1(false);
    auto res2 = res1;

    BOOST_CHECK(!res1);
    BOOST_CHECK(!res2);
    BOOST_CHECK(res1.wells() == std::vector<std::string>());
    BOOST_CHECK(res2.wells() == std::vector<std::string>());

    BOOST_CHECK(!res1.has_well("NO"));
    BOOST_CHECK(!res2.has_well("NO"));
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
