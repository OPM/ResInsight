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
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif
#define NVERBOSE // to suppress our messages when throwing

#define BOOST_TEST_MODULE NNCTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

using namespace Opm;

BOOST_AUTO_TEST_CASE(noNNC)
{
    Opm::ParseContext parseContext;
    const std::string filename = "testdata/integration_tests/NNC/noNNC.DATA";
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck(parser->parseFile(filename, parseContext));
    Opm::EclipseStateConstPtr eclipseState(new EclipseState(deck , parseContext));
    auto eclGrid = eclipseState->getInputGrid();
    Opm::NNC nnc(deck, eclGrid);
    BOOST_CHECK(!nnc.hasNNC());
}

BOOST_AUTO_TEST_CASE(readDeck)
{
    Opm::ParseContext parseContext;
    const std::string filename = "testdata/integration_tests/NNC/NNC.DATA";
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck(parser->parseFile(filename, parseContext));
    Opm::EclipseStateConstPtr eclipseState(new EclipseState(deck , parseContext));
    auto eclGrid = eclipseState->getInputGrid();
    Opm::NNC nnc(deck, eclGrid);
    BOOST_CHECK(nnc.hasNNC());
    const std::vector<NNCdata>& nncdata = nnc.nncdata();

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
    Opm::ParseContext parseContext;
    const std::string filename = "testdata/integration_tests/NNC/NNC.DATA";
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck(parser->parseFile(filename, parseContext));
    Opm::EclipseStateConstPtr eclipseState(new EclipseState(deck , parseContext));
    auto eclGrid = eclipseState->getInputGrid();
    Opm::NNC nnc(deck, eclGrid);
    const std::vector<NNCdata>& nncdata = nnc.nncdata();

    // test add NNC
    nnc.addNNC(2,2,2.0);
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
    const std::vector<NNCdata>& nncdata = nnc.nncdata();
    BOOST_CHECK_EQUAL(nnc.numNNC(), 1);
    BOOST_CHECK_EQUAL(nncdata[0].cell1, 2);
    BOOST_CHECK_EQUAL(nncdata[0].cell1, 2);
    BOOST_CHECK_EQUAL(nncdata[0].trans, 2.0);
}

