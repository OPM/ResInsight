/*
  Copyright (C) 2015 Statoil ASA

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANREGILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <opm/input/eclipse/EclipseState/Tables/Regdims.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

Regdims::Regdims() :
    m_NTFIP( ParserKeywords::REGDIMS::NTFIP::defaultValue ),
    m_NMFIPR( ParserKeywords::REGDIMS::NMFIPR::defaultValue ),
    m_NRFREG( ParserKeywords::REGDIMS::NRFREG::defaultValue ),
    m_NTFREG( ParserKeywords::REGDIMS::NTFREG::defaultValue ),
    m_NPLMIX( ParserKeywords::REGDIMS::NPLMIX::defaultValue )
{ }

Regdims::Regdims(const Deck& deck) :
    Regdims()
{
    if (deck.hasKeyword("REGDIMS")) {
        const auto& record = deck["REGDIMS"][0].getRecord( 0 );
        m_NTFIP   = record.getItem("NTFIP").get<int>(0);
        m_NMFIPR  = record.getItem("NMFIPR").get<int>(0);
        m_NRFREG  = record.getItem("NRFREG").get<int>(0);
        m_NTFREG  = record.getItem("NTFREG").get<int>(0);
        m_NPLMIX  = record.getItem("NPLMIX").get<int>(0);
    }
}

}
