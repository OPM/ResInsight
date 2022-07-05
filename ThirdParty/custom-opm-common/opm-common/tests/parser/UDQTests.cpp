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

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQAssign.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQContext.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunction.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/Well/NameOrder.hpp>
#include <opm/input/eclipse/Schedule/Well/WellMatcher.hpp>

#include <opm/input/eclipse/Utility/Typetools.hpp>

#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/UDAValue.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Opm;

namespace {
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
} // namespace anonymous

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
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def_group(udqp, "GUOPRL", 0, location, {"(", "5000",  "-",  "GOPR",  "LOWER",  "*", "0.13",  "-",  "GOPR",  "UPPER",  "*", "0.15", ")" , "*",  "0.89"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, {}, st, udq_state);
    double gopr_lower = 1234;
    double gopr_upper = 4321;

    st.update_group_var("LOWER", "GOPR", gopr_lower);
    st.update_group_var("UPPER", "GOPR", gopr_upper);

    auto res_group = def_group.eval(context);
    BOOST_CHECK_EQUAL( res_group["UPPER"].get(), (5000 - gopr_lower*0.13 - gopr_upper*0.15)*0.89);
    BOOST_CHECK_EQUAL( res_group["UPPER"].get(), (5000 - gopr_lower*0.13 - gopr_upper*0.15)*0.89);

    BOOST_CHECK_THROW(context.get_group_var("LOWER", "GGPR"), std::exception);
    auto empty_value = context.get_group_var("NO_SUCH_GROUP", "GOPR");
    BOOST_CHECK(!empty_value);
}



BOOST_AUTO_TEST_CASE(SUBTRACT)
{
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def(udqp, "WU", 0, location, {"16", "-", "8", "-", "4", "-", "2", "-", "1"});
    UDQDefine scalar(udqp, "WU", 0, location, {"16"});

    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1"})), st, udq_state);

    st.update_well_var("P1", "WOPR", 4);
    auto res = def.eval(context);
    BOOST_CHECK_EQUAL( res[0].get(), 1.0);

    auto res2 = scalar.eval(context);
    BOOST_CHECK_EQUAL( res2[0].get(), 16.0);
}

BOOST_AUTO_TEST_CASE(TEST)
{
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def1(udqp, "WUWI3",0, location, {"GOPR" , "MAU", "*", "2.0", "*", "0.25", "*", "10"});
    UDQDefine def2(udqp, "WUWI3",0, location, {"2.0", "*", "0.25", "*", "3"});
    UDQDefine def3(udqp, "WUWI3",0, location, {"GOPR" , "FIELD", "-", "2.0", "*", "3"});
    UDQDefine def4(udqp, "WUWI3",0, location, {"FOPR" , "/",  "2"});

    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2"})), st, udq_state);

    st.update_group_var("MAU", "GOPR", 4);
    st.update_group_var("XXX", "GOPR", 5);
    st.update_group_var("FIELD", "GOPR", 6);
    st.update_well_var("P1", "WBHP", 0.5);
    st.update_well_var("P2", "WBHP", 0.5);
    st.update("FOPR", 2);

    auto res1 = def1.eval(context);
    BOOST_CHECK_EQUAL( res1["P1"].get(), 20 );
    BOOST_CHECK_EQUAL( res1["P2"].get(), 20 );

    auto res2 = def2.eval(context);
    BOOST_CHECK_EQUAL( res2["P1"].get(), 1.50 );
    BOOST_CHECK_EQUAL( res2["P2"].get(), 1.50 );

    auto res3 = def3.eval(context);
    BOOST_CHECK_EQUAL( res3["P1"].get(), 0.00 );
    BOOST_CHECK_EQUAL( res3["P2"].get(), 0.00 );

    auto res4 = def4.eval(context);
    BOOST_CHECK_EQUAL( res4["P1"].get(), 1.00 );
    BOOST_CHECK_EQUAL( res4["P2"].get(), 1.00 );

    /*
      This expression has a well set as target type, and involves group with
      wildcard that is not supported by flow.
    */
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUWI2",0, location, {"GOPR", "G*", "*", "2.0"}), OpmInputError);

    /*
      UDQVarType == BLOCK is not yet supported.
    */
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUWI2",0, location, {"BPR", "1","1", "1", "*", "2.0"}), OpmInputError);
}


BOOST_AUTO_TEST_CASE(MIX_SCALAR) {
    UDQFunctionTable udqft;
    UDQParams udqp;
    KeywordLocation location;
    UDQDefine def_add(udqp, "WU",0, location, {"WOPR", "+", "1"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1"})), st, udq_state);

    st.update_well_var("P1", "WOPR", 1);

    auto res_add = def_add.eval(context);
    BOOST_CHECK_EQUAL( res_add["P1"].get() , 2);
}


BOOST_AUTO_TEST_CASE(UDQ_TABLE_EXCEPTION) {
    UDQParams udqp;
    KeywordLocation location;
    BOOST_CHECK_THROW(UDQDefine(udqp, "WU",0, location, {"TUPRICE[WOPR]"}), std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(UDQFieldSetTest) {
    std::vector<std::string> wells = {"P1", "P2", "P3", "P4"};
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2", "P3", "P4"})), st, udq_state);

    st.update_well_var("P1", "WOPR", 1.0);
    st.update_well_var("P2", "WOPR", 2.0);
    st.update_well_var("P3", "WOPR", 3.0);
    st.update_well_var("P4", "WOPR", 4.0);

    /*{
        UDQDefine def_fxxx(udqp, "FU_SCALAR", {"123"});
        auto fxxx_res = def_fxxx.eval(context);
        BOOST_CHECK_EQUAL( fxxx_res[0].get(), 123.0 );
        BOOST_CHECK( fxxx_res.var_type() == UDQVarType::FIELD_VAR);
    }
    */

    {
        UDQDefine def_fopr(udqp, "FUOPR",0, location, {"SUM", "(", "WOPR", ")"});
        auto fopr_res = def_fopr.eval(context);
        BOOST_CHECK_EQUAL( fopr_res[0].get(), 10.0 );
    }
}

BOOST_AUTO_TEST_CASE(UDQWellSetNANTest) {
    std::vector<std::string> wells = {"P1", "P2", "I1", "I2"};
    UDQSet ws = UDQSet::wells("NAME", wells);
    UDQSet ws2 = UDQSet::wells("NAME", wells);

    BOOST_CHECK(ws == ws2);

    for (std::size_t i = 0; i < 4; i++)
        ws.assign(i, i*1.0);

    BOOST_CHECK_EQUAL(ws.defined_size(), 4U);

    ws.assign(1,std::numeric_limits<double>::quiet_NaN());
    ws.assign(3,std::numeric_limits<double>::quiet_NaN());
    BOOST_CHECK_EQUAL(ws.defined_size(), 2U);

    BOOST_CHECK(ws.has("P1"));
    BOOST_CHECK(ws.has("P2"));
}


BOOST_AUTO_TEST_CASE(UDQWellSetTest) {
    std::vector<std::string> wells = {"P1", "P2", "I1", "I2"};
    UDQSet ws = UDQSet::wells("NAME", wells);
    UDQSet ws2 = UDQSet::wells("NAME", wells, 100.0);

    BOOST_CHECK_EQUAL(4U, ws.size());
    ws.assign("P1", 1.0);

    const auto& value = ws["P1"];
    BOOST_CHECK_EQUAL(value.get(), 1.0);
    BOOST_CHECK_EQUAL(ws["P1"].get(), 1.0);

    BOOST_REQUIRE_THROW(ws.assign("NO_SUCH_WELL", 1.0), std::out_of_range);
    BOOST_REQUIRE_THROW(ws[10], std::out_of_range);
    BOOST_REQUIRE_THROW(ws["NO_SUCH_WELL"], std::out_of_range);

    ws.assign("*", 2.0);
    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws[w].get(), 2.0);

    ws.assign(3.0);
    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws[w].get(), 3.0);

    ws.assign("P*", 4.0);
    BOOST_CHECK_EQUAL(ws["P1"].get(), 4.0);
    BOOST_CHECK_EQUAL(ws["P2"].get(), 4.0);

    ws.assign("I2", 5.0);
    BOOST_CHECK_EQUAL(ws["I2"].get(), 5.0);


    for (const auto& w : wells)
        BOOST_CHECK_EQUAL(ws2[w].get(), 100.0);

    UDQSet scalar = UDQSet::scalar("NAME", 1.0);
    BOOST_CHECK_EQUAL(scalar.size() , 1U);
    BOOST_CHECK_EQUAL(scalar[0].get(), 1.0);

    UDQSet empty = UDQSet::empty("EMPTY");
    BOOST_CHECK_EQUAL(empty.size() , 0U);
}


BOOST_AUTO_TEST_CASE(UDQ_GROUP_TEST) {
    std::vector<std::string> groups = {"G1", "G2", "G3", "G4"};
    UDQSet gs = UDQSet::groups("NAME", groups);

    BOOST_CHECK_EQUAL(4U, gs.size());
    gs.assign("G1", 1.0);

    const auto& value = gs["G1"];
    BOOST_CHECK_EQUAL(value.get(), 1.0);
    {
        KeywordLocation location;
        UDQParams udqp;
        UDQFunctionTable udqft(udqp);
        UDQDefine def_fopr(udqp, "FUOPR",0, location, {"SUM", "(", "GOPR", ")"});
        SummaryState st(TimeService::now());
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, {}, st, udq_state);

        st.update_group_var("G1", "GOPR", 1.0);
        st.update_group_var("G2", "GOPR", 2.0);
        st.update_group_var("G3", "GOPR", 3.0);
        st.update_group_var("G4", "GOPR", 4.0);


        auto res = def_fopr.eval(context);
        BOOST_CHECK_EQUAL(res[0].get(), 10.0);
    }
}



BOOST_AUTO_TEST_CASE(UDQ_DEFINETEST) {
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    KeywordLocation location;
    {
        UDQDefine def(udqp, "WUBHP",0, location, {"WBHP"});
        SummaryState st(TimeService::now());
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, WellMatcher(NameOrder({"W1", "W2", "W3"})), st, udq_state);

        st.update_well_var("W1", "WBHP", 11);
        st.update_well_var("W2", "WBHP", 2);
        st.update_well_var("W3", "WBHP", 3);
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(res.size(), 3U);
        BOOST_CHECK_EQUAL( res["W1"].get(), 11 );
        BOOST_CHECK_EQUAL( res["W2"].get(), 2 );
        BOOST_CHECK_EQUAL( res["W3"].get(), 3 );

        BOOST_CHECK_THROW( context.get_well_var("W3", "WWCT"), std::exception);
        auto empty_value = context.get_well_var("NO_SUCH_WELL", "WBHP");
        BOOST_CHECK(!empty_value);
    }
    {
        UDQDefine def(udqp, "WUBHP",0, location, {"WBHP" , "'P*'"});
        SummaryState st(TimeService::now());
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, WellMatcher(NameOrder({"I1", "I2", "P1", "P2"})), st, udq_state);


        st.update_well_var("P1", "WBHP", 1);
        st.update_well_var("P2", "WBHP", 2);
        st.update_well_var("I1", "WBHP", 1);
        st.update_well_var("I2", "WBHP", 2);
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(res.size(), 4U);
        BOOST_CHECK_EQUAL( res["P1"].get(), 1 );
        BOOST_CHECK_EQUAL( res["P2"].get(), 2 );
        BOOST_CHECK_EQUAL( res["I1"].defined(), false);
        BOOST_CHECK_EQUAL( res["I1"].defined(), false);
    }
    {
        UDQDefine def(udqp, "WUBHP",0, location, {"NINT" , "(", "WBHP", ")"});
        SummaryState st(TimeService::now());
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2", "I1", "I2"})), st, udq_state);
        st.update_well_var("P1", "WBHP", 4);
        st.update_well_var("P2", "WBHP", 3);
        st.update_well_var("I1", "WBHP", 2);
        st.update_well_var("I2", "WBHP", 1);

        auto res = def.eval(context);
        BOOST_CHECK_EQUAL( res["P1"].get(), 4 );
        BOOST_CHECK_EQUAL( res["P2"].get(), 3 );
        BOOST_CHECK_EQUAL( res["I1"].get(), 2 );
        BOOST_CHECK_EQUAL( res["I2"].get(), 1 );
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
    BOOST_CHECK_EQUAL(2U, udq.assignments().size());

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

  BOOST_CHECK_THROW( make_schedule(input), std::exception);
}





