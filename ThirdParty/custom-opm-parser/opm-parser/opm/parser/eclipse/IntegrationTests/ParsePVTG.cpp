/*
  Copyright (C) 2013 by Andreas Lauser

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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>


#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;

const char *pvtgData = "\n\
TABDIMS\n\
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT\n\
     1      2     30    24    10    20  /\n\
\n\
PVTG\n\
--\n\
     20.00    0.00002448   0.061895     0.01299\n\
              0.00001224   0.061810     0.01300\n\
              0.00000000   0.061725     0.01300 /\n\
     40.00    0.00000628   0.030252     0.01383\n\
              0.00000314   0.030249     0.01383\n\
              0.00000000   0.030245     0.01383 /\n\
/\n\
    197.66    0.00006327   1*           0.02160\n\
              0.00003164   *            0.02122\n\
              0.00000000   0.005860     0.02086 /\n\
    231.13    0.00010861   0.005042     0.02477\n\
              0.00005431   0.005061     0.02389\n\
              0.00000000   0.005082     0.02306 /\n\
 /\n";


static void check_parser(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(pvtgData, ParseContext());
    const auto& kw1 = deck->getKeyword("PVTG" , 0);
    BOOST_CHECK_EQUAL(5U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    const auto& record1 = kw1.getRecord(1);
    const auto& record2 = kw1.getRecord(2);
    const auto& record3 = kw1.getRecord(3);
    const auto& record4 = kw1.getRecord(4);

    const auto& item0_0 = record0.getItem("GAS_PRESSURE");
    const auto& item0_1 = record0.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item0_0.size());
    BOOST_CHECK_EQUAL(9U , item0_1.size());
    BOOST_CHECK_EQUAL(2U , record0.size());

    const auto& item1_0 = record1.getItem("GAS_PRESSURE");
    const auto& item1_1 = record1.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item1_0.size());
    BOOST_CHECK_EQUAL(9U , item1_1.size());
    BOOST_CHECK_EQUAL(2U , record1.size());

    const auto& item2_0 = record2.getItem("GAS_PRESSURE");
    const auto& item2_1 = record2.getItem("DATA");
    BOOST_CHECK( item2_0.defaultApplied(0));
    BOOST_CHECK_EQUAL(0U , item2_1.size());
    BOOST_CHECK_EQUAL(2U , record2.size());


    const auto& item3_0 = record3.getItem("GAS_PRESSURE");
    const auto& item3_1 = record3.getItem("DATA");
    BOOST_CHECK( !item3_1.defaultApplied(0));
    BOOST_CHECK( item3_1.defaultApplied(1));
    BOOST_CHECK( !item3_1.defaultApplied(2));
    BOOST_CHECK( !item3_1.defaultApplied(3));
    BOOST_CHECK( item3_1.defaultApplied(4));
    BOOST_CHECK( !item3_1.defaultApplied(5));
    BOOST_CHECK_EQUAL(1U , item3_0.size());
    BOOST_CHECK_EQUAL(9U , item3_1.size());
    BOOST_CHECK_EQUAL(2U , record3.size());


    const auto& item4_0 = record4.getItem("GAS_PRESSURE");
    const auto& item4_1 = record4.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item4_0.size());
    BOOST_CHECK_EQUAL(9U , item4_1.size());
    BOOST_CHECK_EQUAL(2U , record4.size());

    /*
    {
        Opm::PvtgTable pvtgTable;
        pvtgTable.initFORUNITTESTONLY(kw1, 0);

        const auto &outerTable = *pvtgTable.getOuterTable();
        const auto &innerTable0 = *pvtgTable.getInnerTable(0);

        BOOST_CHECK_EQUAL(2U, outerTable.numRows());
        BOOST_CHECK_EQUAL(4U, outerTable.numColumns());
        BOOST_CHECK_EQUAL(3U, innerTable0.numRows());
        BOOST_CHECK_EQUAL(3U, innerTable0.numColumns());

        BOOST_CHECK_EQUAL(20.0e5, outerTable.getPressureColumn()[0]);
        BOOST_CHECK_EQUAL(0.00002448, outerTable.getOilSolubilityColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getOilSolubilityColumn()[0], innerTable0.getOilSolubilityColumn()[0]);
        BOOST_CHECK_EQUAL(0.061895, outerTable.getGasFormationFactorColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getGasFormationFactorColumn()[0], innerTable0.getGasFormationFactorColumn()[0]);
        BOOST_CHECK_EQUAL(1.299e-5, outerTable.getGasViscosityColumn()[0]);
        BOOST_CHECK_EQUAL(outerTable.getGasViscosityColumn()[0], innerTable0.getGasViscosityColumn()[0]);
    }
    */
}


BOOST_AUTO_TEST_CASE( parse_PVTG_OK ) {
    ParserPtr parser(new Parser());
    check_parser( parser );
}
