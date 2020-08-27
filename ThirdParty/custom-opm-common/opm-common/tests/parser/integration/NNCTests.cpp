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

#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#define BOOST_TEST_MODULE NNCTests

#include <boost/test/unit_test.hpp>

using namespace Opm;

inline std::string pathprefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE(noNNC)
{
    Parser parser;
    auto deck = parser.parseFile(pathprefix() + "NNC/noNNC.DATA");
    EclipseState eclipseState(deck);
    const auto& nnc = eclipseState.getInputNNC();
    BOOST_CHECK(!eclipseState.hasInputNNC());
    BOOST_CHECK(!nnc.hasNNC());
}

BOOST_AUTO_TEST_CASE(readDeck)
{
    Parser parser;
    auto deck = parser.parseFile(pathprefix() + "NNC/NNC.DATA");
    EclipseState eclipseState(deck);
    const auto& nnc = eclipseState.getInputNNC();
    BOOST_CHECK(nnc.hasNNC());
    const std::vector<NNCdata>& nncdata = nnc.data();

    // test the NNCs in nnc.DATA
    BOOST_CHECK_EQUAL(nnc.numNNC(), 4);
    BOOST_CHECK_EQUAL(nncdata[0].cell1, 0);
    BOOST_CHECK_EQUAL(nncdata[0].cell2, 1);
    BOOST_CHECK_EQUAL(nncdata[0].trans, 0.5 * Opm::Metric::Transmissibility);
    BOOST_CHECK_EQUAL(nncdata[1].cell1, 0);
    BOOST_CHECK_EQUAL(nncdata[1].cell2, 10);
    BOOST_CHECK_EQUAL(nncdata[1].trans, 1.0 * Opm::Metric::Transmissibility);

}

BOOST_AUTO_TEST_CASE(addNNCfromDeck)
{
    Parser parser;
    auto deck = parser.parseFile(pathprefix() + "NNC/NNC.DATA");
    EclipseState eclipseState(deck);
    auto nnc = eclipseState.getInputNNC();
    BOOST_CHECK(nnc.hasNNC());
    const std::vector<NNCdata>& nncdata = nnc.data();

    BOOST_CHECK_EQUAL(nnc.numNNC(), 4);
    // test add NNC
    nnc.addNNC(2, 2, 2.0);
    BOOST_CHECK_EQUAL(nnc.numNNC(), 5);
    BOOST_CHECK_EQUAL(nncdata[4].cell1, 2);
    BOOST_CHECK_EQUAL(nncdata[4].cell2, 2);
    BOOST_CHECK_EQUAL(nncdata[4].trans, 2.0);
}

BOOST_AUTO_TEST_CASE(addNNC)
{
    Opm::NNC nnc;
    // add NNC
    nnc.addNNC(2,2,2.0);
    const std::vector<NNCdata>& nncdata = nnc.data();
    BOOST_CHECK_EQUAL(nnc.numNNC(), 1);
    BOOST_CHECK_EQUAL(nncdata[0].cell1, 2);
    BOOST_CHECK_EQUAL(nncdata[0].cell1, 2);
    BOOST_CHECK_EQUAL(nncdata[0].trans, 2.0);
}

