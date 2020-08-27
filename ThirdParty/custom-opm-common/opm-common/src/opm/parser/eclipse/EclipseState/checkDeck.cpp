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
#include <opm/parser/eclipse/EclipseState/checkDeck.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/common/utility/String.hpp>

namespace Opm {
bool checkDeck( const Deck& deck, const Parser& parser, const ParseContext& parseContext, ErrorGuard& errorGuard, size_t enabledChecks) {
    bool deckValid = true;

    // make sure that the deck does not contain unknown keywords
    if (enabledChecks & UnknownKeywords) {
        size_t keywordIdx = 0;
        for (; keywordIdx < deck.size(); keywordIdx++) {
            const auto& keyword = deck.getKeyword(keywordIdx);
            if (!parser.isRecognizedKeyword( keyword.name() ) ) {
                std::string msg("Keyword '" + keyword.name() + "' is unknown.");
                const auto& location = keyword.location();
                OpmLog::warning( Log::fileMessage(location, msg) );
                deckValid = false;
            }
        }
    }

    // make sure all mandatory sections are present and that their order is correct
    if (enabledChecks & SectionTopology) {
        bool ensureKeywordSection = enabledChecks & KeywordSection;
        deckValid = deckValid && DeckSection::checkSectionTopology(deck, parser, ensureKeywordSection);
    }

    const std::string& deckUnitSystem = uppercase(deck.getActiveUnitSystem().getName());
    for (const auto& keyword : deck.getKeywordList("FILEUNIT")) {
        const std::string& fileUnitSystem = uppercase(keyword->getRecord(0).getItem("FILE_UNIT_SYSTEM").getTrimmedString(0));
        if (fileUnitSystem != deckUnitSystem) {
            const auto& location = keyword->location();
            std::string msg =
                "Unit system " + fileUnitSystem + " specified via the FILEUNIT keyword at "
                + location.filename + ":" + std::to_string(location.lineno)
                + " does not correspond to the unit system used by the deck ("
                + deckUnitSystem + ")";
            parseContext.handleError(ParseContext::UNIT_SYSTEM_MISMATCH, msg, errorGuard);
            deckValid = false;
        }
    }

    return deckValid;
}
}
