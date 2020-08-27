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
#include "config.h"
#include <iostream>
#include <memory>

#define BOOST_TEST_MODULE PY_ACTION_TESTER
#include <boost/test/unit_test.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/PyAction.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(ParsePYACTION) {
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("PYACTION.DATA");

    auto keyword = deck.getKeyword<ParserKeywords::PYACTION>(0);
    const auto& record0 = keyword.getRecord(0);
    const auto& record1 = keyword.getRecord(1);

    auto run_count = Action::PyAction::from_string(record0.getItem(1).get<std::string>(0));
    const std::string& ok_module = deck.makeDeckPath(record1.getItem(0).get<std::string>(0));
    Action::PyAction pyaction(python, "ACT1", run_count, ok_module);
    BOOST_CHECK_EQUAL(pyaction.name(), "ACT1");
}


#ifdef EMBEDDED_PYTHON
BOOST_AUTO_TEST_CASE(ParsePYACTION_Modules) {
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("PYACTION.DATA");
    auto keyword = deck.getKeyword<ParserKeywords::PYACTION>(0);
    const auto& record0 = keyword.getRecord(0);
    const auto& record1 = keyword.getRecord(1);

    const auto& name = record0.getItem(0).get<std::string>(0);
    auto run_count = Action::PyAction::from_string(record0.getItem(1).get<std::string>(0));
    const std::string& ok_module = deck.makeDeckPath(record1.getItem(0).get<std::string>(0));
    Action::PyAction pyaction(python, "ACT1", run_count, ok_module);

    const std::string& broken_module = deck.makeDeckPath("action_missing_run.py");
    BOOST_CHECK_THROW(Action::PyAction(python , "ACT2", run_count, broken_module), std::runtime_error);

    const std::string& broken_module2 = deck.makeDeckPath("action_syntax_error.py");
    BOOST_CHECK_THROW(Action::PyAction(python , "ACT2", run_count, broken_module2), std::runtime_error);

    const std::string& missing_module = deck.makeDeckPath("no_such_module.py");
    BOOST_CHECK_THROW(Action::PyAction(python , "ACT2", run_count, missing_module), std::invalid_argument);
}
#endif

