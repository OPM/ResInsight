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

#define BOOST_TEST_MODULE SimpleTableTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

// generic table classes
#include <opm/parser/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

// keyword specific table classes
#include <opm/parser/eclipse/EclipseState/Tables/PlyrockTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Regdims.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Tabdims.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyadsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlymaxTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/FoamadsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/FoammobTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PbvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PdvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/DenT.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TLMixpar.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <stdexcept>
#include <iostream>

using namespace Opm;

namespace {

Opm::Deck createSingleRecordDeck() {
    const char *deckData =
        "TABDIMS\n"
        " 2 /\n"
        "\n"
        "SWOF\n"
        " 1 2 3 4\n"
        " 5 6 7 8 /\n"
        " 9 10 11 12 /\n";

    Opm::Parser parser;
    return parser.parseString(deckData);
}


Opm::Deck createSingleRecordDeckWithVd() {
    const char *deckData =
        "RUNSPEC\n"
        "ENDSCALE\n"
        "2* 1 2 /\n"
        "PROPS\n"
        "TABDIMS\n"
        " 2 /\n"
        "\n"
        "SWFN\n"
        "0.22 .0   7.0 \n"
        "0.3  .0   4.0 \n"
        "0.5  .24  2.5 \n"
        "0.8  .65  1.0 \n"
        "0.9  .83  .5  \n"
        "1.0  1.00 .0 /\n"
        "/\n"
        "IMPTVD\n"
        "3000.0 6*0.1 0.31 1*0.1\n"
        "9000.0 6*0.1 0.32 1*0.1/\n"
        "ENPTVD\n"
        "3000.0 0.20 0.20 1.0 0.0 0.04 1.0 0.18 0.22\n"
        "9000.0 0.22 0.22 1.0 0.0 0.04 1.0 0.18 0.22 /";

    Opm::Parser parser;
    return parser.parseString(deckData);
}

Opm::Deck createSingleRecordDeckWithJFunc() {
    const char *deckData =
        "RUNSPEC\n"
        "ENDSCALE\n"
        "2* 1 2 /\n"
        "PROPS\n"
        "JFUNC\n"
        "  WATER 22.0 /\n"
        "TABDIMS\n"
        " 2 /\n"
        "\n"
        "SWFN\n"
        "0.22 .0   7.0 \n"
        "0.3  .0   4.0 \n"
        "0.5  .24  2.5 \n"
        "0.8  .65  1.0 \n"
        "0.9  .83  .5  \n"
        "1.0  1.00 .0 /\n"
        "/\n"
        "IMPTVD\n"
        "3000.0 6*0.1 0.31 1*0.1\n"
        "9000.0 6*0.1 0.32 1*0.1/\n"
        "ENPTVD\n"
        "3000.0 0.20 0.20 1.0 0.0 0.04 1.0 0.18 0.22\n"
        "9000.0 0.22 0.22 1.0 0.0 0.04 1.0 0.18 0.22 /";

    Opm::Parser parser;
    return parser.parseString(deckData);
}

Opm::Deck createSingleRecordDeckWithJFuncBoth() {
    const char *deckData =
        "RUNSPEC\nENDSCALE\n2* 1 2 /\nPROPS\n"
        "JFUNC\n * 55.0 88.0 /\n" // ASTERISK MEANS DEFAULT VALUE
        "TABDIMS\n 2 /\n";
    Opm::Parser parser;
    return parser.parseString(deckData);
}

Opm::Deck createSingleRecordDeckWithFullJFunc() {
    const char *deckData =
        "RUNSPEC\nENDSCALE\n2* 1 2 /\nPROPS\n"
        "JFUNC\n WATER 2.7182 3.1416 0.6 0.7 Z /\n"
        "TABDIMS\n 2 /\n";
    Opm::Parser parser;
    return parser.parseString(deckData);
}

Opm::Deck createSingleRecordDeckWithJFuncBrokenFlag() {
    const char *deckData =
        "RUNSPEC\nENDSCALE\n2* 1 2 /\nPROPS\n"
        "JFUNC\n GARBAGE 55.0 88.0 /\n"
        "TABDIMS\n 2 /\n";
    Opm::Parser parser;
    return parser.parseString(deckData);
}

Opm::Deck createSingleRecordDeckWithJFuncBrokenDirection() {
    const char *deckData =
        "RUNSPEC\nENDSCALE\n2* 1 2 /\nPROPS\n"
        "JFUNC\n * * * * * XZ /\n"
        "TABDIMS\n 2 /\n";
    Opm::Parser parser;
    return parser.parseString(deckData);
}


/// used in BOOST_CHECK_CLOSE
static float epsilon() {
    return 0.00001;
}
}

BOOST_AUTO_TEST_CASE( CreateTables ) {
    auto deck = createSingleRecordDeck();
    Opm::TableManager tables(deck);
    auto& tabdims = tables.getTabdims();
    BOOST_CHECK_EQUAL( tabdims.getNumSatTables() , 2 );
    BOOST_CHECK( !tables.useImptvd() );
    BOOST_CHECK( !tables.useEnptvd() );
}

BOOST_AUTO_TEST_CASE( CreateTablesWithVd ) {
    auto deck = createSingleRecordDeckWithVd();
    Opm::TableManager tables(deck);
    auto& tabdims = tables.getTabdims();
    BOOST_CHECK_EQUAL( tabdims.getNumSatTables() , 2 );
    BOOST_CHECK( tables.useImptvd() );
    BOOST_CHECK( tables.useEnptvd() );
}

BOOST_AUTO_TEST_CASE( CreateTablesWithJFunc ) {
    auto deck = createSingleRecordDeckWithJFunc();
    Opm::TableManager tables(deck);
    const Opm::Tabdims& tabdims = tables.getTabdims();
    BOOST_CHECK_EQUAL(tabdims.getNumSatTables(), 2);
    BOOST_CHECK(tables.useImptvd());
    BOOST_CHECK(tables.useEnptvd());

    const auto& swfnTab = tables.getSwfnTables();

    const float swfnDataVerbatim[] =
        {0.22, 0.00, 7.00, 0.30, 0.00, 4.00, 0.50, 0.24, 2.50,
         0.80, 0.65, 1.00, 0.90, 0.83, 0.50, 1.00, 1.00, 0.00};


    for (size_t tab = 0; tab < swfnTab.size(); tab++) {
        const auto& t = swfnTab.getTable(tab);

        //TODO uncomment BOOST_CHECK_THROW( t.getColumn("PCOW"), std::invalid_argument );

        for (size_t c_idx = 0; c_idx < t.numColumns(); c_idx++) {
            const auto& col = t.getColumn(c_idx);
            for (size_t i = 0; i < col.size(); i++) {
                int idx = c_idx + i*3;
                BOOST_CHECK_CLOSE( col[i], swfnDataVerbatim[idx], epsilon());
            }
        }
    }

    const auto& tt = swfnTab.getTable<Opm::SwfnTable>(0);
    //TODO uncomment BOOST_CHECK_THROW(tt.getPcowColumn(), std::invalid_argument);

    const auto& col = tt.getJFuncColumn();
    for (size_t i = 0; i < col.size(); i++) {
        BOOST_CHECK_CLOSE(col[i], swfnDataVerbatim[i*3 + 2], epsilon());
    }

    BOOST_CHECK(tables.useJFunc());
}
/*****************************************************************/



