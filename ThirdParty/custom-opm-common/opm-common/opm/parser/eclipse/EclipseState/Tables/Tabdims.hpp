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
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

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


        explicit Tabdims(const Deck& deck) :
            Tabdims()
        {
            if (deck.hasKeyword("TABDIMS")) {
                const auto& record = deck.getKeyword( "TABDIMS" , 0 ).getRecord( 0 );
                m_ntsfun = record.getItem("NTSFUN").get<int>(0);
                m_ntpvt  = record.getItem("NTPVT").get<int>(0);
                m_nssfun = record.getItem("NSSFUN").get<int>(0);
                m_nppvt  = record.getItem("NPPVT").get<int>(0);
                m_ntfip  = record.getItem("NTFIP").get<int>(0);
                m_nrpvt  = record.getItem("NRPVT").get<int>(0);
            }
        }

        static Tabdims serializeObject()
        {
            Tabdims result;
            result.m_ntsfun = 1;
            result.m_ntpvt = 2;
            result.m_nssfun = 3;
            result.m_nppvt = 4;
            result.m_ntfip = 5;
            result.m_nrpvt = 6;

            return result;
        }

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

        bool operator==(const Tabdims& data) const {
            return this->getNumSatTables() == data.getNumSatTables() &&
                   this->getNumPVTTables() == data.getNumPVTTables() &&
                   this->getNumSatNodes() == data.getNumSatNodes() &&
                   this->getNumPressureNodes() == data.getNumPressureNodes() &&
                   this->getNumFIPRegions() == data.getNumFIPRegions() &&
                   this->getNumRSNodes() == data.getNumRSNodes();
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_ntsfun);
            serializer(m_ntpvt);
            serializer(m_nssfun);
            serializer(m_nppvt);
            serializer(m_ntfip);
            serializer(m_nrpvt);
        }

    private:
        size_t m_ntsfun,m_ntpvt,m_nssfun,m_nppvt,m_ntfip,m_nrpvt;
    };
}


#endif
