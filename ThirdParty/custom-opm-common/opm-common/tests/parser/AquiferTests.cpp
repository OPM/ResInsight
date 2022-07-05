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

#include <opm/input/eclipse/EclipseState/Aquifer/Aquancon.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferCT.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/Aquifetp.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferConfig.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

#include <opm/input/eclipse/Units/Units.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <cstddef>
#include <initializer_list>
#include <stdexcept>

using namespace Opm;

namespace {

EclipseGrid makeGrid()
{
    EclipseGrid grid(3, 3, 3);

    std::vector<int> actnum(27, 1);
    for (const std::size_t layer : { 0, 1, 2 })
        actnum[grid.getGlobalIndex(0, 0, layer)] = 0;

    grid.resetACTNUM(actnum);
    return grid;
}

Deck createAquiferCTDeck()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

AQUDIMS
1* 1* 2 100 1 1000 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

PROPS
AQUTAB
 0.01 0.112
 0.05 0.229 /

SOLUTION
AQUCT
   1 2000.0 1.5 100 .3 3.0e-5 330 10 360.0 1 2 /
/
)");
}

Deck createAquiferCTDeckDefaultP0()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

AQUDIMS
1* 1* 2 100 1 1000 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

PROPS
AQUTAB
 0.01 0.112
 0.05 0.229 /

SOLUTION
AQUCT
   1 2000.0 1* 100 .3 3.0e-5 330 10 360.0 1 2 /
/
)");
}

AquiferCT init_aquiferct(const Deck& deck)
{
    EclipseState eclState( deck );
    return AquiferCT(eclState.getTableManager(), deck);
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(AquiferCTTest)
{
    {
        const auto aquiferct = init_aquiferct(createAquiferCTDeck());
        BOOST_REQUIRE_EQUAL(aquiferct.size(), 1U);

        for (const auto& it : aquiferct) {
            BOOST_CHECK_EQUAL(it.aquiferID, 1);
            BOOST_CHECK_CLOSE(it.porosity, 0.3, 1.0e-8);
            BOOST_CHECK_EQUAL(it.inftableID, 2);
            BOOST_CHECK_MESSAGE(it.initial_pressure.has_value(), "Initial pressure must be defined in CT aquifer");
            BOOST_CHECK_CLOSE(it.initial_pressure.value(), 1.5e5, 1e-6);
        }
    }

    {
        const auto aquiferct = init_aquiferct(createAquiferCTDeckDefaultP0());
        for (const auto& it : aquiferct) {
            BOOST_CHECK_EQUAL(it.aquiferID, 1);
            BOOST_CHECK_CLOSE(it.porosity, 0.3, 1.0e-8);
            BOOST_CHECK_EQUAL(it.inftableID, 2);
            BOOST_CHECK_MESSAGE(! it.initial_pressure.has_value(), "Initial pressure must NOT be defined in CT aquifer");
        }

        auto data = aquiferct.data();
        AquiferCT aq2(data);
        BOOST_CHECK( aq2 == aquiferct );
    }
}

namespace {

Deck createAQUANCONDeck_DEFAULT_INFLUX2()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

SOLUTION
AQUANCON
   1      2  2  1    1   1  1  J-  1.0 /
   1      2  2  1    1   1  1  J-   /
/
)");
}

Deck createAQUANCONDeck_DEFAULT_INFLUX1()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

SOLUTION
AQUANCON
   1      1  3  1    1   1  1  J-   /
/
AQUANCON
   2      1  1  2    2   1  1  J-   /
/
)");
}

Deck createAQUANCONDeck_DEFAULT_ILLEGAL()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

SOLUTION
AQUANCON
   1      1  3  1    1   1  1  J-   /
/
AQUANCON
   2      1  2  1    2   1  1  J-   /
