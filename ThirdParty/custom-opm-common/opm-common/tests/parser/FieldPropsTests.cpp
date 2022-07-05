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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <sstream>
#include <string>

#define BOOST_TEST_MODULE FieldPropsTests

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/EclipseState/Grid/FieldProps.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateFieldProps) {
    EclipseGrid grid(10,10,10);
    Deck deck;
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    BOOST_CHECK(!fpm.try_get<double>("PORO"));
    BOOST_CHECK(!fpm.try_get<double>("PORO"));
    BOOST_CHECK(!fpm.try_get<double>("NO_SUCH_KEYWOWRD"));
    BOOST_CHECK(!fpm.try_get<int>("NO_SUCH_KEYWOWRD"));

    BOOST_CHECK_THROW(fpm.get_double("PORO"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get_global_double("PERMX"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get_copy<double>("PERMX"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get_int("NOT_SUPPORTED"), std::logic_error);
    BOOST_CHECK_THROW(fpm.get_double("NOT_SUPPORTED"), std::logic_error);

    BOOST_CHECK_THROW(fpm.get_global_double("NO1"), std::logic_error);
    BOOST_CHECK_THROW(fpm.get_global_int("NO2"), std::logic_error);
}



BOOST_AUTO_TEST_CASE(CreateFieldProps2) {
    std::string deck_string = R"(
GRID

PORO
   1000*0.10 /

BOX
  1 3 1 3 1 3 /

PORV
  27*100 /

ACTNUM
   27*1 /

PERMX
  27*0.6/


)";
    std::vector<int> actnum(1000, 1);
    for (std::size_t i=0; i< 1000; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,10), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    BOOST_CHECK(!fpm.has_double("NO-PORO"));
    BOOST_CHECK(fpm.has_double("PORO"));
    const auto& poro1 = fpm.get_double("PORO");
    BOOST_CHECK_EQUAL(poro1.size(), grid.getNumActive());

    const auto& poro2 = fpm.try_get<double>("PORO");
    BOOST_CHECK(poro1 == *poro2);

    BOOST_CHECK(!fpm.has_double("NO-PORO"));

    // PERMX keyword is not fully initialized
    BOOST_CHECK(!fpm.try_get<double>("PERMX"));
    BOOST_CHECK(!fpm.has_double("PERMX"));
    BOOST_CHECK_THROW(fpm.get_double("PERMX"), std::runtime_error);
    {
        const auto& keys = fpm.keys<double>();
        BOOST_CHECK_EQUAL(keys.size(), decltype(keys.size()){1});
        BOOST_CHECK(std::find(keys.begin(), keys.end(), "PORO")  != keys.end());
        BOOST_CHECK(std::find(keys.begin(), keys.end(), "PERMX") == keys.end());

        // The PORV property should be extracted with the special function
        // fp.porv() and not the general get<double>() functionality.
        BOOST_CHECK(std::find(keys.begin(), keys.end(), "PORV") == keys.end());
    }
    {
        const auto& keys = fpm.keys<int>();
        BOOST_CHECK_EQUAL(keys.size(), decltype(keys.size()){0});

        BOOST_CHECK(std::find(keys.begin(), keys.end(), "ACTNUM") == keys.end());
    }
}



BOOST_AUTO_TEST_CASE(CreateFieldPropsForActnum) {
    std::string deck_string = R"(
GRID

BOX
  1 3 1 3 1 1 /

ACTNUM
   9*0 /

BOX
  1 3 1 2 1 1 /

ACTNUM
   6*1 /

ENDBOX

-- Re-enable (3,3,1)
EQUALS
    ACTNUM 1 3 3 3 3 1 1 /
/
)";

    Deck deck = Parser{}.parseString(deck_string);
    EclipseGrid grid{GridDims{3, 3, 1}};
    FieldProps fp(deck, grid);
    std::vector<int> expected_actnum = { 1, 1, 1,
                                         1, 1, 1,
                                         0, 0, 1  };
    auto actnum = fp.actnumRaw();
    BOOST_CHECK_EQUAL_COLLECTIONS(actnum.begin(), actnum.end(), expected_actnum.begin(), expected_actnum.end());
}


BOOST_AUTO_TEST_CASE(CreateFieldPropsForActnumButNoActnumInDeck) {
    std::string deck_string = R"(
GRID
)";

    Deck deck = Parser{}.parseString(deck_string);
    EclipseGrid grid{GridDims{3, 3, 1}};
    FieldProps fp(deck, grid);
    std::vector<int> expected_actnum = { 1, 1, 1,
                                         1, 1, 1,
                                         1, 1, 1  };
    auto actnum = fp.actnumRaw();
    BOOST_CHECK_EQUAL_COLLECTIONS(actnum.begin(), actnum.end(), expected_actnum.begin(), expected_actnum.end());
}


BOOST_AUTO_TEST_CASE(INVALID_COPY) {
    std::string deck_string = R"(
GRID

COPY
   PERMX PERMY /
/
)";

    EclipseGrid grid(EclipseGrid(10,10,10));
    Deck deck = Parser{}.parseString(deck_string);
    BOOST_CHECK_THROW( FieldPropsManager(deck, Phases{true, true, true}, grid, TableManager()), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(GRID_RESET) {
    std::string deck_string = R"(
REGIONS

SATNUM
0 1 2 3 4 5 6 7 8
/
)";
    std::vector<int> actnum1 = {1,1,1,0,0,0,1,1,1};
    EclipseGrid grid(3,1,3); grid.resetACTNUM(actnum1);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& s1 = fpm.get_int("SATNUM");
    BOOST_CHECK_EQUAL(s1.size(), decltype(s1.size()){6});
    BOOST_CHECK_EQUAL(s1[0], 0);
    BOOST_CHECK_EQUAL(s1[1], 1);
    BOOST_CHECK_EQUAL(s1[2], 2);
    BOOST_CHECK_EQUAL(s1[3], 6);
    BOOST_CHECK_EQUAL(s1[4], 7);
    BOOST_CHECK_EQUAL(s1[5], 8);
    BOOST_CHECK_EQUAL(fpm.active_size(), decltype(fpm.active_size()){6});

    std::vector<int> actnum2 = {1,0,1,0,0,0,1,0,1};
    fpm.reset_actnum(actnum2);

    BOOST_CHECK_EQUAL(s1.size(), decltype(s1.size()){4});
    BOOST_CHECK_EQUAL(s1[0], 0);
    BOOST_CHECK_EQUAL(s1[1], 2);
    BOOST_CHECK_EQUAL(s1[2], 6);
    BOOST_CHECK_EQUAL(s1[3], 8);
    BOOST_CHECK_EQUAL(fpm.active_size(), decltype(fpm.active_size()){4});

    BOOST_CHECK_THROW(fpm.reset_actnum(actnum1), std::logic_error);
}

BOOST_AUTO_TEST_CASE(ADDREG) {
    std::string deck_string = R"(
GRID

PORO
   6*0.1 /

MULTNUM
 2 2 2 1 1 1 /

ADDREG
  PORO 1.0 1 M /
/

)";
    std::vector<int> actnum1 = {1,1,0,0,1,1};
    EclipseGrid grid(3,2,1); grid.resetACTNUM(actnum1);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& poro = fpm.get_double("PORO");
    BOOST_CHECK_EQUAL(poro.size(), decltype(poro.size()){4});
    BOOST_CHECK_EQUAL(poro[0], 0.10);
    BOOST_CHECK_EQUAL(poro[3], 1.10);
}



BOOST_AUTO_TEST_CASE(ASSIGN) {
    Fieldprops::FieldData<int> data({}, 100, 0);
    std::vector<int> wrong_size(50);

    BOOST_CHECK_THROW( data.default_assign( wrong_size ), std::invalid_argument );

    std::vector<int> ext_data(100);
    std::iota(ext_data.begin(), ext_data.end(), 0);
    data.default_assign( ext_data );

    BOOST_CHECK(data.valid());
    BOOST_CHECK(data.data == ext_data);
}

BOOST_AUTO_TEST_CASE(Defaulted1) {
    std::string deck_string = R"(
GRID

BOX
  1 10 1 10 1 1 /

NTG
  100*2 /

)";

    EclipseGrid grid(EclipseGrid(10,10, 2));
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& ntg = fpm.get_double("NTG");
    const auto& defaulted = fpm.defaulted<double>("NTG");

    for (std::size_t g=0; g < 100; g++) {
        BOOST_CHECK_EQUAL(ntg[g], 2);
        BOOST_CHECK_EQUAL(defaulted[g], false);

        BOOST_CHECK_EQUAL(ntg[g + 100], 1);
        BOOST_CHECK_EQUAL(defaulted[g + 100], true);
    }
}

