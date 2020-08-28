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

#define BOOST_TEST_MODULE UDQTests
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Utility/Typetools.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQContext.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQAssign.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunction.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(TYPE_COERCION) {
    BOOST_CHECK( UDQVarType::SCALAR == UDQ::coerce(UDQVarType::SCALAR, UDQVarType::SCALAR) );

    BOOST_CHECK( UDQVarType::WELL_VAR == UDQ::coerce(UDQVarType::SCALAR, UDQVarType::WELL_VAR));
    BOOST_CHECK( UDQVarType::WELL_VAR == UDQ::coerce(UDQVarType::WELL_VAR, UDQVarType::FIELD_VAR));

    BOOST_CHECK( UDQVarType::GROUP_VAR == UDQ::coerce(UDQVarType::SCALAR, UDQVarType::GROUP_VAR));
    BOOST_CHECK( UDQVarType::GROUP_VAR == UDQ::coerce(UDQVarType::GROUP_VAR, UDQVarType::FIELD_VAR));

    BOOST_CHECK_THROW( UDQ::coerce(UDQVarType::GROUP_VAR, UDQVarType::WELL_VAR), std::logic_error );
    BOOST_CHECK_THROW( UDQ::coerce(UDQVarType::WELL_VAR, UDQVarType::GROUP_VAR), std::logic_error );
}


BOOST_AUTO_TEST_CASE(GROUP_VARIABLES)
{
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def_group(udqp, "GUOPRL",{"(", "5000",  "-",  "GOPR",  "LOWER",  "*", "0.13",  "-",  "GOPR",  "UPPER",  "*", "0.15", ")" , "*",  "0.89"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);
    double gopr_lower = 1234;
    double gopr_upper = 4321;

    st.update_group_var("LOWER", "GOPR", gopr_lower);
    st.update_group_var("UPPER", "GOPR", gopr_upper);

    auto res_group = def_group.eval(context);
    BOOST_CHECK_EQUAL( res_group["UPPER"].value(), (5000 - gopr_lower*0.13 - gopr_upper*0.15)*0.89);
    BOOST_CHECK_EQUAL( res_group["UPPER"].value(), (5000 - gopr_lower*0.13 - gopr_upper*0.15)*0.89);
}



BOOST_AUTO_TEST_CASE(SUBTRACT)
{
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def(udqp, "WU", {"16", "-", "8", "-", "4", "-", "2", "-", "1"});
    UDQDefine scalar(udqp, "WU", {"16"});

    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 4);
    auto res = def.eval(context);
    BOOST_CHECK_EQUAL( res[0].value(), 1.0);

    auto res2 = scalar.eval(context);
    BOOST_CHECK_EQUAL( res2[0].value(), 16.0);
}


BOOST_AUTO_TEST_CASE(TEST)
{
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def1(udqp, "WUWI3", {"GOPR" , "MAU", "*", "2.0", "*", "0.25", "*", "10"});
    UDQDefine def2(udqp, "WUWI3", {"2.0", "*", "0.25", "*", "3"});
    UDQDefine def3(udqp, "WUWI3", {"GOPR" , "FIELD", "-", "2.0", "*", "3"});
    UDQDefine def4(udqp, "WUWI3", {"FOPR" , "/",  "2"});

    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_group_var("MAU", "GOPR", 4);
    st.update_group_var("XXX", "GOPR", 5);
    st.update_group_var("FIELD", "GOPR", 6);
    st.update_well_var("P1", "WBHP", 0.5);
    st.update_well_var("P2", "WBHP", 0.5);
    st.update("FOPR", 2);

    auto res1 = def1.eval(context);
    BOOST_CHECK_EQUAL( res1["P1"].value(), 20 );
    BOOST_CHECK_EQUAL( res1["P2"].value(), 20 );

    auto res2 = def2.eval(context);
    BOOST_CHECK_EQUAL( res2["P1"].value(), 1.50 );
    BOOST_CHECK_EQUAL( res2["P2"].value(), 1.50 );

    auto res3 = def3.eval(context);
    BOOST_CHECK_EQUAL( res3["P1"].value(), 0.00 );
    BOOST_CHECK_EQUAL( res3["P2"].value(), 0.00 );

    auto res4 = def4.eval(context);
    BOOST_CHECK_EQUAL( res4["P1"].value(), 1.00 );
    BOOST_CHECK_EQUAL( res4["P2"].value(), 1.00 );

    /*
      This expression has a well set as target type, and involves group with
      wildcard that is not supported by flow.
    */
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUWI2", {"GOPR", "G*", "*", "2.0"}), std::logic_error);

    /*
      UDQVarType == BLOCK is not yet supported.
    */
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUWI2", {"BPR", "1","1", "1", "*", "2.0"}), std::logic_error);
}

Schedule make_schedule(const std::string& input) {
    Parser parser;
    auto python = std::make_shared<Python>();

    auto deck = parser.parseString(input);
    if (deck.hasKeyword("DIMENS")) {
        EclipseState es(deck);
        return Schedule(deck, es, python);
    } else {
        EclipseGrid grid(10,10,10);
        TableManager table ( deck );
        FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
        Runspec runspec (deck);
        return Schedule(deck, grid , fp, runspec, python);
    }
}


BOOST_AUTO_TEST_CASE(MIX_SCALAR) {
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_add(udqp, "WU", {"WOPR", "+", "1"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 1);

    auto res_add = def_add.eval(context);
    BOOST_CHECK_EQUAL( res_add["P1"].value() , 2);
}


BOOST_AUTO_TEST_CASE(UDQ_TABLE_EXCEPTION) {
    UDQParams udqp;
    BOOST_CHECK_THROW(UDQDefine(udqp, "WU", {"TUPRICE[WOPR]"}), std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(UDQFieldSetTest) {
    std::vector<std::string> wells = {"P1", "P2", "P3", "P4"};
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 1.0);
    st.update_well_var("P2", "WOPR", 2.0);
    st.update_well_var("P3", "WOPR", 3.0);
    st.update_well_var("P4", "WOPR", 4.0);

    /*{
        UDQDefine def_fxxx(udqp, "FU_SCALAR", {"123"});
        auto fxxx_res = def_fxxx.eval(context);
        BOOST_CHECK_EQUAL( fxxx_res[0].value(), 123.0 );
        BOOST_CHECK( fxxx_res.var_type() == UDQVarType::FIELD_VAR);
    }
    */

    {
        UDQDefine def_fopr(udqp, "FUOPR", {"SUM", "(", "WOPR", ")"});
        auto fopr_res = def_fopr.eval(context);
        BOOST_CHECK_EQUAL( fopr_res[0].value(), 10.0 );
    }
}


BOOST_AUTO_TEST_CASE(UDQWellSetTest) {
    std::vector<std::string> wells = {"P1", "P2", "I1", "I2"};
    UDQSet ws = UDQSet::wells("NAME", wells);
    UDQSet ws2 = UDQSet::wells("NAME", wells, 100.0);

    BOOST_CHECK_EQUAL(4, ws.size());
    ws.assign("P1", 1.0);

    const auto& value = ws["P1"];
    BOOST_CHECK_EQUAL(value.value(), 1.0);
    BOOST_CHECK_EQUAL(ws["P1"].value(), 1.0);

    BOOST_REQUIRE_THROW(ws.assign("NO_SUCH_WELL", 1.0), std::out_of_range);
    BOOST_REQUIRE_THROW(ws[10], std::out_of_range);
    BOOST_REQUIRE_THROW(ws["NO_SUCH_WELL"], std::out_of_range);

    ws.assign("*", 2.0);
    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws[w].value(), 2.0);

    ws.assign(3.0);
    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws[w].value(), 3.0);

    ws.assign("P*", 4.0);
    BOOST_CHECK_EQUAL(ws["P1"].value(), 4.0);
    BOOST_CHECK_EQUAL(ws["P2"].value(), 4.0);

    ws.assign("I2", 5.0);
    BOOST_CHECK_EQUAL(ws["I2"].value(), 5.0);


    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws2[w].value(), 100.0);

    UDQSet scalar = UDQSet::scalar("NAME", 1.0);
    BOOST_CHECK_EQUAL(scalar.size() , 1);
    BOOST_CHECK_EQUAL(scalar[0].value(), 1.0);

    UDQSet empty = UDQSet::empty("EMPTY");
    BOOST_CHECK_EQUAL(empty.size() , 0);
}


