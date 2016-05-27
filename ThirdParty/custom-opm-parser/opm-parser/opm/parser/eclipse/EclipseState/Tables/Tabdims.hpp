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

#ifndef TABDIMS_HPP
#define TABDIMS_HPP

/*
  The Tabdims class is a small utility class designed to hold on to
  the values from the TABDIMS keyword.
*/

#include <opm/parser/eclipse/Parser/ParserKeywords/T.hpp>

namespace Opm {
    class Tabdims {
    public:

        /*
          The TABDIMS keyword has a total of 25 items; most of them
          are ECLIPSE300 only and quite exotic. Here we only
          internalize the most common items.
        */
        Tabdims() :
            m_ntsfun( ParserKeywords::TABDIMS::NTSFUN::defaultValue ),
            m_ntpvt( ParserKeywords::TABDIMS::NTPVT::defaultValue ),
            m_nssfun( ParserKeywords::TABDIMS::NSSFUN::defaultValue ),
            m_nppvt(  ParserKeywords::TABDIMS::NPPVT::defaultValue ),
            m_ntfip(  ParserKeywords::TABDIMS::NTFIP::defaultValue ),
            m_nrpvt(  ParserKeywords::TABDIMS::NRPVT::defaultValue )
        { }


        Tabdims(size_t ntsfun, size_t ntpvt, size_t nssfun , size_t nppvt, size_t ntfip , size_t nrpvt) :
            m_ntsfun( ntsfun ),
            m_ntpvt( ntpvt ),
            m_nssfun( nssfun ),
            m_nppvt( nppvt ),
            m_ntfip( ntfip ),
            m_nrpvt( nrpvt ) {}


        size_t getNumSatTables() const {
            return m_ntsfun;
        }

        size_t getNumPVTTables() const {
            return m_ntpvt;
        }

        size_t getNumSatNodes() const {
            return m_nssfun;
        }

        size_t getNumPressureNodes() const {
            return m_nppvt;
        }

        size_t getNumFIPRegions() const {
            return m_ntfip;
        }

        size_t getNumRSNodes() const {
            return m_nrpvt;
        }

    private:
        size_t m_ntsfun,m_ntpvt,m_nssfun,m_nppvt,m_ntfip,m_nrpvt;
    };
}


#endif