BOOST_AUTO_TEST_CASE(Defaulted2) {
    std::string deck_string = R"(
GRID

BOX
  1 10 1 10 1 1 /

NTG
  100*2 /

)";

    std::vector<int> actnum(150, 1);
    {
        for (std::size_t i = 0; i < 50; i++)
            actnum.push_back(0);
    }
    EclipseGrid grid(EclipseGrid(10,10, 2), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& ntg = fpm.get_global_double("NTG");

    for (std::size_t g=0; g < 100; g++) {
        BOOST_CHECK_EQUAL(ntg[g], 2);
        BOOST_CHECK_EQUAL(ntg[g + 100], 1);
    }
}

BOOST_AUTO_TEST_CASE(PORV) {
    std::string deck_string = R"(
GRID

PORO
  500*0.10 /

BOX
  1 10 1 10 2 2 /

NTG
  100*2 /

ENDBOX

MULTNUM
  500*1 /

BOX
   1 10 1 10 5 5 /

MULTNUM
  100*2 /

ENDBOX

EDIT

BOX
  1 10 1 10 4 4 /

MULTPV
  100*4 /


ENDBOX

BOX
  1 10 1 10 3 3 /

PORV
  100*3 /

ENDBOX


MULTREGP
  2 8 F /  -- This should be ignored
  2 5 M /
/

ENDBOX

)";

    EclipseGrid grid(10,10, 5);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& poro = fpm.get_double("PORO");
    const auto& ntg = fpm.get_double("NTG");
    const auto& multpv = fpm.get_double("MULTPV");
    const auto& porv = fpm.porv();

    // All cells should be active for this grid
    BOOST_CHECK_EQUAL(porv.size(), grid.getNumActive());
    BOOST_CHECK_EQUAL(porv.size(), grid.getCartesianSize());

    // k = 0: poro * V
    for (std::size_t g = 0; g < 100; g++) {
        BOOST_CHECK_CLOSE(porv[g], grid.getCellVolume(g) * poro[g], 1e-13);
        BOOST_CHECK_EQUAL(porv[g], 0.10);
        BOOST_CHECK_EQUAL(poro[g], 0.10);
        BOOST_CHECK_EQUAL(ntg[g], 1.0);
        BOOST_CHECK_EQUAL(multpv[g], 1.0);
    }

    // k = 1: poro * NTG * V
    for (std::size_t g = 100; g < 200; g++) {
        BOOST_CHECK_CLOSE(porv[g], grid.getCellVolume(g) * poro[g] * ntg[g], 1e-13);
        BOOST_CHECK_EQUAL(porv[g], 0.20);
        BOOST_CHECK_EQUAL(poro[g], 0.10);
        BOOST_CHECK_EQUAL(ntg[g], 2.0);
        BOOST_CHECK_EQUAL(multpv[g], 1.0);
    }

    // k = 2: PORV - explicitly set
    for (std::size_t g = 200; g < 300; g++) {
        BOOST_CHECK_EQUAL(poro[g], 0.10);
        BOOST_CHECK_EQUAL(ntg[g], 1.0);
        BOOST_CHECK_EQUAL(multpv[g], 1.0);
        BOOST_CHECK_EQUAL(porv[g],3.0);
    }

    // k = 3: poro * V * multpv
    for (std::size_t g = 300; g < 400; g++) {
        BOOST_CHECK_CLOSE(porv[g], multpv[g] * grid.getCellVolume(g) * poro[g] * ntg[g], 1e-13);
        BOOST_CHECK_EQUAL(porv[g], 0.40);
        BOOST_CHECK_EQUAL(poro[g], 0.10);
        BOOST_CHECK_EQUAL(ntg[g], 1.0);
        BOOST_CHECK_EQUAL(multpv[g], 4.0);
    }

    // k = 4: poro * V * MULTREGP
    for (std::size_t g = 400; g < 500; g++) {
        BOOST_CHECK_CLOSE(porv[g], grid.getCellVolume(g) * poro[g] * 5.0, 1e-13);
        BOOST_CHECK_EQUAL(porv[g], 0.50);
        BOOST_CHECK_EQUAL(poro[g], 0.10);
    }

    std::vector<int> actnum(500, 1);
    actnum[0] = 0;
    grid.resetACTNUM(actnum);

    fpm.reset_actnum(actnum);
    auto porv_global = fpm.porv(true);
    auto porv_active = fpm.porv(false);
    BOOST_CHECK_EQUAL( porv_active.size(), grid.getNumActive());
    BOOST_CHECK_EQUAL( porv_global.size(), grid.getCartesianSize());
    BOOST_CHECK_EQUAL( porv_global[0], 0);
    for (std::size_t g = 1; g < grid.getCartesianSize(); g++) {
        BOOST_CHECK_EQUAL(porv_active[g - 1], porv_global[g]);
        BOOST_CHECK_EQUAL(porv_global[g], porv[g]);
    }
}


BOOST_AUTO_TEST_CASE(LATE_GET_SATFUNC) {
    const char* deckString =
        "RUNSPEC\n"
        "\n"
        "OIL\n"
        "GAS\n"
        "WATER\n"
        "TABDIMS\n"
        "3 /\n"
        "\n"
        "METRIC\n"
        "\n"
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "PERMX\n"
        " 27*1000 /\n"
        "MAXVALUE\n"
        "  PERMX 100 4* 1  1/\n"
        "/\n"
        "MINVALUE\n"
        "  PERMX 10000 4* 3  3/\n"
        "/\n"
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
        "PORO \n"
        "  27*0.15 /\n"
        "PROPS\n"
        "\n"
        "SWOF\n"
        // table 1
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.1    0        1.0      2.0\n"
        "  0.15   0        0.9      1.0\n"
        "  0.2    0.01     0.5      0.5\n"
        "  0.93   0.91     0.0      0.0\n"
        "/\n"
        // table 2
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.00   0        1.0      2.0\n"
        "  0.05   0.01     1.0      2.0\n"
        "  0.10   0.02     0.9      1.0\n"
        "  0.15   0.03     0.5      0.5\n"
        "  0.852  1.00     0.0      0.0\n"
        "/\n"
        // table 3
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.00   0.00     0.9      2.0\n"
        "  0.05   0.02     0.8      1.0\n"
        "  0.10   0.03     0.5      0.5\n"
        "  0.801  1.00     0.0      0.0\n"
        "/\n"
        "\n"
        "SGOF\n"
        // table 1
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.00   0.00     0.9      2.0\n"
        "  0.05   0.02     0.8      1.0\n"
        "  0.10   0.03     0.5      0.5\n"
        "  0.80   1.00     0.0      0.0\n"
        "/\n"
        // table 2
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.05   0.00     1.0      2\n"
        "  0.10   0.02     0.9      1\n"
        "  0.15   0.03     0.5      0.5\n"
        "  0.85   1.00     0.0      0\n"
        "/\n"
        // table 3
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.1    0        1.0      2\n"
        "  0.15   0        0.9      1\n"
        "  0.2    0.01     0.5      0.5\n"
        "  0.9    0.91     0.0      0\n"
        "/\n"
        "\n"
        "REGIONS\n"
        "\n"
        "SATNUM\n"
        "9*1 9*2 9*3 /\n"
        "\n"
        "IMBNUM\n"
        "9*3 9*2 9*1 /\n"
        "\n"
        "SOLUTION\n"
        "\n"
        "SCHEDULE\n";

    Opm::Parser parser;

    auto deck = parser.parseString(deckString);
    Opm::TableManager tm(deck);
    Opm::EclipseGrid eg(deck);
    Opm::FieldPropsManager fp(deck, Phases{true, true, true}, eg, tm);

    const auto& fp_swu = fp.get_global_double("SWU");
    BOOST_CHECK_EQUAL(fp_swu[1 + 0 * 3*3], 0.93);
    BOOST_CHECK_EQUAL(fp_swu[1 + 1 * 3*3], 0.852);
    BOOST_CHECK_EQUAL(fp_swu[1 + 2 * 3*3], 0.801);

    const auto& fp_sgu = fp.get_global_double("ISGU");
    BOOST_CHECK_EQUAL(fp_sgu[1 + 0 * 3*3], 0.9);
    BOOST_CHECK_EQUAL(fp_sgu[1 + 1 * 3*3], 0.85);
    BOOST_CHECK_EQUAL(fp_sgu[1 + 2 * 3*3], 0.80);

}