/
)");
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(AquanconTest_DEFAULT_INFLUX)
{
    auto deck1 = createAQUANCONDeck_DEFAULT_INFLUX1();
    const auto& grid = makeGrid();
    Aquancon aqcon(grid, deck1);

    const auto& cells_aq1 = aqcon[1];
    /*
      The cells I = 0..2 are connected to aquifer 1; cell I==0 is inactive and
      not counted here ==> a total of 2 cells are connected to aquifer 1.
    */
    BOOST_CHECK_EQUAL(cells_aq1.size(), 2U);

    const auto& cells_aq2 = aqcon[2];
    BOOST_CHECK_EQUAL(cells_aq2.size(), 1U);
    BOOST_CHECK(aqcon.active());

    auto deck2 = createAQUANCONDeck_DEFAULT_INFLUX2();

    // The cell (2,1,1) is attached to both aquifer 1 and aquifer 2 - that is illegal.
    auto deck3 = createAQUANCONDeck_DEFAULT_ILLEGAL();
    BOOST_CHECK_THROW(Aquancon( grid, deck3), std::invalid_argument);
}


// allowing aquifer exists inside the reservoir

namespace {

Deck createAQUANCONDeck_ALLOW_INSIDE_AQUAN_OR_NOT()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

SOLUTION
AQUFETP
  1 20.0 1000.0 2000. 0.000001 200.0 /
  2 20.0 1000.0 2000. 0.000001 200.0 /
/
AQUANCON
   1      1  1   1   1   1  1  J-  2* YES /
   1      2  2   1   1   1  1  J-  2* YES /
   1      2  2   2   2   1  1  J-  2* YES /
   2      1  1   1   1   3  3  J-  2* NO /
   2      2  2   1   1   3  3  J-  2* NO /
   2      2  2   2   2   3  3  J-  2* NO /
/
)");
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(AquanconTest_ALLOW_AQUIFER_INSIDE_OR_NOT)
{
    auto deck = createAQUANCONDeck_ALLOW_INSIDE_AQUAN_OR_NOT();
    const EclipseState eclState( deck );
    const Aquancon aqucon( eclState.getInputGrid(), deck);

    const auto& data = aqucon.data();
    const Aquancon aq2(data);

    BOOST_CHECK(aqucon == aq2);
    auto cells1 = aqucon[1];
    auto cells2 = aqucon[2];
    BOOST_CHECK_EQUAL(cells1.size() , 2U);
    BOOST_CHECK_EQUAL(cells2.size() , 1U);
}