BOOST_AUTO_TEST_CASE(SwofTable_Tests) {
    const char *deckData =
        "TABDIMS\n"
        "2 /\n"
        "\n"
        "SWOF\n"
        " 1 2 3 4\n"
        " 5 6 7 8/\n"
        "  9 10 11 12\n"
        " 13 14 15 16\n"
        " 17 18 19 20/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);

    Opm::SwofTable swof1Table(deck.getKeyword("SWOF").getRecord(0).getItem(0), false);
    Opm::SwofTable swof2Table(deck.getKeyword("SWOF").getRecord(1).getItem(0), false);

    BOOST_CHECK_EQUAL(swof1Table.numRows(), 2);
    BOOST_CHECK_EQUAL(swof2Table.numRows(), 3);

    BOOST_CHECK_EQUAL(swof1Table.numColumns(), 4);
    BOOST_CHECK_EQUAL(swof2Table.numColumns(), 4);

    BOOST_CHECK_EQUAL(swof1Table.getSwColumn().front(), 1.0);
    BOOST_CHECK_EQUAL(swof1Table.getSwColumn().back(), 5.0);

    BOOST_CHECK_EQUAL(swof1Table.getKrwColumn().front(), 2.0);
    BOOST_CHECK_EQUAL(swof1Table.getKrwColumn().back(), 6.0);

    BOOST_CHECK_EQUAL(swof1Table.getKrowColumn().front(), 3.0);
    BOOST_CHECK_EQUAL(swof1Table.getKrowColumn().back(), 7.0);

    BOOST_CHECK_EQUAL(swof1Table.getPcowColumn().front(), 4.0e5);
    BOOST_CHECK_EQUAL(swof1Table.getPcowColumn().back(), 8.0e5);

    // for the second table, we only check the first column and trust
    // that everything else is fine...
    BOOST_CHECK_EQUAL(swof2Table.getSwColumn().front(), 9.0);
    BOOST_CHECK_EQUAL(swof2Table.getSwColumn().back(), 17.0);
}