BOOST_AUTO_TEST_CASE(GET_TEMP) {
    std::string deck_string = R"(
GRID

PORO
   200*0.15 /

)";

    EclipseGrid grid(10,10, 2);
    Deck deck = Parser{}.parseString(deck_string);
    std::vector<int> actnum(200, 1); actnum[0] = 0;
    grid.resetACTNUM(actnum);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    BOOST_CHECK(!fpm.has_double("NTG"));
    const auto& ntg = fpm.get_copy<double>("NTG");
    BOOST_CHECK(!fpm.has_double("NTG"));
    BOOST_CHECK(ntg.size() == grid.getNumActive());


    BOOST_CHECK(fpm.has_double("PORO"));
    const auto& poro1 = fpm.get_copy<double>("PORO");
    BOOST_CHECK(fpm.has_double("PORO"));
    const auto& poro2 = fpm.get_copy<double>("PORO");
    BOOST_CHECK(fpm.has_double("PORO"));
    BOOST_CHECK( poro1 == poro2 );
    BOOST_CHECK( &poro1 != &poro2 );
    BOOST_CHECK( poro1.size() == grid.getNumActive());

    BOOST_CHECK(!fpm.has_int("SATNUM"));
    const auto& satnum = fpm.get_copy<int>("SATNUM", true);
    BOOST_CHECK(!fpm.has_int("SATNUM"));
    BOOST_CHECK(satnum.size() == grid.getCartesianSize());

    //The PERMY keyword can not be default initialized
    BOOST_CHECK_THROW(fpm.get_copy<double>("PERMY"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(GET_TEMPI) {
    std::string deck_string = R"(
RUNSPEC

EQLDIMS
/
PROPS

RTEMPVD
   0.5 0
   1.5 100 /


)";

    EclipseGrid grid(1,1, 2);
    Deck deck = Parser{}.parseString(deck_string);
    Opm::TableManager tm(deck);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, tm);

    const auto& tempi = fpm.get_double("TEMPI");
    double celcius_offset = 273.15;
    BOOST_CHECK_CLOSE( tempi[0], 0 + celcius_offset , 1e-6);
    BOOST_CHECK_CLOSE( tempi[1], 100 + celcius_offset , 1e-6);
}

BOOST_AUTO_TEST_CASE(GridAndEdit) {
    const std::string deck_string = R"(
RUNSPEC

GRID
MULTZ
  125*2 /
MULTX
  125*2 /
MULTX
  125*2 /
PORO
  125*0.15 /
EDIT
MULTZ
  125*2 /
)";

    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deck_string);
    Opm::EclipseGrid grid(5,5,5);
    Opm::TableManager tm(deck);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, tm);

    const auto& multz = fpm.get_double("MULTZ");
    const auto& multx = fpm.get_double("MULTX");
    BOOST_CHECK_EQUAL( multz[0], 4 );
    BOOST_CHECK_EQUAL( multx[0], 2 );
}

BOOST_AUTO_TEST_CASE(OPERATE) {
    std::string deck_string = R"(
GRID

PORO
   6*1.0 /

OPERATE
    PORO   1  3   1  1   1   1  'MAXLIM'   PORO 0.50 /
    PORO   1  3   2  2   1   1  'MAXLIM'   PORO 0.25 /
/


PERMX
   6*1/

PERMY
   6*1000/

OPERATE
    PERMX   1  3   1  1   1   1  'MINLIM'   PERMX 2 /
    PERMX   1  3   2  2   1   1  'MINLIM'   PERMX 4 /
    PERMY   1  3   1  1   1   1  'MAXLIM'   PERMY 100 /
    PERMY   1  3   2  2   1   1  'MAXLIM'   PERMY 200 /
    PERMZ   1  3   1  1   1   1  'MULTA'    PERMY 2 1000 /
    PERMZ   1  3   2  2   1   1  'MULTA'    PERMX 3  300 /
/




)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::permeability, raw_value); };
    EclipseGrid grid(3,2,1);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& poro = fpm.get_double("PORO");
    BOOST_CHECK_EQUAL(poro[0], 0.50);
    BOOST_CHECK_EQUAL(poro[3], 0.25);

    const auto& permx = fpm.get_double("PERMX");
    BOOST_CHECK_EQUAL(permx[0], to_si(2));
    BOOST_CHECK_EQUAL(permx[3], to_si(4));

    const auto& permy = fpm.get_double("PERMY");
    BOOST_CHECK_EQUAL(permy[0], to_si(100));
    BOOST_CHECK_EQUAL(permy[3], to_si(200));

    const auto& permz = fpm.get_double("PERMZ");
    for (std::size_t i = 0; i < 3; i++) {
        BOOST_CHECK_CLOSE(permz[i]  , 2*permy[i]   + to_si(1000), 1e-13);
        BOOST_CHECK_CLOSE(permz[i+3], 3*permx[i+3] + to_si(300), 1e-13);
    }
}

BOOST_AUTO_TEST_CASE(EPS_Props_Inconsistent) {
    BOOST_CHECK_THROW(const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
10 10 3 /

-- No GAS keyword

PROPS
SGCR    -- Requires 'GAS'
300*0.001 /
)"), Opm::OpmInputError);

    BOOST_CHECK_THROW(const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
10 10 3 /

OIL
GAS
-- No ENDSCALE keyword

PROPS
SOGCR    -- Requires 'ENDSCALE'
300*0.05 /
)"), Opm::OpmInputError);

    BOOST_CHECK_THROW(const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
10 10 3 /

OIL
-- GAS
ENDSCALE
/

PROPS
SOGCR    -- Requires 'GAS'
300*0.05 /
)"), Opm::OpmInputError);

    BOOST_CHECK_THROW(const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
10 10 3 /

-- OIL
GAS
ENDSCALE
/

PROPS
SOGCR    -- Requires 'OIL'
300*0.05 /
)"), Opm::OpmInputError);
}

namespace {
    std::string satfunc_model_setup()
    {
        return { R"(
RUNSPEC

DIMENS
6 6 3 /

OIL
GAS
DISGAS
WATER
METRIC

TABDIMS
/

GRID

DXV
  6*100.0 /
DYV
  6*100.0 /
DZV
  3*5.0 /

TOPS
  36*2000.0 /

PERMX
  108*100.0 /
PERMY
  108*100.0 /
PERMZ
  108*10.0 /
PORO
  108*0.3 /

PROPS

DENSITY
  850.0  1014.1  1.02 /

)" };

    }

    std::string satfunc_family_I()
    {
        // Saturation Region 1 from opm-tests/model1/include/sattab_basemod1.sattab

        return { R"(
SWOF
0.071004 0.000000 1.000000 7.847999
0.091004 0.000001 0.882459 4.365734
0.111004 0.000046 0.775956 2.730007
0.131004 0.000251 0.679729 1.845613
0.151004 0.000739 0.593050 1.319233
0.171004 0.001631 0.515222 0.983255
0.191004 0.003052 0.445580 0.757089
0.211004 0.005126 0.383490 0.598332
0.231004 0.007976 0.328347 0.483059
0.251004 0.011727 0.279578 0.396991
0.271004 0.016503 0.236638 0.331205
0.291004 0.022430 0.199013 0.279911
0.311004 0.029632 0.166213 0.239222
0.331004 0.038234 0.137781 0.206460
0.351004 0.048361 0.113284 0.179731
0.371004 0.060138 0.092316 0.157670
0.391004 0.073691 0.074499 0.139272
0.411004 0.089145 0.059479 0.123784
0.431004 0.106626 0.046927 0.110637
0.451004 0.126260 0.036540 0.099391
0.471004 0.148171 0.028035 0.089704
0.491004 0.172487 0.021156 0.081308
0.511004 0.199332 0.015668 0.073987
0.531004 0.228832 0.011357 0.067570
0.551004 0.261115 0.008030 0.061917
0.571004 0.296305 0.005516 0.056913
0.591004 0.334530 0.003662 0.052467
0.611004 0.375914 0.002333 0.048498
0.631004 0.420585 0.001413 0.044944
0.651004 0.468668 0.000804 0.041749
0.671004 0.520291 0.000422 0.038868
0.691004 0.575579 0.000199 0.036261
0.711004 0.634659 0.000080 0.033897
0.731004 0.697657 0.000026 0.031746
0.751004 0.764701 0.000006 0.029784
0.771004 0.835916 0.000001 0.027991
0.791004 0.911429 0.000000 0.026347
0.800794 0.950000 0.000000 0.025592
0.990000 1* 0.000000 0.015139
1.000000 1.000000 0.000000 0.000000
/

SGOF
0.000000 0.000000 1.000000 0.000000
0.030000 0.000000 0.896942 0.000000
0.060000 0.002411 0.801286 0.000000
0.090000 0.008238 0.712749 0.000000
0.120000 0.016904 0.631047 0.000000
0.150000 0.028151 0.555898 0.000000
0.180000 0.041812 0.487020 0.000000
0.210000 0.057767 0.424134 0.000000
0.240000 0.075921 0.366958 0.000000
0.270000 0.096200 0.315214 0.000000
0.300000 0.118539 0.268622 0.000000
0.330000 0.142884 0.226906 0.000000
0.360000 0.169188 0.189789 0.000000
0.390000 0.197407 0.156994 0.000000
0.420000 0.227505 0.128247 0.000000
0.450000 0.259448 0.103275 0.000000
0.480000 0.293205 0.081803 0.000000
0.510000 0.328748 0.063562 0.000000
0.540000 0.366051 0.048280 0.000000
0.570000 0.405089 0.035689 0.000000
0.600000 0.445840 0.025521 0.000000
0.630000 0.488284 0.017511 0.000000
0.660000 0.532400 0.011396 0.000000
0.690000 0.578171 0.006912 0.000000
0.720000 0.625578 0.003801 0.000000
0.750000 0.674606 0.001807 0.000000
0.780000 0.725239 0.000675 0.000000
0.810000 0.777461 0.000156 0.000000
0.840000 0.831260 0.000009 0.000000
0.858996 0.866135 0.000000 0.000000
0.888996 0.922479 0.000000 0.000000
0.918996 0.980364 0.000000 0.000000
0.928996 1.000000 0.000000 0.000000
/
)" };
    }

