/*
  Copyright (C) 2015 Statoil ASA

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

#include <opm/input/eclipse/EclipseState/Tables/Tabdims.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

Tabdims::Tabdims() :
    m_ntsfun( ParserKeywords::TABDIMS::NTSFUN::defaultValue ),
    m_ntpvt( ParserKeywords::TABDIMS::NTPVT::defaultValue ),
    m_nssfun( ParserKeywords::TABDIMS::NSSFUN::defaultValue ),
    m_nppvt(  ParserKeywords::TABDIMS::NPPVT::defaultValue ),
    m_ntfip(  ParserKeywords::TABDIMS::NTFIP::defaultValue ),
    m_nrpvt(  ParserKeywords::TABDIMS::NRPVT::defaultValue )
{ }


Tabdims::Tabdims(const Deck& deck) :
    Tabdims()
{
    if (deck.hasKeyword("TABDIMS")) {
        const auto& record = deck["TABDIMS"][0].getRecord(0);
        m_ntsfun = record.getItem("NTSFUN").get<int>(0);
        m_ntpvt  = record.getItem("NTPVT").get<int>(0);
        m_nssfun = record.getItem("NSSFUN").get<int>(0);
        m_nppvt  = record.getItem("NPPVT").get<int>(0);
        m_ntfip  = record.getItem("NTFIP").get<int>(0);
        m_nrpvt  = record.getItem("NRPVT").get<int>(0);
    }
}

}