namespace {

Deck createAquifetpDeck()
{
    return Parser{}.parseString(R"(RUNSPEC
DIMENS
3 3 3 /

AQUDIMS
1* 1* 2 100 1 1000 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

PROPS
AQUTAB
 0.01 0.112
 0.05 0.229 /

SOLUTION
AQUFETP
1  70000.0  4.0e3 2.0e9 1.0e-5	500 1 0	0 /
/
)");
}

Deck createNullAquifetpDeck()
{
    return Parser{}.parseString(R"(RUNSPEC
DIMENS
3 3 3 /

AQUDIMS
1* 1* 2 100 1 1000 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

PROPS
AQUTAB
 0.01 0.112
 0.05 0.229 /

SOLUTION
)");
}

Deck createAquifetpDeck_defaultPressure()
{
    return Parser{}.parseString(R"(DIMENS
3 3 3 /

AQUDIMS
1* 1* 2 100 1 1000 /

GRID
ACTNUM
 0 8*1 0 8*1 0 8*1 /

DXV
1 1 1 /

DYV
1 1 1 /

DZV
1 1 1 /

TOPS
  9*100 /

PORO
  27*0.15 /

PROPS
AQUTAB
 0.01 0.112
 0.05 0.229 /

SOLUTION
AQUFETP
1  70000.0  1* 2.0e9 1.0e-5	500 1 0	0 /
/
)");
}

Aquifetp init_aquifetp(Deck& deck)
{
    return { TableManager{ deck }, deck };
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(AquifetpTest)
{
    auto aqufetp_deck = createAquifetpDeck();
    const auto& aquifetp = init_aquifetp(aqufetp_deck);
    for (const auto& it : aquifetp) {
        BOOST_CHECK_EQUAL(it.aquiferID, 1);
        BOOST_CHECK_CLOSE(it.initial_watvolume, 2.0e9, 1.0e-8);
        BOOST_CHECK_CLOSE(it.prod_index, 500/86400e5, 1.0e-8);
        BOOST_CHECK_MESSAGE(it.initial_pressure.has_value(), "Fetkovich aquifer must have initial pressure value");
    }
    const auto& data = aquifetp.data();
    Aquifetp aq2(data);
    BOOST_CHECK_MESSAGE(aq2 == aquifetp, "Copy constructor must produce equal object");

    auto aqufetp_deck_null = createNullAquifetpDeck();
    const auto& aquifetp_null = init_aquifetp(aqufetp_deck_null);
    BOOST_CHECK_EQUAL(aquifetp_null.size(), 0U);

    auto aqufetp_deck_default = createAquifetpDeck_defaultPressure();
    const auto& aquifetp_default = init_aquifetp(aqufetp_deck_default);
    for (const auto& it : aquifetp_default) {
        BOOST_CHECK_EQUAL(it.aquiferID, 1);
        BOOST_CHECK_CLOSE(it.initial_watvolume, 2.0e9, 1.0e-8);
        BOOST_CHECK_CLOSE(it.prod_index, 500/86400e5, 1.0e-8);
        BOOST_CHECK_MESSAGE(!it.initial_pressure.has_value(), "Fetkovich aquifer mut NOT have initial pressure value when defaulted");
    }
}

BOOST_AUTO_TEST_CASE(TEST_CREATE)
{
    Opm::Aqudims aqudims;

    BOOST_CHECK_EQUAL( aqudims.getNumAqunum() , 1U );
    BOOST_CHECK_EQUAL( aqudims.getNumConnectionNumericalAquifer() , 1U );
    BOOST_CHECK_EQUAL( aqudims.getNumInfluenceTablesCT() , 1U );
    BOOST_CHECK_EQUAL( aqudims.getNumRowsInfluenceTable() , 36U );
    BOOST_CHECK_EQUAL( aqudims.getNumAnalyticAquifers() , 1U );
    BOOST_CHECK_EQUAL( aqudims.getNumRowsAquancon() , 1U );
    BOOST_CHECK_EQUAL( aqudims.getNumAquiferLists() , 0U );
    BOOST_CHECK_EQUAL( aqudims.getNumAnalyticAquifersSingleList() , 0U );
}

BOOST_AUTO_TEST_CASE(Test_Aquifer_Config)
{
    const std::string deck_string = R"(
DIMENS
   3 3 3 /
GRID
DX
  27*1 /
DY
  27*1 /
DZ
  27*1 /
TOPS
  9*1 /
PORO
  27*1 /
)";
    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deck_string);
    Opm::EclipseState ecl_state(deck);
    Opm::TableManager tables;
    Opm::AquiferConfig conf(tables, ecl_state.getInputGrid(), deck, ecl_state.fieldProps());
    BOOST_CHECK(!conf.active());


    const auto& fetp  = conf.fetp();
    const auto& ct    = conf.ct();
    const auto& conn  = conf.connections();
    Opm::AquiferConfig conf2(fetp, ct, conn);
    BOOST_CHECK( conf == conf2 );
}

