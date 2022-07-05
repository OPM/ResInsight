/*
  Copyright 2019 SINTEF Digital, Mathematics and Cybernetics.
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE FoamTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/InitConfig/FoamConfig.hpp>


using namespace Opm;


static Deck createDeck() {
    // Using a raw string literal with xxx as delimiter.
    const char *deckData = R"xxx(
RUNSPEC

DIMENS
 10 10 10 /
TABDIMS
3 /
GRID
DX
1000*0.25 /
DY
1000*0.25 /
DZ
1000*0.25 /
TOPS
100*0.25 /
FAULTS
  'F1'  1  1  1  4   1  4  'X' /
  'F2'  5  5  1  4   1  4  'X-' /
/
MULTFLT
  'F1' 0.50 /
  'F2' 0.50 /
/
PORO
   1000*0.15/
EDIT
MULTFLT /
  'F2' 0.25 /
/
WATER

OIL

GAS

FOAM

TITLE
The title

START
8 MAR 1998 /

PROPS

FOAMOPTS
GAS TAB /

FOAMROCK
1 2000 /
2 1800 /
2 2400 /

FOAMFSC
1 2 0.3 /
4 5 /
6 /


REGIONS
SWAT
1000*1 /
SATNUM
1000*2 /
)xxx"; // End of raw string literal with xxx as delimiter.
    Parser parser;
    return parser.parseString( deckData );
}

static Deck createFailingDeck() {
    // Using a raw string literal with xxx as delimiter.
    const char *deckData = R"xxx(
RUNSPEC

DIMENS
 10 10 10 /
TABDIMS
3 /
GRID
DX
1000*0.25 /
DY
1000*0.25 /
DZ
1000*0.25 /
TOPS
100*0.25 /
FAULTS
  'F1'  1  1  1  4   1  4  'X' /
  'F2'  5  5  1  4   1  4  'X-' /
/
MULTFLT
  'F1' 0.50 /
  'F2' 0.50 /
/
PORO
   1000*0.15/
EDIT
MULTFLT /
  'F2' 0.25 /
/
WATER

OIL

GAS

FOAM

TITLE
The title

START
8 MAR 1998 /

PROPS
FOAMFSC
1 2 0.3 /
4 5 /
6 /

-- This will fail, as FOAMROCK is missing

REGIONS
SWAT
1000*1 /
SATNUM
1000*2 /
)xxx"; // End of raw string literal with xxx as delimiter.
    Parser parser;
    return parser.parseString( deckData );
}




BOOST_AUTO_TEST_CASE(FoamConfigTest) {
    auto deck = createDeck();
    EclipseState state(deck);
    const FoamConfig& fc = state.getInitConfig().getFoamConfig();
    BOOST_REQUIRE_EQUAL(fc.size(), 3U);
    BOOST_CHECK_EQUAL(fc.getRecord(0).referenceSurfactantConcentration(), 1.0);
    BOOST_CHECK_EQUAL(fc.getRecord(0).exponent(), 2.0);
    BOOST_CHECK_EQUAL(fc.getRecord(0).minimumSurfactantConcentration(), 0.3);
    BOOST_CHECK(fc.getRecord(0).allowDesorption());
    BOOST_CHECK_EQUAL(fc.getRecord(0).rockDensity(), 2000.0);

    BOOST_CHECK_EQUAL(fc.getRecord(1).referenceSurfactantConcentration(), 4.0);
    BOOST_CHECK_EQUAL(fc.getRecord(1).exponent(), 5.0);
    BOOST_CHECK_EQUAL(fc.getRecord(1).minimumSurfactantConcentration(), 1e-20); // Defaulted.
    BOOST_CHECK(!fc.getRecord(1).allowDesorption());
    BOOST_CHECK_EQUAL(fc.getRecord(1).rockDensity(), 1800.0);

    BOOST_CHECK_EQUAL(fc.getRecord(2).referenceSurfactantConcentration(), 6.0);
    BOOST_CHECK_EQUAL(fc.getRecord(2).exponent(), 1.0); // Defaulted.
    BOOST_CHECK_EQUAL(fc.getRecord(2).minimumSurfactantConcentration(), 1e-20); // Defaulted.
    BOOST_CHECK(!fc.getRecord(2).allowDesorption());
    BOOST_CHECK_EQUAL(fc.getRecord(2).rockDensity(), 2400.0);
}

BOOST_AUTO_TEST_CASE(FoamConfigFailureTest) {
    BOOST_CHECK_THROW( createFailingDeck(), std::exception );
}

