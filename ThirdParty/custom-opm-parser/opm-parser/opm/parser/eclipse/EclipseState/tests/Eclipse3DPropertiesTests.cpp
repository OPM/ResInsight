/*
 Copyright 2016 Statoil ASA.

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

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE Eclipse3DPropertiesTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/Units/ConversionFactors.hpp>

static Opm::DeckPtr createDeck() {
    const char *deckData = "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 10 10 10 /\n"
            "GRID\n"
            "FAULTS \n"
            "  'F1'  1  1  1  4   1  4  'X' / \n"
            "  'F2'  5  5  1  4   1  4  'X-' / \n"
            "/\n"
            "MULTFLT \n"
            "  'F1' 0.50 / \n"
            "  'F2' 0.50 / \n"
            "/\n"
            "EDIT\n"
            "MULTFLT /\n"
            "  'F2' 0.25 / \n"
            "/\n"
            "OIL\n"
            "\n"
            "GAS\n"
            "\n"
            "PROPS\n"
            "REGIONS\n"
            "swat\n"
            "1000*1 /\n"
            "SATNUM\n"
            "1000*2 /\n"
            "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}

static Opm::DeckPtr createValidIntDeck() {
    const char *deckData = "RUNSPEC\n"
            "GRIDOPTS\n"
            "  'YES'  2 /\n"
            "\n"
            "DIMENS\n"
            " 5 5 1 /\n"
            "GRID\n"
            "MULTNUM \n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "/\n"
            "SATNUM\n"
            " 25*1 \n"
            "/\n"
            "ADDREG\n"
            "  satnum 11 1    M / \n"
            "  SATNUM 20 2      / \n"
            "/\n"
            "EDIT\n"
            "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}

static Opm::DeckPtr createValidPERMXDeck() {
    const char *deckData = "RUNSPEC\n"
            "GRIDOPTS\n"
            "  'YES'  2 /\n"
            "\n"
            "DIMENS\n"
            " 5 5 1 /\n"
            "GRID\n"
            "MULTNUM \n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "1  1  2  2 2\n"
            "/\n"
            "BOX\n"
            "  1 2  1 5 1 1 / \n"
            "PERMZ\n"
            "  10*1 /\n"
            "ENDBOX\n"
            "BOX\n"
            "  3 5  1 5 1 1 / \n"
            "PERMZ\n"
            "  15*2 /\n"
            "ENDBOX\n"
            "PERMX\n"
            "25*1 /\n"
            "ADDREG\n"
            "  PERMX 1 1     / \n"
            "  PERMX 3 2     / \n"
            "/\n"
            "EDIT\n"
            "\n";

    Opm::ParserPtr parser(new Opm::Parser());
    return parser->parseString(deckData, Opm::ParseContext());
}


/// Setup fixture
struct Setup
{
    Opm::ParseContext parseContext;
    Opm::DeckPtr deck;
    Opm::TableManager tablemanager;
    Opm::EclipseGrid grid;
    Opm::Eclipse3DProperties props;

    explicit Setup(Opm::DeckPtr deckArg) :
            deck(deckArg),
            tablemanager(*deck),
            grid(deck),
            props(*deck, tablemanager, grid)
    {
    }
};


BOOST_AUTO_TEST_CASE(HasDeckProperty) {
    Setup s(createDeck());
    BOOST_CHECK(s.props.hasDeckIntGridProperty("SATNUM"));
}

BOOST_AUTO_TEST_CASE(SupportsProperty) {
    Setup s(createDeck());
    std::vector<std::string> keywordList = {
        // int props
        "ACTNUM", "SATNUM", "IMBNUM", "PVTNUM", "EQLNUM", "ENDNUM", "FLUXNUM", "MULTNUM", "FIPNUM", "MISCNUM", "OPERNUM",
        // double props
        "TEMPI", "MULTPV", "PERMX", "permy", "PERMZ", "SWATINIT", "THCONR", "NTG"
    };

    for (auto keyword : keywordList)
        BOOST_CHECK(s.props.supportsGridProperty(keyword));

}

BOOST_AUTO_TEST_CASE(DefaultRegionFluxnum) {
    Setup s(createDeck());
    BOOST_CHECK_EQUAL(s.props.getDefaultRegionKeyword(), "FLUXNUM");
}

BOOST_AUTO_TEST_CASE(UnsupportedKeywordsThrows) {
    Setup s(createDeck());
    BOOST_CHECK_THROW(s.props.hasDeckIntGridProperty("NONO"), std::logic_error);
    BOOST_CHECK_THROW(s.props.hasDeckDoubleGridProperty("NONO"), std::logic_error);

    BOOST_CHECK_THROW(s.props.getIntGridProperty("NONO"), std::logic_error);
    BOOST_CHECK_THROW(s.props.getDoubleGridProperty("NONO"), std::logic_error);

    BOOST_CHECK_NO_THROW(s.props.hasDeckIntGridProperty("FluxNUM"));
    BOOST_CHECK_NO_THROW(s.props.supportsGridProperty("NONO"));
}

BOOST_AUTO_TEST_CASE(IntGridProperty) {
    Setup s(createDeck());
    int cnt = 0;
    for (auto x : s.props.getIntGridProperty("SaTNuM").getData()) {
        BOOST_CHECK_EQUAL(x, 2);
        cnt++;
    }
    BOOST_CHECK_EQUAL(cnt, 1000);
}

BOOST_AUTO_TEST_CASE(AddregIntSetCorrectly) {
    Opm::DeckPtr deck = createValidIntDeck();
    Setup s(deck);

    const auto& property = s.props.getIntGridProperty("SATNUM");
    for (size_t j = 0; j < 5; j++)
        for (size_t i = 0; i < 5; i++) {
            if (i < 2)
                BOOST_CHECK_EQUAL(12, property.iget(i, j, 0));
            else
                BOOST_CHECK_EQUAL(21, property.iget(i, j, 0));
        }

}

BOOST_AUTO_TEST_CASE(PermxUnitAppliedCorrectly) {
    Opm::DeckPtr deck = createValidPERMXDeck();
    Setup s(deck);
    const auto& permx = s.props.getDoubleGridProperty("PermX");

    for (size_t j = 0; j < 5; j++)
        for (size_t i = 0; i < 5; i++) {
            if (i < 2)
                BOOST_CHECK_CLOSE(2 * Opm::Metric::Permeability, permx.iget(i, j, 0), 0.0001);
            else
                BOOST_CHECK_CLOSE(4 * Opm::Metric::Permeability, permx.iget(i, j, 0), 0.0001);
        }
}

BOOST_AUTO_TEST_CASE(getRegions) {
    const char* input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 2 2 1 /\n"
            "GRID\n"
            "DXV \n 2*400 /\n"
            "DYV \n 2*400 /\n"
            "DZV \n 1*400 /\n"
            "REGIONS\n"
            "FIPNUM\n"
            "1 1 2 3 /\n";

    auto deck = Opm::Parser().parseString(input, Opm::ParseContext());
    Setup s( deck );

    std::vector< int > ref = { 1, 2, 3 };
    const auto regions = s.props.getRegions( "FIPNUM" );

    BOOST_CHECK_EQUAL_COLLECTIONS( ref.begin(), ref.end(),
                                   regions.begin(), regions.end() );

    BOOST_CHECK( s.props.getRegions( "EQLNUM" ).empty() );
}