    std::string satfunc_family_II()
    {
        // Saturation Region 1 from opm-tests/model1/include/sattab_basemod1.sattab
        // rephrased as Family II.

        return { R"(
SWFN
0.071004 0.000000 7.847999
0.091004 0.000001 4.365734
0.111004 0.000046 2.730007
0.131004 0.000251 1.845613
0.151004 0.000739 1.319233
0.171004 0.001631 0.983255
0.191004 0.003052 0.757089
0.211004 0.005126 0.598332
0.231004 0.007976 0.483059
0.251004 0.011727 0.396991
0.271004 0.016503 0.331205
0.291004 0.022430 0.279911
0.311004 0.029632 0.239222
0.331004 0.038234 0.206460
0.351004 0.048361 0.179731
0.371004 0.060138 0.157670
0.391004 0.073691 0.139272
0.411004 0.089145 0.123784
0.431004 0.106626 0.110637
0.451004 0.126260 0.099391
0.471004 0.148171 0.089704
0.491004 0.172487 0.081308
0.511004 0.199332 0.073987
0.531004 0.228832 0.067570
0.551004 0.261115 0.061917
0.571004 0.296305 0.056913
0.591004 0.334530 0.052467
0.611004 0.375914 0.048498
0.631004 0.420585 0.044944
0.651004 0.468668 0.041749
0.671004 0.520291 0.038868
0.691004 0.575579 0.036261
0.711004 0.634659 0.033897
0.731004 0.697657 0.031746
0.751004 0.764701 0.029784
0.771004 0.835916 0.027991
0.791004 0.911429 0.026347
0.800794 0.950000 0.025592
0.990000 0.997490 0.015139
1.000000 1.000000 0.000000
/

SGFN
0.000000 0.000000 0.000000
0.030000 0.000000 0.000000
0.060000 0.002411 0.000000
0.090000 0.008238 0.000000
0.120000 0.016904 0.000000
0.150000 0.028151 0.000000
0.180000 0.041812 0.000000
0.210000 0.057767 0.000000
0.240000 0.075921 0.000000
0.270000 0.096200 0.000000
0.300000 0.118539 0.000000
0.330000 0.142884 0.000000
0.360000 0.169188 0.000000
0.390000 0.197407 0.000000
0.420000 0.227505 0.000000
0.450000 0.259448 0.000000
0.480000 0.293205 0.000000
0.510000 0.328748 0.000000
0.540000 0.366051 0.000000
0.570000 0.405089 0.000000
0.600000 0.445840 0.000000
0.630000 0.488284 0.000000
0.660000 0.532400 0.000000
0.690000 0.578171 0.000000
0.720000 0.625578 0.000000
0.750000 0.674606 0.000000
0.780000 0.725239 0.000000
0.810000 0.777461 0.000000
0.840000 0.831260 0.000000
0.858996 0.866135 0.000000
0.888996 0.922479 0.000000
0.918996 0.980364 0.000000
0.928996 1.000000 0.000000
/

SOF3
0.000000 0.000000 0.000000
0.010000 0.000000 0.000000
0.040000 0.000000 0.000000
0.070000 0.000000 0.000000
0.088996 0.000000 0.000009
0.118996 0.000000 0.000156
0.148996 0.000000 0.000675
0.178996 0.000000 0.001807
0.199206 0.000000 0.003150
0.208996 0.000000 0.003801
0.228996 0.000001 0.005875
0.238996 0.000004 0.006912
0.248996 0.000006 0.008407
0.268996 0.000026 0.011396
0.288996 0.000080 0.015473
0.298996 0.000140 0.017511
0.308996 0.000199 0.020181
0.328996 0.000422 0.025521
0.348996 0.000804 0.032300
0.358996 0.001109 0.035689
0.368996 0.001413 0.039886
0.388996 0.002333 0.048280
0.408996 0.003662 0.058468
0.418996 0.004589 0.063562
0.428996 0.005516 0.069642
0.448996 0.008030 0.081803
0.468996 0.011357 0.096118
0.478996 0.013513 0.103275
0.488996 0.015668 0.111599
0.508996 0.021156 0.128247
0.528996 0.028035 0.147412
0.538996 0.032288 0.156994
0.548996 0.036540 0.167926
0.568996 0.046927 0.189789
0.588996 0.059479 0.214534
0.598996 0.066989 0.226906
0.608996 0.074499 0.240811
0.628996 0.092316 0.268622
0.648996 0.113284 0.299683
0.658996 0.125533 0.315214
0.668996 0.137781 0.332462
0.688996 0.166213 0.366958
0.708996 0.199013 0.405075
0.718996 0.217826 0.424134
0.728996 0.236638 0.445096
0.748996 0.279578 0.487020
0.768996 0.328347 0.532939
0.778996 0.355919 0.555898
0.788996 0.383490 0.580948
0.808996 0.445580 0.631047
0.828996 0.515222 0.685515
0.838996 0.554136 0.712749
0.848996 0.593050 0.742261
0.868996 0.679729 0.801286
0.888996 0.775956 0.865057
0.898996 0.829208 0.896942
0.908996 0.882459 0.931295
0.928996 1.000000 1.000000
/ )" };
    }

    std::string tolCrit(const double t)
    {
        std::ostringstream os;

        os << "TOLCRIT\n  " << std::scientific << std::setw(11) << t << "\n/\n";

        return os.str();
    }

    std::string end()
    {
        return { R"(
END
)" };
    }
} // namespace Anonymous

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_I_TolCrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.0;

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.071004, 1.0e-10);  // == SWL.  TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 1.0 - 0.791004, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(sogcr[0], 1.0 - 0.858996 - 0.071004, 1.0e-10);  // Include SWL
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_II_TolCrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.0;

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.071004, 1.0e-10);  // == SWL.  TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 1.0 - 0.791004, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(sogcr[0], 1.0 - 0.858996 - 0.071004, 1.0e-10);  // Include SWL
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_I_TolCrit_Default) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = es.runspec().saturationFunctionControls()
        .minimumRelpermMobilityThreshold(); // 1.0e-6.

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.091004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 0.228996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.070000, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_II_TolCrit_Default) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = es.runspec().saturationFunctionControls()
        .minimumRelpermMobilityThreshold(); // 1.0e-6.

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.091004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 0.228996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.070000, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_I_TolCrit_Large) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.01;

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.231004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 0.448996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.238996, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.09, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(RawTableEndPoints_Family_II_TolCrit_Large) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.01;

    auto rtepPtr = satfunc::getRawTableEndpoints(tm, ph, tolcrit);

    // Water end-points
    {
        const auto swl  = rtepPtr.connate .water;
        const auto swcr = rtepPtr.critical.water;
        const auto swu  = rtepPtr.maximum .water;
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.231004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = rtepPtr.critical.oil_in_water;
        const auto sogcr = rtepPtr.critical.oil_in_gas;
        BOOST_CHECK_CLOSE(sowcr[0], 0.448996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.248996, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT
    }

    // Gas end-points
    {
        const auto sgl  = rtepPtr.connate .gas;
        const auto sgcr = rtepPtr.critical.gas;
        const auto sgu  = rtepPtr.maximum .gas;
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.09, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);
    }
}

