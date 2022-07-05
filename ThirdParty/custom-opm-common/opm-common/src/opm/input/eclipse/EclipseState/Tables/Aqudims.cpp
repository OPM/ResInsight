/*
  Copyright (C) 2017 TNO

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

#include <opm/input/eclipse/EclipseState/Tables/Aqudims.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

Aqudims::Aqudims() :
    m_mxnaqn( ParserKeywords::AQUDIMS::MXNAQN::defaultValue ),
    m_mxnaqc( ParserKeywords::AQUDIMS::MXNAQC::defaultValue ),
    m_niftbl( ParserKeywords::AQUDIMS::NIFTBL::defaultValue ),
    m_nriftb( ParserKeywords::AQUDIMS::NRIFTB::defaultValue ),
    m_nanaqu( ParserKeywords::AQUDIMS::NANAQU::defaultValue ),
    m_ncamax( ParserKeywords::AQUDIMS::NCAMAX::defaultValue ),
    m_mxnali( ParserKeywords::AQUDIMS::MXNALI::defaultValue ),
    m_mxaaql( ParserKeywords::AQUDIMS::MXAAQL::defaultValue )

{ }

Aqudims::Aqudims(const Deck& deck) :
    Aqudims()
{
    if (deck.hasKeyword("AQUDIMS")) {
        const auto& record = deck[ "AQUDIMS" ][0].getRecord( 0 );
        m_mxnaqn  = record.getItem("MXNAQN").get<int>(0);
        m_mxnaqc  = record.getItem("MXNAQC").get<int>(0);
        m_niftbl  = record.getItem("NIFTBL").get<int>(0);
        m_nriftb  = record.getItem("NRIFTB").get<int>(0);
        m_nanaqu  = record.getItem("NANAQU").get<int>(0);
        m_ncamax  = record.getItem("NCAMAX").get<int>(0);
        m_mxnali  = record.getItem("MXNALI").get<int>(0);
        m_mxaaql  = record.getItem("MXAAQL").get<int>(0);
    }
}

}
