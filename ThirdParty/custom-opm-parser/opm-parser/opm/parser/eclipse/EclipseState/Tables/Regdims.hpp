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

#ifndef REGDIMS_HPP
#define REGDIMS_HPP

/*
  The Regdims class is a small utility class designed to hold on to
  the values from the REGDIMS keyword.
*/

#include <opm/parser/eclipse/Parser/ParserKeywords/R.hpp>

namespace Opm {
    class Regdims {
    public:

        Regdims() :
            m_NTFIP( ParserKeywords::REGDIMS::NTFIP::defaultValue ),
            m_NMFIPR( ParserKeywords::REGDIMS::NMFIPR::defaultValue ),
            m_NRFREG( ParserKeywords::REGDIMS::NRFREG::defaultValue ),
            m_NTFREG( ParserKeywords::REGDIMS::NTFREG::defaultValue ),
            m_NPLMIX( ParserKeywords::REGDIMS::NPLMIX::defaultValue )
        { }

        Regdims(size_t ntfip , size_t nmfipr , size_t nrfregr , size_t ntfreg , size_t nplmix) :
            m_NTFIP( ntfip ),
            m_NMFIPR( nmfipr ),
            m_NRFREG( nrfregr ),
            m_NTFREG(  ntfreg ),
            m_NPLMIX( nplmix )
        {}

        size_t getNTFIP() const {
            return m_NTFIP;
        }


        size_t getNMFIPR() const {
            return m_NMFIPR;
        }


        size_t getNRFREG() const {
            return m_NRFREG;
        }


        size_t getNTFREG() const {
            return m_NTFREG;
        }


        size_t getNPLMIX() const {
            return m_NPLMIX;
        }




    private:
        size_t m_NTFIP;
        size_t m_NMFIPR;
        size_t m_NRFREG;
        size_t m_NTFREG;
        size_t m_NPLMIX;
    };
}


#endif