BOOST_AUTO_TEST_CASE(UDQ_DEFINE_WITH_SLASH) {
    const std::string input = R"(
UDQ
 DEFINE WUWCT WWPR / ( WWPR + WOPR ) /
/
)";
    Parser parser;
    auto deck = parser.parseString(input);
    const auto& udq = deck["UDQ"].back();
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

    BOOST_CHECK_EQUAL( w1["P12"].get(), 4.0 );
    BOOST_CHECK_EQUAL( w1["P1"].defined(), false );
    BOOST_CHECK_EQUAL( w1["P2"].defined(), false );

    BOOST_CHECK_EQUAL( w2["P12"].get(), 8.0 );
    BOOST_CHECK_EQUAL( w2["P1"].get(), 8.0 );
    BOOST_CHECK_EQUAL( w2["P2"].get(), 8.0 );
}




BOOST_AUTO_TEST_CASE(UDQ_CONTEXT) {
    SummaryState st(TimeService::now());
    UDQFunctionTable func_table;
    UDQParams udqp;
    UDQState udq_state(udqp.undefinedValue());
    UDQContext ctx(func_table, {}, st, udq_state);
    BOOST_CHECK_EQUAL(*ctx.get("JAN"), 1.0);
    BOOST_CHECK_THROW(ctx.get("NO_SUCH_KEY"), std::out_of_range);

    for (std::string& key : std::vector<std::string>({"MSUMLINS", "MSUMNEWT", "NEWTON", "TCPU"}))
        BOOST_CHECK_NO_THROW( ctx.get(key) );

    st.update("SX:KEY", 1.0);
    BOOST_CHECK_EQUAL(*ctx.get("SX:KEY") , 1.0 );
}

BOOST_AUTO_TEST_CASE(UDQ_SET) {
    UDQSet s1("NAME", 5);

    for (const auto& v : s1) {
        BOOST_CHECK_EQUAL(false, v.defined());
        BOOST_REQUIRE_THROW( v.get(), std::invalid_argument);
    }
    BOOST_CHECK_EQUAL(s1.defined_size(), 0U);

    s1.assign(1);
    for (const auto& v : s1) {
        BOOST_CHECK_EQUAL(true, v.defined());
        BOOST_CHECK_EQUAL( v.get(), 1.0);
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
        BOOST_CHECK_EQUAL(v0.get(), 25);

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
            BOOST_CHECK_EQUAL( v.get(), 2.0);
        }

        for (const auto& v : s3) {
            BOOST_CHECK_EQUAL(true, v.defined());
            BOOST_CHECK_EQUAL( v.get(), 4.0);
        }

        for (const auto& v : s4) {
            BOOST_CHECK_EQUAL(true, v.defined());
            BOOST_CHECK_EQUAL( v.get(), 0);
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
        BOOST_CHECK_EQUAL(result[0].get(), 7);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORM1"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 7);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORM2"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), std::sqrt(1 + 4+ 16));
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("NORMI"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("MIN"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("MAX"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEA"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 7.0/3);
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEG"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), std::exp((std::log(1) + std::log(2.0) + std::log(4))/3));
    }
    {
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("PROD"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL(result[0].get(), 8.0);
    }
    {
        UDQSet arg2("NAME", 4);
        arg2.assign(0,1);
        arg2.assign(2,4);
        arg2.assign(3,4);
        const auto& func = dynamic_cast<const UDQScalarFunction&>(udqft.get("AVEH"));
        auto result = func.eval(arg2);
        BOOST_CHECK_EQUAL(result[0].get(), 2.0);
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

        BOOST_CHECK_EQUAL( result.defined_size(), 3U );
        BOOST_CHECK_EQUAL( result[0].get(), 0);
        BOOST_CHECK_EQUAL( result[2].get(), 0);
        BOOST_CHECK_EQUAL( result[4].get(), 1);

        result = UDQBinaryFunction::EQ(0.20, arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK_EQUAL( result[2].get(), 0);
        BOOST_CHECK_EQUAL( result[4].get(), 1);

        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("=="));
        result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].get(), 0);
        BOOST_CHECK_EQUAL( result[2].get(), 0);
        BOOST_CHECK_EQUAL( result[4].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("<"));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result.defined_size(), 3U );
        BOOST_CHECK_EQUAL( result[0].get(), 0);
        BOOST_CHECK_EQUAL( result[2].get(), 1);
        BOOST_CHECK_EQUAL( result[4].get(), 0);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get(">"));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result.defined_size(), 3U );
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK_EQUAL( result[2].get(), 0);
        BOOST_CHECK_EQUAL( result[4].get(), 0);
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
            BOOST_CHECK_EQUAL( result[i].get(), (i+1)*(i+1));
    }
    {
        auto result = UDQBinaryFunction::GE(1.0, arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].get(), 1);

        // This is bisarre - but due to the large epsilon 2 and 2.5 compare as
        // equal; and then we evaluate 2 >= 2.5 as TRUE!
        BOOST_CHECK_EQUAL( result[2].get(), 1);
        BOOST_CHECK_EQUAL( result[4].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get("<="));
        auto result = func.eval(arg1, arg2);
        BOOST_CHECK_EQUAL( result[0].get(), 0);
        BOOST_CHECK_EQUAL( result[2].get(), 1);
        BOOST_CHECK_EQUAL( result[4].get(), 1);


    }
}


BOOST_AUTO_TEST_CASE(CMP_FUNCTIONS2) {
    UDQFunctionTable udqft;
    auto arg1 = UDQSet::scalar("NAME", 0);
    auto arg2 = UDQSet::scalar("NAME", 0);

    auto eq = UDQBinaryFunction::EQ(0, arg1, arg2);
    BOOST_CHECK_EQUAL(eq[0].get(), 1);
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
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK_EQUAL( result[2].get(), 2);
        BOOST_CHECK_EQUAL( result[4].get(), 4);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("DEF"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK_EQUAL( result[2].get(), 1);
        BOOST_CHECK_EQUAL( result[4].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("UNDEF"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[1].get(), 1);
        BOOST_CHECK_EQUAL( result[3].get(), 1);
        BOOST_CHECK_EQUAL( result.defined_size(), 2U);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("EXP"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].get(), std::exp(1));
        BOOST_CHECK_EQUAL( result[2].get(), std::exp(2));
        BOOST_CHECK_EQUAL( result[4].get(), std::exp(4));
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("IDV"));
        auto result = func.eval(arg);
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK_EQUAL( result[1].get(), 0);
        BOOST_CHECK_EQUAL( result[2].get(), 1);
        BOOST_CHECK_EQUAL( result[3].get(), 0);
        BOOST_CHECK_EQUAL( result[4].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("LOG"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, 10);
        arg_local.assign(2,1000);

        auto result = func.eval(arg_local);
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].get(), 3);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("NINT"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, 0.75);
        arg_local.assign(2, 1.25);

        auto result = func.eval(arg_local);
        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].get(), 1);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("RANDN"));
        UDQSet arg_local("NAME", 3);
        arg_local.assign(0, -1.0);
        arg_local.assign(2, -1.0);

        auto result1 = func.eval(arg_local);
        auto result2 = func.eval(arg_local);
        BOOST_CHECK( result1[0].get() != -1.0);
        BOOST_CHECK( !result1[1] );
        BOOST_CHECK( result1[2].get() != -1.0);

        BOOST_CHECK( result1[0].get() != result2[0].get());
        BOOST_CHECK( result1[2].get() != result2[2].get());
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("SORTA"));
        auto result = func.eval(arg);

        BOOST_CHECK_EQUAL( result[0].get(), 1);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].get(), 2);
        BOOST_CHECK( !result[3] );
        BOOST_CHECK_EQUAL( result[4].get(), 3);
    }
    {
        const auto& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get("SORTD"));
        auto result = func.eval(arg);

        BOOST_CHECK_EQUAL( result[0].get(), 3);
        BOOST_CHECK( !result[1] );
        BOOST_CHECK_EQUAL( result[2].get(), 2);
        BOOST_CHECK( !result[3] );
        BOOST_CHECK_EQUAL( result[4].get(), 1);
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
    BOOST_CHECK_EQUAL( 3U, result.defined_size() );
    BOOST_CHECK_EQUAL( 2, result[0].get() );
    BOOST_CHECK_EQUAL( 2, result[2].get() );
    BOOST_CHECK_EQUAL( 3, result[3].get() );
}