BOOST_AUTO_TEST_CASE(UDQ_GROUP_TEST) {
    std::vector<std::string> groups = {"G1", "G2", "G3", "G4"};
    UDQSet gs = UDQSet::groups("NAME", groups);

    BOOST_CHECK_EQUAL(4, gs.size());
    gs.assign("G1", 1.0);

    const auto& value = gs["G1"];
    BOOST_CHECK_EQUAL(value.value(), 1.0);
    {
        UDQParams udqp;
        UDQFunctionTable udqft(udqp);
        UDQDefine def_fopr(udqp, "FUOPR", {"SUM", "(", "GOPR", ")"});
        SummaryState st(std::chrono::system_clock::now());
        UDQContext context(udqft, st);

        st.update_group_var("G1", "GOPR", 1.0);
        st.update_group_var("G2", "GOPR", 2.0);
        st.update_group_var("G3", "GOPR", 3.0);
        st.update_group_var("G4", "GOPR", 4.0);


        auto res = def_fopr.eval(context);
        BOOST_CHECK_EQUAL(res[0].value(), 10.0);
    }
}



BOOST_AUTO_TEST_CASE(UDQ_DEFINETEST) {
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    {
        UDQDefine def(udqp, "WUBHP", {"WBHP"});
        SummaryState st(std::chrono::system_clock::now());
        UDQContext context(udqft, st);

        st.update_well_var("W1", "WBHP", 11);
        st.update_well_var("W2", "WBHP", 2);
        st.update_well_var("W3", "WBHP", 3);
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(res.size(), 3);
        BOOST_CHECK_EQUAL( res["W1"].value(), 11 );
        BOOST_CHECK_EQUAL( res["W2"].value(), 2 );
        BOOST_CHECK_EQUAL( res["W3"].value(), 3 );
    }
    {
        UDQDefine def(udqp, "WUBHP", {"WBHP" , "'P*'"});
        SummaryState st(std::chrono::system_clock::now());
        UDQContext context(udqft, st);


        st.update_well_var("P1", "WBHP", 1);
        st.update_well_var("P2", "WBHP", 2);
        st.update_well_var("I1", "WBHP", 1);
        st.update_well_var("I2", "WBHP", 2);
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(res.size(), 4);
        BOOST_CHECK_EQUAL( res["P1"].value(), 1 );
        BOOST_CHECK_EQUAL( res["P2"].value(), 2 );
        BOOST_CHECK_EQUAL( res["I1"].defined(), false);
        BOOST_CHECK_EQUAL( res["I1"].defined(), false);
    }
    {
        UDQDefine def(udqp, "WUBHP", {"NINT" , "(", "WBHP", ")"});
        SummaryState st(std::chrono::system_clock::now());
        UDQContext context(udqft, st);
        st.update_well_var("P1", "WBHP", 4);
        st.update_well_var("P2", "WBHP", 3);
        st.update_well_var("I1", "WBHP", 2);
        st.update_well_var("I2", "WBHP", 1);

        auto res = def.eval(context);
        BOOST_CHECK_EQUAL( res["P1"].value(), 4 );
        BOOST_CHECK_EQUAL( res["P2"].value(), 3 );
        BOOST_CHECK_EQUAL( res["I1"].value(), 2 );
        BOOST_CHECK_EQUAL( res["I2"].value(), 1 );
    }
}





BOOST_AUTO_TEST_CASE(KEYWORDS) {
    const std::string input = R"(
RUNSPEC
UDQDIMS
   10* 'N'/
UDQPARAM
  3* 0.25 /
)";
    Parser parser;

    auto deck = parser.parseString(input);
    auto runspec = Runspec(deck);
    auto udq_params = runspec.udqParams();

    BOOST_CHECK_EQUAL(0.25, udq_params.cmpEpsilon());

    // The reseed parameter is set to false, so the repeated calls to reseedRNG() should have
    // no effect.

    udq_params.reseedRNG(100);
    auto r1 = udq_params.true_rng()();
    udq_params.reseedRNG(100);
    auto r2 = udq_params.true_rng()();

    BOOST_CHECK( r1 != r2 );
}


BOOST_AUTO_TEST_CASE(ENUM_CONVERSION) {
    BOOST_CHECK_THROW(UDQ::varType("WWCT"), std::invalid_argument);
    BOOST_CHECK_THROW(UDQ::varType("XUCT"), std::invalid_argument);

    BOOST_CHECK(UDQ::varType("WUBHP") == UDQVarType::WELL_VAR);
    BOOST_CHECK(UDQ::varType("GUBHP") == UDQVarType::GROUP_VAR);
    BOOST_CHECK(UDQ::varType("CUBHP") == UDQVarType::CONNECTION_VAR);
    BOOST_CHECK(UDQ::varType("FUBHP") == UDQVarType::FIELD_VAR);
    BOOST_CHECK(UDQ::varType("RUBHP") == UDQVarType::REGION_VAR);
    BOOST_CHECK(UDQ::varType("AUBHP") == UDQVarType::AQUIFER_VAR);
    BOOST_CHECK(UDQ::varType("SUBHP") == UDQVarType::SEGMENT_VAR);

    BOOST_CHECK(UDQ::targetType("WBHP") == UDQVarType::WELL_VAR);
    BOOST_CHECK(UDQ::targetType("GBHP") == UDQVarType::GROUP_VAR);
    BOOST_CHECK(UDQ::targetType("CBHP") == UDQVarType::CONNECTION_VAR);
    BOOST_CHECK(UDQ::targetType("FBHP") == UDQVarType::FIELD_VAR);
    BOOST_CHECK(UDQ::targetType("RBHP") == UDQVarType::REGION_VAR);
    BOOST_CHECK(UDQ::targetType("ABHP") == UDQVarType::AQUIFER_VAR);
    BOOST_CHECK(UDQ::targetType("SBHP") == UDQVarType::SEGMENT_VAR);

    BOOST_REQUIRE_THROW( UDQ::actionType("INVALID_ACTION"), std::invalid_argument);
    BOOST_CHECK(UDQ::actionType("DEFINE") == UDQAction::DEFINE );
    BOOST_CHECK(UDQ::actionType("UNITS") == UDQAction::UNITS );
    BOOST_CHECK(UDQ::actionType("ASSIGN") == UDQAction::ASSIGN );
}


