/*
Copyright 2017 TNO.
Copyright 2020 Equinor.

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

#define BOOST_TEST_MODULE AquiferCTTest

#include <boost/test/unit_test.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/Aquancon.hpp>
#include <opm/parser/eclipse/EclipseState/AquiferCT.hpp>
#include <opm/parser/eclipse/EclipseState/Aquifetp.hpp>
#include <opm/parser/eclipse/EclipseState/AquiferConfig.hpp>

using namespace Opm;


EclipseGrid makeGrid() {
    EclipseGrid grid(3,3,3);
    std::vector<int> actnum(27,1);
    actnum[0] = 0;
    actnum[9] = 0;
    actnum[18] = 0;
    grid.resetACTNUM(actnum);
    return grid;
}



inline Deck createAquiferCTDeck() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "AQUDIMS\n"
        "1* 1* 2 100 1 1000 /\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PORO\n"
        "  27*0.15 /\n"
        "PROPS\n"
        "AQUTAB\n"
        " 0.01 0.112 \n"
        " 0.05 0.229 /\n"
        "SOLUTION\n"
        "\n"
        "AQUCT\n"
        "   1 2000.0 1.5 100 .3 3.0e-5 330 10 360.0 1 2 /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}

inline Deck createAquiferCTDeckDefaultP0() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "AQUDIMS\n"
        "1* 1* 2 100 1 1000 /\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PORO\n"
        "  27*0.15 /\n"
        "PROPS\n"
        "AQUTAB\n"
        " 0.01 0.112 \n"
        " 0.05 0.229 /\n"
        "SOLUTION\n"
        "\n"
        "AQUCT\n"
        "   1 2000.0 1* 100 .3 3.0e-5 330 10 360.0 1 2 /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}

AquiferCT init_aquiferct(const Deck& deck){
    EclipseState eclState( deck );
    return AquiferCT(eclState.getTableManager(), deck);
}

BOOST_AUTO_TEST_CASE(AquiferCTTest){
    auto deck = createAquiferCTDeck();
    {
        auto aquiferct = init_aquiferct(deck);
        for (const auto& it : aquiferct){
            BOOST_CHECK_EQUAL(it.aquiferID , 1);
            BOOST_CHECK_EQUAL(it.phi_aq , 0.3);
            BOOST_CHECK_EQUAL(it.inftableID , 2);
            BOOST_CHECK(it.p0.first == true);
            BOOST_CHECK_CLOSE(it.p0.second, 1.5e5, 1e-6);
        }
        BOOST_CHECK_EQUAL(aquiferct.size(), 1);
    }

    auto deck_default_p0 = createAquiferCTDeckDefaultP0();
    {
        auto aquiferct = init_aquiferct(deck_default_p0);
        for (const auto& it : aquiferct){
            BOOST_CHECK_EQUAL(it.aquiferID , 1);
            BOOST_CHECK_EQUAL(it.phi_aq , 0.3);
            BOOST_CHECK_EQUAL(it.inftableID , 2);
            BOOST_CHECK(it.p0.first == false);
        }
        auto data = aquiferct.data();
        AquiferCT aq2(data);
        BOOST_CHECK( aq2 == aquiferct );
    }
}

inline Deck createAQUANCONDeck_DEFAULT_INFLUX2() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PORO\n"
        "  27*0.15 /\n"
        "SOLUTION\n"
        "\n"
        "AQUANCON\n"
        "   1      2  2  1    1   1  1  J-  1.0 /\n"
        "   1      2  2  1    1   1  1  J-   /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}


inline Deck createAQUANCONDeck_DEFAULT_INFLUX1() {
  const char *deckData =
    "DIMENS\n"
    "3 3 3 /\n"
    "\n"
    "GRID\n"
    "\n"
    "ACTNUM\n"
    " 0 8*1 0 8*1 0 8*1 /\n"
    "DXV\n"
    "1 1 1 /\n"
    "\n"
    "DYV\n"
    "1 1 1 /\n"
    "\n"
    "DZV\n"
    "1 1 1 /\n"
    "\n"
    "TOPS\n"
    "9*100 /\n"
    "PORO\n"
    "  27*0.15 /\n"
    "\n"
    "SOLUTION\n"
    "\n"
    "AQUANCON\n"
    "   1      1  3  1    1   1  1  J-   /\n"
    "/\n"
    "AQUANCON\n"
    "   2      1  1  2    2   1  1  J-   /\n"
    "/ \n";

  Parser parser;
  return parser.parseString(deckData);
}

inline Deck createAQUANCONDeck_DEFAULT_ILLEGAL() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "PORO\n"
        "  27*0.15 /\n"
        "\n"
        "SOLUTION\n"
        "\n"
        "AQUANCON\n"
        "   1      1  3  1    1   1  1  J-   /\n"
        "/\n"
        "AQUANCON\n"
        "   2      1  2  1    2   1  1  J-   /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}

inline Deck createAQUANCONDeck() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PORO\n"
        "  27*0.15 /\n"
        "SOLUTION\n"
        "\n"
        "AQUANCON\n"
        "   1      1  1  1    1   1  1  J-  1.0 1.0 NO /\n"
        "   1      1  3  1    3   3  3  I+  0.5 1.0 NO /\n"
        "   1      1  3  1    3   3  3  J+  0.75 1.0 NO /\n"
        "   1      1  3  1    3   3  3  J-  2.75 1.0 NO /\n"
        "   1      2  3  2    3   1  1  I+  2.75 1.0 NO /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}




BOOST_AUTO_TEST_CASE(AquanconTest_DEFAULT_INFLUX) {
    auto deck1 = createAQUANCONDeck_DEFAULT_INFLUX1();
    const auto& grid = makeGrid();
    Aquancon aqcon(grid, deck1);

    const auto& cells_aq1 = aqcon[1];
    /*
      The cells I = 0..2 are connected to aquifer 1; cell I==0 is inactive and
      not counted here ==> a total of 2 cells are connected to aquifer 1.
    */
    BOOST_CHECK_EQUAL(cells_aq1.size(), 2);

    const auto& cells_aq2 = aqcon[2];
    BOOST_CHECK_EQUAL(cells_aq2.size(), 1);
    BOOST_CHECK(aqcon.active());

    auto deck2 = createAQUANCONDeck_DEFAULT_INFLUX2();
    BOOST_CHECK_THROW(Aquancon( grid, deck2), std::invalid_argument);

    // The cell (2,1,1) is attached to both aquifer 1 and aquifer 2 - that is illegal.
    auto deck3 = createAQUANCONDeck_DEFAULT_ILLEGAL();
    BOOST_CHECK_THROW(Aquancon( grid, deck3), std::invalid_argument);
}


