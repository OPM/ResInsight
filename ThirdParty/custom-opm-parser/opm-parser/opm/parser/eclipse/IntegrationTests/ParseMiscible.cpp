/*
  Copyright 2015 IRIS AS.
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

#define BOOST_TEST_MODULE ParseMiscible
#include <math.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/SorwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgcwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PmiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MsfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TlpmixpaTable.hpp>

using namespace Opm;

const std::string miscibleData = "\n\
MISCIBLE\n\
2  3 /\n\
\n";

const std::string miscibleTightData = "\n\
MISCIBLE\n\
1  2 /\n\
\n";

const std::string sorwmisData = "\n\
SORWMIS\n\
.00 .00 \n\
.50 .00 \n\
1.0 .00 /\n\
.00 .00 \n\
.30 .20 \n\
1.0 .80 /\n\
\n";

const std::string sgcwmisData = "\n\
SGCWMIS\n\
.00 .00 \n\
.20 .00 \n\
1.0 .00 /\n\
.00 .00 \n\
.80 .20 \n\
1.0 .70 /\n\
\n";

BOOST_AUTO_TEST_CASE( PARSE_SORWMIS)
{
ParserPtr parser(new Parser());

// missing miscible keyword
BOOST_CHECK_THROW (parser->parseString(sorwmisData, ParseContext()), std::invalid_argument );

//too many tables
BOOST_CHECK_THROW( parser->parseString(miscibleTightData + sorwmisData, ParseContext()), std::invalid_argument);

DeckPtr deck1 =  parser->parseString(miscibleData + sorwmisData, ParseContext());

const auto& sorwmis = deck1->getKeyword("SORWMIS");
const auto& miscible = deck1->getKeyword("MISCIBLE");

const auto& miscible0 = miscible.getRecord(0);
const auto& sorwmis0 = sorwmis.getRecord(0);
const auto& sorwmis1 = sorwmis.getRecord(1);


// test number of columns
size_t ntmisc = miscible0.getItem(0).get< int >(0);
Opm::SorwmisTable sorwmisTable0(sorwmis0.getItem(0));
BOOST_CHECK_EQUAL(sorwmisTable0.numColumns(),ntmisc);

// test table input 1
BOOST_CHECK_EQUAL(3U, sorwmisTable0.getWaterSaturationColumn().size());
BOOST_CHECK_EQUAL(1.0, sorwmisTable0.getWaterSaturationColumn()[2]);
BOOST_CHECK_EQUAL(0.0, sorwmisTable0.getMiscibleResidualOilColumn()[2]);

// test table input 2
 Opm::SorwmisTable sorwmisTable1(sorwmis1.getItem(0));
BOOST_CHECK_EQUAL(sorwmisTable1.numColumns(),ntmisc);

BOOST_CHECK_EQUAL(3U, sorwmisTable1.getWaterSaturationColumn().size());
BOOST_CHECK_EQUAL(0.3, sorwmisTable1.getWaterSaturationColumn()[1]);
BOOST_CHECK_EQUAL(0.8, sorwmisTable1.getMiscibleResidualOilColumn()[2]);
}

BOOST_AUTO_TEST_CASE( PARSE_SGCWMIS)
{
    ParserPtr parser(new Parser());

    DeckPtr deck1 =  parser->parseString(miscibleData + sgcwmisData, ParseContext());

    const auto& sgcwmis = deck1->getKeyword("SGCWMIS");
    const auto& miscible = deck1->getKeyword("MISCIBLE");

    const auto& miscible0 = miscible.getRecord(0);
    const auto& sgcwmis0 = sgcwmis.getRecord(0);
    const auto& sgcwmis1 = sgcwmis.getRecord(1);


    // test number of columns
    size_t ntmisc = miscible0.getItem(0).get< int >(0);
    Opm::SgcwmisTable sgcwmisTable0(sgcwmis0.getItem(0));
    BOOST_CHECK_EQUAL(sgcwmisTable0.numColumns(),ntmisc);

    // test table input 1
    BOOST_CHECK_EQUAL(3U, sgcwmisTable0.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(0.2, sgcwmisTable0.getWaterSaturationColumn()[1]);
    BOOST_CHECK_EQUAL(0.0, sgcwmisTable0.getMiscibleResidualGasColumn()[1]);

    // test table input 2
    Opm::SgcwmisTable sgcwmisTable1(sgcwmis1.getItem(0));
    BOOST_CHECK_EQUAL(sgcwmisTable1.numColumns(),ntmisc);

    BOOST_CHECK_EQUAL(3U, sgcwmisTable1.getWaterSaturationColumn().size());
    BOOST_CHECK_EQUAL(0.8, sgcwmisTable1.getWaterSaturationColumn()[1]);
    BOOST_CHECK_EQUAL(0.2, sgcwmisTable1.getMiscibleResidualGasColumn()[1]);
}

const char *miscData = "\n\
MISCIBLE\n\
1  3 /\n\
\n\
MISC\n\
 0.0 0.0 \n\
 0.1 0.5 \n\
 1.0 1.0 /\n\
\n";

const char *miscOutOfRangeData = "\n\
MISCIBLE\n\
1  3 /\n\
\n\
MISC\n\
0.0 0.0 \n\
1.0 0.5 \n\
2.0 1.0 /\n\
\n";

const char *miscTooSmallRangeData = "\n\
MISCIBLE\n\
1  3 /\n\
\n\
MISC\n\
0.0 0.0 \n\
1.0 0.5 /\n\
\n";

BOOST_AUTO_TEST_CASE(PARSE_MISC)
{
    ParserPtr parser(new Parser());

    // out of range MISC keyword
    DeckPtr deck1 = parser->parseString(miscOutOfRangeData, ParseContext());
    const auto& item = deck1->getKeyword("MISC").getRecord(0).getItem(0);
    Opm::MiscTable miscTable1(item);


    // too litle range of MISC keyword
    DeckPtr deck2 = parser->parseString(miscTooSmallRangeData, ParseContext());
    const auto& item2 = deck2->getKeyword("MISC").getRecord(0).getItem(0);
    Opm::MiscTable miscTable2(item2);

    // test table input
    DeckPtr deck3 =  parser->parseString(miscData, ParseContext());
    const auto& item3 = deck3->getKeyword("MISC").getRecord(0).getItem(0);
    Opm::MiscTable miscTable3(item3);
    BOOST_CHECK_EQUAL(3U, miscTable3.getSolventFractionColumn().size());
    BOOST_CHECK_EQUAL(0.1, miscTable3.getSolventFractionColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, miscTable3.getMiscibilityColumn()[1]);
}

const char *pmiscData = "\n\
MISCIBLE\n\
1  3 /\n\
\n\
PMISC\n\
100 0.0 \n\
200 0.5 \n\
500 1.0 /\n\
\n";

BOOST_AUTO_TEST_CASE(PARSE_PMISC)
{
    ParserPtr parser(new Parser());

    // test table input
    DeckPtr deck =  parser->parseString(pmiscData, ParseContext());
    Opm::PmiscTable pmiscTable(deck->getKeyword("PMISC").getRecord(0).getItem(0));
    BOOST_CHECK_EQUAL(3U, pmiscTable.getOilPhasePressureColumn().size());
    BOOST_CHECK_EQUAL(200*1e5, pmiscTable.getOilPhasePressureColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, pmiscTable.getMiscibilityColumn()[1]);
}

const char *msfnData = "\n\
TABDIMS\n\
2 /\n\
\n\
MSFN\n\
0.0 0.0 1.0 \n\
1.0 1.0 0.0 /\n\
0.0 0.0 1.0 \n\
0.5 0.3 0.7 \n\
1.0 1.0 0.0 /\n\
\n";

BOOST_AUTO_TEST_CASE(PARSE_MSFN)
{
ParserPtr parser(new Parser());
DeckPtr deck =  parser->parseString(msfnData, ParseContext());

// test table input 1
 Opm::MsfnTable msfnTable1(deck->getKeyword("MSFN").getRecord(0).getItem(0));
 BOOST_CHECK_EQUAL(2U, msfnTable1.getGasPhaseFractionColumn().size());
 BOOST_CHECK_EQUAL(1.0, msfnTable1.getGasPhaseFractionColumn()[1]);
 BOOST_CHECK_EQUAL(1.0, msfnTable1.getGasSolventRelpermMultiplierColumn()[1]);
 BOOST_CHECK_EQUAL(0.0, msfnTable1.getOilRelpermMultiplierColumn()[1]);

// test table input 2
Opm::MsfnTable msfnTable2(deck->getKeyword("MSFN").getRecord(1).getItem(0));
BOOST_CHECK_EQUAL(3U, msfnTable2.getGasPhaseFractionColumn().size());
BOOST_CHECK_EQUAL(0.5, msfnTable2.getGasPhaseFractionColumn()[1]);
BOOST_CHECK_EQUAL(0.3, msfnTable2.getGasSolventRelpermMultiplierColumn()[1]);
BOOST_CHECK_EQUAL(0.7, msfnTable2.getOilRelpermMultiplierColumn()[1]);
}

const char *tlpmixpa = "\n\
MISCIBLE\n\
1  3 /\n\
\n\
TLPMIXPA\n\
100 0.0 \n\
200 0.5 \n\
500 1.0 /\n\
\n";

BOOST_AUTO_TEST_CASE(PARSE_TLPMIXPA)
{
    ParserPtr parser(new Parser());

    // test table input
    DeckPtr deck =  parser->parseString(tlpmixpa, ParseContext());
    Opm::TlpmixpaTable tlpmixpaTable(deck->getKeyword("TLPMIXPA").getRecord(0).getItem(0));
    BOOST_CHECK_EQUAL(3U, tlpmixpaTable.getOilPhasePressureColumn().size());
    BOOST_CHECK_EQUAL(200*1e5, tlpmixpaTable.getOilPhasePressureColumn()[1]);
    BOOST_CHECK_EQUAL(0.5, tlpmixpaTable.getMiscibilityColumn()[1]);
}