// =====================================================================

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_I_Tolcrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.0;

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values (TOLCRIT = 0.0)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 1.0, 1.0e-10);       // Krow(So=1-Swcr-Sgl) = Krow(So=0.928996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.896942, 1.0e-10);  // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0, 1.0e-10);       // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.866135, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.911429, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.791004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_II_Tolcrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.0;

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 1.0, 1.0e-10);      // Krow(So=1-Swcr-Sgl) = Krow(So=0.928996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.896942, 1.0e-10); // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0, 1.0e-10);      // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.866135, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.911429, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.791004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_I_Tolcrit_Default) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = es.runspec().saturationFunctionControls()
        .minimumRelpermMobilityThreshold(); // 1.0e-6.

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 0.882459, 1.0e-10); // Krow(So=1-Swcr-Sgl) = Krow(So=0.908996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.896942, 1.0e-10); // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0, 1.0e-10);      // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.866135, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.835916, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.771004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_II_Tolcrit_Default) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = es.runspec().saturationFunctionControls()
        .minimumRelpermMobilityThreshold(); // 1.0e-6.

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 0.882459, 1.0e-10); // Krow(So=1-Swcr-Sgl) = Krow(So=0.908996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.896942, 1.0e-10); // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0     , 1.0e-10); // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.866135, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.835916, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.771004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_I_Tolcrit_Large) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.01;

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 0.328347, 1.0e-10); // Krow(So=1-Swcr-Sgl) = Krow(So=0.768996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.712749, 1.0e-10); // Krog(So=1-Sgcr-Swl) = Krog(So=0.838996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0, 1.0e-10);      // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.578171, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.690000)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.261115, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.551004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

BOOST_AUTO_TEST_CASE(RawFunctionValues_Family_II_Tolcrit_Large) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    const auto& tm      = es.getTableManager();
    const auto& ph      = es.runspec().phases();
    const auto  tolcrit = 0.01;

    const auto rtepPtr  = satfunc::getRawTableEndpoints(tm, ph, tolcrit);
    const auto rfuncPtr = satfunc::getRawFunctionValues(tm, ph, rtepPtr);

    // Oil values
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rw [0], 0.328347, 1.0e-10); // Krow(So=1-Swcr-Sgl) = Krow(So=0.768996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.rg [0], 0.712749, 1.0e-10); // Krog(So=1-Sgcr-Swl) = Krog(So=0.838996)
    BOOST_CHECK_CLOSE(rfuncPtr.kro.max[0], 1.0, 1.0e-10);      // Krow(So=Somax) = Krog(So=Somax)

    // Gas values
    BOOST_CHECK_CLOSE(rfuncPtr.krg.r  [0], 0.562914, 1.0e-10); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.680000)
    BOOST_CHECK_CLOSE(rfuncPtr.krg.max[0], 1.0     , 1.0e-10); // Krg(Sgmax) = Krg(Sg=0.928996)

    // Water values
    BOOST_CHECK_CLOSE(rfuncPtr.krw.r  [0], 0.261115, 1.0e-10); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.551004)
    BOOST_CHECK_CLOSE(rfuncPtr.krw.max[0], 1.0     , 1.0e-10); // Krw(Swmax) = Krw(Sw=1)
}

// =====================================================================

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_I_TolCrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + tolCrit(0.0) + satfunc_family_I() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.071004, 1.0e-10);  // == SWL.  TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.791004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.911429, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 1.0 - 0.791004, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(sogcr[0], 1.0 - 0.858996 - 0.071004, 1.0e-10);  // Include SWL

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.928996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 1.0, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(krorg[0], 0.896942, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0, 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.866135, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_II_TolCrit_Zero) {
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + tolCrit(0.0) + satfunc_family_II() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.071004, 1.0e-10);  // == SWL.  TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.791004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.911429, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 1.0 - 0.791004, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(sogcr[0], 1.0 - 0.858996 - 0.071004, 1.0e-10);  // Include SWL

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.928996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 1.0, 1.0e-10);  // TOLCRIT = 0.0
        BOOST_CHECK_CLOSE(krorg[0], 0.896942, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0, 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.866135, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_I_TolCrit_Default) {
    // TOLCRIT = 1.0e-6
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_I() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.091004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.771004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.835916, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 1.0 - 0.771004, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT.
        BOOST_CHECK_CLOSE(sogcr[0], 1.0 - 0.858996 - 0.071004, 1.0e-10);  // Include SWL

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.908996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 0.882459, 1.0e-10);
        BOOST_CHECK_CLOSE(krorg[0], 0.896942, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0, 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.866135, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_II_TolCrit_Default) {
    // TOLCRIT = 1.0e-6
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + satfunc_family_II() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.091004, 1.0e-10); // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.771004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.835916, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 0.228996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.070000, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.908996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.898996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 0.882459, 1.0e-10);
        BOOST_CHECK_CLOSE(krorg[0], 0.896942, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0     , 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");  // Max Sg for which Krg(Sg) <= TOLCRIT
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.03, 1.0e-10);
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.858996)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.866135, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_I_TolCrit_Large) {
    // TOLCRIT = 0.01
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + tolCrit(0.01) + satfunc_family_I() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.231004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.551004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.261115, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 0.448996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.238996, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.768996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.838996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 0.328347, 1.0e-10);
        BOOST_CHECK_CLOSE(krorg[0], 0.712749, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0, 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.090000, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.690000)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.578171, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(SatFunc_EndPts_Family_II_TolCrit_Large) {
    // TOLCRIT = 0.01
    const auto es = ::Opm::EclipseState {
        ::Opm::Parser{}.parseString(satfunc_model_setup() + tolCrit(0.01) + satfunc_family_II() + end())
    };

    auto fp = es.fieldProps();

    // Water end-points
    {
        const auto swl  = fp.get_double("SWL");
        const auto swcr = fp.get_double("SWCR");
        const auto swu  = fp.get_double("SWU");
        BOOST_CHECK_CLOSE(swl [0], 0.071004, 1.0e-10);
        BOOST_CHECK_CLOSE(swcr[0], 0.231004, 1.0e-10);  // Max Sw for which Krw(Sw) <= TOLCRIT
        BOOST_CHECK_CLOSE(swu [0], 1.0     , 1.0e-10);

        const auto krwr = fp.get_double("KRWR"); // Krw(Sw=1-Sowcr-Sgl) = Krw(Sw=0.551004)
        const auto krw  = fp.get_double("KRW");  // Krw(Swmax) = Krw(Sw=1)
        BOOST_CHECK_CLOSE(krwr[0], 0.261115, 1.0e-10);
        BOOST_CHECK_CLOSE(krw [0], 1.0     , 1.0e-10);

        const auto pcw = fp.get_double("PCW");
        BOOST_CHECK_CLOSE(pcw[0], 7.847999*unit::barsa, 1.0e-10);
    }

    // Oil end-points
    {
        const auto sowcr = fp.get_double("SOWCR");
        const auto sogcr = fp.get_double("SOGCR");
        BOOST_CHECK_CLOSE(sowcr[0], 0.448996, 1.0e-10);  // Max So for which Krow(So) <= TOLCRIT
        BOOST_CHECK_CLOSE(sogcr[0], 0.248996, 1.0e-10);  // Max So for which Krog(So) <= TOLCRIT

        const auto krorw = fp.get_double("KRORW");  // Krow(So=1-Swcr-Sgl) = Krow(So=0.768996)
        const auto krorg = fp.get_double("KRORG");  // Krog(So=1-Sgcr-Swl) = Krog(So=0.838996)
        const auto kro   = fp.get_double("KRO");    // Krow(So=Somax) = Krog(So=Somax)
        BOOST_CHECK_CLOSE(krorw[0], 0.328347, 1.0e-10);
        BOOST_CHECK_CLOSE(krorg[0], 0.712749, 1.0e-10);
        BOOST_CHECK_CLOSE(kro  [0], 1.0, 1.0e-10);
    }

    // Gas end-points
    {
        const auto sgl  = fp.get_double("SGL");
        const auto sgcr = fp.get_double("SGCR");
        const auto sgu  = fp.get_double("SGU");
        BOOST_CHECK_CLOSE(sgl [0], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(sgcr[0], 0.090000, 1.0e-10);  // Max Sg for which Krg(Sg) <= TOLCRIT
        BOOST_CHECK_CLOSE(sgu [0], 0.928996, 1.0e-10);

        const auto krgr = fp.get_double("KRGR"); // Krg(Sg=1-Sogcr-Swl) = Krg(Sg=0.680000)
        const auto krg  = fp.get_double("KRG");  // Krg(Sgmax) = Krg(Sg=0.928996)
        BOOST_CHECK_CLOSE(krgr[0], 0.562914, 1.0e-10);
        BOOST_CHECK_CLOSE(krg [0], 1.0     , 1.0e-10);

        const auto pcg = fp.get_double("PCG");
        BOOST_CHECK_CLOSE(pcg[0], 0.0, 1.0e-10);
    }
}

// =====================================================================

BOOST_AUTO_TEST_CASE(GET_FIPXYZ) {
    std::string deck_string = R"(
GRID

PORO
   200*0.15 /

REGIONS

FIPNUM
  200*1 /

FIPXYZ
  100*1 100*2 /
)";

    EclipseGrid grid(10,10, 2);
    Deck deck = Parser{}.parseString(deck_string);
    BOOST_CHECK(deck.hasKeyword("FIPXYZ"));
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& fipxyz = fpm.get_int("FIPXYZ");
    BOOST_CHECK_EQUAL(fipxyz[0],   1);
    BOOST_CHECK_EQUAL(fipxyz[100], 2);
}