// allowing aquifer exists inside the reservoir
inline Deck createAQUANCONDeck_ALLOW_INSIDE_AQUAN_OR_NOT() {
    const char *deckData =
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PORO\n"
        "  27*0.15 /\n"
        "SOLUTION\n"
        "\n"
        "AQUFETP\n"
        "  1 20.0 1000.0 2000. 0.000001 200.0 /\n"
        "  2 20.0 1000.0 2000. 0.000001 200.0 /\n"
        "/\n"
        "AQUANCON\n"
        "   1      1  1   1   1   1  1  J-  2* YES /\n"
        "   1      2  2   1   1   1  1  J-  2* YES /\n"
        "   1      2  2   2   2   1  1  J-  2* YES /\n"
        "   2      1  1   1   1   3  3  J-  2* NO /\n"
        "   2      2  2   1   1   3  3  J-  2* NO /\n"
        "   2      2  2   2   2   3  3  J-  2* NO /\n"
        "/ \n";

    Parser parser;
    return parser.parseString(deckData);
}

BOOST_AUTO_TEST_CASE(AquanconTest_ALLOW_AQUIFER_INSIDE_OR_NOT) {
    auto deck = createAQUANCONDeck_ALLOW_INSIDE_AQUAN_OR_NOT();
    const EclipseState eclState( deck );
    const Aquancon aqucon( eclState.getInputGrid(), deck);

    const auto& data = aqucon.data();
    const Aquancon aq2(data);

    BOOST_CHECK(aqucon == aq2);
    auto cells1 = aqucon[1];
    auto cells2 = aqucon[2];
    BOOST_CHECK_EQUAL(cells1.size() , 2);
    BOOST_CHECK_EQUAL(cells2.size() , 1);
}

inline Deck createAquifetpDeck() {
  const char *deckData =
  "DIMENS\n"
  "3 3 3 /\n"
  "\n"
  "AQUDIMS\n"
  "1* 1* 2 100 1 1000 /\n"
  "GRID\n"
  "\n"
  "ACTNUM\n"
  " 0 8*1 0 8*1 0 8*1 /\n"
  "DXV\n"
  "1 1 1 /\n"
  "\n"
  "DYV\n"
  "1 1 1 /\n"
  "\n"
  "DZV\n"
  "1 1 1 /\n"
  "\n"
  "TOPS\n"
  "9*100 /\n"
  "\n"
  "PROPS\n"
  "AQUTAB\n"
  " 0.01 0.112 \n"
  " 0.05 0.229 /\n"
  "SOLUTION\n"
  "\n"
  "AQUFETP\n"
  "1  70000.0  4.0e3 2.0e9 1.0e-5	500 1 0	0 /\n"
  "/\n";

  Parser parser;
  return parser.parseString(deckData);
}