BOOST_AUTO_TEST_CASE(FUNCTIONS_INVALID_ARGUMENT) {
    UDQSet arg("NAME",3);
    arg.assign(0, -1);
    BOOST_REQUIRE_THROW( UDQScalarFunction::AVEG(arg), std::invalid_argument);
    BOOST_REQUIRE_THROW( UDQUnaryElementalFunction::LOG(arg), std::invalid_argument);
    BOOST_REQUIRE_THROW( UDQUnaryElementalFunction::LN(arg), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(UDQ_SET_DIV) {
    UDQSet s("NAME", 6);
    s.assign(0,1);
    s.assign(2,2);
    s.assign(4,5);
    s.assign(5,0);

    auto result = 10 / s;
    BOOST_CHECK_EQUAL( result.defined_size(), 3U);
    BOOST_CHECK_EQUAL( result[0].get(), 10);
    BOOST_CHECK_EQUAL( result[2].get(), 5);
    BOOST_CHECK_EQUAL( result[4].get(), 2);
}



BOOST_AUTO_TEST_CASE(UDQASSIGN_TEST) {
    UDQAssign as1("WUPR", std::vector<std::string>{}, 1.0, 1);
    UDQAssign as2("WUPR", std::vector<std::string>{"P*"}, 2.0, 2);
    UDQAssign as3("WUPR", std::vector<std::string>{"P1"}, 4.0, 3);
    std::vector<std::string> ws1 = {"P1", "P2", "I1", "I2"};

    auto res1 = as1.eval(ws1);
    BOOST_CHECK_EQUAL(res1.size(), 4U);
    BOOST_CHECK_EQUAL(res1["P1"].get(), 1.0);
    BOOST_CHECK_EQUAL(res1["I2"].get(), 1.0);

    auto res2 = as2.eval(ws1);
    BOOST_CHECK_EQUAL(res2["P1"].get(), 2.0);
    BOOST_CHECK_EQUAL(res2["P2"].get(), 2.0);
    BOOST_CHECK(!res2["I1"].defined());
    BOOST_CHECK(!res2["I2"].defined());

    auto res3 = as3.eval(ws1);
    BOOST_CHECK_EQUAL(res3["P1"].get(), 4.0);
    BOOST_CHECK(!res3["P2"].defined());
    BOOST_CHECK(!res3["I1"].defined());
    BOOST_CHECK(!res3["I2"].defined());
}

BOOST_AUTO_TEST_CASE(UDQ_POW_TEST) {
    KeywordLocation location;
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_pow1(udqp, "WU",0, location, {"WOPR", "+", "WWPR", "*", "WGOR", "^", "WWIR"});
    UDQDefine def_pow2(udqp, "WU",0, location, {"(", "WOPR", "+", "WWPR", ")", "^", "(", "WOPR", "+" , "WGOR", "*", "WWIR", "-", "WBHP", ")"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    NameOrder wo; wo.add("P1");
    UDQContext context(udqft, WellMatcher(wo), st, udq_state);

    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P1", "WWPR", 2);
    st.update_well_var("P1", "WGOR", 3);
    st.update_well_var("P1", "WWIR", 4);
    st.update_well_var("P1", "WBHP", 7);

    auto res_pow1 = def_pow1.eval(context);
    auto res_pow2 = def_pow2.eval(context);
    BOOST_CHECK_EQUAL( res_pow1["P1"].get() , 1 + 2 * std::pow(3,4));
    BOOST_CHECK_EQUAL( res_pow2["P1"].get() , std::pow(1 + 2, 1 + 3*4 - 7));
}

BOOST_AUTO_TEST_CASE(UDQ_CMP_TEST) {
    KeywordLocation location;
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_cmp(udqp, "WU",0, location, {"WOPR", ">", "WWPR", "+", "WGOR", "*", "WWIR"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2"})), st, udq_state);

    st.update_well_var("P1", "WOPR",  0);
    st.update_well_var("P1", "WWPR", 10);
    st.update_well_var("P1", "WGOR", -3);
    st.update_well_var("P1", "WWIR",  4);

    st.update_well_var("P2", "WOPR",  0);
    st.update_well_var("P2", "WWPR", -2);
    st.update_well_var("P2", "WGOR",  4);
    st.update_well_var("P2", "WWIR",  1);

    auto res_cmp = def_cmp.eval(context);
    BOOST_CHECK_EQUAL( res_cmp["P1"].get() , 1.0);
    BOOST_CHECK_EQUAL( res_cmp["P2"].get() , 0.0);
}

/*BOOST_AUTO_TEST_CASE(UDQPARSE_ERROR) {
    setUDQFunctionTable udqft;
    UDQDefine def1(udqft, "WUBHP", {"WWCT", "+"});
}
*/

BOOST_AUTO_TEST_CASE(UDQ_SCALAR_SET) {
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft;
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"PA1", "PB2", "PC3", "PD4"})), st, udq_state);

    st.update_well_var("PA1", "WOPR", 1);
    st.update_well_var("PB2", "WOPR", 2);
    st.update_well_var("PC3", "WOPR", 3);
    st.update_well_var("PD4", "WOPR", 4);

    st.update_well_var("PA1", "WWPR", 1);
    st.update_well_var("PB2", "WWPR", 2);
    st.update_well_var("PC3", "WWPR", 3);
    st.update_well_var("PD4", "WWPR", 4);

    {
        UDQDefine def(udqp, "WUOPR",0, location, {"WOPR", "'PA*'"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4U, res.size());
        auto well1 = res["PA1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.get() , 1);

        auto well2 = res["PB2"];
        BOOST_CHECK( !well2.defined() );

        auto well4 = res["PD4"];
        BOOST_CHECK( !well4.defined() );
    }
    {
        UDQDefine def(udqp, "WUOPR",0, location, {"1"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4U, res.size());
        auto well1 = res["PA1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.get() , 1);

        auto well2 = res["PB2"];
        BOOST_CHECK( well2.defined() );
        BOOST_CHECK_EQUAL(well2.get() , 1);

        auto well4 = res["PD4"];
        BOOST_CHECK( well4.defined() );
        BOOST_CHECK_EQUAL(well4.get() , 1);
    }
    {
        UDQDefine def(udqp, "WUOPR",0, location, {"WOPR", "'PA1'"});
        auto res = def.eval(context);
        BOOST_CHECK_EQUAL(4U, res.size());
        auto well1 = res["PA1"];
        BOOST_CHECK( well1.defined() );
        BOOST_CHECK_EQUAL(well1.get() , 1);

        auto well2 = res["PB2"];
        BOOST_CHECK( well2.defined() );
        BOOST_CHECK_EQUAL(well2.get() , 1);

        auto well4 = res["PD4"];
        BOOST_CHECK( well4.defined() );
        BOOST_CHECK_EQUAL(well4.get() , 1);

        BOOST_CHECK_EQUAL( "WUOPR", res.name() );
    }
}

BOOST_AUTO_TEST_CASE(UDQ_SORTD_NAN) {
    UDQParams udqp;
    UDQFunctionTable udqft;
    KeywordLocation location;
    UDQDefine def(udqp, "WUPR1" ,0, location, {"1", "/", "(", "WWIR", "'OP*'" , ")"});
    UDQDefine def_sort(udqp , "WUPR3",0, location, {"SORTD", "(", "WUPR1", ")" });
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"OP1", "OP2", "OP3", "OP4"})), st, udq_state);

    st.update_well_var("OP1", "WWIR", 1.0);
    st.update_well_var("OP2", "WWIR", 2.0);
    st.update_well_var("OP3", "WWIR", 3.0);
    st.update_well_var("OP4", "WWIR", 4.0);

    auto res1 = def.eval(context);
    context.update_define(0, def.keyword(), res1);

    auto res_sort1 = def_sort.eval(context);
    context.update_define(0, def_sort.keyword(), res_sort1);
    BOOST_CHECK_EQUAL(res_sort1["OP1"].get(), 1.0);
    BOOST_CHECK_EQUAL(res_sort1["OP2"].get(), 2.0);
    BOOST_CHECK_EQUAL(res_sort1["OP3"].get(), 3.0);
    BOOST_CHECK_EQUAL(res_sort1["OP4"].get(), 4.0);
    BOOST_CHECK( st.has_well_var("OP1", "WUPR3"));
    BOOST_CHECK( st.has_well_var("OP4", "WUPR3"));

    st.update_well_var("OP1", "WWIR", 0);
    auto res2 = def.eval(context);
    BOOST_CHECK_EQUAL(res2.defined_size(), 3U);

    context.update_define(0, def.keyword(), res2);
    BOOST_CHECK( st.has_well_var("OP4", "WUPR1"));

    auto res_sort2 = def_sort.eval(context);
    context.update_define(0, def.keyword(), res2);

    BOOST_CHECK_EQUAL(res_sort2.defined_size(), 3U);
    BOOST_CHECK_EQUAL(res_sort2["OP2"].get(), 1.0);
    BOOST_CHECK_EQUAL(res_sort2["OP3"].get(), 2.0);
    BOOST_CHECK_EQUAL(res_sort2["OP4"].get(), 3.0);
    BOOST_CHECK( st.has_well_var("OP4", "WUPR3"));
}



BOOST_AUTO_TEST_CASE(UDQ_SORTA) {
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft;
    UDQDefine def1(udqp, "WUPR1" ,0, location, {"1", "/", "(", "WWCT", "'OP*'", "+", "0.00001", ")"});
    UDQDefine def_sort(udqp , "WUPR3",0, location, {"SORTA", "(", "WUPR1", ")" });
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"OPL01", "OPL02", "OPU01", "OPU02"})), st, udq_state);

    st.update_well_var("OPL01", "WWCT", 0.7);
    st.update_well_var("OPL02", "WWCT", 0.8);
    st.update_well_var("OPU01", "WWCT", 0.0);
    st.update_well_var("OPU02", "WWCT", 0.0);

    auto res1 = def1.eval(context);
    context.update_define(0, def1.keyword(), res1);

    auto res_sort = def_sort.eval(context);
    BOOST_CHECK_EQUAL(res_sort["OPL02"].get(), 1.0);
    BOOST_CHECK_EQUAL(res_sort["OPL01"].get(), 2.0);
    BOOST_CHECK_EQUAL(res_sort["OPU01"].get() + res_sort["OPU02"].get(), 7.0);
}



BOOST_AUTO_TEST_CASE(UDQ_BASIC_MATH_TEST) {
    UDQParams udqp;
    UDQFunctionTable udqft;
    KeywordLocation location;
    UDQDefine def_add(udqp, "WU2OPR",0, location, {"WOPR", "+", "WOPR"});
    UDQDefine def_sub(udqp, "WU2OPR",0, location, {"WOPR", "-", "WOPR"});
    UDQDefine def_mul(udqp, "WU2OPR",0, location, {"WOPR", "*", "WOPR"});
    UDQDefine def_div(udqp, "WU2OPR",0, location, {"WOPR", "/", "WOPR"});
    UDQDefine def_muladd(udqp, "WUX",0, location, {"WOPR", "+", "WOPR", "*", "WOPR"});
    UDQDefine def_wuwct(udqp , "WUWCT",0, location, {"WWPR", "/", "(", "WOPR", "+", "WWPR", ")"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2", "P3", "P4"})), st, udq_state);

    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P2", "WOPR", 2);
    st.update_well_var("P3", "WOPR", 3);
    st.update_well_var("P4", "WOPR", 4);

    st.update_well_var("P1", "WWPR", 1);
    st.update_well_var("P2", "WWPR", 2);
    st.update_well_var("P3", "WWPR", 3);
    st.update_well_var("P4", "WWPR", 4);

    auto res_add = def_add.eval(context);
    BOOST_CHECK_EQUAL( res_add.size(), 4U);
    BOOST_CHECK_EQUAL( res_add["P1"].get(), 2);
    BOOST_CHECK_EQUAL( res_add["P2"].get(), 4);
    BOOST_CHECK_EQUAL( res_add["P3"].get(), 6);
    BOOST_CHECK_EQUAL( res_add["P4"].get(), 8);

    auto res_sub = def_sub.eval(context);
    BOOST_CHECK_EQUAL( res_sub.size(), 4U);
    BOOST_CHECK_EQUAL( res_sub["P1"].get(), 0);
    BOOST_CHECK_EQUAL( res_sub["P2"].get(), 0);
    BOOST_CHECK_EQUAL( res_sub["P3"].get(), 0);
    BOOST_CHECK_EQUAL( res_sub["P4"].get(), 0);

    auto res_div = def_div.eval(context);
    BOOST_CHECK_EQUAL( res_div.size(), 4U);
    BOOST_CHECK_EQUAL( res_div["P1"].get(), 1);
    BOOST_CHECK_EQUAL( res_div["P2"].get(), 1);
    BOOST_CHECK_EQUAL( res_div["P3"].get(), 1);
    BOOST_CHECK_EQUAL( res_div["P4"].get(), 1);

    auto res_mul = def_mul.eval(context);
    BOOST_CHECK_EQUAL( res_mul.size(), 4U);
    BOOST_CHECK_EQUAL( res_mul["P1"].get(), 1);
    BOOST_CHECK_EQUAL( res_mul["P2"].get(), 4);
    BOOST_CHECK_EQUAL( res_mul["P3"].get(), 9);
    BOOST_CHECK_EQUAL( res_mul["P4"].get(),16);

    auto res_muladd = def_muladd.eval(context);
    BOOST_CHECK_EQUAL( res_muladd.size(), 4U);
    BOOST_CHECK_EQUAL( res_muladd["P1"].get(), 1 + 1);
    BOOST_CHECK_EQUAL( res_muladd["P2"].get(), 4 + 2);
    BOOST_CHECK_EQUAL( res_muladd["P3"].get(), 9 + 3);
    BOOST_CHECK_EQUAL( res_muladd["P4"].get(),16 + 4);

    auto res_wuwct= def_wuwct.eval(context);
    BOOST_CHECK_EQUAL( res_wuwct.size(), 4U);
    BOOST_CHECK_EQUAL( res_wuwct["P1"].get(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P2"].get(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P3"].get(),0.50);
    BOOST_CHECK_EQUAL( res_wuwct["P4"].get(),0.50);
}

BOOST_AUTO_TEST_CASE(DECK_TEST) {
    KeywordLocation location;
    UDQParams udqp;
    UDQFunctionTable udqft(udqp);
    UDQDefine def(udqp, "WUOPRL",0, location, {"(", "WOPR", "OP1", "-", "150", ")", "*", "0.90"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"OP1", "OP2", "OP3"})), st, udq_state);

    st.update_well_var("OP1", "WOPR", 300);
    st.update_well_var("OP2", "WOPR", 3000);
    st.update_well_var("OP3", "WOPR", 30000);

    auto res = def.eval(context);
    BOOST_CHECK_EQUAL(res.size(), 3U);
    for (std::size_t index = 0; index < res.size(); index++)
        BOOST_CHECK( res[index].get() == (300 - 150)*0.90);
}