BOOST_AUTO_TEST_CASE(GLOBAL_FIELD1) {
    std::string deck_string = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTZ
  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 /

)";
    std::vector<int> actnum(27, 1);
    for (std::size_t i=9; i< 18; i++)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(3,3,3), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    auto multz = fpm.get_double("MULTZ");
    auto multz_global = fpm.get_global_double("MULTZ");
    for (std::size_t index = 0; index < multz_global.size(); index++)
        BOOST_CHECK_EQUAL(index * 1.0, multz_global[index]);
}

BOOST_AUTO_TEST_CASE(GLOBAL_FIELD2)
{
    std::string deck_string = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTZ
  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 /

EDIT

MULTZ
  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 /

)";
    std::vector<int> actnum(27, 1);
    for (std::size_t i=9; i< 18; i++)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(3,3,3), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    auto multz = fpm.get_double("MULTZ");
    auto multz_global = fpm.get_global_double("MULTZ");
    for (std::size_t index = 0; index < multz_global.size(); index++)
        BOOST_CHECK_EQUAL(index * index * 1.0, multz_global[index]);
}

BOOST_AUTO_TEST_CASE(GLOBAL_FIELD3)
{
    std::string deck_string = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTZ
  0   1  2  3  4  5  6  7  8
  9*0
  18 19 20 21 22 23 24 25 26 /

EQUALS
   MULTZ 99 1 3 1 3 2 2 /
/


)";
    std::vector<int> actnum(27, 1);
    for (std::size_t i=9; i< 18; i++)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(3,3,3), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    auto multz = fpm.get_double("MULTZ");
    auto multz_global = fpm.get_global_double("MULTZ");
    for (std::size_t index = 0; index < multz_global.size(); index++) {
        if (index <= 8 || index >= 18)
            BOOST_CHECK_EQUAL(index * 1.0, multz_global[index]);
        else
            BOOST_CHECK_EQUAL(99, multz_global[index]);
    }
}

namespace {
FieldPropsManager make_fp(const std::string& deck_string) {
    std::vector<int> actnum(27, 1);
    for (std::size_t i=9; i< 18; i++)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(3,3,3), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    return FieldPropsManager(deck, Phases{true, true, true}, grid, TableManager());
}
}

BOOST_AUTO_TEST_CASE(GLOBAL_UNSUPPORTED) {
    // Operations involving two keywords can not update a global keyword.
    std::string invalid_copy = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTX
 27*1.0 /

COPY
   MULTX MULTZ
/

)";

    // Can not update a global keyword with xxxxREG operations
    std::string invalid_region = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTZ
 27*1.0 /

EQUALREG
   MULTZ  2.0 1 /
/

)";

    // Can not update a global keyword with the OPERATE keyword
    std::string invalid_operate = R"(
GRID

PORO
   27*0.10 /

ACTNUM
   9*1 9*0 9*1 /

MULTZ
 27*1.0 /

OPERATE
   MULTZ 1  3   1  1   1   1  'MAXLIM'   MULTZ 0.50 /
/

)";


    BOOST_CHECK_THROW(make_fp(invalid_copy), OpmInputError);
    BOOST_CHECK_THROW(make_fp(invalid_region), std::logic_error);
    BOOST_CHECK_THROW(make_fp(invalid_operate), std::logic_error);
}



BOOST_AUTO_TEST_CASE(TRAN_Calculator) {
    std::string deck_string = R"(
GRID

PORO
   300*0.10 /

EDIT

TRANX
  300*0.10 /

BOX
  1 10 1 10 1 1 /

TRANX
   100*0.20 /

ENDBOX

MULTIPLY
  TRANY  2.0 /
/

ADD
  TRANY 3 1 10 1 10 2 2 /
/

MAXVALUE
  TRANZ 0 1 10 1 10 1 1 /
/

MINVALUE
  TRANZ 3 1 10 1 10 2 2 /
/

)";
    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::transmissibility, raw_value); };
    std::vector<int> actnum(300, 1);
    for (std::size_t i=0; i< 300; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,3), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    std::vector<double> tranx( grid.getNumActive() );
    std::vector<double> trany( grid.getNumActive(), to_si(1.0) );
    std::vector<double> tranz( grid.getNumActive(), to_si(1.0) );
    BOOST_CHECK(!fpm.has_double("TRANX"));

    BOOST_CHECK_THROW(fpm.apply_tran("TRANA", tranx), std::out_of_range);
    BOOST_CHECK(!fpm.tran_active("TRANA"));

    fpm.apply_tran("TRANX", tranx);
    fpm.apply_tran("TRANY", trany);
    fpm.apply_tran("TRANZ", tranz);

    for (std::size_t k=0; k < 3; k++) {
        for (std::size_t j=0; j < 10; j++) {
            for (std::size_t i=0; i < 10; i++) {
                if (grid.cellActive(i,j,k)) {
                    auto a = grid.activeIndex(i,j,k);

                    if (k==0)
                        BOOST_CHECK_CLOSE(tranx[a], to_si(0.20), 1e-13);
                    else
                        BOOST_CHECK_CLOSE(tranx[a], to_si(0.10), 1e-13);

                    if (k == 0)
                        BOOST_CHECK_CLOSE(trany[a], to_si(2.0), 1e-13);
                    else if (k == 1)
                        BOOST_CHECK_CLOSE(trany[a], to_si(5.0), 1e-13);
                    else if (k == 2)
                        BOOST_CHECK_CLOSE(trany[a], to_si(2.0), 1e-13);

                    if (k==0)
                        BOOST_CHECK(tranz[a] <= to_si(0));
                    else if (k == 1)
                        BOOST_CHECK(tranz[a] >= to_si(3.0));
                    else if (k == 2)
                        BOOST_CHECK_CLOSE(tranz[a], to_si(1.0), 1e-13);
                }
            }
        }
    }
    auto buffer = fpm.serialize_tran();
    fpm.deserialize_tran(buffer);

    BOOST_CHECK(fpm.tran_active("TRANX"));
    BOOST_CHECK(fpm.tran_active("TRANY"));
    BOOST_CHECK(fpm.tran_active("TRANZ"));
}

BOOST_AUTO_TEST_CASE(TRAN_KEYS) {
    std::string deck_string = R"(
GRID

PORO
   300*0.10 /

EDIT

EQUALS
  TRANX 0 1 1 1 1 1 1 /
/

)";
    EclipseGrid grid(10,10,3);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());

    const auto& keys = fpm.keys<double>();
    BOOST_CHECK_EQUAL(keys.size(), 2);

    BOOST_CHECK_THROW( fpm.get_double_field_data("TRANX0"), std::exception );
    BOOST_CHECK_NO_THROW( fpm.get_double_field_data("TRANX0", true));
}