BOOST_AUTO_TEST_CASE(UDQ_KEWYORDS) {
    const std::string input = R"(
RUNSPEC
UDQDIMS
   10* 'Y'/
UDQPARAM
  3* 0.25 /
SCHEDULE
UDQ
  ASSIGN WUBHP 0.0 /
  UNITS  WUBHP 'BARSA' /
  DEFINE FUOPR  AVEG(WOPR) + 1/
  ASSIGN WUXUNIT 0.0 /
  DEFINE FUOPR  AVEG(WOPR)/
/
DATES
  10 'JAN' 2010 /
/
UDQ
  ASSIGN WUBHP 0.0 /
  DEFINE FUOPR  AVEG(WOPR)/
  UNITS  WUBHP 'BARSA' /  -- Repeating the same unit multiple times is superfluous but OK
/
)";

    auto schedule = make_schedule(input);
    const auto& udq = schedule.getUDQConfig(0);
    BOOST_CHECK_EQUAL(2, udq.assignments().size());

    BOOST_CHECK_THROW( udq.unit("NO_SUCH_KEY"), std::invalid_argument );
    BOOST_CHECK_EQUAL( udq.unit("WUBHP"), "BARSA");
    BOOST_CHECK( udq.has_keyword("WUBHP") );
    BOOST_CHECK( !udq.has_keyword("NO_SUCH_KEY") );
    BOOST_CHECK( !udq.has_unit("WUXUNIT"));
    BOOST_CHECK( udq.has_unit("WUBHP"));

    Parser parser;
    auto deck = parser.parseString(input);
    auto udq_params1 = UDQParams(deck);
    BOOST_CHECK_EQUAL(0.25, udq_params1.cmpEpsilon());
    auto& sim_rng1 = udq_params1.sim_rng();
    auto& true_rng1 = udq_params1.true_rng();

    auto udq_params2 = UDQParams(deck);
    auto& sim_rng2 = udq_params2.sim_rng();
    auto& true_rng2 = udq_params2.true_rng();

    BOOST_CHECK( sim_rng1() == sim_rng2() );
    BOOST_CHECK( true_rng1() != true_rng2() );

    udq_params1.reseedRNG(100);
    udq_params2.reseedRNG(100);
    BOOST_CHECK( true_rng1() == true_rng2() );
}

BOOST_AUTO_TEST_CASE(UDQ_CHANGE_UNITS_ILLEGAL) {
  const std::string input = R"(
RUNSPEC
UDQDIMS
   10* 'Y'/
UDQPARAM
  3* 0.25 /
SCHEDULE
UDQ
  ASSIGN WUBHP 0.0 /
  UNITS  WUBHP 'BARSA' /
  DEFINE FUOPR  AVEG(WOPR) + 1/
/
DATES
  10 'JAN' 2010 /
/
UDQ
  ASSIGN WUBHP 0.0 /
  DEFINE FUOPR  AVEG(WOPR) + 1/
  UNITS  WUBHP 'HOURS' /  -- Changing unit runtime is *not* supported
/
)";

  BOOST_CHECK_THROW( make_schedule(input), std::invalid_argument);
}





BOOST_AUTO_TEST_CASE(UDQ_DEFINE_WITH_SLASH) {
    const std::string input = R"(
UDQ
 DEFINE WUWCT WWPR / ( WWPR + WOPR ) /
/
)";
    Parser parser;
    auto deck = parser.parseString(input);
    const auto& udq = deck.getKeyword("UDQ");
    const auto& record = udq.getRecord(0);
    const auto& data_item = record.getItem("DATA");
    const auto& data = RawString::strings( data_item.getData<RawString>() );
    std::vector<std::string> exp = {"WWPR", "/", "(", "WWPR", "+", "WOPR", ")"};
    BOOST_CHECK_EQUAL_COLLECTIONS(data.begin(), data.end(),
                                  exp.begin(), exp.end());
}


BOOST_AUTO_TEST_CASE(UDQ_ASSIGN_DATA) {
    const std::string input = R"(
RUNSPEC
UDQDIMS
   10* 'Y'/
UDQPARAM
  3* 0.25 /
SCHEDULE
UDQ
ASSIGN WU1 P12 4.0 /
ASSIGN WU2 8.0 /
/
)";
    const auto schedule = make_schedule(input);
    const auto& udq = schedule.getUDQConfig(0);
    const auto& assignments = udq.assignments();
    const auto& ass0 = assignments[0];
    const auto& ass1 = assignments[1];
    auto w1 = ass0.eval({"P1", "P2", "P12"});
    auto w2 = ass1.eval({"P1", "P2", "P12"});
    BOOST_CHECK_EQUAL(w1.name(), "WU1");
    BOOST_CHECK_EQUAL(w2.name(), "WU2");

    BOOST_CHECK_EQUAL( w1["P12"].value(), 4.0 );
    BOOST_CHECK_EQUAL( w1["P1"].defined(), false );
    BOOST_CHECK_EQUAL( w1["P2"].defined(), false );

    BOOST_CHECK_EQUAL( w2["P12"].value(), 8.0 );
    BOOST_CHECK_EQUAL( w2["P1"].value(), 8.0 );
    BOOST_CHECK_EQUAL( w2["P2"].value(), 8.0 );
}




BOOST_AUTO_TEST_CASE(UDQ_CONTEXT) {
    SummaryState st(std::chrono::system_clock::now());
    UDQFunctionTable func_table;
    UDQParams udqp;
    UDQContext ctx(func_table, st);
    BOOST_CHECK_EQUAL(ctx.get("JAN"), 1.0);

    BOOST_REQUIRE_THROW(ctx.get("NO_SUCH_KEY"), std::out_of_range);

    for (std::string& key : std::vector<std::string>({"ELAPSED", "MSUMLINS", "MSUMNEWT", "NEWTON", "TCPU", "TIME", "TIMESTEP"}))
        BOOST_CHECK_NO_THROW( ctx.get(key) );

    st.update("SUMMARY:KEY", 1.0);
    BOOST_CHECK_EQUAL(ctx.get("SUMMARY:KEY") , 1.0 );
}

BOOST_AUTO_TEST_CASE(UDQ_SET) {
    UDQSet s1("NAME", 5);

    for (const auto& v : s1) {
        BOOST_CHECK_EQUAL(false, v.defined());
        BOOST_REQUIRE_THROW( v.value(), std::invalid_argument);
    }
    BOOST_CHECK_EQUAL(s1.defined_size(), 0);

    s1.assign(1);
    for (const auto& v : s1) {
        BOOST_CHECK_EQUAL(true, v.defined());
        BOOST_CHECK_EQUAL( v.value(), 1.0);
    }
    BOOST_CHECK_EQUAL(s1.defined_size(), s1.size());

    s1.assign(0,0.0);
    {
        UDQSet s2("NAME", 6);
        BOOST_REQUIRE_THROW(s1 + s2, std::logic_error);
    }
    {
        UDQSet s2("NAME", 5);
        s2.assign(0, 25);
        auto s3 = s1 + s2;

        auto v0 = s3[0];
        BOOST_CHECK_EQUAL(v0.value(), 25);

        auto v4 = s3[4];
        BOOST_CHECK( !v4.defined() );
    }
    s1.assign(0,1.0);
    {
        UDQSet s2 = s1 + 1.0;
        UDQSet s3 = s2 * 2.0;
        UDQSet s4 = s1 - 1.0;
        for (const auto& v : s2) {
            BOOST_CHECK_EQUAL(true, v.defined());
            BOOST_CHECK_EQUAL( v.value(), 2.0);
        }

        for (const auto& v : s3) {
            BOOST_CHECK_EQUAL(true, v.defined());
            BOOST_CHECK_EQUAL( v.value(), 4.0);
        }

        for (const auto& v : s4) {
            BOOST_CHECK_EQUAL(true, v.defined());
            BOOST_CHECK_EQUAL( v.value(), 0);
        }
    }
}


