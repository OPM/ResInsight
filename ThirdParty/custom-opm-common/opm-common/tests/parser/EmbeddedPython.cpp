/*
  Copyright 2019 Equinor ASA.

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
#include <memory>

#define BOOST_TEST_MODULE EMBEDDED_PYTHON

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

using namespace Opm;

#ifndef EMBEDDED_PYTHON

BOOST_AUTO_TEST_CASE(INSTANTIATE) {
    Python python;
    BOOST_CHECK(!python.enabled());
    BOOST_CHECK_THROW(python.exec("print('Hello world')"), std::logic_error);
    BOOST_CHECK(! Python::supported() );

    BOOST_CHECK_THROW( Python{Python::Enable::ON}, std::logic_error );
    Python python_cond(Python::Enable::TRY);
    BOOST_CHECK(!python_cond.enabled());

    Python python_off(Python::Enable::OFF);
    BOOST_CHECK(!python_off.enabled());
}

#else

BOOST_AUTO_TEST_CASE(INSTANTIATE) {
    auto python = std::make_shared<Python>();
    BOOST_CHECK(Python::supported());
    BOOST_CHECK(python->enabled());
    BOOST_CHECK_NO_THROW(python->exec("import sys"));

    Parser parser;
    Deck deck;
    std::string python_code = R"(
print('Parser: {}'.format(context.parser))
print('Deck: {}'.format(context.deck))
kw = context.DeckKeyword( context.parser['FIELD'] )
context.deck.add(kw)
)";
    BOOST_CHECK_NO_THROW( python->exec(python_code, parser, deck));
    BOOST_CHECK( deck.hasKeyword("FIELD") );
}



BOOST_AUTO_TEST_CASE(PYINPUT_BASIC) {

    Parser parser;
    std::string input = R"(
        START             -- 0
        31 AUG 1993 /
        RUNSPEC
        PYINPUT
        kw = context.DeckKeyword( context.parser['FIELD'] )
        context.deck.add(kw)
        PYEND
        DIMENS
        2 2 1 /
        PYINPUT
        import numpy as np
        dx = np.array([0.25, 0.25, 0.25, 0.25])
        active_unit_system = context.deck.active_unit_system()
        default_unit_system = context.deck.default_unit_system()
        kw = context.DeckKeyword( context.parser['DX'], dx, active_unit_system, default_unit_system )
        context.deck.add(kw)
        PYEND
        DY
        4*0.25 /
        )";

    Deck deck = parser.parseString(input);
    BOOST_CHECK( deck.hasKeyword("START") );
    BOOST_CHECK( deck.hasKeyword("FIELD") );
    BOOST_CHECK( deck.hasKeyword("DIMENS") );
    BOOST_CHECK( deck.hasKeyword("DX") );
    auto DX = deck.getKeyword("DX");
    std::vector<double> dx_data = DX.getSIDoubleData();
    BOOST_CHECK_EQUAL( dx_data.size(), 4 );
    BOOST_CHECK_EQUAL( dx_data[2], 0.25 * 0.3048 );
    BOOST_CHECK( deck.hasKeyword("DY") );

}


BOOST_AUTO_TEST_CASE(PYACTION) {
    Parser parser;
    auto python = std::make_shared<Python>(Python::Enable::ON);
    auto deck = parser.parseFile("EMBEDDED_PYTHON.DATA");
    auto ecl_state = EclipseState(deck);
    auto schedule = Schedule(deck, ecl_state, python);

    SummaryState st(std::chrono::system_clock::now());
    const auto& pyaction_kw = deck.getKeyword<ParserKeywords::PYACTION>(0);
    const std::string& fname = pyaction_kw.getRecord(1).getItem(0).get<std::string>(0);
    Action::PyAction py_action(python, "WCLOSE", Action::PyAction::RunCount::unlimited, deck.makeDeckPath(fname));
    st.update_well_var("PROD1", "WWCT", 0);
    py_action.run(ecl_state, schedule, 10, st);

    st.update("FOPR", 0);
    py_action.run(ecl_state, schedule, 10, st);

    st.update("FOPR", 100);
    st.update_well_var("PROD1", "WWCT", 0.90);
    py_action.run(ecl_state, schedule, 10, st);

    const auto& well1 = schedule.getWell("PROD1", 10);
    const auto& well2 = schedule.getWell("PROD2", 10);
    BOOST_CHECK( well1.getStatus() == Well::Status::SHUT );
    BOOST_CHECK( well2.getStatus() == Well::Status::OPEN );
    BOOST_CHECK( st.has("RUN_COUNT") );
}


BOOST_AUTO_TEST_CASE(Python_Constructor) {
    Python python_off(Python::Enable::OFF);
    BOOST_CHECK(!python_off.enabled());

    Python python_on(Python::Enable::ON);
    BOOST_CHECK(python_on.enabled());

    //.enabled() Can only have one Python interpreter active at any time
    BOOST_CHECK_THROW(Python(Python::Enable::ON), std::logic_error);
}

BOOST_AUTO_TEST_CASE(Python_Constructor2) {
    Python python_cond1(Python::Enable::TRY);
    BOOST_CHECK(python_cond1.enabled());

    Python python_cond2(Python::Enable::TRY);
    BOOST_CHECK(!python_cond2.enabled());
}

#endif





