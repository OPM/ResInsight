/*
  Copyright 2015 IRIS

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

#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/Grid/GridDims.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#define BOOST_TEST_MODULE NNCTests

#include <boost/test/unit_test.hpp>

using namespace Opm;


const std::string no_nnc_input = R"(
RUNSPEC

OIL
GAS
WATER


DIMENS
   10 10  1  /

GRID

DXV
10*1000.0
/

DYV
10*1000.0
/

DZ
100*20.0
/

TOPS
100*10
/

PORO
  100*0.15 /
)";

const std::string actnum = R"(
RUNSPEC

OIL
GAS
WATER


DIMENS
   10 10  1  /

GRID

DXV
10*1000.0
/

DYV
10*1000.0
/

DZ
100*20.0
/

TOPS
100*10
/

ACTNUM
10*0 90*1
/

NNC
1 1 1 1 3 1 0.5 /  -- Inactive cell
3 1 1 3 3 1 0.5 /  -- Inactive cell
2 2 1 3 3 1 1.0 /  -- Valid
100 100 100 200 200 200 1.0 / -- Very invalid
/

PORO
  100*0.15 /

EDIT

EDITNNC
5 1 1 1 5 3 0.5 /   -- Inactive cell
2 2 1 3 3 1 0.5 /   -- Valid - coupled to NNC
4 4 1 5 5 1 2.0 /   -- Valid
-1 4 4 -1 7 7 1.0 / -- Very invalid
/

)";




const std::string nnc_input = R"(
RUNSPEC

OIL
GAS
WATER


DIMENS
   10 10  1  /

GRID

DXV
10*1000.0
/

DYV
10*1000.0
/

DZ
100*20.0
/

TOPS
100*10
/

PORO
   100*0.15 /

NNC
1 1 1 2 1 1 0.5 /
1 1 1 1 2 1 1.0 /
/

NNC
1 1 1 2 1 1 0.5 /
1 2 1 1 2 1 1.0 /
/
)";



const std::string editnnc_input = R"(
RUNSPEC

OIL
GAS
WATER


DIMENS
   10 10  1  /

GRID

NNC
   7 1 1 3 1 1 0.1 /
   3 1 1 5 1 1 0.1 /
/


DXV
10*1000.0
/

DYV
10*1000.0
/

DZ
100*20.0
/

TOPS
100*10
/

PORO
   100*0.15 /

EDIT


EDITNNC
5 1 1 3 1 1 2.0 /
3 1 1 1 1 1 0.1 /
1 1 1 1 2 1 0.01 / -- This is ignored because the two cells are ijk neighbours
2 1 1 2 3 1 0.2 /
/

EDITNNC
1 1 1 2 1 1 0.1 /  -- Ignored
2 1 1 2 3 1 0.3 /
/
)";


std::optional<NNCdata> find_nnc(const std::vector<NNCdata>& v, std::size_t c1, std::size_t c2) {
    if (c1 > c2)
        return find_nnc(v, c2, c1);

    auto iter = std::find_if(v.begin(), v.end(), [c1,c2](const NNCdata& nnc) { return nnc.cell1 == c1 && nnc.cell2 == c2; });
    if (iter != v.end())
        return *iter;

    return {};
}

void check_edit_nnc(const std::vector<NNCdata>& v, std::size_t c1, std::size_t c2, double t) {
    const auto& nnc = find_nnc(v, c1, c2);
    BOOST_REQUIRE(nnc.has_value());
    BOOST_CHECK_CLOSE(nnc->trans, t , 1e-6);
}

void check_nnc(const std::vector<NNCdata>& v, std::size_t c1, std::size_t c2, double t) {
    check_edit_nnc(v, c1, c2, t*Opm::Metric::Transmissibility);
}

bool has_nnc(const std::vector<NNCdata>& v, std::size_t c1, std::size_t c2) {
    const auto& nnc = find_nnc(v, c1, c2);
    return nnc.has_value();
}

void check_order(const std::vector<NNCdata>& d) {
    for (const auto& nnc : d)
        BOOST_CHECK(nnc.cell1 <= nnc.cell2);

    if (d.size() <= 1)
        return;

    for (std::size_t index=1; index < d.size(); index++) {
        const auto& nnc1 = d[index - 1];
        const auto& nnc2 = d[index];

        if (nnc1 < nnc2)
            continue;

        BOOST_CHECK_EQUAL(nnc1.cell1, nnc2.cell1);
        BOOST_CHECK_EQUAL(nnc1.cell2, nnc2.cell2);
    }
}


void check_order(const NNC& nnc) {
    check_order(nnc.input());
    check_order(nnc.edit());
}



BOOST_AUTO_TEST_CASE(noNNC)
{
    Parser parser;
    auto deck = parser.parseString(no_nnc_input);
    EclipseState eclipseState(deck);
    const auto& nnc = eclipseState.getInputNNC();
    check_order(nnc);
    BOOST_CHECK(!eclipseState.hasInputNNC());
    BOOST_CHECK(nnc.input().empty());
}

BOOST_AUTO_TEST_CASE(readDeck)
{
    Parser parser;
    auto deck = parser.parseString(nnc_input);
    EclipseState eclipseState(deck);
    const auto& grid = eclipseState.getInputGrid();
    const auto& nnc = eclipseState.getInputNNC();
    check_order(nnc);
    BOOST_CHECK(!nnc.input().empty());
    const std::vector<NNCdata>& nncdata = nnc.input();

    // test the NNCs in nnc.DATA
    BOOST_CHECK_EQUAL(nncdata.size(), 4);
    check_nnc(nncdata, grid.getGlobalIndex(0,0,0), grid.getGlobalIndex(1,0,0), 0.50);
    check_nnc(nncdata, grid.getGlobalIndex(0,0,0), grid.getGlobalIndex(0,1,0), 1.00);

    const auto& loc = nnc.input_location( nncdata[0] );
    BOOST_CHECK_EQUAL(loc.keyword, "NNC");
}



BOOST_AUTO_TEST_CASE(noNNC_EDIT)
{
    Parser parser;
    auto deck = parser.parseString(no_nnc_input);
    EclipseState eclipseState(deck);
    const auto& editnnc = eclipseState.getInputNNC();
    BOOST_CHECK(editnnc.edit().empty());
}


BOOST_AUTO_TEST_CASE(readDeck_EDIT)
{
    Parser parser;
    auto deck = parser.parseString(editnnc_input);
    EclipseGrid grid(10,10,10);

    NNC nnc(grid, deck);
    const std::vector<NNCdata>& data = nnc.edit();

    BOOST_CHECK_EQUAL(data.size(), 2); //neighbouring connections in EDITNNC are ignored
    BOOST_CHECK(!has_nnc(data, grid.getGlobalIndex(0,0,0), grid.getGlobalIndex(0,1,0)));
    BOOST_CHECK(!has_nnc(data, grid.getGlobalIndex(0,0,0), grid.getGlobalIndex(1,0,0)));

    check_order(nnc);
    check_edit_nnc(data, grid.getGlobalIndex(2,0,0), grid.getGlobalIndex(0,0,0), 0.1);
    check_edit_nnc(data, grid.getGlobalIndex(1,0,0), grid.getGlobalIndex(1,2,0), 0.06);

    const std::vector<NNCdata>& input = nnc.input();
    check_nnc(input, grid.getGlobalIndex(2,0,0), grid.getGlobalIndex(4,0,0), 0.20);
    check_nnc(input, grid.getGlobalIndex(2,0,0), grid.getGlobalIndex(6,0,0), 0.10);
}


BOOST_AUTO_TEST_CASE(ACTNUM)
{
    Parser parser;
    auto deck = parser.parseString(actnum);
    EclipseState eclipseState(deck);
    const auto& grid = eclipseState.getInputGrid();
    const auto& editnnc = eclipseState.getInputNNC();
    const auto& edit = editnnc.edit();
    const auto& input = editnnc.input();
    BOOST_CHECK_EQUAL(edit.size(), 1);
    BOOST_CHECK_EQUAL(input.size(), 1);
    check_nnc(input, grid.getGlobalIndex(1,1,0), grid.getGlobalIndex(2,2,0), 0.5);
    check_edit_nnc(edit, grid.getGlobalIndex(3,3,0), grid.getGlobalIndex(4,4,0), 2.0);
    check_order(editnnc);
}