BOOST_AUTO_TEST_CASE(PbvdTable_Tests) {
    const char *deckData =
        "EQLDIMS\n"
        "2 /\n"
        "\n"
        "PBVD\n"
        " 1 1 \n"
        " 2 1 / \n"
        "  3 2\n"
        " 2 2\n"
        " 1 2/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);

    Opm::PbvdTable pbvdTable1(deck.getKeyword("PBVD").getRecord(0).getItem(0));

    BOOST_CHECK_EQUAL(pbvdTable1.numRows(), 2);
    BOOST_CHECK_EQUAL(pbvdTable1.numColumns(), 2);
    BOOST_CHECK_EQUAL(pbvdTable1.getDepthColumn().front(), 1);
    BOOST_CHECK_EQUAL(pbvdTable1.getDepthColumn().back(), 2);
    BOOST_CHECK_EQUAL(pbvdTable1.getPbubColumn().front(), 100000); // 1 barsa

    // depth must be increasing down the column.
    BOOST_CHECK_THROW(Opm::PbvdTable pbvdTable2(deck.getKeyword("PBVD").getRecord(1).getItem(0)), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(PdvdTable_Tests) {
    const char *deckData =
        "EQLDIMS\n"
        "2 /\n"
        "\n"
        "PDVD\n"
        " 1 1 \n"
        " 2 1 / \n"
        "  3 2\n"
        " 2 2\n"
        " 1 2/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);

    Opm::PdvdTable pdvdTable1(deck.getKeyword("PDVD").getRecord(0).getItem(0));

    BOOST_CHECK_EQUAL(pdvdTable1.numRows(), 2);
    BOOST_CHECK_EQUAL(pdvdTable1.numColumns(), 2);
    BOOST_CHECK_EQUAL(pdvdTable1.getDepthColumn().front(), 1);
    BOOST_CHECK_EQUAL(pdvdTable1.getDepthColumn().back(), 2);
    BOOST_CHECK_EQUAL(pdvdTable1.getPdewColumn().front(), 100000); // 1 barsa

    // depth must be increasing down the column.
    BOOST_CHECK_THROW(Opm::PdvdTable pdvdTable2(deck.getKeyword("PDVD").getRecord(1).getItem(0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(SgwfnTable_Tests) {
    const char *deckData =
        "TABDIMS\n"
        "2 /\n"
        "\n"
        "SGWFN\n"
        " 1 2 3 4\n"
        " 5 6 7 8/\n"
        "  9 10 11 12\n"
        " 13 14 15 16\n"
        " 17 18 19 20/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);


    Opm::SgwfnTable sgwfn1Table(deck.getKeyword("SGWFN").getRecord(0).getItem(0));
    Opm::SgwfnTable sgwfn2Table(deck.getKeyword("SGWFN").getRecord(1).getItem(0));

    BOOST_CHECK_EQUAL(sgwfn1Table.numRows(), 2);
    BOOST_CHECK_EQUAL(sgwfn2Table.numRows(), 3);

    BOOST_CHECK_EQUAL(sgwfn1Table.numColumns(), 4);
    BOOST_CHECK_EQUAL(sgwfn2Table.numColumns(), 4);

    BOOST_CHECK_EQUAL(sgwfn1Table.getSgColumn().front(), 1.0);
    BOOST_CHECK_EQUAL(sgwfn1Table.getSgColumn().back(), 5.0);

    BOOST_CHECK_EQUAL(sgwfn1Table.getKrgColumn().front(), 2.0);
    BOOST_CHECK_EQUAL(sgwfn1Table.getKrgColumn().back(), 6.0);

    BOOST_CHECK_EQUAL(sgwfn1Table.getKrgwColumn().front(), 3.0);
    BOOST_CHECK_EQUAL(sgwfn1Table.getKrgwColumn().back(), 7.0);

    BOOST_CHECK_EQUAL(sgwfn1Table.getPcgwColumn().front(), 4.0e5);
    BOOST_CHECK_EQUAL(sgwfn1Table.getPcgwColumn().back(), 8.0e5);

    // for the second table, we only check the first column and trust
    // that everything else is fine...
    BOOST_CHECK_EQUAL(sgwfn2Table.getSgColumn().front(), 9.0);
    BOOST_CHECK_EQUAL(sgwfn2Table.getSgColumn().back(), 17.0);
}

BOOST_AUTO_TEST_CASE(SgofTable_Tests) {
    const char *deckData =
        "TABDIMS\n"
        "2 /\n"
        "\n"
        "SGOF\n"
        " 1 2 3 4\n"
        " 5 6 7 8/\n"
        "  9 10 11 12\n"
        " 13 14 15 16\n"
        " 17 18 19 20/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);

    Opm::SgofTable sgof1Table(deck.getKeyword("SGOF").getRecord(0).getItem(0), false);
    Opm::SgofTable sgof2Table(deck.getKeyword("SGOF").getRecord(1).getItem(0), false);

    BOOST_CHECK_EQUAL(sgof1Table.numRows(), 2);
    BOOST_CHECK_EQUAL(sgof2Table.numRows(), 3);

    BOOST_CHECK_EQUAL(sgof1Table.numColumns(), 4);
    BOOST_CHECK_EQUAL(sgof2Table.numColumns(), 4);

    BOOST_CHECK_EQUAL(sgof1Table.getSgColumn().front(), 1.0);
    BOOST_CHECK_EQUAL(sgof1Table.getSgColumn().back(), 5.0);

    BOOST_CHECK_EQUAL(sgof1Table.getKrgColumn().front(), 2.0);
    BOOST_CHECK_EQUAL(sgof1Table.getKrgColumn().back(), 6.0);

    BOOST_CHECK_EQUAL(sgof1Table.getKrogColumn().front(), 3.0);
    BOOST_CHECK_EQUAL(sgof1Table.getKrogColumn().back(), 7.0);

    BOOST_CHECK_EQUAL(sgof1Table.getPcogColumn().front(), 4.0e5);
    BOOST_CHECK_EQUAL(sgof1Table.getPcogColumn().back(), 8.0e5);

    // for the second table, we only check the first column and trust
    // that everything else is fine...
    BOOST_CHECK_EQUAL(sgof2Table.getSgColumn().front(), 9.0);
    BOOST_CHECK_EQUAL(sgof2Table.getSgColumn().back(), 17.0);
}

BOOST_AUTO_TEST_CASE(PlyadsTable_Tests) {
    {
        const char *correctDeckData =
            "TABDIMS\n"
            "/\n"
            "PLYADS\n"
            "0.00    0.0 \n"
            "0.25    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000030 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(correctDeckData);
        const auto& plyadsKeyword = deck.getKeyword("PLYADS");
        Opm::PlyadsTable plyadsTable(plyadsKeyword.getRecord(0).getItem(0));


        BOOST_CHECK_CLOSE(plyadsTable.getPolymerConcentrationColumn().front(), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(plyadsTable.getPolymerConcentrationColumn().back(), 3.0, 1e-6);

        BOOST_CHECK_CLOSE(plyadsTable.getAdsorbedPolymerColumn().front(), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(plyadsTable.getAdsorbedPolymerColumn().back(), 0.000030, 1e-6);
    }

    {
        // first column not strictly monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "PLYADS\n"
            "0.00    0.0 \n"
            "0.00    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000030 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& plyadsKeyword = deck.getKeyword("PLYADS");

        BOOST_CHECK_THROW(Opm::PlyadsTable(plyadsKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }

    {
        // second column not monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "PLYADS\n"
            "0.00    0.0 \n"
            "0.25    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000029 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& plyadsKeyword = deck.getKeyword("PLYADS");

        BOOST_CHECK_THROW(Opm::PlyadsTable(plyadsKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(FoamadsTable_Tests) {
    {
        const char *correctDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMADS\n"
            "0.00    0.0 \n"
            "0.25    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000030 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(correctDeckData);
        const auto& foamadsKeyword = deck.getKeyword("FOAMADS");
        Opm::FoamadsTable foamadsTable(foamadsKeyword.getRecord(0).getItem(0));


        BOOST_CHECK_CLOSE(foamadsTable.getFoamConcentrationColumn().front(), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(foamadsTable.getFoamConcentrationColumn().back(), 3.0, 1e-6);

        BOOST_CHECK_CLOSE(foamadsTable.getAdsorbedFoamColumn().front(), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(foamadsTable.getAdsorbedFoamColumn().back(), 0.000030, 1e-6);
    }

    {
        // first column not strictly monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMADS\n"
            "0.00    0.0 \n"
            "0.00    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000030 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& foamadsKeyword = deck.getKeyword("FOAMADS");

        BOOST_CHECK_THROW(Opm::FoamadsTable(foamadsKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }

    {
        // second column not monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMADS\n"
            "0.00    0.0 \n"
            "0.25    0.000010\n"
            "0.50    0.000018\n"
            "0.75    0.000023\n"
            "1.00    0.000027\n"
            "1.25    0.000030\n"
            "1.50    0.000030\n"
            "1.75    0.000030\n"
            "2.00    0.000030\n"
            "3.00    0.000029 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& foamadsKeyword = deck.getKeyword("FOAMADS");

        BOOST_CHECK_THROW(Opm::FoamadsTable(foamadsKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(FoammobTable_Tests) {
    {
        const char *correctDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMMOB\n"
            "0.00    1.0 \n"
            "0.01    0.5\n"
            "0.02    0.1\n"
            "0.03    0.1 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(correctDeckData);
        const auto& foammobKeyword = deck.getKeyword("FOAMMOB");
        Opm::FoammobTable foammobTable(foammobKeyword.getRecord(0).getItem(0));


        BOOST_CHECK_CLOSE(foammobTable.getFoamConcentrationColumn().front(), 0.0, 1e-6);
        BOOST_CHECK_CLOSE(foammobTable.getFoamConcentrationColumn().back(), 0.03, 1e-6);

        BOOST_CHECK_CLOSE(foammobTable.getMobilityMultiplierColumn().front(), 1.0, 1e-6);
        BOOST_CHECK_CLOSE(foammobTable.getMobilityMultiplierColumn().back(), 0.1, 1e-6);
    }

    {
        // first column not strictly monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMMOB\n"
            "0.00    1.0 \n"
            "0.01    0.5\n"
            "0.02    0.1\n"
            "0.02    0.1 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& foammobKeyword = deck.getKeyword("FOAMMOB");

        BOOST_CHECK_THROW(Opm::FoammobTable(foammobKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }

    {
        // second column not monotonic
        const char *incorrectDeckData =
            "TABDIMS\n"
            "/\n"
            "FOAMMOB\n"
            "0.00    1.0 \n"
            "0.01    0.5\n"
            "0.02    0.1\n"
            "0.03    0.11 /\n";
        Opm::Parser parser;
        auto deck = parser.parseString(incorrectDeckData);
        const auto& foammobKeyword = deck.getKeyword("FOAMMOB");

        BOOST_CHECK_THROW(Opm::FoammobTable(foammobKeyword.getRecord(0).getItem(0)), std::invalid_argument);
    }
}




/**
 * Tests "happy path" for a VFPPROD table, i.e., when everything goes well
 */
BOOST_AUTO_TEST_CASE(VFPProdTable_happy_Test) {
    const char *deckData = "\
VFPPROD \n\
-- Table Depth  Rate   WFR   GFR   TAB ALQ    UNITS  BODY    \n\
-- ----- ----- ----- ----- ----- ----- --- -------- -----    \n\
      5  32.9  'LIQ' 'WCT' 'GOR' 'THP' ' ' 'METRIC' 'BHP'  / \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- WFR axis  \n\
13 17 /      \n\
-- GFR axis  \n\
19 23 /      \n\
-- ALQ axis  \n\
29 31 /      \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
1 1 1 1 1.5 2.5 3.5 /    \n\
2 1 1 1 4.5 5.5 6.5 /    \n\
1 2 1 1 7.5 8.5 9.5 /    \n\
2 2 1 1 10.5 11.5 12.5 / \n\
1 1 2 1 13.5 14.5 15.5 / \n\
2 1 2 1 16.5 17.5 18.5 / \n\
1 2 2 1 19.5 20.5 21.5 / \n\
2 2 2 1 22.5 23.5 24.5 / \n\
1 1 1 2 25.5 26.5 27.5 / \n\
2 1 1 2 28.5 29.5 30.5 / \n\
1 2 1 2 31.5 32.5 33.5 / \n\
2 2 1 2 34.5 35.5 36.5 / \n\
1 1 2 2 37.5 38.5 39.5 / \n\
2 1 2 2 40.5 41.5 42.5 / \n\
1 2 2 2 43.5 44.5 45.5 / \n\
2 2 2 2 46.5 47.5 48.5 / \n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    auto units = Opm::UnitSystem::newMETRIC();
    const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");

    BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

    Opm::VFPProdTable vfpprodTable(vfpprodKeyword, units);


    BOOST_CHECK_EQUAL(vfpprodTable.getTableNum(), 5);
    BOOST_CHECK_EQUAL(vfpprodTable.getDatumDepth(), 32.9);
    BOOST_CHECK_EQUAL(vfpprodTable.getFloType(), Opm::VFPProdTable::FLO_LIQ);
    BOOST_CHECK_EQUAL(vfpprodTable.getWFRType(), Opm::VFPProdTable::WFR_WCT);
    BOOST_CHECK_EQUAL(vfpprodTable.getGFRType(), Opm::VFPProdTable::GFR_GOR);
    BOOST_CHECK_EQUAL(vfpprodTable.getALQType(), Opm::VFPProdTable::ALQ_UNDEF);

    //Flo axis
    {
        const std::vector<double>& flo = vfpprodTable.getFloAxis();
        BOOST_REQUIRE_EQUAL(flo.size(), 3);

        //Unit of FLO is SM3/day, convert to SM3/second
        double conversion_factor = 1.0 / (60*60*24);
        BOOST_CHECK_EQUAL(flo[0], 1*conversion_factor);
        BOOST_CHECK_EQUAL(flo[1], 3*conversion_factor);
        BOOST_CHECK_EQUAL(flo[2], 5*conversion_factor);
    }

    //THP axis
    {
        const std::vector<double>& thp = vfpprodTable.getTHPAxis();
        BOOST_REQUIRE_EQUAL(thp.size(), 2);

        //Unit of THP is barsa => convert to pascal
        double conversion_factor = 100000.0;
        BOOST_CHECK_EQUAL(thp[0], 7*conversion_factor);
        BOOST_CHECK_EQUAL(thp[1], 11*conversion_factor);
    }

    //WFR axis
    {
        const std::vector<double>& wfr = vfpprodTable.getWFRAxis();
        BOOST_REQUIRE_EQUAL(wfr.size(), 2);

        //Unit of WFR is SM3/SM3
        BOOST_CHECK_EQUAL(wfr[0], 13);
        BOOST_CHECK_EQUAL(wfr[1], 17);
    }

    //GFR axis
    {
        const std::vector<double>& gfr = vfpprodTable.getGFRAxis();
        BOOST_REQUIRE_EQUAL(gfr.size(), 2);

        //Unit of GFR is SM3/SM3
        BOOST_CHECK_EQUAL(gfr[0], 19);
        BOOST_CHECK_EQUAL(gfr[1], 23);
    }

    //ALQ axis
    {
        const std::vector<double>& alq = vfpprodTable.getALQAxis();
        BOOST_REQUIRE_EQUAL(alq.size(), 2);

        //Unit of ALQ undefined
        BOOST_CHECK_EQUAL(alq[0], 29);
        BOOST_CHECK_EQUAL(alq[1], 31);
    }

    //The data itself
    {
        typedef Opm::VFPProdTable::array_type::size_type size_type;
        const auto size = vfpprodTable.shape();

        BOOST_CHECK_EQUAL(size[0], 2);
        BOOST_CHECK_EQUAL(size[1], 2);
        BOOST_CHECK_EQUAL(size[2], 2);
        BOOST_CHECK_EQUAL(size[3], 2);
        BOOST_CHECK_EQUAL(size[4], 3);

        //Table given as BHP => barsa. Convert to pascal
        double conversion_factor = 100000.0;

        double index = 0.5;
        for (size_type a = 0; a < size[3]; ++a) {
            for (size_type g = 0;  g < size[2]; ++g) {
                for (size_type w = 0; w < size[1]; ++w) {
                    for (size_type t = 0; t < size[0]; ++t) {
                        for (size_type f = 0; f < size[4]; ++f) {
                            index += 1.0;
                            BOOST_CHECK_EQUAL(const_cast<const VFPProdTable&>(vfpprodTable)(t,w,g,a,f), index*conversion_factor);
                        }
                    }
                }
            }
        }
    }
}



/**
 * Checks that the VFPPROD table will succeed with a minimal set of
 * specified values.
 */
BOOST_AUTO_TEST_CASE(VFPProdTable_minimal_Test) {
    const char *deckData = "\
VFPPROD \n\
-- Table Depth  Rate   WFR   GFR      \n\
-- ----- ----- ----- ----- -----      \n\
      5  32.9  'LIQ' 'WCT' 'GOR'    / \n\
-- Rate axis \n\
1 /          \n\
-- THP axis  \n\
7 /          \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
29 /         \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
1 1 1 1 1.5 /    \n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
    auto units = Opm::UnitSystem::newMETRIC();

    BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

    Opm::VFPProdTable vfpprodTable(vfpprodKeyword, units);

    BOOST_CHECK_EQUAL(vfpprodTable.getTableNum(), 5);
    BOOST_CHECK_EQUAL(vfpprodTable.getDatumDepth(), 32.9);
    BOOST_CHECK_EQUAL(vfpprodTable.getFloType(), Opm::VFPProdTable::FLO_LIQ);
    BOOST_CHECK_EQUAL(vfpprodTable.getWFRType(), Opm::VFPProdTable::WFR_WCT);
    BOOST_CHECK_EQUAL(vfpprodTable.getGFRType(), Opm::VFPProdTable::GFR_GOR);
    BOOST_CHECK_EQUAL(vfpprodTable.getALQType(), Opm::VFPProdTable::ALQ_UNDEF);

    //Flo axis
    {
        const std::vector<double>& flo = vfpprodTable.getFloAxis();
        BOOST_REQUIRE_EQUAL(flo.size(), 1);

        //Unit of FLO is SM3/day, convert to SM3/second
        double conversion_factor = 1.0 / (60*60*24);
        BOOST_CHECK_EQUAL(flo[0], 1*conversion_factor);
    }

    //THP axis
    {
        const std::vector<double>& thp = vfpprodTable.getTHPAxis();
        BOOST_REQUIRE_EQUAL(thp.size(), 1);

        //Unit of THP is barsa => convert to pascal
        double conversion_factor = 100000.0;
        BOOST_CHECK_EQUAL(thp[0], 7*conversion_factor);
    }

    //WFR axis
    {
        const std::vector<double>& wfr = vfpprodTable.getWFRAxis();
        BOOST_REQUIRE_EQUAL(wfr.size(), 1);

        //Unit of WFR is SM3/SM3
        BOOST_CHECK_EQUAL(wfr[0], 13);
    }

    //GFR axis
    {
        const std::vector<double>& gfr = vfpprodTable.getGFRAxis();
        BOOST_REQUIRE_EQUAL(gfr.size(), 1);

        //Unit of GFR is SM3/SM3
        BOOST_CHECK_EQUAL(gfr[0], 19);
    }

    //ALQ axis
    {
        const std::vector<double>& alq = vfpprodTable.getALQAxis();
        BOOST_REQUIRE_EQUAL(alq.size(), 1);

        //Unit of ALQ undefined
        BOOST_CHECK_EQUAL(alq[0], 29);
    }

    //The data itself
    {
        const auto size = vfpprodTable.shape();

        //Table given as BHP => barsa. Convert to pascal
        double conversion_factor = 100000.0;

        BOOST_CHECK_EQUAL(size[0]*size[1]*size[2]*size[3]*size[4], 1);
        BOOST_CHECK_EQUAL(const_cast<const VFPProdTable&>(vfpprodTable)(0,0,0,0,0), 1.5*conversion_factor);
    }
}

BOOST_AUTO_TEST_CASE(JFuncTestThrowingOnBrokenData) {
    auto deck = createSingleRecordDeckWithJFuncBrokenFlag();
    BOOST_CHECK_THROW(Opm::TableManager tm (deck), std::invalid_argument);

    auto deck2 = createSingleRecordDeckWithJFuncBrokenDirection();
    BOOST_CHECK_THROW(Opm::TableManager tm2 (deck2), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(JFuncTestThrowingGalore) {
    auto deck = createSingleRecordDeckWithVd();
    Opm::TableManager tables(deck);
    BOOST_CHECK(!tables.useJFunc());
    //TODO uncomment BOOST_CHECK_THROW(tables.getJFunc(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(JFuncTest) {
    const auto deck = createSingleRecordDeckWithJFuncBoth();
    Opm::TableManager tables(deck);
    BOOST_CHECK(tables.useJFunc());

    const Opm::JFunc& jt = tables.getJFunc();
    BOOST_CHECK(jt.flag() == Opm::JFunc::Flag::BOTH);
    BOOST_CHECK_CLOSE(jt.owSurfaceTension(), 55.0, epsilon());
    BOOST_CHECK_CLOSE(jt.goSurfaceTension(), 88.0, epsilon());
    BOOST_CHECK_CLOSE(jt.alphaFactor(), 0.5, epsilon()); // default
    BOOST_CHECK_CLOSE(jt.betaFactor(),  0.5, epsilon());  // default
    BOOST_CHECK(jt.direction() == Opm::JFunc::Direction::XY); // default

    // full specification = WATER 2.7182 3.1416 0.6 0.7 Z
    const auto deck2 = createSingleRecordDeckWithFullJFunc();
    Opm::TableManager tables2(deck2);
    BOOST_CHECK(tables2.useJFunc());

    const auto& jt2 = tables2.getJFunc();
    BOOST_CHECK(jt2.flag() == Opm::JFunc::Flag::WATER);
    BOOST_CHECK_CLOSE(jt2.owSurfaceTension(), 2.7182, epsilon());
    BOOST_CHECK_THROW(jt2.goSurfaceTension(), std::invalid_argument);
    BOOST_CHECK_CLOSE(jt2.alphaFactor(), 0.6, epsilon()); // default
    BOOST_CHECK_CLOSE(jt2.betaFactor(),  0.7, epsilon());  // default
    BOOST_CHECK(jt2.direction() == Opm::JFunc::Direction::Z); // default
}



/**
 * Spot checks that the VFPPROD table will fail nicely when given invalid data
 */
BOOST_AUTO_TEST_CASE(VFPProdTable_sad_Test) {
    /**
     * Missing value in table
     */
    {
        const char *missing_values = "\
VFPPROD \n\
-- Table Depth  Rate   WFR   GFR      \n\
-- ----- ----- ----- ----- -----      \n\
      5  32.9  'LIQ' 'WCT' 'GOR'    / \n\
-- Rate axis \n\
1 2 /        \n\
-- THP axis  \n\
7 /          \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
29 /         \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
-- Will fail, as rate axis requires two elements            \n\
1 1 1 1 1.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_values);
        const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpprodKeyword, units), std::invalid_argument);
    }



    /**
     * Missing value in table #2
     */
    {
        const char *missing_values = "\
VFPPROD \n\
-- Table Depth  Rate   WFR   GFR      \n\
-- ----- ----- ----- ----- -----      \n\
      5  32.9  'LIQ' 'WCT' 'GOR'    / \n\
-- Rate axis \n\
1 /          \n\
-- THP axis  \n\
7 9 /        \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
29 /         \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
-- Will fail, as two entries are required                   \n\
1 1 1 1 1.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_values);
        const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpprodKeyword, units), std::invalid_argument);
    }


    /**
     * Missing items in header
     */
    {
        const char *missing_metadata = "\
VFPPROD \n\
-- Table Depth   \n\
-- ----- -----   \n\
      5  32.9  / \n\
-- Rate axis \n\
1 2 /        \n\
-- THP axis  \n\
7 /          \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
29 /         \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
1 1 1 1 1.5 2.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_metadata);
        const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpprodKeyword, units), std::invalid_argument);
    }



    /**
     * Wrong items in header
     */
    {
        const char *wrong_metadata = "\
VFPPROD \n\
-- Table Depth   \n\
-- ----- -----   \n\
      5  32.9  'WCT' 'LIC' 'GARBAGE'    / \n\
-- Rate axis \n\
1 2 /        \n\
-- THP axis  \n\
7 /          \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
29 /         \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
1 1 1 1 1.5 2.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(wrong_metadata);
        const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpprodKeyword, units), std::invalid_argument);
    }



    /**
     * Wrong axes in header
     */
    {
        const char *missing_axes = "\
VFPPROD \n\
-- Table Depth   \n\
-- ----- -----   \n\
      5  32.9  'LIC' 'WCT' 'OGR'    / \n\
-- Rate axis \n\
1 2 /        \n\
-- THP axis  \n\
7 /          \n\
-- WFR axis  \n\
13 /         \n\
-- GFR axis  \n\
19 /         \n\
-- ALQ axis  \n\
-- Missing!  \n\
-- Table data with THP# WFR# GFR# ALQ# <values 1-num_rates> \n\
1 1 1 1 1.5 2.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_axes);
        const auto& vfpprodKeyword = deck.getKeyword("VFPPROD");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPPROD"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpprodKeyword, units), std::invalid_argument);
    }
}





/**
 * Tests "happy path" for a VFPPROD table, i.e., when everything goes well
 */
BOOST_AUTO_TEST_CASE(VFPInjTable_happy_Test) {
    const char *deckData = "\
VFPINJ \n\
-- Table Depth  Rate   TAB  UNITS  BODY    \n\
-- ----- ----- ----- ----- ------ -----    \n\
       5  32.9   WAT   THP METRIC   BHP /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- Table data with THP# <values 1-num_rates> \n\
1 1.5 2.5 3.5 /    \n\
2 4.5 5.5 6.5 /    \n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const auto& vfpprodKeyword = deck.getKeyword("VFPINJ");
    auto units = Opm::UnitSystem::newMETRIC();

    BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

    Opm::VFPInjTable vfpinjTable(vfpprodKeyword, units);

    BOOST_CHECK_EQUAL(vfpinjTable.getTableNum(), 5);
    BOOST_CHECK_EQUAL(vfpinjTable.getDatumDepth(), 32.9);
    BOOST_CHECK_EQUAL(vfpinjTable.getFloType(), Opm::VFPInjTable::FLO_WAT);

    //Flo axis
    {
        const std::vector<double>& flo = vfpinjTable.getFloAxis();
        BOOST_REQUIRE_EQUAL(flo.size(), 3);

        //Unit of FLO is SM3/day, convert to SM3/second
        double conversion_factor = 1.0 / (60*60*24);
        BOOST_CHECK_EQUAL(flo[0], 1*conversion_factor);
        BOOST_CHECK_EQUAL(flo[1], 3*conversion_factor);
        BOOST_CHECK_EQUAL(flo[2], 5*conversion_factor);
    }

    //THP axis
    {
        const std::vector<double>& thp = vfpinjTable.getTHPAxis();
        BOOST_REQUIRE_EQUAL(thp.size(), 2);

        //Unit of THP is barsa => convert to pascal
        double conversion_factor = 100000.0;
        BOOST_CHECK_EQUAL(thp[0], 7*conversion_factor);
        BOOST_CHECK_EQUAL(thp[1], 11*conversion_factor);
    }

    //The data itself
    {
        typedef Opm::VFPInjTable::array_type::size_type size_type;
        const auto size = vfpinjTable.shape();

        BOOST_CHECK_EQUAL(size[0], 2);
        BOOST_CHECK_EQUAL(size[1], 3);

        //Table given as BHP => barsa. Convert to pascal
        double conversion_factor = 100000.0;

        double index = 0.5;
        for (size_type t = 0; t < size[0]; ++t) {
            for (size_type f = 0; f < size[1]; ++f) {
                index += 1.0;
                BOOST_CHECK_EQUAL(const_cast<const VFPInjTable&>(vfpinjTable)(t,f), index*conversion_factor);
            }
        }
    }
}











BOOST_AUTO_TEST_CASE(TestTableContainer) {
    auto deck = createSingleRecordDeck();
    Opm::TableManager tables( deck );
    BOOST_CHECK_EQUAL( false , tables.hasTables("SGOF") );
    BOOST_CHECK_EQUAL( false , tables.hasTables("STUPID") );

    BOOST_CHECK_THROW( tables.getTables("STUPID") , std::invalid_argument);
    BOOST_CHECK_THROW( tables["STUPID"] , std::invalid_argument);
}

/**
 * Spot checks that the VFPPROD table will fail nicely when given invalid data
 */
BOOST_AUTO_TEST_CASE(VFPInjTable_sad_Test) {
    /**
     * Missing value in table
     */
    {
        const char *missing_values = "\
VFPINJ \n\
-- Table Depth  Rate   TAB  UNITS  BODY    \n\
-- ----- ----- ----- ----- ------ -----    \n\
       5  32.9   WAT   THP METRIC   BHP /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- Table data with THP# <values 1-num_rates> \n\
-- Will fail, as rate axis requires three elements  \n\
1 1.5 2.5 /    \n\
2 4.5 5.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_values);
        const auto& vfpinjKeyword = deck.getKeyword("VFPINJ");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpinjKeyword, units), std::invalid_argument);
    }



    /**
     * Missing value in table #2
     */
    {
        const char *missing_values = "\
VFPINJ \n\
-- Table Depth  Rate   TAB  UNITS  BODY    \n\
-- ----- ----- ----- ----- ------ -----    \n\
       5  32.9   WAT   THP METRIC   BHP /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- Table data with THP# <values 1-num_rates> \n\
-- Will fail, as two entries are required                   \n\
1 1.5 2.5 3.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_values);
        const auto& vfpinjKeyword = deck.getKeyword("VFPINJ");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpinjKeyword, units), std::invalid_argument);
    }


    /**
     * Missing items in header
     */
    {
        const char *missing_metadata = "\
VFPINJ \n\
-- Table Depth      \n\
-- ----- -----      \n\
       5  32.9   /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- Table data with THP# <values 1-num_rates> \n\
1 1.5 2.5 3.5 /    \n\
2 4.5 5.5 6.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_metadata);
        const auto& vfpinjKeyword = deck.getKeyword("VFPINJ");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpinjKeyword, units), std::invalid_argument);
    }



    /**
     * Wrong items in header
     */
    {
        const char *wrong_metadata = "\
VFPINJ \n\
-- Table Depth  Rate   TAB  UNITS  BODY    \n\
-- ----- ----- ----- ----- ------ -----    \n\
       5  32.9   GOR   BHP    FOO  GAGA /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
7 11 /       \n\
-- Table data with THP# <values 1-num_rates> \n\
1 1.5 2.5 3.5 /    \n\
2 4.5 5.5 6.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(wrong_metadata);
        const auto& vfpinjKeyword = deck.getKeyword("VFPINJ");
        auto units(Opm::UnitSystem::newMETRIC());
        BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpinjKeyword, units), std::invalid_argument);
    }



    /**
     * Wrong axes in header
     */
    {
        const char *missing_axes = "\
VFPINJ \n\
-- Table Depth  Rate   TAB  UNITS  BODY    \n\
-- ----- ----- ----- ----- ------ -----    \n\
       5  32.9   WAT   THP METRIC   BHP /  \n\
-- Rate axis \n\
1 3 5 /      \n\
-- THP axis  \n\
-- Missing!  \n\
-- Table data with THP# <values 1-num_rates> \n\
1 1.5 2.5 3.5 /    \n\
2 4.5 5.5 6.5 /    \n";

        Opm::Parser parser;
        auto deck = parser.parseString(missing_axes);
        const auto& vfpinjKeyword = deck.getKeyword("VFPINJ");
        auto units = Opm::UnitSystem::newMETRIC();
        BOOST_CHECK_EQUAL(deck.count("VFPINJ"), 1);

        BOOST_CHECK_THROW(Opm::VFPProdTable(vfpinjKeyword, units), std::invalid_argument);
    }
}


BOOST_AUTO_TEST_CASE( TestPLYMWINJ ) {
    const char *inputstring =
        "PLYMWINJ \n"
        "   2   /    -- table number \n"
        "   0.0     200.0   800.0   / -- throughput values \n"
        "   0.0     1.0   2.0  3.0  / -- velocity values \n"
        "   -- the rest will be the polymer molecular weight \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   16. /\n"
        "   20.    16.   14.   12. /\n"
        "   20.    12.   8.    4. /\n"
        "/\n"
        "PLYMWINJ \n"
        "   3   /    -- table number \n"
        "   0.0     100.0  / -- throughput values \n"
        "   0.0     1.0   2.0  / -- velocity values \n"
        "   -- the rest will be the polymer molecular weight \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   /\n"
        "   20.    16.   14.   /\n"
        "/\n";

    Opm::Parser parser;
    const Opm::Deck deck = parser.parseString(inputstring);
    const Opm::TableManager tables( deck );
    const auto& plymwinjtables = tables.getPlymwinjTables();

    BOOST_CHECK_EQUAL( plymwinjtables.size(), 2 );

    BOOST_CHECK( plymwinjtables.find(1) == plymwinjtables.end() );

    {
        const auto searchtable2 = plymwinjtables.find(2);
        BOOST_CHECK( searchtable2 != plymwinjtables.end() );
        const auto& table2 = searchtable2->second;
        BOOST_CHECK_EQUAL( searchtable2->first, table2.getTableNumber() );
        BOOST_CHECK_EQUAL( table2.getTableNumber(), 2 );

        const std::vector<double>& throughputs = table2.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 3 );
        BOOST_CHECK_EQUAL( throughputs[1], 200.0 );
        const std::vector<double>& velocities = table2.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 4 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& mwdata = table2.getMoleWeights();

        BOOST_CHECK_EQUAL( mwdata.size(), throughputs.size() );
        for (const auto& data : mwdata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        BOOST_CHECK_EQUAL(mwdata[2][3], 4.0);
        BOOST_CHECK_EQUAL(mwdata[1][1], 16.0);
    }

    {
        const auto searchtable3 = plymwinjtables.find(3);
        BOOST_CHECK( searchtable3 != plymwinjtables.end() );
        const auto& table3 = searchtable3->second;
        BOOST_CHECK_EQUAL( searchtable3->first, table3.getTableNumber() );
        BOOST_CHECK_EQUAL( table3.getTableNumber(), 3 );

        const std::vector<double>& throughputs = table3.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 2 );
        BOOST_CHECK_EQUAL( throughputs[1], 100.0 );
        const std::vector<double>& velocities = table3.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 3 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& mwdata = table3.getMoleWeights();

        BOOST_CHECK_EQUAL( mwdata.size(), throughputs.size() );
        for (const auto& data : mwdata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        BOOST_CHECK_EQUAL(mwdata[1][2], 14.0);
        BOOST_CHECK_EQUAL(mwdata[0][0], 20.0);
    }
}

BOOST_AUTO_TEST_CASE( TestSKPRWAT ) {
    const char *inputstring =
        "SKPRWAT \n"
        "   1   /    -- table number \n"
        "   0.0     200.0   800.0   / -- throughput values \n"
        "   0.0     1.0   2.0  3.0  / -- velocity values \n"
        "   -- the rest will be the skin pressure \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   16. /\n"
        "   20.    16.   14.   12. /\n"
        "   20.    12.   8.    4. /\n"
        "/\n"
        "SKPRWAT \n"
        "   2   /    -- table number \n"
        "   0.0     100.0  / -- throughput values \n"
        "   0.0     1.0   2.0  / -- velocity values \n"
        "   -- the rest will be the skin pressure \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   /\n"
        "   20.    16.   14.   /\n"
        "/\n";



    Opm::Parser parser;
    const Opm::Deck deck = parser.parseString(inputstring);
    const Opm::TableManager tables( deck );
    const auto& skprwattables = tables.getSkprwatTables();

    BOOST_CHECK_EQUAL( skprwattables.size(), 2 );

    BOOST_CHECK( skprwattables.find(3) == skprwattables.end() );

    {
        const auto searchtable1 = skprwattables.find(1);
        BOOST_CHECK( searchtable1 != skprwattables.end() );
        const auto& table1 = searchtable1->second;
        BOOST_CHECK_EQUAL( searchtable1->first, table1.getTableNumber() );
        BOOST_CHECK_EQUAL( table1.getTableNumber(), 1 );

        const std::vector<double>& throughputs = table1.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 3 );
        BOOST_CHECK_EQUAL( throughputs[1], 200.0 );
        const std::vector<double>& velocities = table1.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 4 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& skindata = table1.getSkinPressures();

        BOOST_CHECK_EQUAL( skindata.size(), throughputs.size() );
        for (const auto& data : skindata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        constexpr double barsa = 1.0e5;
        BOOST_CHECK_EQUAL(skindata[2][3], 4.0 * barsa);
        BOOST_CHECK_EQUAL(skindata[1][1], 16.0 * barsa);
    }

    {
        const auto searchtable2 = skprwattables.find(2);
        BOOST_CHECK( searchtable2 != skprwattables.end() );
        const auto& table2 = searchtable2->second;
        BOOST_CHECK_EQUAL( searchtable2->first, table2.getTableNumber() );
        BOOST_CHECK_EQUAL( table2.getTableNumber(), 2 );

        const std::vector<double>& throughputs = table2.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 2 );
        BOOST_CHECK_EQUAL( throughputs[1], 100.0 );
        const std::vector<double>& velocities = table2.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 3 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& skindata = table2.getSkinPressures();

        BOOST_CHECK_EQUAL( skindata.size(), throughputs.size() );
        for (const auto& data : skindata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        constexpr double barsa = 1.0e5;
        BOOST_CHECK_EQUAL(skindata[1][2], 14.0 * barsa);
        BOOST_CHECK_EQUAL(skindata[0][0], 20.0 * barsa);
    }
}

BOOST_AUTO_TEST_CASE( TestSKPRPOLY ) {
    const char *inputstring =
        "SKPRPOLY \n"
        "   1   2.0 /    -- table number & reference concentration \n"
        "   0.0     200.0   800.0   / -- throughput values \n"
        "   0.0     1.0   2.0  3.0  / -- velocity values \n"
        "   -- the rest will be the skin pressure \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   16. /\n"
        "   20.    16.   14.   12. /\n"
        "   20.    12.   8.    4. /\n"
        "/\n"
        "SKPRPOLY \n"
        "   2   3.0 /    -- table number & reference concentration \n"
        "   0.0     100.0  / -- throughput values \n"
        "   0.0     1.0   2.0  / -- velocity values \n"
        "   -- the rest will be the skin pressure \n"
        "   -- each row corresponds to one sample points in the throughput direction \n"
        "   20.    19.   18.   /\n"
        "   20.    16.   14.   /\n"
        "/\n";

    Opm::Parser parser;
    const Opm::Deck deck = parser.parseString(inputstring);
    const Opm::TableManager tables( deck );
    const auto& skprpolytables = tables.getSkprpolyTables();

    BOOST_CHECK_EQUAL( skprpolytables.size(), 2 );

    BOOST_CHECK( skprpolytables.find(4) == skprpolytables.end() );

    {
        const auto searchtable1 = skprpolytables.find(1);
        BOOST_CHECK( searchtable1 != skprpolytables.end() );
        const auto& table1 = searchtable1->second;
        BOOST_CHECK_EQUAL( searchtable1->first, table1.getTableNumber() );
        BOOST_CHECK_EQUAL( table1.getTableNumber(), 1 );

        BOOST_CHECK_EQUAL( table1.referenceConcentration(), 2.0 );
        const std::vector<double>& throughputs = table1.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 3 );
        BOOST_CHECK_EQUAL( throughputs[1], 200.0 );
        const std::vector<double>& velocities = table1.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 4 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& skindata = table1.getSkinPressures();

        BOOST_CHECK_EQUAL( skindata.size(), throughputs.size() );
        for (const auto& data : skindata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        constexpr double barsa = 1.0e5;
        BOOST_CHECK_EQUAL(skindata[2][3], 4.0 * barsa);
        BOOST_CHECK_EQUAL(skindata[1][1], 16.0 * barsa);
    }

    {
        const auto searchtable2 = skprpolytables.find(2);
        BOOST_CHECK( searchtable2 != skprpolytables.end() );
        const auto& table2 = searchtable2->second;
        BOOST_CHECK_EQUAL( searchtable2->first, table2.getTableNumber() );
        BOOST_CHECK_EQUAL( table2.getTableNumber(), 2 );

        BOOST_CHECK_EQUAL( table2.referenceConcentration(), 3.0 );
        const std::vector<double>& throughputs = table2.getThroughputs();
        BOOST_CHECK_EQUAL( throughputs.size(), 2 );
        BOOST_CHECK_EQUAL( throughputs[1], 100.0 );
        const std::vector<double>& velocities = table2.getVelocities();
        BOOST_CHECK_EQUAL( velocities.size(), 3 );
        constexpr double dayinseconds = 86400.;
        BOOST_CHECK_EQUAL( velocities[2], 2.0 / dayinseconds );
        const std::vector<std::vector<double>>& skindata = table2.getSkinPressures();

        BOOST_CHECK_EQUAL( skindata.size(), throughputs.size() );
        for (const auto& data : skindata) {
            BOOST_CHECK_EQUAL( data.size(), velocities.size() );
        }
        constexpr double barsa = 1.0e5;
        BOOST_CHECK_EQUAL(skindata[1][2], 14.0 * barsa);
        BOOST_CHECK_EQUAL(skindata[0][0], 20.0 * barsa);
    }
}

BOOST_AUTO_TEST_CASE( TestPLYROCK ) {
    const char *data =
        "TABDIMS\n"
        "  2 /\n"
        "\n"
        "PLYROCK\n"
        " 1 2 3 4 5 /\n"
        " 10 20 30 40 50 /\n";

    Opm::Parser parser;
    auto deck = parser.parseString(data);
    Opm::TableManager tables( deck );
    const Opm::TableContainer& plyrock = tables.getPlyrockTables();

    BOOST_CHECK_EQUAL( plyrock.size() , 2 ) ;
    const Opm::PlyrockTable& table0 = plyrock.getTable<Opm::PlyrockTable>(0);
    const Opm::PlyrockTable& table1 = plyrock.getTable<Opm::PlyrockTable>(1);

    BOOST_CHECK_EQUAL( table0.numColumns() , 5 );
    BOOST_CHECK_EQUAL( table0.getDeadPoreVolumeColumn()[0] , 1.0 );
    BOOST_CHECK_EQUAL( table0.getMaxAdsorbtionColumn()[0] , 5.0 );

    BOOST_CHECK_EQUAL( table1.numColumns() , 5 );
    BOOST_CHECK_EQUAL( table1.getDeadPoreVolumeColumn()[0] , 10.0 );
    BOOST_CHECK_EQUAL( table1.getMaxAdsorbtionColumn()[0] , 50.0 );
}


BOOST_AUTO_TEST_CASE( TestPLYMAX ) {
    const char *data =
        "REGDIMS\n"
        "  9* 2 /\n"
        "\n"
        "PLYMAX\n"
        " 1 2 /\n"
        " 10 20 /\n";

    Opm::Parser parser;
    auto deck = parser.parseString(data);
    Opm::TableManager tables( deck );
    const Opm::TableContainer& plymax = tables.getPlymaxTables();

    BOOST_CHECK_EQUAL( plymax.size() , 2 ) ;
    const Opm::PlymaxTable& table0 = plymax.getTable<Opm::PlymaxTable>(0);
    const Opm::PlymaxTable& table1 = plymax.getTable<Opm::PlymaxTable>(1);

    BOOST_CHECK_EQUAL( table0.numColumns() , 2 );
    BOOST_CHECK_EQUAL( table0.getPolymerConcentrationColumn()[0] , 1.0 );
    BOOST_CHECK_EQUAL( table0.getMaxPolymerConcentrationColumn()[0] , 2.0 );

    BOOST_CHECK_EQUAL( table1.numColumns() , 2 );
    BOOST_CHECK_EQUAL( table1.getPolymerConcentrationColumn()[0] , 10.0 );
    BOOST_CHECK_EQUAL( table1.getMaxPolymerConcentrationColumn()[0] , 20.0 );
}

BOOST_AUTO_TEST_CASE( TestParseDENSITY ) {
    const std::string data = R"(
      TABDIMS
        1* 1 /

      DENSITY
        1.1 1.2 1.3 /
    )";

    Opm::Parser parser;
    auto deck = parser.parseString(data);
    Opm::TableManager tables( deck );
    const auto& density = tables.getDensityTable();
    BOOST_CHECK_EQUAL( 1.1, density[0].oil );
    BOOST_CHECK_EQUAL( 1.2, density[0].water );
    BOOST_CHECK_EQUAL( 1.3, density[0].gas );
}

BOOST_AUTO_TEST_CASE( TestParseROCK ) {
    const std::string data = R"(
      TABDIMS
        1* 2 * * 8/

      ROCK
        1.1 1.2 /
        2.1 2.2 /
    )";

    Opm::Parser parser;
    auto deck = parser.parseString(data);
    Opm::TableManager tables( deck );
    const auto& rock = tables.getRockTable();
    BOOST_CHECK_EQUAL( 1.1 * 1e5,  rock[0].reference_pressure );
    BOOST_CHECK_EQUAL( 1.2 * 1e-5, rock[0].compressibility );

    BOOST_CHECK_EQUAL( 2.1 * 1e5,  rock[1].reference_pressure );
    BOOST_CHECK_EQUAL( 2.2 * 1e-5, rock[1].compressibility );

    BOOST_CHECK_THROW( rock.at( 2 ), std::out_of_range );
    BOOST_CHECK_EQUAL( 8 , tables.numFIPRegions( ));
}

BOOST_AUTO_TEST_CASE( TestParsePVCDO ) {
    const std::string data = R"(
      TABDIMS
        1* 1 /

      REGDIMS
        25 /

      PVCDO
        3600 1.12 1.6e-5 0.88 0.0 /
    )";

    Opm::Parser parser;
    auto deck = parser.parseString(data);
    Opm::TableManager tables( deck );
    const auto& pvcdo = tables.getPvcdoTable();

    BOOST_CHECK_CLOSE( 3600.00, pvcdo[ 0 ].reference_pressure / 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 1.12,    pvcdo[ 0 ].volume_factor, 1e-5 );
    BOOST_CHECK_CLOSE( 1.6e-5,  pvcdo[ 0 ].compressibility * 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 0.88,    pvcdo[ 0 ].viscosity * 1e3, 1e-5 );
    BOOST_CHECK_CLOSE( 0.0,     pvcdo[ 0 ].viscosibility * 1e5, 1e-5 );

    BOOST_CHECK_THROW( pvcdo.at( 1 ), std::out_of_range );
    BOOST_CHECK_EQUAL( 25 , tables.numFIPRegions( ));

    const std::string malformed = R"(
      TABDIMS
        1* 1 /

      PVCDO
        -- cannot be defaulted
        3600 1* 1.6e-5 0.88 0.0 /
    )";

    auto illegal_default = parser.parseString( malformed );
    BOOST_CHECK_THROW( TableManager{ illegal_default }, std::invalid_argument );
}

BOOST_AUTO_TEST_CASE( TestParseTABDIMS ) {
    const char *data =
      "TABDIMS\n"
      "  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 /\n";
    Opm::Parser parser;
    BOOST_CHECK_NO_THROW( parser.parseString(data));
}

BOOST_AUTO_TEST_CASE (Regdims_Entries) {

    // All defaulted
    {
        const auto input = std::string {
            R"~(
REGDIMS
/
)~"     };

        const auto tabMgr = ::Opm::TableManager {
            ::Opm::Parser{}.parseString(input)
        };

        const auto& rd = tabMgr.getRegdims();

        BOOST_CHECK_EQUAL(rd.getNTFIP() , std::size_t{1});
        BOOST_CHECK_EQUAL(rd.getNMFIPR(), std::size_t{1});
        BOOST_CHECK_EQUAL(rd.getNRFREG(), std::size_t{0});
        BOOST_CHECK_EQUAL(rd.getNTFREG(), std::size_t{0});
    }

    // All user-specified
    {
        const auto input = std::string {
            R"~(
REGDIMS
  11 22 33 44 55 66 77 88 99 110
/
)~"     };

        const auto tabMgr = ::Opm::TableManager {
            ::Opm::Parser{}.parseString(input)
        };

        const auto& rd = tabMgr.getRegdims();

        BOOST_CHECK_EQUAL(rd.getNTFIP() , std::size_t{11});
        BOOST_CHECK_EQUAL(rd.getNMFIPR(), std::size_t{22});
        BOOST_CHECK_EQUAL(rd.getNRFREG(), std::size_t{33});
        BOOST_CHECK_EQUAL(rd.getNTFREG(), std::size_t{44});
    }
}




BOOST_AUTO_TEST_CASE(DENT) {
    const auto deck_string = R"(
RUNSPEC

TABDIMS
   1  3 /

PROPS

GASDENT
   1  2  3 /
   4  5  6 /
   7  8  9 /

OILDENT
   1  2  3 /
   4  5  6 /
   7  8  9 /

)";

    Opm::Parser parser;
    const auto& deck = parser.parseString(deck_string);
    Opm::TableManager tables(deck);
    Opm::DenT gd(deck.getKeyword("GASDENT"));
    Opm::DenT od(deck.getKeyword("OILDENT"));
    const auto& wd = tables.WatDenT();

    BOOST_CHECK_EQUAL(gd.size(), 3);
    BOOST_CHECK( gd == od );
    BOOST_CHECK( wd.size() == 0);
}



BOOST_AUTO_TEST_CASE(TLMIXPAR) {
    const auto deck_string = R"(
RUNSPEC

MISCIBLE
 2 /

PROPS

TLMIXPAR
  0  0.25 /
  0.25    /

)";
    Opm::Parser parser;
    const auto& deck = parser.parseString(deck_string);
    Opm::TLMixpar tlm(deck);
    BOOST_CHECK_EQUAL(tlm.size(), 2);

    const auto& r0 = tlm[0];
    const auto& r1 = tlm[1];

    BOOST_CHECK_EQUAL( r0.viscosity_parameter, 0);
    BOOST_CHECK_EQUAL( r0.density_parameter, 0.25);
    BOOST_CHECK_EQUAL( r1.viscosity_parameter, 0.25);
    BOOST_CHECK_EQUAL( r1.density_parameter, 0.25);

    BOOST_CHECK_THROW(tlm[2], std::out_of_range);
}
