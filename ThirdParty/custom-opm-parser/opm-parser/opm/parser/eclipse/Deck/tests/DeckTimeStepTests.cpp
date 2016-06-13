/*
  Copyright 2015 Statoil ASA.

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


#define BOOST_TEST_MODULE DeckTimeStepTests

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <opm/parser/eclipse/Deck/DeckTimeStep.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/SCHEDULESection.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>


using namespace Opm;

BOOST_AUTO_TEST_CASE(testDeckTimeStepInitialize) {

  BOOST_CHECK_NO_THROW(DeckTimeStep deckTimeStep);
}

BOOST_AUTO_TEST_CASE(testDeckTimeStepTSTEP) {
    Opm::Parser parser;
    std::string input =
                  "START             -- 0 \n"
                  "19 JUN 2007 / \n"
                  "SCHEDULE\n"
                  "DATES             -- 1,2\n"
                  " 10  OKT 2008 / \n"
                  " 11  OKT 2008 / \n"
                  "/\n"
                  "WELSPECS\n"
                  "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                  "/\n"
                  "COMPDAT\n"
                  " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                  " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                  " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                  "/\n"
                  "DATES             -- 3,4\n"
                  " 20  JAN 2010 / \n"
                  " 21  JAN 2010 / \n"
                  "/\n";


    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);

    SCHEDULESection scheduleSection = SCHEDULESection(*deck);
    DeckTimeStepConstPtr step1 = scheduleSection.getDeckTimeStep(0);
    DeckTimeStepConstPtr step2 = scheduleSection.getDeckTimeStep(1);
    DeckTimeStepConstPtr step3 = scheduleSection.getDeckTimeStep(2);
    DeckTimeStepConstPtr step4 = scheduleSection.getDeckTimeStep(3);
    BOOST_CHECK_EQUAL(step1->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step2->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step3->hasKeyword("WELSPECS"), true);
    BOOST_CHECK_EQUAL(step4->hasKeyword("WELSPECS"), false);

}



BOOST_AUTO_TEST_CASE(testDeckTimeStepDATES) {
  Opm::Parser parser;
  std::string input =
          "RUNSPEC\n"
          "INIT\n"
          "UNIFOUT\n"
          "OIL\n"
          "GAS\n"
          "WATER\n"
          "METRIC\n"
          "DIMENS\n"
          "3 3 3/\n"
          "GRID\n"
          "DXV\n"
          "1.0 2.0 3.0 /\n"
          "DYV\n"
          "4.0 5.0 6.0 /\n"
          "DZV\n"
          "7.0 8.0 9.0 /\n"
          "TOPS\n"
          "9*100 /\n"
          "PROPS\n"
          "PORO\n"
          "27*0.3 /\n"
          "PERMX\n"
          "27*1 /\n"
          "SOLUTION\n"
          "RPTRST\n"
          "BASIC=2\n"
          "/\n"
          "SCHEDULE\n"
          "TSTEP\n"
          "1.0 2.0 3.0 4.0 /\n"
          "WELSPECS\n"
          "'INJ' 'G' 1 1 2000 'GAS' /\n"
          "'PROD' 'G' 3 3 1000 'OIL' /\n"
          "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);

    SCHEDULESection scheduleSection = SCHEDULESection(*deck);
    DeckTimeStepConstPtr step1 = scheduleSection.getDeckTimeStep(0);
    DeckTimeStepConstPtr step2 = scheduleSection.getDeckTimeStep(1);
    DeckTimeStepConstPtr step3 = scheduleSection.getDeckTimeStep(2);
    DeckTimeStepConstPtr step4 = scheduleSection.getDeckTimeStep(3);
    DeckTimeStepConstPtr step5 = scheduleSection.getDeckTimeStep(4);
    BOOST_CHECK_EQUAL(step1->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step2->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step3->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step4->hasKeyword("WELSPECS"), false);
    BOOST_CHECK_EQUAL(step5->hasKeyword("WELSPECS"), true);

}