BOOST_AUTO_TEST_CASE(TRAN2) {
    std::string deck_string = R"(
GRID

PORO
   1000*0.10 /

EDIT

EQUALS
  TRANX 1  1 10 1 10 1 1 /
  TRANX 2  1 10 1 10 2 2 /
  TRANX 3  1 10 1 10 3 3 /
  TRANX 4  1 10 1 10 4 4 /
  TRANX 5  1 10 1 10 5 5 /
  TRANX 6  1 10 1 10 6 6 /
  TRANX 7  1 10 1 10 7 7 /
  TRANX 8  1 10 1 10 8 8 /
  TRANX 9  1 10 1 10 9 9 /
  TRANX 10 1 10 1 10 10 10 /
  TRANY 1  1 10 1 10 1 1 /
  TRANY 2  1 10 1 10 2 2 /
  TRANY 3  1 10 1 10 3 3 /
  TRANY 4  1 10 1 10 4 4 /
  TRANY 5  1 10 1 10 5 5 /
  TRANY 6  1 10 1 10 6 6 /
  TRANY 7  1 10 1 10 7 7 /
  TRANY 8  1 10 1 10 8 8 /
  TRANY 9  1 10 1 10 9 9 /
  TRANY 10 1 10 1 10 10 10 /
  TRANZ 1  1 10 1 10 1 1 /
  TRANZ 2  1 10 1 10 2 2 /
  TRANZ 3  1 10 1 10 3 3 /
  TRANZ 4  1 10 1 10 4 4 /
  TRANZ 5  1 10 1 10 5 5 /
  TRANZ 6  1 10 1 10 6 6 /
  TRANZ 7  1 10 1 10 7 7 /
  TRANZ 8  1 10 1 10 8 8 /
  TRANZ 9  1 10 1 10 9 9 /
  TRANZ 10 1 10 1 10 10 10 /
/

MULTIPLY
  TRANX 1  1 10 1 10 1 1 /
  TRANX 2  1 10 1 10 2 2 /
  TRANX 3  1 10 1 10 3 3 /
  TRANX 4  1 10 1 10 4 4 /
  TRANX 5  1 10 1 10 5 5 /
  TRANX 6  1 10 1 10 6 6 /
  TRANX 7  1 10 1 10 7 7 /
  TRANX 8  1 10 1 10 8 8 /
  TRANX 9  1 10 1 10 9 9 /
  TRANX 10 1 10 1 10 10 10 /
  TRANZ 1  1 10 1 10 1 1 /
  TRANZ 2  1 10 1 10 2 2 /
  TRANZ 3  1 10 1 10 3 3 /
  TRANZ 4  1 10 1 10 4 4 /
  TRANZ 5  1 10 1 10 5 5 /
  TRANZ 6  1 10 1 10 6 6 /
  TRANZ 7  1 10 1 10 7 7 /
  TRANZ 8  1 10 1 10 8 8 /
  TRANZ 9  1 10 1 10 9 9 /
  TRANZ 10 1 10 1 10 10 10 /
  TRANZ 1  1 10 1 10 1 1 /
  TRANZ 2  1 10 1 10 2 2 /
  TRANZ 3  1 10 1 10 3 3 /
  TRANZ 4  1 10 1 10 4 4 /
  TRANZ 5  1 10 1 10 5 5 /
  TRANZ 6  1 10 1 10 6 6 /
  TRANZ 7  1 10 1 10 7 7 /
  TRANZ 8  1 10 1 10 8 8 /
  TRANZ 9  1 10 1 10 9 9 /
  TRANZ 10 1 10 1 10 10 10 /
/

MULTIPLY
  TRANX 1  1 10 1 10 1 1 /
  TRANX 2  1 10 1 10 2 2 /
  TRANX 3  1 10 1 10 3 3 /
  TRANX 4  1 10 1 10 4 4 /
  TRANX 5  1 10 1 10 5 5 /
  TRANX 6  1 10 1 10 6 6 /
  TRANX 7  1 10 1 10 7 7 /
  TRANX 8  1 10 1 10 8 8 /
  TRANX 9  1 10 1 10 9 9 /
  TRANX 10 1 10 1 10 10 10/
/

)";
    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::transmissibility, raw_value); };
    std::vector<int> actnum(1000, 1);
    for (std::size_t i=0; i< 1000; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,10), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    std::vector<double> tranx( grid.getNumActive() );
    std::vector<double> trany( grid.getNumActive() );
    std::vector<double> tranz( grid.getNumActive() );

    fpm.apply_tran("TRANX", tranx);
    fpm.apply_tran("TRANY", trany);
    fpm.apply_tran("TRANZ", tranz);

    for (std::size_t k=0; k < 10; k++) {
        for (std::size_t j=0; j < 10; j++) {
            for (std::size_t i=0; i < 10; i++) {
                if (grid.cellActive(i,j,k)) {
                    auto a = grid.activeIndex(i,j,k);
                    BOOST_CHECK_CLOSE( tranx[a], to_si( (k+1)*(k+1)*(k+1)) , 1e-6);
                    BOOST_CHECK_CLOSE( trany[a], to_si( k+1 ) , 1e-6);
                    BOOST_CHECK_CLOSE( tranz[a], to_si( (k+1)*(k+1)*(k+1) ), 1e-6);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TRAN_OPERATE_UNSUPPORTED) {
    std::string deck_string = R"(
GRID

PORO
   1000*0.10 /

EDIT


OPERATE
    TRANX 1  3   2  2   1   1  'MAXLIM'   PORO 0.25 /
/
)";
    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    std::vector<int> actnum(1000, 1);
    for (std::size_t i=0; i< 1000; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,10), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    BOOST_CHECK_THROW( FieldPropsManager(deck, Phases{true, true, true}, grid, TableManager()), std::logic_error );
}

BOOST_AUTO_TEST_CASE(TRAN_REGION_UNSUPPORTED) {
    std::string deck_base = R"(
GRID

PORO
   1000*0.10 /

EDIT
)";

    std::string multireg = R"(
MULTIREG
    TRANX 1.0 1 /
/
)";

    std::string equalreg = R"(
EQUALREG
    TRANX 1.0 1 /
/
)";

    std::string addreg = R"(
ADDREG
    TRANX 1.0 1 /
/
)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    std::vector<int> actnum(1000, 1);
    for (std::size_t i=0; i< 1000; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,10), actnum);

    for (const auto& region_op : { multireg, equalreg, addreg }) {
        Deck deck = Parser{}.parseString(deck_base + region_op);
        BOOST_CHECK_THROW( FieldPropsManager(deck, Phases{true, true, true}, grid, TableManager()), std::logic_error );
    }
}

BOOST_AUTO_TEST_CASE(OPERATION_VALID_KEYWORD) {
    std::string deck_string1 = R"(
GRID

PORO
   200*0.15 /

REGIONS

FIPNUM
  200*1 /

MAXVALUE
   POROX  0.25 /
/

)";


    std::string deck_string2 = R"(
GRID

PORO
   200*0.15 /

REGIONS

FIPNUM
  200*1 /

MAXVALUE
   TRANX 0.25 /
   TRANY 0.25 /
   TRANZ 0.25 /
/

)";

    std::string deck_string3 = R"(
GRID

PORO
   200*0.15 /

REGIONS

FIPXYZ
  200*1 /

MAXVALUE
   FIPXYZ 5 /
/

)";
    EclipseGrid grid(10,10, 2);
    Deck deck1 = Parser{}.parseString(deck_string1);
    Deck deck2 = Parser{}.parseString(deck_string2);
    Deck deck3 = Parser{}.parseString(deck_string3);

    // This case should throw because we refer to unsupported keyword POROX
    BOOST_CHECK_THROW( FieldPropsManager(deck1, Phases{true, true, true}, grid, TableManager()), OpmInputError );

    // This is legitimate and should not throw; however the keywords TRANX,
    // TRANY and TRANZ are not supported in the normal sense and some special
    // casing will be required.
    BOOST_CHECK_NO_THROW( FieldPropsManager(deck2, Phases{true, true, true}, grid, TableManager()));

    // This is legitimate and should not throw; however the FIPXYZ is an
    // autogenerated keyword and (maybe ??) not supported in the normal sense
    // and some special casing might be required.
    BOOST_CHECK_NO_THROW( FieldPropsManager(deck3, Phases{true, true, true}, grid, TableManager()));
}

BOOST_AUTO_TEST_CASE(REGION_OPERATION) {
    std::string deck_string1 = R"(
GRID

PORO
   200*0.15 /

PERMX
   200*1 /

REGIONS

FIPNUM
  200*1 /

MULTNUM
  50*1 50*2 100*3 /

EDIT

MULTIREG
   PERMX 1 1 M/
   PERMX 2 2 M/
   PERMX 3 3 M/
/

COPYREG
   PERMX  PERMY  1 M  /
   PERMX  PERMY  2 M  /
   PERMX  PERMY  3 M  /
/

)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::permeability, raw_value); };
    EclipseGrid grid(10,10, 2);
    Deck deck1 = Parser{}.parseString(deck_string1);
    FieldPropsManager fp(deck1, Phases{true, true, true}, grid, TableManager());
    const auto& permx = fp.get_double("PERMX");
    const auto& permy = fp.get_double("PERMY");
    const auto& multn = fp.get_int("MULTNUM");
    for (std::size_t g = 0; g < 200; g++) {
        BOOST_CHECK_CLOSE(to_si(multn[g]), permx[g], 1e-5);
        BOOST_CHECK_EQUAL(permx[g], permy[g]);
    }
}

