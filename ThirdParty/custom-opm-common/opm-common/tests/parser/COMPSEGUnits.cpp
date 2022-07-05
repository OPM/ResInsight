/*
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

#define BOOST_TEST_MODULE COMPSEGUNITS

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>

using namespace Opm;

inline Deck createCOMPSEGSDeck() {
    const char *deckData =
        "COMPSEGS\n"
        " WELL /\n"
        " 1 1 1 1  100  100  X  10/\n"
        "/\n";

    Parser parser;
    return parser.parseString(deckData);
}



BOOST_AUTO_TEST_CASE(CreateDimension) {
    auto deck = createCOMPSEGSDeck();
    const auto& keyword = deck.get<ParserKeywords::COMPSEGS>().back();
    const auto& record = keyword.getRecord(1);
    BOOST_CHECK_NO_THROW( record.getItem<ParserKeywords::COMPSEGS::DISTANCE_START>().getSIDouble(0) );
}



