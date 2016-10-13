/*
  Copyright 2014 Andreas Lauser

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
#include "checkDeck.hpp"

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

namespace Opm {
bool checkDeck(DeckConstPtr deck, ParserConstPtr parser, size_t enabledChecks) {
    bool deckValid = true;

    // make sure that the deck does not contain unknown keywords
    if (enabledChecks & UnknownKeywords) {
        size_t keywordIdx = 0;
        for (; keywordIdx < deck->size(); keywordIdx++) {
            const auto& keyword = deck->getKeyword(keywordIdx);
            if (!parser->isRecognizedKeyword( keyword.name() ) ) {
                std::string msg("Keyword '" + keyword.name() + "' is unknown.");
                deck->getMessageContainer().warning(msg, keyword.getFileName(), keyword.getLineNumber());
                deckValid = false;
            }
        }
    }

    // make sure all mandatory sections are present and that their order is correct
    if (enabledChecks & SectionTopology) {
        bool ensureKeywordSection = enabledChecks & KeywordSection;
        deckValid = deckValid && Section::checkSectionTopology(*deck, *parser, ensureKeywordSection);
    }

    return deckValid;
}
}