inline Deck createNullAquifetpDeck(){
  const char *deckData =
  "DIMENS\n"
  "3 3 3 /\n"
  "\n"
  "AQUDIMS\n"
  "1* 1* 2 100 1 1000 /\n"
  "GRID\n"
  "\n"
  "ACTNUM\n"
  " 0 8*1 0 8*1 0 8*1 /\n"
  "DXV\n"
  "1 1 1 /\n"
  "\n"
  "DYV\n"
  "1 1 1 /\n"
  "\n"
  "DZV\n"
  "1 1 1 /\n"
  "\n"
  "TOPS\n"
  "9*100 /\n"
  "\n"
  "PROPS\n"
  "AQUTAB\n"
  " 0.01 0.112 \n"
  " 0.05 0.229 /\n"
  "SOLUTION\n"
  ;

  Parser parser;
  return parser.parseString(deckData);
}

inline Deck createAquifetpDeck_defaultPressure(){
  const char *deckData =
  "DIMENS\n"
  "3 3 3 /\n"
  "\n"
  "AQUDIMS\n"
  "1* 1* 2 100 1 1000 /\n"
  "GRID\n"
  "\n"
  "ACTNUM\n"
  " 0 8*1 0 8*1 0 8*1 /\n"
  "DXV\n"
  "1 1 1 /\n"
  "\n"
  "DYV\n"
  "1 1 1 /\n"
  "\n"
  "DZV\n"
  "1 1 1 /\n"
  "\n"
  "TOPS\n"
  "9*100 /\n"
  "\n"
  "PROPS\n"
  "AQUTAB\n"
  " 0.01 0.112 \n"
  " 0.05 0.229 /\n"
  "SOLUTION\n"
  "\n"
  "AQUFETP\n"
  "1  70000.0  1* 2.0e9 1.0e-5	500 1 0	0 /\n"
  "/\n";

  Parser parser;
  return parser.parseString(deckData);
}

inline Aquifetp init_aquifetp(Deck& deck){
  Aquifetp aqufetp(deck);
  return aqufetp;
}

BOOST_AUTO_TEST_CASE(AquifetpTest){
  auto aqufetp_deck = createAquifetpDeck();
  const auto& aquifetp = init_aquifetp(aqufetp_deck);
  for (const auto& it : aquifetp){
    BOOST_CHECK_EQUAL(it.aquiferID , 1);
    BOOST_CHECK_EQUAL(it.V0, 2.0e9);
    BOOST_CHECK_EQUAL(it.J, 500/86400e5);
    BOOST_CHECK( it.p0.first );
  }
  const auto& data = aquifetp.data();
  Aquifetp aq2(data);
  BOOST_CHECK(aq2 == aquifetp);

  auto aqufetp_deck_null = createNullAquifetpDeck();
  const auto& aquifetp_null = init_aquifetp(aqufetp_deck_null);
  BOOST_CHECK_EQUAL(aquifetp_null.size(), 0);

  auto aqufetp_deck_default = createAquifetpDeck_defaultPressure();
  const auto& aquifetp_default = init_aquifetp(aqufetp_deck_default);
  for (const auto& it : aquifetp_default){
    BOOST_CHECK_EQUAL(it.aquiferID , 1);
    BOOST_CHECK_EQUAL(it.V0, 2.0e9);
    BOOST_CHECK_EQUAL(it.J, 500/86400e5);
    BOOST_CHECK( !it.p0.first );
  }

}

BOOST_AUTO_TEST_CASE(TEST_CREATE) {
      Opm::Aqudims aqudims;

      BOOST_CHECK_EQUAL( aqudims.getNumAqunum() , 1 );
      BOOST_CHECK_EQUAL( aqudims.getNumConnectionNumericalAquifer() , 1 );
      BOOST_CHECK_EQUAL( aqudims.getNumInfluenceTablesCT() , 1 );
      BOOST_CHECK_EQUAL( aqudims.getNumRowsInfluenceTable() , 36 );
      BOOST_CHECK_EQUAL( aqudims.getNumAnalyticAquifers() , 1 );
      BOOST_CHECK_EQUAL( aqudims.getNumRowsAquancon() , 1 );
      BOOST_CHECK_EQUAL( aqudims.getNumAquiferLists() , 0 );
      BOOST_CHECK_EQUAL( aqudims.getNumAnalyticAquifersSingleList() , 0 );
}

BOOST_AUTO_TEST_CASE(Test_Aquifer_Config) {
    const std::string deck_string = R"(
DIMENS
   3 3 3 /
)";
    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deck_string);
    Opm::TableManager tables;
    Opm::EclipseGrid grid(10,10,10);
    Opm::AquiferConfig conf(tables, grid, deck);
    BOOST_CHECK(!conf.active());


    const auto& fetp  = conf.fetp();
    const auto& ct    = conf.ct();
    const auto& conn  = conf.connections();
    Opm::AquiferConfig conf2(fetp, ct, conn);
    BOOST_CHECK( conf == conf2 );
}