BOOST_AUTO_TEST_CASE(OPERATE_RADIAL_PERM) {
    std::string deck_string = R"(
GRID

PORO
   6*1.0 /

OPERATE
    PORO   1  3   1  1   1   1  'MAXLIM'   PORO 0.50 /
    PORO   1  3   2  2   1   1  'MAXLIM'   PORO 0.25 /
/


PERMR
   6*1/

PERMTHT
   6*1000/

OPERATE
    PERMR   1  3   1  1   1   1  'MINLIM'   PERMR 2 /
    PERMR   1  3   2  2   1   1  'MINLIM'   PERMR 4 /
    PERMTHT   1  3   1  1   1   1  'MAXLIM'   PERMTHT 100 /
    PERMTHT   1  3   2  2   1   1  'MAXLIM'   PERMTHT 200 /
    PERMZ   1  3   1  1   1   1  'MULTA'    PERMTHT 2 1000 /
    PERMZ   1  3   2  2   1   1  'MULTA'    PERMR 3  300 /
/




)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::permeability, raw_value); };
    EclipseGrid grid(3,2,1);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, Phases{true, true, true}, grid, TableManager());
    const auto& poro = fpm.get_double("PORO");
    BOOST_CHECK_EQUAL(poro[0], 0.50);
    BOOST_CHECK_EQUAL(poro[3], 0.25);

    const auto& permx = fpm.get_double("PERMX");
    BOOST_CHECK_EQUAL(permx[0], to_si(2));
    BOOST_CHECK_EQUAL(permx[3], to_si(4));

    const auto& permy = fpm.get_double("PERMY");
    BOOST_CHECK_EQUAL(permy[0], to_si(100));
    BOOST_CHECK_EQUAL(permy[3], to_si(200));

    const auto& permz = fpm.get_double("PERMZ");
    for (std::size_t i = 0; i < 3; i++) {
        BOOST_CHECK_CLOSE(permz[i]  , 2*permy[i]   + to_si(1000), 1e-13);
        BOOST_CHECK_CLOSE(permz[i+3], 3*permx[i+3] + to_si(300), 1e-13);
    }

    BOOST_CHECK(permx == fpm.get_double("PERMR"));
    BOOST_CHECK(permy == fpm.get_double("PERMTHT"));
}

BOOST_AUTO_TEST_CASE(REGION_OPERATION_RADIAL_PERM) {
    std::string deck_string1 = R"(
GRID

PORO
   200*0.15 /

PERMR
   200*1 /

REGIONS

FIPNUM
  200*1 /

MULTNUM
  50*1 50*2 100*3 /

EDIT

MULTIREG
   PERMR 1 1 M/
   PERMR 2 2 M/
   PERMR 3 3 M/
/

COPYREG
   PERMR  PERMTHT  1 M  /
   PERMR  PERMTHT  2 M  /
   PERMR  PERMTHT  3 M  /
/

)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::permeability, raw_value); };
    EclipseGrid grid(10,10, 2);
    Deck deck1 = Parser{}.parseString(deck_string1);
    FieldPropsManager fp(deck1, Phases{true, true, true}, grid, TableManager());
    const auto& permx = fp.get_double("PERMX");
    const auto& permy = fp.get_double("PERMY");
    const auto& multn = fp.get_int("MULTNUM");
    for (std::size_t g = 0; g < 200; g++) {
        BOOST_CHECK_CLOSE(to_si(multn[g]), permx[g], 1e-5);
        BOOST_CHECK_EQUAL(permx[g], permy[g]);
    }

    BOOST_CHECK(permx == fp.get_double("PERMR"));
    BOOST_CHECK(permy == fp.get_double("PERMTHT"));
}

BOOST_AUTO_TEST_CASE(REGION_OPERATION_MIXED_PERM) {
    std::string deck_string1 = R"(
GRID

PORO
   200*0.15 /

PERMX
   200*1 /

REGIONS

FIPNUM
  200*1 /

MULTNUM
  50*1 50*2 100*3 /

EDIT

MULTIREG
   PERMR 1 1 M/
   PERMX 2 2 M/
   PERMR 3 3 M/
/

COPYREG
   PERMR  PERMTHT  1 M  /
   PERMR  PERMY  2 M  /
   PERMX  PERMY  3 M  /
/

)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    auto to_si = [&unit_system](double raw_value) { return unit_system.to_si(UnitSystem::measure::permeability, raw_value); };
    EclipseGrid grid(10,10, 2);
    Deck deck1 = Parser{}.parseString(deck_string1);
    TableManager tables;
    FieldPropsManager fp(deck1, Phases{true, true, true}, grid, tables);
    const auto& permx = fp.get_double("PERMX");
    const auto& permy = fp.get_double("PERMY");
    const auto& multn = fp.get_int("MULTNUM");
    for (std::size_t g = 0; g < 200; g++) {
        BOOST_CHECK_CLOSE(to_si(multn[g]), permx[g], 1e-5);
        BOOST_CHECK_EQUAL(permx[g], permy[g]);
    }

    BOOST_CHECK(permx == fp.get_double("PERMR"));
    BOOST_CHECK(permy == fp.get_double("PERMTHT"));

    FieldPropsManager fp2(deck1, Phases{true, true, true}, grid, tables);
    BOOST_CHECK( fp == fp2 );
    BOOST_CHECK( FieldPropsManager::rst_cmp(fp, fp2) );


    const auto& ntg = fp2.get_double("NTG");
    BOOST_CHECK( !(fp == fp2) );
    BOOST_CHECK( FieldPropsManager::rst_cmp(fp, fp2) );
    (void) ntg;

    const auto& satnum = fp.get_int("SATNUM");
    BOOST_CHECK( !(fp == fp2) );
    BOOST_CHECK( FieldPropsManager::rst_cmp(fp, fp2) );
    (void) satnum;
}

BOOST_AUTO_TEST_CASE(SCHEDULE_MULTZ) {
    std::string deck_string1 = R"(
GRID

PORO
   200*0.15 /

PERMX
   200*1 /

REGIONS

FIPNUM
  200*1 /

MULTNUM
  50*1 50*2 100*3 /


EDIT

MULTZ
  200*3 /

SCHEDULE

TSTEP
  1 /

BOX
   1 10 1 10 1 1 /


MULTZ
  100*2 /

ENDBOX

TSTEP
 1 /

BOX
   1 10 1 10 2 2 /


MULTZ
  100*4 /

ENDBOX

TSTEP
  10 /

MULTZ
  100*2 100*4/

MULTZ
  200*10 /

)";

    UnitSystem unit_system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    EclipseGrid grid(10,10, 2);
    Deck deck = Parser{}.parseString(deck_string1);
    TableManager tables;
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, tables);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    Schedule sched(deck, grid, fp, runspec, python);
    const auto& multz0 = fp.get_double("MULTZ");
    for (std::size_t k=0; k < 2; k++) {
        for (std::size_t ij=0; ij < 100; ij++)
            BOOST_CHECK_EQUAL(multz0[ij + k*100], 3.0);
    }

    // Observe that the MULTZ multiplier is reset to 1.0 for every timestep

    const auto& sched1 = sched[1];
    fp.apply_schedule_keywords(sched1.geo_keywords());
    const auto& multz1 = fp.get_double("MULTZ");
    for (std::size_t ij=0; ij < 100; ij++) {
        BOOST_CHECK_EQUAL(multz1[ij]      , 2.0);
        BOOST_CHECK_EQUAL(multz1[ij + 100], 1.0);
    }


    const auto& sched2 = sched[2];
    fp.apply_schedule_keywords(sched2.geo_keywords());
    const auto& multz2 = fp.get_double("MULTZ");
    for (std::size_t ij=0; ij < 100; ij++) {
        BOOST_CHECK_EQUAL(multz2[ij]      , 1.0);
        BOOST_CHECK_EQUAL(multz2[ij + 100], 4.0);
    }


    const auto& sched3 = sched[3];
    fp.apply_schedule_keywords(sched3.geo_keywords());
    for (std::size_t ij=0; ij < 100; ij++) {
        BOOST_CHECK_EQUAL(multz2[ij]      , 20.0);
        BOOST_CHECK_EQUAL(multz2[ij + 100], 40.0);
    }
}