BOOST_AUTO_TEST_CASE(UDQPARSE_TEST1) {
    KeywordLocation location;
    UDQParams udqp;
    UDQDefine def1(udqp, "WUBHP",0, location, {"1/(WWCT", "'W1*')"});
    BOOST_CHECK_EQUAL( def1.input_string() , "1 / (WWCT 'W1*')");

    UDQDefine def2(udqp, "WUBHP",0, location, {"2 * (1",  "+" , "WBHP)"});
    BOOST_CHECK_EQUAL( def2.input_string() , "2 * (1 + WBHP)");
}

BOOST_AUTO_TEST_CASE(INPUT_STRING_SCIENTIFIC_NOTATION) {
    const auto schedule = make_schedule(R"(
SCHEDULE
UDQ
DEFINE FU_THREE (3000000 + FU_ONE*1500000 + 1000000*FU_TWO)/365 /
/
)");

    const auto& udq = schedule.getUDQConfig(0);
    const auto def = udq.definitions();

    BOOST_CHECK_EQUAL(def.size(), 1ULL);

    const auto expect_input_string = std::string {
        "(3E+06 + FU_ONE * 1.5E+06 + 1E+06 * FU_TWO) / 365"
    };

    BOOST_CHECK_EQUAL(def[0].input_string(), expect_input_string);
}

BOOST_AUTO_TEST_CASE(UDQ_PARSE_ERROR) {
    UDQParams udqp;
    ParseContext parseContext;
    ErrorGuard errors;
    std::vector<std::string> tokens = {"WBHP", "+"};
    KeywordLocation location;
    parseContext.update(ParseContext::UDQ_PARSE_ERROR, InputError::IGNORE);
    {
        UDQDefine def1(udqp, "WUBHP",0, location, tokens, parseContext, errors);
        SummaryState st(TimeService::now());
        UDQFunctionTable udqft(udqp);
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, WellMatcher(NameOrder({"P1"})), st, udq_state);
        st.update_well_var("P1", "WBHP", 1);

        auto res = def1.eval(context);
        BOOST_CHECK_EQUAL(res["P1"].get(), udqp.undefinedValue());
    }

    parseContext.update(ParseContext::UDQ_PARSE_ERROR, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( UDQDefine(udqp, "WUBHP",0, location, tokens, parseContext, errors), OpmInputError);
}

BOOST_AUTO_TEST_CASE(UDQ_TYPE_ERROR) {
    UDQParams udqp;
    ParseContext parseContext;
    ErrorGuard errors;
    std::vector<std::string> tokens1 = {"WBHP", "+", "1"};
    std::vector<std::string> tokens2 = {"SUM", "(", "WBHP", ")"};
    KeywordLocation location;
    parseContext.update(ParseContext::UDQ_TYPE_ERROR, InputError::IGNORE);
    {
        UDQDefine def1(udqp, "FUBHP",0, location, tokens1, parseContext, errors);
        UDQDefine def2(udqp, "WUBHP",0, location, tokens2, parseContext, errors);

        SummaryState st(TimeService::now());
        UDQFunctionTable udqft(udqp);
        UDQState udq_state(udqp.undefinedValue());
        UDQContext context(udqft, WellMatcher(NameOrder({"P1", "P2"})), st, udq_state);
        st.update_well_var("P1", "WBHP", 1);
        st.update_well_var("P2", "WBHP", 2);

        auto res1 = def1.eval(context);
        BOOST_CHECK_EQUAL(res1[0].get(), udqp.undefinedValue());

        auto res2 = def2.eval(context);
        BOOST_CHECK_EQUAL(res2.size(), st.num_wells());
        for (std::size_t index = 0; index < res2.size(); index++)
            BOOST_CHECK_EQUAL(res2[index].get(), 3);
    }

    parseContext.update(ParseContext::UDQ_TYPE_ERROR, InputError::THROW_EXCEPTION);

    // This fails because the well expression (WBHP + 1) is assigned to the field variable FUBHP
    BOOST_CHECK_THROW( UDQDefine(udqp, "FUBHP",0, location, tokens1, parseContext, errors), OpmInputError);
}




BOOST_AUTO_TEST_CASE(UDA_VALUE) {
    UDAValue value0;
    BOOST_CHECK(value0.is<double>());
    BOOST_CHECK(!value0.is<std::string>());
    BOOST_CHECK_EQUAL( value0.get<double>(), 0);
    BOOST_CHECK_THROW( value0.get<std::string>(), std::invalid_argument);
    value0.update(10);
    BOOST_CHECK_EQUAL( value0.get<double>(), 10);
    BOOST_CHECK_THROW( value0.get<std::string>(), std::invalid_argument);
    value0.update("STRING");
    BOOST_CHECK_EQUAL( value0.get<std::string>(), std::string("STRING"));
    BOOST_CHECK_THROW( value0.get<double>(), std::invalid_argument);


    UDAValue value1(10);
    BOOST_CHECK(value1.is<double>());
    BOOST_CHECK(!value1.is<std::string>());
    BOOST_CHECK_EQUAL( value1.get<double>(), 10);
    BOOST_CHECK_NO_THROW( value1.assert_numeric() );
    value1 *= 10;
    BOOST_CHECK_EQUAL( value1.get<double>(), 100);


    UDAValue value2("FUBHP");
    BOOST_CHECK(!value2.is<double>());
    BOOST_CHECK(value2.is<std::string>());
    BOOST_CHECK_EQUAL( value2.get<std::string>(), std::string("FUBHP"));
    BOOST_CHECK_THROW( value2.get<double>(), std::invalid_argument);
    BOOST_CHECK_THROW( value2.assert_numeric("SHould contain numeric value"), std::invalid_argument);
    BOOST_CHECK_THROW( value2 *= 10, std::exception );
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
    UNITS  WUBHP 'BARSA' /
    UNITS  WUOPR 'SM3/DAY' /
    DEFINE WUWCT WWPR / (WWPR + WOPR) /  3
    UNITS  WUWCT '1' /
    DEFINE FUOPR SUM(WOPR) /             4
    UNITS  FUOPR 'SM3/DAY' /
    UNITS  FUXXX 'SM3/DAY' /
/
UDQ
    ASSIGN WUBHPX P2 12 /                5
    DEFINE FUOPRX SUM(WOPR) /            6
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);


    const auto& input = udq.input();
    const auto& def = udq.definitions();
    BOOST_CHECK_EQUAL(input.size(), 7U);
    BOOST_CHECK_EQUAL(udq.size(), 7U);

    BOOST_CHECK( input[0].is<UDQAssign>() );
    BOOST_CHECK( input[1].is<UDQAssign>() );
    BOOST_CHECK( input[2].is<UDQAssign>() );
    BOOST_CHECK( input[3].is<UDQDefine>() );
    BOOST_CHECK( input[4].is<UDQDefine>() );
    BOOST_CHECK( input[5].is<UDQAssign>() );
    BOOST_CHECK( input[6].is<UDQDefine>() );

    BOOST_CHECK_EQUAL( input[1].unit(), "SM3/DAY" );

    BOOST_CHECK_EQUAL(def[0].keyword(), "WUWCT");
    BOOST_CHECK_EQUAL(def[1].keyword(), "FUOPR");
    BOOST_CHECK_EQUAL(def[2].keyword(), "FUOPRX");

    BOOST_CHECK_EQUAL( input[3].get<UDQDefine>().keyword(), "WUWCT");
    BOOST_CHECK_EQUAL( input[4].get<UDQDefine>().keyword(), "FUOPR");
    BOOST_CHECK_EQUAL( input[6].get<UDQDefine>().keyword(), "FUOPRX");

    BOOST_CHECK( !udq.has_keyword("FUXXX") );
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
    BOOST_CHECK_EQUAL(input.size(), 5U);
    BOOST_CHECK_EQUAL(udq.size(), 5U);

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
    BOOST_CHECK_EQUAL( usage.iuad().size(), 0U );

    UDAValue uda1("WUX");
    conf.add_assign(uda1.get<std::string>(), std::vector<std::string>{}, 100, 0);

    const auto& iuad = usage.iuad();
    usage.update(conf, uda1, "W1", UDAControl::WCONPROD_ORAT);
    BOOST_CHECK_EQUAL( usage.iuad().size(), 1U);
    BOOST_CHECK_EQUAL( iuad[0].use_count, 1U);

    usage.update(conf, uda1, "W1", UDAControl::WCONPROD_GRAT);
    BOOST_CHECK_EQUAL( usage.iuad().size(), 2U);
    BOOST_CHECK_EQUAL( iuad[1].use_count, 1U);

    const auto& rec = iuad[0];
    BOOST_CHECK_EQUAL(rec.udq, "WUX");
    BOOST_CHECK(rec.control == UDAControl::WCONPROD_ORAT);


    for (std::size_t index = 0; index < usage.iuad().size(); index++) {
        const auto& record = iuad[index];
        BOOST_CHECK_EQUAL(record.input_index, 0U);

        if (index == 0)
            BOOST_CHECK(record.control == UDAControl::WCONPROD_ORAT);
        else
            BOOST_CHECK(record.control == UDAControl::WCONPROD_GRAT);
    }

}