BOOST_AUTO_TEST_CASE(Test_Aquifer_Config_Active)
{
    const auto deck = Parser{}.parseString(R"(
START             -- 0
10 MAY 2007 /
RUNSPEC

DIMENS
 10 10 10 /
REGDIMS
  3/
AQUDIMS
4 4 1* 1* 3 200 1* 1* /
GRID
DXV
   10*400 /
DYV
   10*400 /
DZV
   10*400 /
TOPS
   100*2202 /
PERMX
  1000*0.25 /
COPY
  PERMX PERMY /
  PERMX PERMZ /
/
PORO
   1000*0.15 /
AQUNUM
  4       1 1 1      15000  5000  0.3  30  2700  / aq cell
  5       2 1 1     150000  9000  0.3  30  2700  / aq cell
  6       3 1 1     150000  9000  0.3  30  2700  / aq cell
  7       4 1 1     150000  9000  0.3  30  2700  / aq cell
/
AQUCON
-- #    I1 I2  J1 J2   K1  K2    Face
   4    1  1   16 18   19  20   'I-'    / connecting cells
   5    2  2   16 18   19  20   'I-'    / connecting cells
   6    3  3   16 18   19  20   'I-'    / connecting cells
   7    4  4   16 18   19  20   'I-'    / connecting cells
/
REGIONS
FIPNUM
200*1 300*2 500*3 /
FIPREG
200*10 300*20 500*30 /
SOLUTION
AQUCT
1    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
2    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
3    2040     1*    1000   .3    3.0e-5     1330     10     360.0   1   1* /
/
AQUANCON
1     1   10     10    2    10  10   'I-'      0.88      1  /
2     9   10     10    10    10  10   'I+'      0.88      1  /
3     9   9      8    10    9   8   'I+'      0.88      1  /
/

END
)");

    const auto es = EclipseState { deck };

    const auto& aquConfig = es.aquifer();

    BOOST_CHECK_MESSAGE(aquConfig.hasAnalyticalAquifer(),
                        "Aquifer configuration object must have analytic aquifers");

    BOOST_CHECK_MESSAGE(aquConfig.hasNumericalAquifer(),
                        "Aquifer configuration object must have numeric aquifers");

    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(1), "Configuration object must have Aquifer ID 1");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(2), "Configuration object must have Aquifer ID 2");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(3), "Configuration object must have Aquifer ID 3");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(4), "Configuration object must have Aquifer ID 4");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(5), "Configuration object must have Aquifer ID 5");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(6), "Configuration object must have Aquifer ID 6");
    BOOST_CHECK_MESSAGE(  aquConfig.hasAquifer(7), "Configuration object must have Aquifer ID 7");
    BOOST_CHECK_MESSAGE(! aquConfig.hasAquifer(8), "Configuration object must NOT have Aquifer ID 8");

    {
        const auto expect = std::vector<int>{ 1, 2, 3 };
        const auto analytic = analyticAquiferIDs(aquConfig);

        BOOST_CHECK_EQUAL_COLLECTIONS(analytic.begin(), analytic.end(),
                                      expect  .begin(), expect  .end());
    }

    {
        const auto expect = std::vector<int>{ 4, 5, 6, 7 };
        const auto numeric = numericAquiferIDs(aquConfig);

        BOOST_CHECK_EQUAL_COLLECTIONS(numeric.begin(), numeric.end(),
                                      expect .begin(), expect .end());
    }
}

namespace {

Deck createNumericalAquiferDeck()
{
    const char *deckData = R"(
DIMENS
 8 15 3 /
AQUDIMS
    3      2      1*       1*     1*       50      1*      1*  /
GRID

DX
  360*10./
DY
   360*10./
DZ
   360*1./
TOPS
   360*100./

PORO
   0. 0.25 0. 357*0.25/
PERMX
    360*1000./
PERMY
    360*1000./
PERMZ
    360*10./

BOX
1	8 15 15 3 3 /

MULTY
 1e9 1e-9 1.0 2.0 3.0 4.0 5.0 6.0/

ENDBOX

-- setting the three cells for numerical aquifer to be inactive
ACTNUM
0 1 0 0 356*1 /

AQUNUM
--AQnr.  I  J  K     Area      Length PHI      K     Depth  Initial.Pr	PVTNUM   SATNUM
   1     1  1  1   1000000.0   10000   0.25   400    2585.00   285.00	 2   2  /
   1     3  1  1   1500000.0   20000   0.24   600    2585.00   285.00	 3   *  /
   1     4  1  1   2000000.0   30000   *   700    2585.00   285.00	 *   3  /
/
AQUCON
--  Connect numerical aquifer to the reservoir
--  Id.nr  I1	I2     J1  J2	 K1  K2    Face    Trans.mult.  Trans.opt.
     1     1	8      15    15	  3   3   'J+'      1.00      1  /
/
    )";