BOOST_AUTO_TEST_CASE(UDQ_FUNCTION_TABLE) {
    UDQFunctionTable udqft;
    BOOST_CHECK(udqft.has_function("SUM"));
    BOOST_CHECK(!udqft.has_function("NO_SUCH_FUNCTION"));
    UDQSet arg("NAME", 5);
    arg.assign(0,1);
    arg.assign(2,2);
    arg.assign(4,4);
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("SUM"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 7);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORM1"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 7);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORM2"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), std::sqrt(1 + 4+ 16));
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORMI"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("MIN"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("MAX"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEA"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 7.0/3);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEG"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), std::exp((std::log(1) + std::log(2.0) + std::log(4))/3));
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("PROD"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].value(), 8.0);
    }
    {
        UDQSet arg2("NAME", 4);
        arg2.assign(0,1);
        arg2.assign(2,4);
        arg2.assign(3,4);
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEH"));
        auto result = func.eval(arg2);
        BOOST_CHECK_EQUAL(result[0].value(), 2.0);
    }
}

BOOST_AUTO_TEST_CASE(CMP_FUNCTIONS) {
    UDQFunctionTable udqft;
    UDQSet arg1("NAME", 5);
    UDQSet arg2("NAME", 5);
    UDQSet arg3("NAME", 3);
    arg1.assign(1,1);

    arg1.assign(0,1);
    arg1.assign(2,2);
    arg1.assign(4,4);

    arg2.assign(0, 0.9);
    arg2.assign(2, 2.5);
    arg2.assign(4, 4.0);

    BOOST_CHECK_THROW(UDQBinaryFunction::EQ(0.25, arg1, arg3), std::logic_error);
    {
        auto result = UDQBinaryFunction::EQ(0, arg1, arg2);

        BOOST_CHECK_EQUAL( result.defined_size(), 3 );
        BOOST_CHECK_EQUAL( result[0].value(), 0);
        BOOST_CHECK_EQUAL( result[2].value(), 0);
        BOOST_CHECK_EQUAL( result[4].value(), 1);

        result = UDQBinaryFunction::EQ(0.20, arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK_EQUAL( result[2].value(), 0);
        BOOST_CHECK_EQUAL( result[4].value(), 1);

        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("=="));
        result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].value(), 0);
        BOOST_CHECK_EQUAL( result[2].value(), 0);
        BOOST_CHECK_EQUAL( result[4].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("<"));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result.defined_size(), 3 );
        BOOST_CHECK_EQUAL( result[0].value(), 0);
        BOOST_CHECK_EQUAL( result[2].value(), 1);
        BOOST_CHECK_EQUAL( result[4].value(), 0);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get(">"));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result.defined_size(), 3 );
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK_EQUAL( result[2].value(), 0);
        BOOST_CHECK_EQUAL( result[4].value(), 0);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("^"));
        UDQSet arg1_local("NAME", 4);
        UDQSet arg2_local("NAME", 4);

        for (std::size_t i=0; i < arg1_local.size(); i++) {
            arg1_local.assign(i, i + 1);
            arg2_local.assign(i, 2);
        }
        auto result = func.eval(arg1_local, arg2_local);
        for (std::size_t i=0; i < arg1_local.size(); i++)
            BOOST_CHECK_EQUAL( result[i].value(), (i+1)*(i+1));
    }
    {
        auto result = UDQBinaryFunction::GE(1.0, arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].value(), 1);

        // This is bisarre - but due to the large epsilon 2 and 2.5 compare as
        // equal; and then we evaluate 2 >= 2.5 as TRUE!
        BOOST_CHECK_EQUAL( result[2].value(), 1);
        BOOST_CHECK_EQUAL( result[4].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("<="));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].value(), 0);
        BOOST_CHECK_EQUAL( result[2].value(), 1);
        BOOST_CHECK_EQUAL( result[4].value(), 1);


    }
}



BOOST_AUTO_TEST_CASE(ELEMENTAL_UNARY_FUNCTIONS) {
    UDQFunctionTable udqft;
    UDQSet arg("NAME", 5);
    arg.assign(0,1);
    arg.assign(2,2);
    arg.assign(4,4);

    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("ABS"));
        UDQSet arg2("NAME", 5);
        arg2.assign(0,1);
        arg2.assign(2,-2);
        arg2.assign(4,4);
        auto result = func.eval(arg2);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK_EQUAL( result[2].value(), 2);
        BOOST_CHECK_EQUAL( result[4].value(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("DEF"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK_EQUAL( result[2].value(), 1);
        BOOST_CHECK_EQUAL( result[4].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("UNDEF"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[1].value(), 1);
        BOOST_CHECK_EQUAL( result[3].value(), 1);
        BOOST_CHECK_EQUAL( result.defined_size(), 2);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("EXP"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].value(), std::exp(1));
        BOOST_CHECK_EQUAL( result[2].value(), std::exp(2));
        BOOST_CHECK_EQUAL( result[4].value(), std::exp(4));
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("IDV"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK_EQUAL( result[1].value(), 0);
        BOOST_CHECK_EQUAL( result[2].value(), 1);
        BOOST_CHECK_EQUAL( result[3].value(), 0);
        BOOST_CHECK_EQUAL( result[4].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("LOG"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, 10);
        arg_local.assign(2,1000);

        auto result = func.eval(arg_local);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].value(), 3);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("NINT"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, 0.75);
        arg_local.assign(2, 1.25);

        auto result = func.eval(arg_local);
        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].value(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("RANDN"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, -1.0);
        arg_local.assign(2, -1.0);

        auto result1 = func.eval(arg_local);
        auto result2 = func.eval(arg_local);
        BOOST_CHECK( result1[0].value() != -1.0);
        BOOST_CHECK( !result1[1] );
        BOOST_CHECK( result1[2].value() != -1.0);

        BOOST_CHECK( result1[0].value() != result2[0].value());
        BOOST_CHECK( result1[2].value() != result2[2].value());
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("SORTA"));
        auto result = func.eval(arg);

        BOOST_CHECK_EQUAL( result[0].value(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].value(), 2);
        BOOST_CHECK( !result[3] );
        BOOST_CHECK_EQUAL( result[4].value(), 3);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("SORTD"));
        auto result = func.eval(arg);

        BOOST_CHECK_EQUAL( result[0].value(), 3);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].value(), 2);
        BOOST_CHECK( !result[3] );
        BOOST_CHECK_EQUAL( result[4].value(), 1);
    }
}


BOOST_AUTO_TEST_CASE(UNION_FUNCTIONS) {
    UDQFunctionTable udqft;
    UDQSet arg1("NAME", 5);
    UDQSet arg2("NAME", 5);

    arg1.assign(0,1);
    arg1.assign(2,2);

    arg2.assign(0, 1.0);
    arg2.assign(3, 3 );

    const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("UADD"));
    auto result = func.eval(arg1, arg2);
    BOOST_CHECK_EQUAL( 3, result.defined_size() );
    BOOST_CHECK_EQUAL( 2, result[0].value() );
    BOOST_CHECK_EQUAL( 2, result[2].value() );
    BOOST_CHECK_EQUAL( 3, result[3].value() );
}




