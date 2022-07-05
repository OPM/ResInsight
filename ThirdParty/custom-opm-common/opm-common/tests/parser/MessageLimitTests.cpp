/*
  Copyright 2016 Statoil ASA.

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

#define BOOST_TEST_MODULE MessageLimitTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/MessageLimits.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(MESSAGES) {
    Opm::Parser parser;
    const std::string input = R"(
START             -- 0
19 JUN 2007 /
RUNSPEC
MESSAGES
  5* 10 /
GRID
PORO
    1000*0.1 /
PERMX
    1000*1 /
PERMY
    1000*0.1 /
PERMZ
    1000*0.01 /
MESSAGES
  5* 77 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
WELSPECS
    'P1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
    'P2'       'OP'   5   5 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
    'I'       'OP'   1   1 1*     'WATER' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
 'P1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'P1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 /
 'P2'  5  5   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
 'P2'  5  5   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 /
 'I'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
WCONHIST
 'P1' 'OPEN' 'ORAT' 5*/
 'P2' 'OPEN' 'ORAT' 5*/
/
MESSAGES
  1 2 /
DATES             -- 2
 15  OKT 2008 /
/
MESSAGES
  10 /
     )";

    auto deck = parser.parseString(input);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule schedule(deck, grid, fp, runspec, python);

    BOOST_CHECK_EQUAL( schedule[0].message_limits().getBugPrintLimit( ) , 77 );   // The pre Schedule initialization

    BOOST_CHECK_EQUAL( schedule[1].message_limits().getMessagePrintLimit( ) , 1 );
    BOOST_CHECK_EQUAL( schedule[1].message_limits().getCommentPrintLimit( ) , 2 );
    BOOST_CHECK_EQUAL( schedule[1].message_limits().getBugPrintLimit( ) , 77 );

    BOOST_CHECK_EQUAL( schedule[2].message_limits().getMessagePrintLimit( ) , 10 );
    BOOST_CHECK_EQUAL( schedule[2].message_limits().getCommentPrintLimit( ) , 2  );
    BOOST_CHECK_EQUAL( schedule[2].message_limits().getBugPrintLimit( ) , 77 );
}
