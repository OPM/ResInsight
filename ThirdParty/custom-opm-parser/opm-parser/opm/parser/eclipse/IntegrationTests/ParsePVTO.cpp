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

#include <opm/parser/eclipse/EclipseState/Tables/PvtoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SimpleTable.hpp>

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

const char *pvtoData = "\n\
TABDIMS\n\
-- NTSFUN NTPVT NSSFUN NPPVT NTFIP NRPVT\n\
     1      2     30    24    10    20  /\n\
\n\
PVTO\n\
--   Rs       PO           BO           MUO\n\
     1e-3     1            1.01         1.02\n\
              250          1.15         0.95\n\
              500          1.20         0.93 /\n\
     1e-2     14.8         1.05         1.03\n\
              251          1.25         0.98\n\
              502          1.30         0.95 /\n\
/\n\
     1e-1     1.1          1.02         1.03\n\
              253          1.16         0.96\n\
              504          1.21         0.97 /\n\
     1e00     15           1.06         1.04\n\
              255          1.26         0.99\n\
              506          1.31         0.96 /\n\
/\n";


static void check_parser(ParserPtr parser) {
    DeckPtr deck =  parser->parseString(pvtoData, ParseContext());
    const auto& kw1 = deck->getKeyword("PVTO" , 0);
    BOOST_CHECK_EQUAL(5U , kw1.size());

    const auto& record0 = kw1.getRecord(0);
    const auto& record1 = kw1.getRecord(1);
    const auto& record2 = kw1.getRecord(2);
    const auto& record3 = kw1.getRecord(3);
    const auto& record4 = kw1.getRecord(4);

    const auto& item0_0 = record0.getItem("RS");
    const auto& item0_1 = record0.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item0_0.size());
    BOOST_CHECK_EQUAL(9U , item0_1.size());
    BOOST_CHECK_EQUAL(2U , record0.size());

    const auto& item1_0 = record1.getItem("RS");
    const auto& item1_1 = record1.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item1_0.size());
    BOOST_CHECK_EQUAL(9U , item1_1.size());
    BOOST_CHECK_EQUAL(2U , record1.size());

    const auto& item2_0 = record2.getItem("RS");
    const auto& item2_1 = record2.getItem("DATA");
    BOOST_CHECK(item2_0.defaultApplied(0));
    BOOST_CHECK_EQUAL(0U , item2_1.size());
    BOOST_CHECK_EQUAL(2U , record2.size());

    const auto& item3_0 = record3.getItem("RS");
    const auto& item3_1 = record3.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item3_0.size());
    BOOST_CHECK_EQUAL(9U , item3_1.size());
    BOOST_CHECK_EQUAL(2U , record3.size());

    const auto& item4_0 = record4.getItem("RS");
    const auto& item4_1 = record4.getItem("DATA");
    BOOST_CHECK_EQUAL(1U , item4_0.size());
    BOOST_CHECK_EQUAL(9U , item4_1.size());
    BOOST_CHECK_EQUAL(2U , record4.size());


    Opm::PvtoTable pvtoTable(kw1 , 0);
    BOOST_CHECK_EQUAL(2, pvtoTable.size());

    const auto &table0 = pvtoTable.getUnderSaturatedTable(0);
    const auto& BO = table0.getColumn( "BO" );

    BOOST_CHECK_EQUAL( 3, table0.numRows());
    BOOST_CHECK_EQUAL( 3, table0.numColumns());
    BOOST_CHECK_EQUAL( BO.front( ) , 1.01 );
    BOOST_CHECK_EQUAL( BO.back( ) , 1.20 );

    BOOST_CHECK_CLOSE(1.15 , table0.evaluate( "BO" , 250*1e5 ) , 1e-6);

    BOOST_CHECK_CLOSE( 1.15 , pvtoTable.evaluate( "BO" , 1e-3 , 250*1e5 ) , 1e-6 );
    BOOST_CHECK_CLOSE( 1.15 , pvtoTable.evaluate( "BO" , 0.0 , 250*1e5 ) , 1e-6 );
}


BOOST_AUTO_TEST_CASE( parse_PVTO_OK ) {
    ParserPtr parser(new Parser());
    check_parser( parser );
}