    Parser parser;
    return parser.parseString(deckData);
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(NumericalAquiferTest)
{
    const Opm::Deck numaquifer_deck = createNumericalAquiferDeck();
    const Opm::EclipseState ecl_state(numaquifer_deck);
    const Opm::EclipseGrid& grid = ecl_state.getInputGrid();

    Opm::NumericalAquifers num_aqu{numaquifer_deck, grid, ecl_state.fieldProps()};

    BOOST_CHECK_EQUAL(num_aqu.numRecords(), 3);
    {
        const auto mD = unit::convert::from(1.0, prefix::milli*unit::darcy);

        const auto* c1 = num_aqu.getAquifer(1).getCellPrt(0);
        const auto* c2 = num_aqu.getAquifer(1).getCellPrt(1);
        const auto* c3 = num_aqu.getAquifer(1).getCellPrt(2);

        BOOST_CHECK_EQUAL(c1->record_id, std::size_t{0});
        BOOST_CHECK_EQUAL(c1->I, std::size_t{0});
        BOOST_CHECK_EQUAL(c1->J, std::size_t{0});
        BOOST_CHECK_EQUAL(c1->K, std::size_t{0});

        BOOST_CHECK_CLOSE(c1->area, 1.0e6, 1.0e-10);
        BOOST_CHECK_CLOSE(c1->length, 10.0e3, 1.0e-10);
        BOOST_CHECK_CLOSE(c1->porosity, 0.25, 1.0e-10);
        BOOST_CHECK_CLOSE(c1->permeability, 400*mD, 1.0e-10);
        BOOST_CHECK_CLOSE(c1->depth, 2585.0, 1.0e-10);
        BOOST_CHECK_MESSAGE(c1->init_pressure.has_value(), "Cell 1 must have an initial pressure");
        BOOST_CHECK_CLOSE(c1->init_pressure.value(), 285.0*unit::barsa, 1.0e-10);
        BOOST_CHECK_EQUAL(c1->pvttable, 2);
        BOOST_CHECK_EQUAL(c1->sattable, 2);
        BOOST_CHECK_EQUAL(c1->global_index, 0);

        BOOST_CHECK_EQUAL(c2->record_id, std::size_t{1});
        BOOST_CHECK_EQUAL(c2->I, std::size_t{2});
        BOOST_CHECK_EQUAL(c2->J, std::size_t{0});
        BOOST_CHECK_EQUAL(c2->K, std::size_t{0});

        BOOST_CHECK_CLOSE(c2->area, 1.5e6, 1.0e-10);
        BOOST_CHECK_CLOSE(c2->length, 20.0e3, 1.0e-10);
        BOOST_CHECK_CLOSE(c2->porosity, 0.24, 1.0e-10);
        BOOST_CHECK_CLOSE(c2->permeability, 600*mD, 1.0e-10);
        BOOST_CHECK_CLOSE(c2->depth, 2585.0, 1.0e-10);
        BOOST_CHECK_MESSAGE(c2->init_pressure.has_value(), "Cell 2 must have an initial pressure");
        BOOST_CHECK_CLOSE(c2->init_pressure.value(), 285.0*unit::barsa, 1.0e-10);
        BOOST_CHECK_EQUAL(c2->pvttable, 3);
        BOOST_CHECK_EQUAL(c2->sattable, 1);
        BOOST_CHECK_EQUAL(c2->global_index, 2);

        BOOST_CHECK_EQUAL(c3->record_id, std::size_t{2});
        BOOST_CHECK_EQUAL(c3->I, std::size_t{3});
        BOOST_CHECK_EQUAL(c3->J, std::size_t{0});
        BOOST_CHECK_EQUAL(c3->K, std::size_t{0});

        BOOST_CHECK_CLOSE(c3->area, 2.0e6, 1.0e-10);
        BOOST_CHECK_CLOSE(c3->length, 30.0e3, 1.0e-10);
        BOOST_CHECK_CLOSE(c3->porosity, 0.25, 1.0e-10);
        BOOST_CHECK_CLOSE(c3->permeability, 700*mD, 1.0e-10);
        BOOST_CHECK_CLOSE(c3->depth, 2585.0, 1.0e-10);
        BOOST_CHECK_MESSAGE(c3->init_pressure.has_value(), "Cell 3 must have an initial pressure");
        BOOST_CHECK_CLOSE(c3->init_pressure.value(), 285.0*unit::barsa, 1.0e-10);
        BOOST_CHECK_EQUAL(c3->pvttable, 1);
        BOOST_CHECK_EQUAL(c3->sattable, 3);
        BOOST_CHECK_EQUAL(c3->global_index, 3);
    }

    // using processed actnum for numerical aquifer connection generation
    std::vector<int> new_actnum(360, 1);
    new_actnum[0] = 0;
    new_actnum[1] = 0;
    new_actnum[3] = 0;
    num_aqu.postProcessConnections(grid, new_actnum);
    BOOST_CHECK(num_aqu.hasAquifer(1));
    BOOST_CHECK(num_aqu.size() == 1);
    const auto all_aquifer_cells = num_aqu.allAquiferCells();
    BOOST_CHECK(all_aquifer_cells.count(0) > 0);
    BOOST_CHECK(all_aquifer_cells.count(2) > 0);
    BOOST_CHECK(all_aquifer_cells.count(3) > 0);
    BOOST_CHECK(all_aquifer_cells.count(1) == 0);

    const auto& aquifer = num_aqu.getAquifer(1);
    BOOST_CHECK(aquifer.numCells() == 3);
    BOOST_CHECK(aquifer.numConnections() == 8 );

    const auto& nncs = aquifer.aquiferConnectionNNCs(grid, ecl_state.fieldProps());
    BOOST_CHECK(nncs.size() == 8 );
    // get the half transmissibilites by using small/large multipler m
    // mab/(ma+b) -> b for ma >> b and -> ma for ma << b
    const double taq = nncs[0].trans;
    const double tcell = nncs[1].trans*1.0e9;
    // now check the multiplier for the rest of the connection i > 2 where m = 1->6
    for (int i = 2; i < 8; ++i) {
      const double mult = (i - 1);
      const double t = mult*tcell*taq / (taq + mult*tcell);
      BOOST_CHECK_CLOSE(t, nncs[i].trans, 1.0e-6);
    }
    BOOST_CHECK(grid.getNumActive() == 360);
    // the three aquifer cells are active
    BOOST_CHECK(grid.cellActive(0, 0, 0));
    BOOST_CHECK(grid.cellActive(2, 0, 0));
    BOOST_CHECK(grid.cellActive(3, 0, 0));

    // checking the pore volume of the aquifer cells
    const auto& porv_data = ecl_state.fieldProps().porv(true);
    BOOST_CHECK_CLOSE(porv_data[0], 2500000000, 1.e-10);
    BOOST_CHECK_CLOSE(porv_data[2], 7200000000, 1.e-10);
    BOOST_CHECK_CLOSE(porv_data[3], 15000000000, 1.e-10);

    const auto& pvtnum = ecl_state.fieldProps().get_int("PVTNUM");
    BOOST_CHECK_EQUAL(pvtnum[0], 2);
    BOOST_CHECK_EQUAL(pvtnum[1], 1); // none aquifer cell
    BOOST_CHECK_EQUAL(pvtnum[2], 3);
    BOOST_CHECK_EQUAL(pvtnum[3], 1);

    const auto& satnum = ecl_state.fieldProps().get_int("SATNUM");
    BOOST_CHECK_EQUAL(satnum[0], 2);
    BOOST_CHECK_EQUAL(satnum[1], 1); // none aquifer cell
    BOOST_CHECK_EQUAL(satnum[2], 1);
    BOOST_CHECK_EQUAL(satnum[3], 3);


    const auto& permx = ecl_state.fieldProps().get_double("PERMX");
    BOOST_CHECK_SMALL(permx[0], 1.e-20);
    BOOST_CHECK_SMALL(permx[2], 1.e-20);
    BOOST_CHECK_SMALL(permx[3], 1.e-20);
    const auto& permy = ecl_state.fieldProps().get_double("PERMY");
    BOOST_CHECK_SMALL(permy[0], 1.e-20);
    BOOST_CHECK_SMALL(permy[2], 1.e-20);
    BOOST_CHECK_SMALL(permy[3], 1.e-20);
    const auto& permz = ecl_state.fieldProps().get_double("PERMZ");
    BOOST_CHECK_SMALL(permz[0], 1.e-20);
    BOOST_CHECK_SMALL(permz[2], 1.e-20);
    BOOST_CHECK_SMALL(permz[3], 1.e-20);
    const auto& poro = ecl_state.fieldProps().get_double("PORO");
    BOOST_CHECK_CLOSE(poro[0], 0.25, 1.e-10);
    BOOST_CHECK_CLOSE(poro[2], 0.24, 1.e-10);
    BOOST_CHECK_CLOSE(poro[3], 0.25, 1.e-10);
}

namespace {

std::pair<Opm::Aquancon, Opm::EclipseGrid> load_aquifer(const std::string& aqucon)
{
    const std::string data1 = R"(
DIMENS
 8 15 3 /
AQUDIMS
    3      2      1*       1*     1*       50      1*      1*  /
GRID

DX
  360*10./
DY
   360*10./
DZ
   360*1./
TOPS
   360*100./

PORO
   0. 0.25 0. 357*0.25/
PERMX
    360*1000./
PERMY
    360*1000./
PERMZ
    360*10./
-- setting the three cells for numerical aquifer to be inactive
ACTNUM
0 1 0 0 356*1 /

AQUNUM
--AQnr.  I  J  K     Area      Length PHI      K     Depth  Initial.Pr	PVTNUM   SATNUM
   1     1  1  1   1000000.0   10000   0.25   400    2585.00   285.00	 2   2  /
   1     3  1  1   1500000.0   20000   0.24   600    2585.00   285.00	 3   *  /
   1     4  1  1   2000000.0   30000   *   700    2585.00   285.00	 *   3  /
/
)";
    Opm::Parser parser;
    auto deck = parser.parseString( data1 + aqucon );
    auto grid = Opm::EclipseGrid( deck );