BOOST_AUTO_TEST_CASE(FUNCTIONS_INVALID_ARGUMENT) {
    UDQSet arg("NAME",3);
    arg.assign(0, -1);
    BOOST_REQUIRE_THROW( UDQScalarFunction::AVEG(arg), std::invalid_argument);
    BOOST_REQUIRE_THROW( UDQUnaryElementalFunction::LOG(arg), std::invalid_argument);
    BOOST_REQUIRE_THROW( UDQUnaryElementalFunction::LN(arg), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(UDQ_SET_DIV) {
    UDQSet s("NAME", 5);
    s.assign(0,1);
    s.assign(2,2);
    s.assign(4,5);

    auto result = 10 / s;
    BOOST_CHECK_EQUAL( result.defined_size(), 3);
    BOOST_CHECK_EQUAL( result[0].value(), 10);
    BOOST_CHECK_EQUAL( result[2].value(), 5);
    BOOST_CHECK_EQUAL( result[4].value(), 2);
}



BOOST_AUTO_TEST_CASE(UDQASSIGN_TEST) {
    UDQAssign as1("WUPR", {}, 1.0);
    UDQAssign as2("WUPR", {"P*"}, 2.0);
    UDQAssign as3("WUPR", {"P1"}, 4.0);
    std::vector<std::string> ws1 = {"P1", "P2", "I1", "I2"};

    auto res1 = as1.eval(ws1);
    BOOST_CHECK_EQUAL(res1.size(), 4);
    BOOST_CHECK_EQUAL(res1["P1"].value(), 1.0);
    BOOST_CHECK_EQUAL(res1["I2"].value(), 1.0);

    auto res2 = as2.eval(ws1);
    BOOST_CHECK_EQUAL(res2["P1"].value(), 2.0);
    BOOST_CHECK_EQUAL(res2["P2"].value(), 2.0);
    BOOST_CHECK(!res2["I1"].defined());
    BOOST_CHECK(!res2["I2"].defined());

    auto res3 = as3.eval(ws1);
    BOOST_CHECK_EQUAL(res3["P1"].value(), 4.0);
    BOOST_CHECK(!res3["P2"].defined());
    BOOST_CHECK(!res3["I1"].defined());
    BOOST_CHECK(!res3["I2"].defined());
}

BOOST_AUTO_TEST_CASE(UDQ_POW_TEST) {
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_pow1(udqp, "WU", {"WOPR", "+", "WWPR", "*", "WGOR", "^", "WWIR"});
    UDQDefine def_pow2(udqp, "WU", {"(", "WOPR", "+", "WWPR", ")", "^", "(", "WOPR", "+" , "WGOR", "*", "WWIR", "-", "WOPT", ")"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P1", "WWPR", 2);
    st.update_well_var("P1", "WGOR", 3);
    st.update_well_var("P1", "WWIR", 4);
    st.update_well_var("P1", "WOPT", 7);

    auto res_pow1 = def_pow1.eval(context);
    auto res_pow2 = def_pow2.eval(context);
    BOOST_CHECK_EQUAL( res_pow1["P1"].value() , 1 + 2 * std::pow(3,4));
    BOOST_CHECK_EQUAL( res_pow2["P1"].value() , std::pow(1 + 2, 1 + 3*4 - 7));
}

BOOST_AUTO_TEST_CASE(UDQ_CMP_TEST) {
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_cmp(udqp, "WU", {"WOPR", ">", "WWPR", "+", "WGOR", "*", "WWIR"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR",  0);
    st.update_well_var("P1", "WWPR", 10);
    st.update_well_var("P1", "WGOR", -3);
    st.update_well_var("P1", "WWIR",  4);

    st.update_well_var("P2", "WOPR",  0);
    st.update_well_var("P2", "WWPR", -2);
    st.update_well_var("P2", "WGOR",  4);
    st.update_well_var("P2", "WWIR",  1);

    auto res_cmp = def_cmp.eval(context);
    BOOST_CHECK_EQUAL( res_cmp["P1"].value() , 1.0);
    BOOST_CHECK_EQUAL( res_cmp["P2"].value() , 0.0);
}

/*BOOST_AUTO_TEST_CASE(UDQPARSE_ERROR) {
    setUDQFunctionTable udqft;
    UDQDefine def1(udqft, "WUBHP", {"WWCT", "+"});
}
*/

BOOST_AUTO_TEST_CASE(UDQ_SCALAR_SET) {
    UDQParams udqp;
    UDQFunctionTable udqft;
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P2", "WOPR", 2);
    st.update_well_var("P3", "WOPR", 3);
    st.update_well_var("P4", "WOPR", 4);

    st.update_well_var("P1", "WWPR", 1);
    st.update_well_var("P2", "WWPR", 2);
    st.update_well_var("P3", "WWPR", 3);
    st.update_well_var("P4", "WWPR", 4);

    {
        UDQDefine def(udqp, "WUOPR", {"WOPR", "'*1'"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4, res.size());
        auto well1 = res["P1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.value() , 1);

        auto well2 = res["P2"];
        BOOST_CHECK( !well2.defined() );

        auto well4 = res["P4"];
        BOOST_CHECK( !well4.defined() );
    }
    {
        UDQDefine def(udqp, "WUOPR", {"1"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4, res.size());
        auto well1 = res["P1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.value() , 1);

        auto well2 = res["P2"];
        BOOST_CHECK( well2.defined() );
        BOOST_CHECK_EQUAL(well2.value() , 1);

        auto well4 = res["P4"];
        BOOST_CHECK( well4.defined() );
        BOOST_CHECK_EQUAL(well4.value() , 1);
    }
    {
        UDQDefine def(udqp, "WUOPR", {"WOPR", "'P1'"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4, res.size());
        auto well1 = res["P1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.value() , 1);

        auto well2 = res["P2"];
        BOOST_CHECK( well2.defined() );
        BOOST_CHECK_EQUAL(well2.value() , 1);

        auto well4 = res["P4"];
        BOOST_CHECK( well4.defined() );
        BOOST_CHECK_EQUAL(well4.value() , 1);
    }
}


BOOST_AUTO_TEST_CASE(UDQ_SORTA) {
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def1(udqp, "WUPR1" , {"1", "/", "(", "WWCT", "'OP*'", "+", "0.00001", ")"});
    UDQDefine def_sort(udqp , "WUPR3", {"SORTA", "(", "WUPR1", ")" });
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("OPL01", "WWCT", 0.7);
    st.update_well_var("OPL02", "WWCT", 0.8);
    st.update_well_var("OPU01", "WWCT", 0.0);
    st.update_well_var("OPU02", "WWCT", 0.0);

    auto res1 = def1.eval(context);
    st.update_well_var("OPL01", "WUPR1", res1["OPL01"].value());
    st.update_well_var("OPL02", "WUPR1", res1["OPL02"].value());
    st.update_well_var("OPU01", "WUPR1", res1["OPU01"].value());
    st.update_well_var("OPU02", "WUPR1", res1["OPU02"].value());

    auto res_sort = def_sort.eval(context);
    BOOST_CHECK_EQUAL(res_sort["OPL02"].value(), 1.0);
    BOOST_CHECK_EQUAL(res_sort["OPL01"].value(), 2.0);
    BOOST_CHECK_EQUAL(res_sort["OPU01"].value() + res_sort["OPU02"].value(), 7.0);
}



BOOST_AUTO_TEST_CASE(UDQ_BASIC_MATH_TEST) {
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def_add(udqp, "WU2OPR", {"WOPR", "+", "WOPR"});
    UDQDefine def_sub(udqp, "WU2OPR", {"WOPR", "-", "WOPR"});
    UDQDefine def_mul(udqp, "WU2OPR", {"WOPR", "*", "WOPR"});
    UDQDefine def_div(udqp, "WU2OPR", {"WOPR", "/", "WOPR"});
    UDQDefine def_muladd(udqp , "WUX", {"WOPR", "+", "WOPR", "*", "WOPR"});
    UDQDefine def_wuwct(udqp , "WUWCT", {"WWPR", "/", "(", "WOPR", "+", "WWPR", ")"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P2", "WOPR", 2);
    st.update_well_var("P3", "WOPR", 3);
    st.update_well_var("P4", "WOPR", 4);

    st.update_well_var("P1", "WWPR", 1);
    st.update_well_var("P2", "WWPR", 2);
    st.update_well_var("P3", "WWPR", 3);
    st.update_well_var("P4", "WWPR", 4);

    auto res_add = def_add.eval(context);
    BOOST_CHECK_EQUAL( res_add.size(), 4);
    BOOST_CHECK_EQUAL( res_add["P1"].value(), 2);
    BOOST_CHECK_EQUAL( res_add["P2"].value(), 4);
    BOOST_CHECK_EQUAL( res_add["P3"].value(), 6);
    BOOST_CHECK_EQUAL( res_add["P4"].value(), 8);

    auto res_sub = def_sub.eval(context);
    BOOST_CHECK_EQUAL( res_sub.size(), 4);
    BOOST_CHECK_EQUAL( res_sub["P1"].value(), 0);
    BOOST_CHECK_EQUAL( res_sub["P2"].value(), 0);
    BOOST_CHECK_EQUAL( res_sub["P3"].value(), 0);
    BOOST_CHECK_EQUAL( res_sub["P4"].value(), 0);

    auto res_div = def_div.eval(context);
    BOOST_CHECK_EQUAL( res_div.size(), 4);
    BOOST_CHECK_EQUAL( res_div["P1"].value(), 1);
    BOOST_CHECK_EQUAL( res_div["P2"].value(), 1);
    BOOST_CHECK_EQUAL( res_div["P3"].value(), 1);
    BOOST_CHECK_EQUAL( res_div["P4"].value(), 1);

    auto res_mul = def_mul.eval(context);
    BOOST_CHECK_EQUAL( res_mul.size(), 4);
    BOOST_CHECK_EQUAL( res_mul["P1"].value(), 1);
    BOOST_CHECK_EQUAL( res_mul["P2"].value(), 4);
    BOOST_CHECK_EQUAL( res_mul["P3"].value(), 9);
    BOOST_CHECK_EQUAL( res_mul["P4"].value(),16);

    auto res_muladd = def_muladd.eval(context);
    BOOST_CHECK_EQUAL( res_muladd.size(), 4);
    BOOST_CHECK_EQUAL( res_muladd["P1"].value(), 1 + 1);
    BOOST_CHECK_EQUAL( res_muladd["P2"].value(), 4 + 2);
    BOOST_CHECK_EQUAL( res_muladd["P3"].value(), 9 + 3);
    BOOST_CHECK_EQUAL( res_muladd["P4"].value(),16 + 4);

    auto res_wuwct= def_wuwct.eval(context);
    BOOST_CHECK_EQUAL( res_wuwct.size(), 4);
    BOOST_CHECK_EQUAL( res_wuwct["P1"].value(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P2"].value(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P3"].value(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P4"].value(),0.50);
}

BOOST_AUTO_TEST_CASE(DECK_TEST) {
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    UDQDefine def(udqp, "WUOPRL", {"(", "WOPR", "OP1", "-", "150", ")", "*", "0.90"});
    SummaryState st(std::chrono::system_clock::now());
    UDQContext context(udqft, st);

    st.update_well_var("OP1", "WOPR", 300);
    st.update_well_var("OP2", "WOPR", 3000);
    st.update_well_var("OP3", "WOPR", 30000);

    auto res = def.eval(context);
    BOOST_CHECK_EQUAL(res.size(), 3);
    for (std::size_t index = 0; index < res.size(); index++)
        BOOST_CHECK( res[index].value() == (300 - 150)*0.90);
}


BOOST_AUTO_TEST_CASE(UDQPARSE_TEST1) {
    UDQParams udqp;
    UDQDefine def1(udqp, "WUBHP", {"1/(WWCT", "'W1*')"});
    BOOST_CHECK_EQUAL( def1.input_string() , "1/(WWCT 'W1*')");

    UDQDefine def2(udqp, "WUBHP", {"2*(1",  "+" , "WBHP)"});
    BOOST_CHECK_EQUAL( def2.input_string() , "2*(1 + WBHP)");
}


BOOST_AUTO_TEST_CASE(UDQ_PARSE_ERROR) {
    UDQParams udqp;
    ParseContext parseContext;
    ErrorGuard errors;
    std::vector<std::string> tokens = {"WBHP", "+"};
    parseContext.update(ParseContext::UDQ_PARSE_ERROR, InputError::IGNORE);
    {
        UDQDefine def1(udqp, "WUBHP", tokens, parseContext, errors);
        SummaryState st(std::chrono::system_clock::now());
        UDQFunctionTable udqft(udqp);
        UDQContext context(udqft, st);
        st.update_well_var("P1", "WBHP", 1);

        auto res = def1.eval(context);
        BOOST_CHECK_EQUAL(res["P1"].value(), udqp.undefinedValue());
    }

    parseContext.update(ParseContext::UDQ_PARSE_ERROR, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUBHP", tokens, parseContext, errors), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(UDQ_TYPE_ERROR) {
    UDQParams udqp;
    ParseContext parseContext;
    ErrorGuard errors;
    std::vector<std::string> tokens1 = {"WBHP", "+", "1"};
    std::vector<std::string> tokens2 = {"SUM", "(", "WBHP", ")"};
    parseContext.update(ParseContext::UDQ_TYPE_ERROR, InputError::IGNORE);
    {
        UDQDefine def1(udqp, "FUBHP", tokens1, parseContext, errors);
        UDQDefine def2(udqp, "WUBHP", tokens2, parseContext, errors);

        SummaryState st(std::chrono::system_clock::now());
        UDQFunctionTable udqft(udqp);
        UDQContext context(udqft, st);
        st.update_well_var("P1", "WBHP", 1);
        st.update_well_var("P2", "WBHP", 2);

        auto res1 = def1.eval(context);
        BOOST_CHECK_EQUAL(res1[0].value(), udqp.undefinedValue());

        auto res2 = def2.eval(context);
        BOOST_CHECK_EQUAL(res2.size(), st.num_wells());
        for (std::size_t index = 0; index < res2.size(); index++)
            BOOST_CHECK_EQUAL(res2[index].value(), 3);
    }

    parseContext.update(ParseContext::UDQ_TYPE_ERROR, InputError::THROW_EXCEPTION);

    // This fails because the well expression (WBHP + 1) is assigned to the field variable FUBHP
    BOOST_CHECK_THROW( UDQDefine(udqp, "FUBHP", tokens1, parseContext, errors), std::invalid_argument);
}




BOOST_AUTO_TEST_CASE(UDA_VALUE) {
    UDAValue value0;
    BOOST_CHECK(value0.is<double>());
    BOOST_CHECK(!value0.is<std::string>());
    BOOST_CHECK_EQUAL( value0.get<double>(), 0);
    BOOST_CHECK_THROW( value0.get<std::string>(), std::invalid_argument);
    value0 = 10;
    BOOST_CHECK_EQUAL( value0.get<double>(), 10);
    BOOST_CHECK_THROW( value0.get<std::string>(), std::invalid_argument);
    value0 = "STRING";
    BOOST_CHECK_EQUAL( value0.get<std::string>(), std::string("STRING"));
    BOOST_CHECK_THROW( value0.get<double>(), std::invalid_argument);


    UDAValue value1(10);
    BOOST_CHECK(value1.is<double>());
    BOOST_CHECK(!value1.is<std::string>());
    BOOST_CHECK_EQUAL( value1.get<double>(), 10);
    BOOST_CHECK_NO_THROW( value1.assert_numeric() );



    UDAValue value2("FUBHP");
    BOOST_CHECK(!value2.is<double>());
    BOOST_CHECK(value2.is<std::string>());
    BOOST_CHECK_EQUAL( value2.get<std::string>(), std::string("FUBHP"));
    BOOST_CHECK_THROW( value2.get<double>(), std::invalid_argument);
    BOOST_CHECK_THROW( value2.assert_numeric("SHould contain numeric value"), std::invalid_argument);
}


/*
  The unit/dimension handling in the UDAvalue is hacky at best.
*/

BOOST_AUTO_TEST_CASE(UDA_VALUE_DIM) {
    UDAValue value0(1);
    Dimension dim(10);
    UDAValue value1(1, dim);

    BOOST_CHECK_EQUAL( value0.get<double>(), 1);
    BOOST_CHECK_EQUAL( value0.getSI(), 1);
    BOOST_CHECK_EQUAL( value1.get<double>(), 1);
    BOOST_CHECK_EQUAL( value1.getSI(), 10);
}


BOOST_AUTO_TEST_CASE(UDQ_INPUT_BASIC) {
    std::string deck_string = R"(
SCHEDULE
UDQ
    ASSIGN WUBHP1 11 /    0
    ASSIGN WUOPR  20 /    1
    ASSIGN WUBHP2 P2 12 / 2
    --ASSIGN WUBHP  0 /  --DUMMY   3
    UNITS  WUBHP 'BARSA' /
    UNITS  WUOPR 'SM3/DAY' /
    DEFINE WUWCT WWPR / (WWPR + WOPR) /  4
    UNITS  WUWCT '1' /
    DEFINE FUOPR SUM(WOPR) /             5
    UNITS  FUOPR 'SM3/DAY' /
    --ASSIGN FUXXX 0 /  --DUMMY            6
    UNITS  FUXXX 'SM3/DAY' /
/
UDQ
    ASSIGN WUBHPX P2 12 /                7
    DEFINE FUOPRX SUM(WOPR) /            8
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);


    const auto& input = udq.input();
    const auto& def = udq.definitions();
    BOOST_CHECK_EQUAL(input.size(), 9);
    BOOST_CHECK_EQUAL(udq.size(), 9);

    BOOST_CHECK( input[0].is<UDQAssign>() );
    BOOST_CHECK( input[1].is<UDQAssign>() );
    BOOST_CHECK( input[2].is<UDQAssign>() );
    BOOST_CHECK( input[3].is<UDQAssign>() );
    BOOST_CHECK( input[4].is<UDQDefine>() );
    BOOST_CHECK( input[5].is<UDQDefine>() );
    BOOST_CHECK( input[6].is<UDQAssign>() );
    BOOST_CHECK( input[7].is<UDQAssign>() );
    BOOST_CHECK( input[8].is<UDQDefine>() );

    BOOST_CHECK_EQUAL( input[5].unit(), "SM3/DAY" );

    BOOST_CHECK_EQUAL(def[0].keyword(), "WUWCT");
    BOOST_CHECK_EQUAL(def[1].keyword(), "FUOPR");
    BOOST_CHECK_EQUAL(def[2].keyword(), "FUOPRX");

    BOOST_CHECK_EQUAL( input[4].get<UDQDefine>().keyword(), "WUWCT");
    BOOST_CHECK_EQUAL( input[5].get<UDQDefine>().keyword(), "FUOPR");
    BOOST_CHECK_EQUAL( input[8].get<UDQDefine>().keyword(), "FUOPRX");

    BOOST_CHECK( udq.has_keyword("FUXXX") );
    const auto wubhp1 = udq["WUBHP1"];
    BOOST_CHECK( wubhp1.is<UDQAssign>() );
}


BOOST_AUTO_TEST_CASE(UDQ_INPUT_OVERWRITE) {
    std::string deck_string = R"(
SCHEDULE
UDQ
    ASSIGN WUBHP1 11 /
    ASSIGN WUOPR  20 /
    ASSIGN WUBHP2 P2 12 /
    --ASSIGN WUBHP  0 / --DUMMY
    UNITS  WUBHP 'BARSA' /
    UNITS  WUOPR 'SM3/DAY' /
    DEFINE WUWCT WWPR / (WWPR + WOPR) /
    UNITS  WUWCT '1' /
    DEFINE FUOPR SUM(WOPR) /
    UNITS  FUOPR 'SM3/DAY' /
/
UDQ
    DEFINE WUBHP1 SUM(WOPR) /
    DEFINE FUOPR MAX(WOPR) /
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);


    const auto& input = udq.input();
    BOOST_CHECK_EQUAL(input.size(), 6);
    BOOST_CHECK_EQUAL(udq.size(), 6);

    BOOST_CHECK( input[0].is<UDQDefine>());
    BOOST_CHECK_EQUAL( input[0].keyword(), "WUBHP1");

    const auto fuopr = udq["FUOPR"];
    BOOST_CHECK( fuopr.is<UDQDefine>() );
    const auto& def2 = fuopr.get<UDQDefine>();
    BOOST_CHECK_EQUAL(def2.input_string(), "MAX(WOPR)");
}


BOOST_AUTO_TEST_CASE(UDQ_USAGE) {
    UDQActive usage;
    UDQParams params;
    UDQConfig conf(params);
    BOOST_CHECK_EQUAL( usage.IUAD_size(), 0 );

    UDAValue uda1("WUX");
    conf.add_assign(uda1.get<std::string>(), {}, 100);

    usage.update(conf, uda1, "W1", UDAControl::WCONPROD_ORAT);
    BOOST_CHECK_EQUAL( usage.IUAD_size(), 1 );
    BOOST_CHECK_EQUAL( usage[0].use_count, 1);

    usage.update(conf, uda1, "W1", UDAControl::WCONPROD_GRAT);
    BOOST_CHECK_EQUAL( usage.IUAD_size(), 2 );
    BOOST_CHECK_EQUAL( usage[1].use_count, 1);

    const auto& rec = usage[0];
    BOOST_CHECK_EQUAL(rec.wgname, "W1");
    BOOST_CHECK_EQUAL(rec.udq, "WUX");
    BOOST_CHECK(rec.control == UDAControl::WCONPROD_ORAT);


    for (std::size_t index = 0; index < usage.IUAD_size(); index++) {
        const auto& record = usage[index];
        BOOST_CHECK_EQUAL(record.input_index, 0);
        BOOST_CHECK_EQUAL(record.wgname, "W1");

        if (index == 0)
            BOOST_CHECK(record.control == UDAControl::WCONPROD_ORAT);
        else
            BOOST_CHECK(record.control == UDAControl::WCONPROD_GRAT);

        index += 1;
    }

}


BOOST_AUTO_TEST_CASE(IntegrationTest) {
#include "data/integration_tests/udq.data"
    auto schedule = make_schedule(deck_string);
    {
        const auto& active = schedule.udqActive(1);
        BOOST_CHECK_EQUAL(active.IUAD_size(), 4);

        BOOST_CHECK(active[0].control == UDAControl::WCONPROD_ORAT);
        BOOST_CHECK(active[1].control == UDAControl::WCONPROD_LRAT);
        BOOST_CHECK(active[2].control == UDAControl::WCONPROD_ORAT);
        BOOST_CHECK(active[3].control == UDAControl::WCONPROD_LRAT);

        BOOST_CHECK(active[0].wgname == "OPL02");
        BOOST_CHECK(active[1].wgname == "OPL02");
        BOOST_CHECK(active[2].wgname == "OPU02");
        BOOST_CHECK(active[3].wgname == "OPU02");

        BOOST_CHECK(active[0].udq == "WUOPRL");
        BOOST_CHECK(active[1].udq == "WULPRL");
        BOOST_CHECK(active[2].udq == "WUOPRU");
        BOOST_CHECK(active[3].udq == "WULPRU");

        BOOST_CHECK(active[0].input_index == 0);
        BOOST_CHECK(active[1].input_index == 1);
        BOOST_CHECK(active[2].input_index == 2);
        BOOST_CHECK(active[3].input_index == 3);

        BOOST_CHECK(active[0].use_count == 1);
        BOOST_CHECK(active[1].use_count == 1);
        BOOST_CHECK(active[2].use_count == 1);
        BOOST_CHECK(active[3].use_count == 1);
    }
}

Schedule make_udq_schedule(const std::string& schedule_string) {
#include "data/integration_tests/udq2.data"
    deck_string += schedule_string;
    return make_schedule(deck_string);
}

BOOST_AUTO_TEST_CASE(IntegrationTest2) {
    const std::string udq_string = R"(
UDQ
DEFINE WUOPRL (WOPR PROD1 - 150) * 0.90 /
DEFINE WULPRL (WLPR PROD1 - 200) * 0.90 /
DEFINE WUOPRU (WOPR PROD2 - 250) * 0.80 /
DEFINE WULPRU (WLPR PROD2 - 300) * 0.80 /
DEFINE WUOPRL (WOPR PROD1 - 170) * 0.60 /
DEFINE WUXO   (WOPR PROD1 - 170) * 0.60 /
DEFINE WUXL   (WOPR PROD1 - 170) * 0.60 /
-- units
UNITS  WUOPRL SM3/DAY /
UNITS  WULPRL SM3/DAY /
UNITS  WUOPRU SM3/DAY /
UNITS  WULPRU SM3/DAY /
/
WCONPROD
  'PROD1'     'OPEN'  'GRUP' WUOPRU  1*  1*  WULPRU 1* 60.0   / single wells
/
WCONPROD
  'PROD2'     'OPEN'  'GRUP' WUOPRU  1*  1*  WULPRU 1* 60.0   / single wells
 /
WCONINJE
 'WINJ1' 'WAT' 'OPEN' 'BHP'  1*  1200  3500  1*  /
 'WINJ2' 'WAT' 'OPEN' 'BHP'  1*    800  3500  1*  /
/
TSTEP
 5 /
WCONPROD
  'PROD2'     'OPEN'  'GRUP' WUXO 1*  1*  WUXL 1* 60.0   / single wells
/
TSTEP
 5 /
WCONPROD
  'PROD1'     'OPEN'  'GRUP' 100 1*  1*  100 1* 60.0   / single wells
/
)";
    auto schedule = make_udq_schedule(udq_string);

    // First timestep
    {
        const auto& udq_active = schedule.udqActive(0);
        BOOST_CHECK(udq_active);
        BOOST_CHECK_EQUAL(udq_active.IUAD_size(), 2);

        const auto& record0 = udq_active[0];
        BOOST_CHECK_EQUAL( record0.uad_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 2);
        BOOST_CHECK_EQUAL( record0.use_count, 2);
        BOOST_CHECK_EQUAL( record0.use_index, 0);

        const auto& record1 = udq_active[1];
        BOOST_CHECK_EQUAL( record1.uad_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 3);
        BOOST_CHECK_EQUAL( record1.use_count, 2);
        BOOST_CHECK_EQUAL( record1.use_index, 2);
    }

    {
        // Second timestep
        //  - The WUOPRU and WULPRU udq are still used in the same manner for the PROD1 well.
        //  - The new UDQs WUXO and WUXL are now used for the PROD2 well.
        const auto& udq_active = schedule.udqActive(1);
        BOOST_CHECK(udq_active);
        BOOST_CHECK_EQUAL(udq_active.IUAD_size(), 4);

        const auto& record0 = udq_active[0];
        BOOST_CHECK_EQUAL( record0.uad_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 2);
        BOOST_CHECK_EQUAL( record0.use_count, 1);
        BOOST_CHECK_EQUAL( record0.use_index, 0);

        const auto& record1 = udq_active[1];
        BOOST_CHECK_EQUAL( record1.uad_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 3);
        BOOST_CHECK_EQUAL( record1.use_count, 1);
        BOOST_CHECK_EQUAL( record1.use_index, 1);

        const auto& record2 = udq_active[2];
        BOOST_CHECK_EQUAL( record2.uad_code, 300004);
        BOOST_CHECK_EQUAL( record2.input_index, 4);
        BOOST_CHECK_EQUAL( record2.use_count, 1);
        BOOST_CHECK_EQUAL( record2.use_index, 2);

        const auto& record3 = udq_active[3];
        BOOST_CHECK_EQUAL( record3.uad_code, 600004);
        BOOST_CHECK_EQUAL( record3.input_index, 5);
        BOOST_CHECK_EQUAL( record3.use_count, 1);
        BOOST_CHECK_EQUAL( record3.use_index, 3);
    }

    {
        // Third timestep
        //  - The new UDQs WUXO and WUXL are now used for the PROD2 well.
        //  - The PROD1 well does not use UDQ
        const auto& udq_active = schedule.udqActive(2);
        BOOST_CHECK(udq_active);
        BOOST_CHECK_EQUAL(udq_active.IUAD_size(), 2);

        const auto& record0 = udq_active[0];
        BOOST_CHECK_EQUAL( record0.uad_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 4);
        BOOST_CHECK_EQUAL( record0.use_count, 1);
        BOOST_CHECK_EQUAL( record0.use_index, 0);

        const auto& record1 = udq_active[1];
        BOOST_CHECK_EQUAL( record1.uad_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 5);
        BOOST_CHECK_EQUAL( record1.use_count, 1);
        BOOST_CHECK_EQUAL( record1.use_index, 1);
    }
    {
        const auto& udq_config = schedule.getUDQConfig(2);
        const auto& def = udq_config.definitions();
        const auto& def1 = def[0];
        const auto& tokens = def1.func_tokens();
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::number ), 1);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::ecl_expr), 1);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::binary_op_sub), 1);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::binary_op_mul), 1);
    }
}