BOOST_AUTO_TEST_CASE(UDQControl_Keyword)
{
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_ORAT) == UDAKeyword::WCONPROD, "WCONPROD_ORAT control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_GRAT) == UDAKeyword::WCONPROD, "WCONPROD_GRAT control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_WRAT) == UDAKeyword::WCONPROD, "WCONPROD_WRAT control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_LRAT) == UDAKeyword::WCONPROD, "WCONPROD_LRAT control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_RESV) == UDAKeyword::WCONPROD, "WCONPROD_RESV control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_BHP)  == UDAKeyword::WCONPROD, "WCONPROD_BHP control keyword must be WCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONPROD_THP)  == UDAKeyword::WCONPROD, "WCONPROD_THP control keyword must be WCONPROD");

    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONINJE_RATE) == UDAKeyword::WCONINJE, "WCONINJE_RATE control keyword must be WCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONINJE_RESV) == UDAKeyword::WCONINJE, "WCONINJE_RESV control keyword must be WCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONINJE_BHP)  == UDAKeyword::WCONINJE, "WCONINJE_BHP control keyword must be WCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WCONINJE_THP)  == UDAKeyword::WCONINJE, "WCONINJE_THP control keyword must be WCONINJE");

    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONPROD_OIL_TARGET)    == UDAKeyword::GCONPROD, "GCONPROD_OIL_TARGET control keyword must be GCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONPROD_WATER_TARGET)  == UDAKeyword::GCONPROD, "GCONPROD_WATER_TARGET control keyword must be GCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONPROD_GAS_TARGET)    == UDAKeyword::GCONPROD, "GCONPROD_GAS_TARGET control keyword must be GCONPROD");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONPROD_LIQUID_TARGET) == UDAKeyword::GCONPROD, "GCONPROD_LIQUID_TARGET control keyword must be GCONPROD");

    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONINJE_SURFACE_MAX_RATE)      == UDAKeyword::GCONINJE, "GCONINJE_SURFACE_MAX_RATE control keyword must be GCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONINJE_RESV_MAX_RATE)         == UDAKeyword::GCONINJE, "GCONINJE_RESV_MAX_RATE control keyword must be GCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONINJE_TARGET_REINJ_FRACTION) == UDAKeyword::GCONINJE, "GCONINJE_TARGET_REINJ_FRACTION control keyword must be GCONINJE");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::GCONINJE_TARGET_VOID_FRACTION)  == UDAKeyword::GCONINJE, "GCONINJE_TARGET_VOID_FRACTION control keyword must be GCONINJE");

    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_ORAT) == UDAKeyword::WELTARG, "WELTARG_ORAT control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_WRAT) == UDAKeyword::WELTARG, "WELTARG_WRAT control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_GRAT) == UDAKeyword::WELTARG, "WELTARG_GRAT control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_LRAT) == UDAKeyword::WELTARG, "WELTARG_LRAT control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_RESV) == UDAKeyword::WELTARG, "WELTARG_RESV control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_BHP)  == UDAKeyword::WELTARG, "WELTARG_BHP control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_THP)  == UDAKeyword::WELTARG, "WELTARG_THP control keyword must be WELTARG");
    BOOST_CHECK_MESSAGE(UDQ::keyword(UDAControl::WELTARG_LIFT) == UDAKeyword::WELTARG, "WELTARG_LIFT control keyword must be WELTARG");

    BOOST_CHECK_THROW(UDQ::keyword(static_cast<UDAControl>(1729)),
                      std::logic_error);
}

BOOST_AUTO_TEST_CASE(UDAControl_IUAD_0)
{
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_ORAT), 300'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_GRAT), 500'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_WRAT), 400'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_LRAT), 600'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_RESV), 700'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_BHP),  800'004);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONPROD_THP),  900'004);

    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONINJE_RATE), 400'003);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONINJE_RESV), 500'003);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONINJE_BHP),  600'003);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WCONINJE_THP),  700'003);

    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONPROD_OIL_TARGET),     200'019);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONPROD_WATER_TARGET),   300'019);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONPROD_GAS_TARGET),     400'019);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONPROD_LIQUID_TARGET),  500'019);

    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONINJE_SURFACE_MAX_RATE),       300'017);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONINJE_RESV_MAX_RATE),          400'017);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONINJE_TARGET_REINJ_FRACTION),  500'017);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::GCONINJE_TARGET_VOID_FRACTION),   600'017);

    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_ORAT),        16);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_WRAT),   100'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_GRAT),   200'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_LRAT),   300'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_RESV),   400'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_BHP),    500'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_THP),    600'016);
    BOOST_CHECK_EQUAL(UDQ::udaCode(UDAControl::WELTARG_LIFT), 1'000'016);

    BOOST_CHECK_THROW(UDQ::udaCode(static_cast<UDAControl>(1729)),
                      std::logic_error);
}

BOOST_AUTO_TEST_CASE(IntegrationTest) {
#include "data/integration_tests/udq.data"
    auto schedule = make_schedule(deck_string);
    {
        const auto& udq_active = schedule[1].udq_active.get();
        const auto& active = udq_active.iuad();
        BOOST_CHECK_EQUAL(active.size(), 6U);

        BOOST_CHECK(active[0].control == UDAControl::WCONPROD_ORAT);
        BOOST_CHECK(active[1].control == UDAControl::WCONPROD_LRAT);
        BOOST_CHECK(active[2].control == UDAControl::WCONPROD_ORAT);
        BOOST_CHECK(active[3].control == UDAControl::WCONPROD_LRAT);
        BOOST_CHECK(active[4].control == UDAControl::GCONPROD_LIQUID_TARGET);
        BOOST_CHECK(active[5].control == UDAControl::GCONPROD_LIQUID_TARGET);

        BOOST_CHECK(active[0].udq == "WUOPRL");
        BOOST_CHECK(active[1].udq == "WULPRL");
        BOOST_CHECK(active[2].udq == "WUOPRU");
        BOOST_CHECK(active[3].udq == "WULPRU");
        BOOST_CHECK(active[4].udq == "GULPR1");
        BOOST_CHECK(active[5].udq == "GUOPR1");

        BOOST_CHECK(active[0].input_index == 0);
        BOOST_CHECK(active[1].input_index == 1);
        BOOST_CHECK(active[2].input_index == 2);
        BOOST_CHECK(active[3].input_index == 3);
        BOOST_CHECK(active[4].input_index == 4);
        BOOST_CHECK(active[5].input_index == 5);

        BOOST_CHECK(active[0].use_count == 1);
        BOOST_CHECK(active[1].use_count == 1);
        BOOST_CHECK(active[2].use_count == 1);
        BOOST_CHECK(active[3].use_count == 1);
        BOOST_CHECK(active[4].use_count == 2);
        BOOST_CHECK(active[5].use_count == 1);
    }
}

namespace {
    Schedule make_udq_schedule(const std::string& schedule_string) {
#include "data/integration_tests/udq2.data"
        deck_string += schedule_string;
        return make_schedule(deck_string);
    }
} // Namespace anonymous

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
        const auto& udq_active = schedule[0].udq_active.get();
        BOOST_CHECK(udq_active);

        const auto& iuad = udq_active.iuad();
        BOOST_CHECK_EQUAL(iuad.size(), 2U);
        const auto& record0 = iuad[0];
        BOOST_CHECK_EQUAL( record0.uda_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 2U);
        BOOST_CHECK_EQUAL( record0.use_count, 2U);
        BOOST_CHECK_EQUAL( record0.use_index, 0U);

        const auto& record1 = iuad[1];
        BOOST_CHECK_EQUAL( record1.uda_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 3U);
        BOOST_CHECK_EQUAL( record1.use_count, 2U);
        BOOST_CHECK_EQUAL( record1.use_index, 2U);
    }

    {
        // Second timestep
        //  - The WUOPRU and WULPRU udq are still used in the same manner for the PROD1 well.
        //  - The new UDQs WUXO and WUXL are now used for the PROD2 well.
        const auto& udq_active = schedule[1].udq_active.get();
        BOOST_CHECK(udq_active);
        const auto& iuad = udq_active.iuad();
        BOOST_CHECK_EQUAL(iuad.size(), 4U);

        const auto& record0 = iuad[0];
        BOOST_CHECK_EQUAL( record0.uda_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 2U);
        BOOST_CHECK_EQUAL( record0.use_count, 1U);
        BOOST_CHECK_EQUAL( record0.use_index, 0U);

        const auto& record1 = iuad[1];
        BOOST_CHECK_EQUAL( record1.uda_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 3U);
        BOOST_CHECK_EQUAL( record1.use_count, 1U);
        BOOST_CHECK_EQUAL( record1.use_index, 1U);

        const auto& record2 = iuad[2];
        BOOST_CHECK_EQUAL( record2.uda_code, 300004);
        BOOST_CHECK_EQUAL( record2.input_index, 4U);
        BOOST_CHECK_EQUAL( record2.use_count, 1U);
        BOOST_CHECK_EQUAL( record2.use_index, 2U);

        const auto& record3 = iuad[3];
        BOOST_CHECK_EQUAL( record3.uda_code, 600004);
        BOOST_CHECK_EQUAL( record3.input_index, 5U);
        BOOST_CHECK_EQUAL( record3.use_count, 1U);
        BOOST_CHECK_EQUAL( record3.use_index, 3U);
    }

    {
        // Third timestep
        //  - The new UDQs WUXO and WUXL are now used for the PROD2 well.
        //  - The PROD1 well does not use UDQ
        const auto& udq_active = schedule[2].udq_active.get();
        BOOST_CHECK(udq_active);
        const auto& iuad = udq_active.iuad();
        BOOST_CHECK_EQUAL(iuad.size(), 2U);

        const auto& record0 = iuad[0];
        BOOST_CHECK_EQUAL( record0.uda_code, 300004);
        BOOST_CHECK_EQUAL( record0.input_index, 4U);
        BOOST_CHECK_EQUAL( record0.use_count, 1U);
        BOOST_CHECK_EQUAL( record0.use_index, 0U);

        const auto& record1 = iuad[1];
        BOOST_CHECK_EQUAL( record1.uda_code, 600004);
        BOOST_CHECK_EQUAL( record1.input_index, 5U);
        BOOST_CHECK_EQUAL( record1.use_count, 1U);
        BOOST_CHECK_EQUAL( record1.use_index, 1U);
    }
    {
        const auto& udq_config = schedule.getUDQConfig(2);
        const auto& def = udq_config.definitions();
        const auto& def1 = def[0];
        const auto& tokens = def1.func_tokens();
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::number ), 1U);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::ecl_expr), 1U);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::binary_op_sub), 1U);
        BOOST_CHECK_EQUAL( tokens.count( UDQTokenType::binary_op_mul), 1U);

        BOOST_CHECK_THROW( udq_config[1000], std::exception );
        BOOST_CHECK(udq_config[0] == udq_config["WUOPRL"]);
        BOOST_CHECK(udq_config[2] == udq_config["WUOPRU"]);
    }
}


BOOST_AUTO_TEST_CASE(UDQ_SCIENTIFIC_LITERAL) {
    std::string deck_string = R"(
SCHEDULE
UDQ
   DEFINE FU 0 -1.25E-2*(1.0E-1 + 2E-1) /
/

)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    UDQParams udqp;
    auto def0 = udq.definitions()[0];
    SummaryState st(TimeService::now());
    UDQFunctionTable udqft(udqp);
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, {}, st, udq_state);

    auto res0 = def0.eval(context);
    BOOST_CHECK_CLOSE( res0[0].get(), -0.00125*3, 1e-6);
}


