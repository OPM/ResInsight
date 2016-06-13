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

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>

using namespace Opm;

std::shared_ptr<const Deck> createCOMPSEGSDeck() {
    const char *deckData =
        "COMPSEGS\n"
        " WELL /\n"
        " 1 1 1 1  100  100  X  10/\n"
        "/\n";

    Parser parser;
    DeckConstPtr deck(parser.parseString(deckData, ParseContext()));
    return deck;
}



BOOST_AUTO_TEST_CASE(CreateDimension) {
    DeckConstPtr deck = createCOMPSEGSDeck();
    const auto& keyword = deck->getKeyword<ParserKeywords::COMPSEGS>();
    const auto& record = keyword.getRecord(1);
    BOOST_CHECK_NO_THROW( record.getItem<ParserKeywords::COMPSEGS::DISTANCE_START>().getSIDouble(0) );
}