    return { Opm::Aquancon(grid, deck), grid };
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(AQUCONN_FUNKYNESS )
{
   {
       const std::string aq = R"(
AQUANCON
     1     1	8      15    15	  3   3   'J+'      1.00      2  /
/
)";
       const auto& [aquconn, _] = load_aquifer(aq);
       (void)_;
       auto cell1 = aquconn[1][0];
       BOOST_CHECK_EQUAL(cell1.influx_coeff, 2.0);
   }


   {
       const std::string aq = R"(
AQUANCON
     1     1	8      15    15	  3   3   'J+'      *      2  /
/
)";
       const auto& [aquconn, grid] = load_aquifer(aq);
       const auto& dims = grid.getCellDims(0,14,2);
       auto cell1 = aquconn[1][0];
       BOOST_CHECK_EQUAL(cell1.influx_coeff, 2.0 * dims[0]*dims[2]);
   }

   {
       const std::string aq = R"(
AQUANCON
     1     1	8      15    15	  3   3   'I+'      *      3  /
/
)";
       const auto& [aquconn, grid] = load_aquifer(aq);
       const auto& dims = grid.getCellDims(0,14,2);
       auto cell1 = aquconn[1][0];
       BOOST_CHECK_EQUAL(cell1.influx_coeff, 3.0 * dims[1]*dims[2]);
   }

   {
       const std::string aq = R"(
AQUANCON
     1     1	8      15    15	  3   3   'I+'      *        3  /
     1     1	8      15    15	  3   3   'I+'      100      4  /
/
)";
       const auto& [aquconn, grid] = load_aquifer(aq);
       const auto& dims = grid.getCellDims(0,14,2);
       auto cell1 = aquconn[1][0];
       BOOST_CHECK_EQUAL(cell1.influx_coeff, 4 * ( 100 + 3.0 * dims[1]*dims[2]));
   }

   {
       const std::string aq = R"(
AQUANCON
     1     1	8      15    15	  3   3   'I+'      100      4  /
     1     1	8      15    15	  3   3   'I+'      *        3  /
     1     1	8      15    15	  3   3   'I+'      77       2  /
/
)";
       const auto& [aquconn, grid] = load_aquifer(aq);
       auto cell1 = aquconn[1][0];
       BOOST_CHECK_EQUAL(cell1.influx_coeff, 2*(77 + 3*(0 + 4*100)));
   }
}