BOOST_AUTO_TEST_CASE(UDQ_NEGATIVE_PREFIX_BASIC) {
    std::string deck_string = R"(
SCHEDULE
UDQ
   DEFINE FUMIN0 - 1.5*FWPR /
   DEFINE FUMIN1 - 1.5*FWPR*(FGPR + FOPR)^3 - 2*FLPR /
   DEFINE FU -2.539E-14 * (FXP1+FXP2)^3 + 1.4464E-8 *(FXP1+FXP2)^2 +0.00028875*(FXP1+FXP2)+2.8541 /
/
)";

    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    UDQParams udqp;
    auto def0 = udq.definitions()[0];
    auto def1 = udq.definitions()[1];
    auto def2 = udq.definitions()[2];
    SummaryState st(TimeService::now());
    UDQFunctionTable udqft(udqp);
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, {}, st, udq_state);
    const double fwpr = 7;
    const double fopr = 4;
    const double fgpr = 7;
    const double flpr = 13;
    const double fxp1 = 1025;
    const double fxp2 = 107;
    st.update("FWPR", fwpr);
    st.update("FOPR", fopr);
    st.update("FGPR", fgpr);
    st.update("FLPR", flpr);
    st.update("FXP1", fxp1);
    st.update("FXP2", fxp2);

    auto res0 = def0.eval(context);
    BOOST_CHECK_EQUAL( res0[0].get(), -1.5*fwpr);

    auto res1 = def1.eval(context);
    BOOST_CHECK_EQUAL( res1[0].get(), -1.5*fwpr*std::pow(fgpr+fopr, 3) - 2*flpr );

    auto res2 = def2.eval(context);
    auto right = -2.5394E-14 * std::pow(fxp1 + fxp2, 3) + 1.4464E-8*std::pow(fxp1 + fxp2, 2) + 0.00028875*(fxp1 + fxp2) + 2.8541;
    BOOST_CHECK_CLOSE( res2[0].get(), right, 1e-6);
}

BOOST_AUTO_TEST_CASE(UDQ_STARSTAR) {
    std::string deck_string = R"(
SCHEDULE
UDQ
   DEFINE WUOPR2   WOPR '*' * WOPR '*' /
   DEFINE WUGASRA  3 - WGLIR '*' /
/
)";

    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    UDQParams udqp;
    auto def0 = udq.definitions()[0];
    auto def1 = udq.definitions()[1];
    SummaryState st(TimeService::now());
    UDQFunctionTable udqft(udqp);
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, WellMatcher(NameOrder({"W1", "W2", "W3"})), st, udq_state);
    st.update_well_var("W1", "WOPR", 1);
    st.update_well_var("W2", "WOPR", 2);
    st.update_well_var("W3", "WOPR", 3);

    st.update_well_var("W1", "WGLIR", 1);
    st.update_well_var("W2", "WGLIR", 2);
    st.update_well_var("W3", "WGLIR", 3);

    auto res0 = def0.eval(context);
    BOOST_CHECK_EQUAL( res0["W1"].get(), 1);
    BOOST_CHECK_EQUAL( res0["W2"].get(), 4);
    BOOST_CHECK_EQUAL( res0["W3"].get(), 9);

    auto res1 = def1.eval(context);
    BOOST_CHECK_EQUAL( res1["W1"].get(), 2);
    BOOST_CHECK_EQUAL( res1["W2"].get(), 1);
    BOOST_CHECK_EQUAL( res1["W3"].get(), 0);
}


BOOST_AUTO_TEST_CASE(UDQ_UADD_PARSER) {
    std::string deck_string = R"(
SCHEDULE
UDQ
   ASSIGN FU_PAR1 10.0 /
   ASSIGN FU_PAR2 2.0 /
   ASSIGN FU_PAR3 3.0 /
   DEFINE FU_UADD FU_PAR1 UADD FU_PAR2 /
   DEFINE FU_UMUL FU_PAR1 UMUL FU_PAR2 + FU_PAR3 /
   DEFINE FU_UMIN FU_PAR1 UMIN FU_PAR2 + FU_PAR3 /
/
)";

    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    SummaryState st(TimeService::now());
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    udq.eval(0, {}, st, udq_state);

    BOOST_CHECK_EQUAL( st.get("FU_UADD"), 12);   // 10 + 2

    // The Uxxx binary set functions have absolutely lowest priority; i.e. the
    // FU_PAR2 + FU_PAR3 expression is evaluated *before* the UMUL and UMIN operations.
    BOOST_CHECK_EQUAL( st.get("FU_UMUL"), 50);   // 10 * (2 + 3)
    BOOST_CHECK_EQUAL( st.get("FU_UMIN"), 5);    // min(10, 2+3)
}


BOOST_AUTO_TEST_CASE(UDQ_DEFINE_ORDER) {
    std::string deck_string = R"(
SCHEDULE
UDQ
ASSIGN FU_PAR1 1.0 /
DEFINE FU_PAR3 FMWPR /
ASSIGN FU_PAR2 0.0 /
DEFINE FU_PAR2 FU_PAR3 /
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    SummaryState st(TimeService::now());
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    st.update("FMWPR", 100);
    udq.eval(0, {}, st, udq_state);

    BOOST_CHECK_EQUAL(st.get("FU_PAR2"), 100);
}

BOOST_AUTO_TEST_CASE(UDQ_UNDEFINED2) {
    std::string deck_string = R"(
SCHEDULE
UDQ
DEFINE FU_PAR2 FU_PAR1 + 1/
DEFINE FU_PAR3 FU_PAR2 + 1/
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    SummaryState st(TimeService::now());
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    udq.eval(0, {}, st, udq_state);

    BOOST_CHECK_EQUAL(st.get("FU_PAR2"), undefined_value);
    BOOST_CHECK_EQUAL(st.get("FU_PAR3"), undefined_value);
}




BOOST_AUTO_TEST_CASE(UDQSTATE) {
    double undefined_value = 1234;
    UDQState st(undefined_value);
    BOOST_CHECK(!st.has("FUXX"));
    BOOST_CHECK(!st.has_well_var("OP1", "WUXX"));
    BOOST_CHECK(!st.has_group_var("G1", "GUXX"));

    // Try to get from symbol which is not UDQ -> logic_error
    BOOST_CHECK_THROW(st.get("FOPR"), std::logic_error);

    // Try to get from a UDQ which has not registered -> out_of_range
    BOOST_CHECK_THROW(st.get("FUPR"), std::out_of_range);

    auto fxpr = UDQSet::scalar("FXPR", 100);
    BOOST_CHECK_THROW(st.add_define(0, "FXPR", fxpr), std::logic_error);

    BOOST_CHECK_THROW(st.get_well_var("OP1", "WUPR"), std::out_of_range);

    auto fupr = UDQSet::scalar("FUPR", 100);
    st.add_define(0, "FUPR", fupr);

    // This is not a well quantity
    BOOST_CHECK_THROW(st.get_well_var("OP1", "FUPR"), std::logic_error);
    BOOST_CHECK_EQUAL(100, st.get("FUPR"));


    auto wupr = UDQSet::wells("WUPR", {"P1", "P2"});
    wupr.assign("P1", 75);
    st.add_define(0, "WUPR", wupr);

    BOOST_CHECK(st.has_well_var("P1", "WUPR"));
    // We have a well P2 - but we have not assigned a value to it!
    BOOST_CHECK(!st.has_well_var("P2", "WUPR"));

    BOOST_CHECK_EQUAL(st.get_well_var("P1", "WUPR"), 75);
    BOOST_CHECK_EQUAL(st.get_well_var("P2", "WUPR"), undefined_value);

    const auto buffer = st.serialize();
    UDQState st2(1067);
    st2.deserialize( buffer );
    BOOST_CHECK(st == st2);
}



BOOST_AUTO_TEST_CASE(UDQ_UADD_PARSER2) {
    std::string deck_string = R"(
SCHEDULE
UDQ
ASSIGN FU_PAR1 1.0 / -- xxxxxxxxxxxxxxxxxxxxxx
ASSIGN FU_PAR2 0.0 /
ASSIGN FU_PAR3 0.0 /
ASSIGN FU_PAR4 0.0 /
ASSIGN FU_PAR5 0.0 /
-- xxxxx xxxx
DEFINE FU_PAR6  FMWPR /
-- xxxxxxxxxxxxxxxxxxxx
DEFINE FU_PAR7 FMWIN /
DEFINE FU_PAR8 FMWPA /
DEFINE FU_PAR9 FMWIA /
-- xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
DEFINE FU_PAR10 (FU_PAR6 - FU_PAR2) + (FU_PAR8 - FU_PAR4) /
DEFINE FU_PAR11 (FU_PAR7 - FU_PAR3) + (FU_PAR9 - FU_PAR5) /
DEFINE FU_PAR12 FU_PAR10 > 0 /
DEFINE FU_PAR13 FU_PAR11 > 0 /
DEFINE FU_PAR14 FU_PAR12 * FU_PAR10  /
DEFINE FU_PAR15 FU_PAR13 * FU_PAR11  /
DEFINE FU_PAR2 FU_PAR6 /
DEFINE FU_PAR3 FU_PAR7 /
DEFINE FU_PAR4 FU_PAR8 /
DEFINE FU_PAR5 FU_PAR9 /
-- xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
ASSIGN FU_PAR16 0.08 /
ASSIGN FU_PAR17 100.0 /
ASSIGN FU_PAR18 6.0 /
ASSIGN FU_PAR19 0.0 /
-- xxxxxxxxxxxxx
DEFINE FU_PAR19 FU_PAR19 + TIMESTEP /
ASSIGN FU_PAR20 800.0 /
ASSIGN FU_PAR21 0.0 /
-- xxxxxxxxxxxxxxxxx
DEFINE FU_PAR21 FU_PAR21 UADD FU_PAR20 * (FU_PAR14 + FU_PAR15) / ((1.0 + 0.08 ) ^ (FU_PAR19 /365)) /
-- xxxxxxxxxxxxxxxxxxxx
ASSIGN FU_PAR22 0.0 /
DEFINE FU_PAR22 FU_PAR22 + FOPR * TIMESTEP * 1E-06 * 6.29 * FU_PAR17 * FU_PAR18 / ((1.0 + 0.08 ) ^ (FU_PAR19 /365)) /
DEFINE FU_PAR23 FU_PAR22 - FU_PAR21 /
-- xxxxxxxxxxxxxxxxxxxxxxxx
ASSIGN FU_PAR24 0.9 /
DEFINE FU_PAR24 FU_PAR24 UADD (FU_PAR14 + FU_PAR15) /
-- xxxxxxxxxxxxxxxxxxxxxxxxxxxx
DEFINE WUGASRA  750000 - WGLIR '*' /
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    SummaryState st(TimeService::now());

    st.update("TIMESTEP", 100);
    st.update("FMWPR", 100);
    st.update("FMWIN", 100);
    st.update("FMWPA", 100);
    st.update("FMWIA", 100);
    st.update("FOPR", 100);
    st.update_well_var("W1", "WGLIR", 1);
    st.update_well_var("W2", "WGLIR", 2);
    st.update_well_var("W3", "WGLIR", 3);

    NameOrder wo({"W1", "W2", "W3"});
    udq.eval(0, WellMatcher(wo), st, udq_state);
    {
        std::unordered_set<std::string> required_keys;
        udq.required_summary(required_keys);
        BOOST_CHECK_EQUAL( required_keys.size(), 7);
        BOOST_CHECK_EQUAL( required_keys.count("TIMESTEP"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("FMWPR"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("WGLIR"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("FOPR"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("FMWIN"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("FMWPA"), 1);
        BOOST_CHECK_EQUAL( required_keys.count("FMWIA"), 1);
    }
}


BOOST_AUTO_TEST_CASE(UDQ_UNDEFINED) {
    std::string deck_string = R"(
SCHEDULE

-- udq #2
UDQ
----XX xxxx xxx
--xxxx xxxx xxxx xxxx
ASSIGN FU_VAR1 0  /
DEFINE FU_VAR1 -2.539E-14 * (FU_VAR91+FU_VAR90)^3 + 1.4464E-8 *(FU_VAR91+FU_VAR90)^2 +0.00028875*(FU_VAR91+FU_VAR90)+2.8541 /
--xxxx xx xxxx xxx xxxx xxxx
ASSIGN FU_VAR2 0  /
DEFINE FU_VAR3 FU_VAR1 > 10 /
DEFINE FU_VAR2 NINT(FU_VAR91 / 35000 + 0.499) * FU_VAR3 /
--xxxx xxx XX xxx xxxx xxxx
ASSIGN FU_VAR4 0  /
--Xxxx xxx XX xxx xxxx, xxxx
ASSIGN FU_VAR5 0  /
DEFINE FU_VAR6 FU_VAR2 != 0 /
DEFINE FU_VAR7 FU_VAR6 * FU_VAR2  - 999 * (1 - FU_VAR6)  /  --Avoiding div by 0
DEFINE FU_VAR5 FU_VAR6 * FU_VAR91 / FU_VAR7 / 24 / 0.95 /
--Xxx xxx XX xxx xxx, xxxxx
DEFINE FU_VAR8 FU_VAR4 != 0 /
ASSIGN FU_VAR9 0  /
--XX Xxx xxxxx xxxxx xxxxx
ASSIGN FU_VAR10 0  /
DEFINE FU_VAR10  -0.00000041232 * FU_VAR5 ^ 2 + 0.0010395 * FU_VAR5 + 0.16504 /
--XX xxx xxxxx xxxxx xxxxx
ASSIGN FU_VAR11 0  /
--xxxxx xxxxx xxxxx xxxxx, xX
ASSIGN FU_VAR12 0  /
ASSIGN FU_VAR13 0  /
ASSIGN FU_VAR14 0  /
DEFINE FU_VAR12 FU_VAR2 * FU_VAR5 * 1E5 * ((FU_VAR1 - 10) / 3600) / 1000 / FU_VAR10 / 0.8938  /
DEFINE FU_VAR14 FU_VAR12 + FU_VAR13  /

-----Xxxxx xxxxx xxxxx
--xxxxx xx XX xxxxx xxxxx xxxxx
ASSIGN FU_VAR15 0  /
DEFINE FU_VAR15 NINT((FU_P1SWI + FU_P1WPR) / 30000 + 0.499) /
--xxxxx xx XX xxxxx xxxxx xxxxx
ASSIGN FU_VAR16 0  /
--xxxx xx  XX XZ, xx/x
ASSIGN FU_VAR17 0  /
DEFINE FU_VAR18 FU_VAR15 != 0  /
DEFINE FU_VAR17 FU_P1WPB * (FU_P1WPR + FU_P1SWI) / (FU_VAR15 + 0.0001) / 24 / 0.95  /
--xxxx xxx xxxx xxxx xxxx, xxxx
DEFINE FU_VAR19 FU_VAR16 != 0  /
ASSIGN FU_VAR20 0  /
--xxxx xxxx xxxx xxxx
ASSIGN FU_VAR21 0  /
DEFINE FU_VAR21 -0.00000035417*FU_VAR17^2 +0.0010673*FU_VAR17 + 0.029286  /
--XX xxxx xxxx xxxx
ASSIGN FU_VAR22 0  /
--XX XX xxxx xxxx, xX
ASSIGN FU_VAR23 0  /
DEFINE FU_VAR23 FU_VAR15 * FU_VAR17 * 1E5 * ((150-10)/3600) / 1000 / FU_VAR21 / 0.8938  /
--xx XX xx xx, xX
ASSIGN FU_VAR24 0  /
--xxxx xxxx xxxx xxxx, xxxx
DEFINE FU_VAR25 FU_VAR23 + FU_VAR24  /

-----xxxx xxxx
--xxxx xxxx xxxx, X xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
ASSIGN FU_VAR26 0 /
--xxxx xxxx xxxx xxxx
ASSIGN FU_VAR27 0  /
DEFINE FU_VAR27 ((FU_P1TGP/1E6*4546.667)/0.9215+1474) * (1-FU_VAR26) + ((FU_P1TGP/1E6*4911)/0.9215+1474) * FU_VAR26  /
--xxxx xxxx xxxx xxxx
ASSIGN FU_VAR28 0  /
--xxxx xxxx xxxx
ASSIGN FU_VAR29 0  /
DEFINE FU_VAR29 FU_VAR27 + FU_VAR28  /

-----xxxx xxxx
ASSIGN FU_VAR30 6682  /
ASSIGN FU_VAR31 0  /
ASSIGN FU_VAR32 4155  /  --xxxx xx xxxx xxxx XX xxxx
ASSIGN FU_VAR33 9685  /
ASSIGN FU_VAR34 4000  /
ASSIGN FU_VAR35 1230  /   --xxxx xx xxxx xxxx XX xxxx
--Total base load
ASSIGN FU_VAR36 0  /
DEFINE FU_VAR36 FU_P1BL + FU_P2BL + FU_VAR32 + FU_VAR33 + FU_VAR34 + FU_VAR35  /

-----XX xxxx xxxx
ASSIGN FU_VAR37 2300 /
ASSIGN FU_VAR38 0 /   --  xxxxxx  xx xxxx  xxxx  XX  xxxx 
ASSIGN FU_VAR39 2300  /  -- xxxx * Y  xxxx xx Z xxxx XX  xxxx 
-- xxxx  xxxx xxxx xxxx
ASSIGN FU_VAR40 0  /
DEFINE FU_VAR40 FU_VAR37 + FU_VAR38 + FU_VAR39 /

-----xxxx xxxx xxxx xxxx
--xxxx xxxx
ASSIGN FU_VAR41 0  /
DEFINE FU_VAR41 0.005 * FU_VAR90  /
DEFINE FU_VAR42 FU_P2WPR < FU_VAR41 /
ASSIGN FU_VAR43 0  /
--xxxx xxxx
ASSIGN FU_VAR44 0  /
--xxxx xxxx xxxx
ASSIGN FU_VAR45 0  /
DEFINE FU_VAR45 FU_VAR43 + FU_VAR44  /

-----xxxx xxxx xxxx
ASSIGN FU_VAR46 0  /
DEFINE FU_VAR47 FU_P1SWI > 10 /
DEFINE FU_VAR46 NINT(FU_P1SWI / 36000 + 0.499) * 761 / 0.9025 * FU_VAR47  /
ASSIGN FU_VAR48 0  /
DEFINE FU_VAR49 FU_P1WPR > 10 /
DEFINE FU_VAR48 NINT(FU_P1WPR / 30576 + 0.499) * 864 / 0.9025 * FU_VAR49  /
DEFINE FU_VAR50 FU_P2SWI > 10 /
ASSIGN FU_VAR51 0  /
DEFINE FU_VAR52 FU_P2WPR > 10 /
ASSIGN FU_VAR53 0  /
--xxxx xxxx xxxx
ASSIGN FU_VAR54 0  /
DEFINE FU_VAR54 FU_VAR46 + FU_VAR48 + FU_VAR51 + FU_VAR53  /

-----xxxx xxxx loadxxxxs
--xxxx xxxx xxxx xxxx xxxx, xxxxxxxx
ASSIGN FU_VAR55 0  /
DEFINE FU_VAR55 FU_VAR91 * 30.6571 / 1000 / 0.8754  /
--xxxx xxxx xxxx xxxx xxxx, xxxx
ASSIGN FU_VAR56 0  /
--xxxx
ASSIGN FU_VAR57 0  /
DEFINE FU_VAR57 FU_VAR91 * 61.4286 / 1000 / 0.9215 + 280  /
--xxxx
ASSIGN FU_VAR58 0  /
--xxxx
ASSIGN FU_VAR59 0  /
DEFINE FU_VAR59 FU_P1TGP * 120 / 1E6   /
--xxxx xxxx xxxx xxxx, xxxx
ASSIGN FU_VAR60 0  /
DEFINE FU_VAR60 (FU_VAR91 + FU_VAR90) * 5.52 / 1000  /
--xxxx xxxx xxxx xxxx, xxxx
ASSIGN FU_VAR61 0  /
DEFINE FU_VAR61 FU_VAR60 + FU_VAR59 + FU_VAR58 + FU_VAR57 + FU_VAR56 + FU_VAR55  /

-----xxxx-xxxx, xxxx
ASSIGN FU_VAR62 0  /
DEFINE FU_VAR62  (FU_VAR61 + FU_VAR54 + FU_VAR45 + FU_VAR40 + FU_VAR36 + FU_VAR29 + FU_VAR25 + FU_VAR14)/1000  /

--xxxx xxxx, xxxx
ASSIGN FU_VAR63 0  /  -- xxxx xxxx xxxx xxxx xxxx xxxx xxxx
--Allowance, MW
ASSIGN FU_VAR64 5  /  --xxxx xxxx xxxx MxxxxW xxxx xxxx XX xxxx
--xxxx
ASSIGN FU_VAR65 0  /
DEFINE FU_VAR65 0.02 * FU_VAR62  /

-----xxxx xxxx xxxx xxxx, xxxx
ASSIGN FU_VAR66 0  /
DEFINE FU_VAR66 FU_VAR65 + FU_VAR64 + FU_VAR63 + FU_VAR62  /

---- xxxx xxxx xxxx
DEFINE FU_VAR67 0  /
DEFINE FU_VAR67 FU_VAR66 * 0.79  /

---- xxxx xxxx xxxx xxxx
DEFINE FU_VAR68 0  /
DEFINE FU_VAR68 FU_VAR67 * 1.08  /
/

-- udq #6

UDQ
ASSIGN FU_VAR90 0.0  /
DEFINE FU_VAR91 GOPR TEST  /
/
)";
    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    SummaryState st(TimeService::now());
    st.update("FMWPR", 100);
    st.update("FMWIN", 100);
    st.update("FMWPA", 100);
    st.update("FMWIA", 100);
    st.update("FOPR", 100);
    st.update("TIMESTEP", 100);
    st.update_well_var("W1", "WGLIR", 1);
    st.update_well_var("W2", "WGLIR", 2);
    st.update_well_var("W3", "WGLIR", 3);
    st.update_group_var("TEST", "GOPR", 1);

    udq.eval(0, {}, st, udq_state);
}



BOOST_AUTO_TEST_CASE(UDQ_KEY_ERROR) {
    std::string deck_string = R"(
-- udq #2
SCHEDULE

UDQ
  DEFINE FU_VAR1 FOPR * 5 /
/
)";

    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    SummaryState st(TimeService::now());

    BOOST_CHECK_THROW(udq.eval(0, {}, st, udq_state), std::exception);
}


BOOST_AUTO_TEST_CASE(UDQ_ASSIGN) {
    std::string deck_string = R"(
-- udq #2
SCHEDULE

UDQ
  ASSIGN FU_VAR1 5  /
  DEFINE FU_VAR1 FU_VAR1 + 5 /
/
)";

    auto schedule = make_schedule(deck_string);
    const auto& udq = schedule.getUDQConfig(0);
    auto undefined_value =  udq.params().undefinedValue();
    UDQState udq_state(undefined_value);
    SummaryState st(TimeService::now());
    {
        std::unordered_set<std::string> required_keys;
        udq.required_summary(required_keys);
        BOOST_CHECK(required_keys.empty());
    }

    udq.eval(0, {}, st, udq_state);
    BOOST_CHECK_EQUAL(st.get("FU_VAR1"), 10);
}

BOOST_AUTO_TEST_CASE(UDQ_ASSIGN_REASSIGN) {
    std::string deck_string = R"(
-- udq #2
SCHEDULE

UDQ
  ASSIGN FU_VAR1 0  /
  DEFINE FU_VAR1 FU_VAR1 + 1 /
/

TSTEP
   1 1 1 1 1 /

UDQ
  ASSIGN FU_VAR1 0  /
  DEFINE FU_VAR1 FU_VAR1 + 1 /
/

TSTEP
   1 1 1 1 1 /

UDQ
  ASSIGN FU_VAR1 0  /
/

TSTEP
   1 1 1 1 1 /

)";

    auto schedule = make_schedule(deck_string);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());

    // Counting: 1,2,3,4,5
    for (std::size_t report_step = 0; report_step < 5; report_step++) {
        const auto& udq = schedule.getUDQConfig(report_step);
        udq.eval(report_step, schedule.wellMatcher(report_step), st, udq_state);
        auto fu_var1 = st.get("FU_VAR1");
        BOOST_CHECK_EQUAL(fu_var1, report_step + 1);
    }

    // Reset to zero and count: 1,2,3,4,5
    for (std::size_t report_step = 5; report_step < 10; report_step++) {
        const auto& udq = schedule.getUDQConfig(report_step);
        udq.eval(report_step, schedule.wellMatcher(report_step), st, udq_state);
        auto fu_var1 = st.get("FU_VAR1");
        BOOST_CHECK_EQUAL(fu_var1, report_step - 4);
    }

    // Reset to zero and stay there.
    for (std::size_t report_step = 10; report_step < 15; report_step++) {
        const auto& udq = schedule.getUDQConfig(report_step);
        udq.eval(report_step, schedule.wellMatcher(report_step),st, udq_state);
        auto fu_var1 = st.get("FU_VAR1");
        BOOST_CHECK_EQUAL(fu_var1, 0);
    }


    const auto& unique = schedule.unique<UDQConfig>();
    BOOST_CHECK_EQUAL( unique.size(), 3 );
    BOOST_CHECK_EQUAL( unique[0].first, 0 );
    BOOST_CHECK_EQUAL( unique[1].first, 5 );
    BOOST_CHECK_EQUAL( unique[2].first, 10 );

    BOOST_CHECK( unique[0].second == schedule.getUDQConfig(0));
    BOOST_CHECK( unique[1].second == schedule.getUDQConfig(5));
    BOOST_CHECK( unique[2].second == schedule.getUDQConfig(10));
}


BOOST_AUTO_TEST_CASE(UDQ_DIV_TEST) {
    KeywordLocation location;
    UDQFunctionTable udqft;
    UDQParams udqp;
    UDQDefine def_div(udqp, "FU",0, location, {"128", "/", "2", "/", "4", "/", "8"});
    SummaryState st(TimeService::now());
    UDQState udq_state(udqp.undefinedValue());
    UDQContext context(udqft, {}, st, udq_state);

    auto res_div = def_div.eval(context);
    BOOST_CHECK_EQUAL( res_div[0].get() , 2.0);
}


BOOST_AUTO_TEST_CASE(UDQ_LEADING_SIGN) {
    std::string deck_string = R"(
SCHEDULE

UDQ
  DEFINE FU_VAR1 - 100 + 215 /
  DEFINE FU_VAR2 (-100 + 200) / 10 /
  DEFINE FU_VAR3 -(100 + 200) * -10 /
  DEFINE FU_VAR4 2^-1 /
  ASSIGN FU_VAR6 2 /
  ASSIGN FU_VAR7 3 /
  DEFINE FU_VAR5 (-0.00000041232 * (FU_VAR6 ^ 2)) + (0.0010395 * FU_VAR7) + 0.16504 /
/

)";

    auto schedule = make_schedule(deck_string);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());

    const auto& udq = schedule.getUDQConfig(0);
    udq.eval(0, {}, st, udq_state);
    auto fu_var1 = st.get("FU_VAR1");
    auto fu_var2 = st.get("FU_VAR2");
    auto fu_var3 = st.get("FU_VAR3");
    auto fu_var4 = st.get("FU_VAR4");
    auto fu_var5 = st.get("FU_VAR5");
    BOOST_CHECK_EQUAL(fu_var1, 115);
    BOOST_CHECK_EQUAL(fu_var2, 10);
    BOOST_CHECK_EQUAL(fu_var3, 3000);
    BOOST_CHECK_EQUAL(fu_var4, 0.5);
    BOOST_CHECK_CLOSE(fu_var5, -0.00000041232 * 4 + 0.0010395 * 3  + 0.16504, 1e-5);
}

BOOST_AUTO_TEST_CASE(UDQ_WLIST) {
    std::string deck_string = R"(
SCHEDULE

WELSPECS
     'P1'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'P2'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'P3'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'P4'         'OP'   20   51  3.92       'OIL'  3*  NO /
/

WLIST
  '*ILIST'  'NEW'  P1 P2 P3 /
/

UDQ
  DEFINE FU_VAR1 SUM(WOPR '*ILIST') /
  DEFINE FU_VAR2 SUM(WOPR '*') /
  DEFINE FU_VAR3 WOPR 'P4' /
/

)";

    auto schedule = make_schedule(deck_string);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());
    const auto& udq = schedule.getUDQConfig(0);
    st.update_well_var("P1", "WOPR", 1);
    st.update_well_var("P2", "WOPR", 2);
    st.update_well_var("P3", "WOPR", 3);
    st.update_well_var("P4", "WOPR", 4);

    udq.eval(0, schedule.wellMatcher(0), st, udq_state);
    auto fu_var1 = st.get("FU_VAR1");
    auto fu_var2 = st.get("FU_VAR2");
    auto fu_var3 = st.get("FU_VAR3");
    BOOST_CHECK_EQUAL(fu_var1, 6);
    BOOST_CHECK_EQUAL(fu_var2, 10);
    BOOST_CHECK_EQUAL(fu_var3, 4);
}

BOOST_AUTO_TEST_CASE(UDQ_MINUS_PAREN) {
    std::string deck_string = R"(
SCHEDULE

UDQ
  DEFINE FU_VAR1 -( -10 + 15) * 10 /
  DEFINE FU_VAR2 -( -(10) + 15) * 10 /
  DEFINE FU_VAR3 -(-10 + 15)*-10 /
  DEFINE FU_VAR4 -(-(10) + 15)*-10 /
/

)";

    auto schedule = make_schedule(deck_string);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());
    const auto& udq = schedule.getUDQConfig(0);
    udq.eval(0, schedule.wellMatcher(0), st, udq_state);

    auto fu_var1 = st.get("FU_VAR1");
    auto fu_var2 = st.get("FU_VAR2");
    auto fu_var3 = st.get("FU_VAR3");
    auto fu_var4 = st.get("FU_VAR4");
    BOOST_CHECK_EQUAL(fu_var1, -50);
    BOOST_CHECK_EQUAL(fu_var2, -50);
    BOOST_CHECK_EQUAL(fu_var3, 50);
    BOOST_CHECK_EQUAL(fu_var4, 50);
}

BOOST_AUTO_TEST_CASE(UDQ_UPDATE) {
    std::string invalid1 = R"(
SCHEDULE

UDQ
  UPDATE FU_XXX /
/
)";

    std::string valid = R"(
SCHEDULE

UDQ
   DEFINE FU_TIME TIME /
/

TSTEP
1 /

UDQ
   UPDATE FU_TIME OFF /
/
TSTEP
1 /

TSTEP
1 /

UDQ
   UPDATE FU_TIME NEXT /
/

TSTEP
1 /

TSTEP
1 /

UDQ
   UPDATE FU_TIME OFF /
/

TSTEP
1 /

)";



    BOOST_CHECK_THROW(make_schedule(invalid1), std::exception);
    auto schedule = make_schedule(valid);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());
    UDQSet result = UDQSet::scalar("RES", 0);
    {
        const auto& udq = schedule.getUDQConfig(0);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( udq_state.define(def.keyword(), def.status()));
        udq_state.add_define(0, def.keyword(), result);
    }
    {
        const auto& udq = schedule.getUDQConfig(1);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( !udq_state.define(def.keyword(), def.status()));
    }
    {
        const auto& udq = schedule.getUDQConfig(2);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( !udq_state.define(def.keyword(), def.status()));
    }
    {
        const auto& udq = schedule.getUDQConfig(3);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( udq_state.define(def.keyword(), def.status()));
        udq_state.add_define(3, def.keyword(), result);
    }
    {
        const auto& udq = schedule.getUDQConfig(4);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( !udq_state.define(def.keyword(), def.status()));
    }
    {
        const auto& udq = schedule.getUDQConfig(5);
        const auto& def = udq.define("FU_TIME");
        BOOST_CHECK( !udq_state.define(def.keyword(), def.status()));
    }
}

BOOST_AUTO_TEST_CASE(UDQ_TYPE_CAST) {

    std::string valid = R"(
SCHEDULE

UDQ
   ASSIGN FUBHPP1 100 /
/

TSTEP
10 /

UDQ
   DEFINE FU_TIME TIME /
   DEFINE WUDELTA WBHP '*' - FUBHPP1 /
   DEFINE WU_TEST WUBHPINI '*' - (WGPR '*')/2000.0 /

/

)";



    auto schedule = make_schedule(valid);
    UDQState udq_state(0);
    SummaryState st(TimeService::now());
    UDQFunctionTable udqft;
    UDQContext context(udqft, WellMatcher(NameOrder({"W1", "W2", "W3"})), st, udq_state);
    st.update_well_var("W1", "WBHP", 400);
    st.update_well_var("W2", "WBHP", 300);
    st.update_well_var("W3", "WBHP", 200);

    const auto& udq = schedule.getUDQConfig(1);
    {
        const auto& ass = udq.assign("FUBHPP1");
        context.update_assign(1, "FUBHPP1", ass.eval());
    }
    const auto& def = udq.define("WUDELTA");
    auto res = def.eval(context);

    BOOST_CHECK_EQUAL(res["W1"].get(), 300);
    BOOST_CHECK_EQUAL(res["W2"].get(), 200);
    BOOST_CHECK_EQUAL(res["W3"].get(), 100);
}

BOOST_AUTO_TEST_CASE(UDQ_TRAILING_COMMENT) {

    std::string valid = R"(
SCHEDULE

UDQ
   ASSIGN FUBHPP1 100 /
/ Comment here

)";


    BOOST_CHECK_NO_THROW( make_schedule(valid) );
}



BOOST_AUTO_TEST_CASE(UDQ_ASSIGN_RST) {
    std::unordered_set<std::string> selector{"W1", "W2"};
    UDQAssign assign("WUBHP", selector, 100, 2);
    auto res = assign.eval( {"W1", "W2", "W3"});
    BOOST_CHECK_EQUAL(res.size(), 3);
    BOOST_CHECK_EQUAL(res["W1"].get(), 100);
    BOOST_CHECK_EQUAL(res["W2"].get(), 100);
    BOOST_CHECK_EQUAL(res["W3"].defined(), false);
}
