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

#ifndef AQUDIMS_HPP
#define AQUDIMS_HPP

#include <cstddef>

/*
  The Aqudims class is a small utility class designed to hold on to
  the values from the AQUDIMS keyword.
*/

namespace Opm {

    class Deck;

    class Aqudims {
    public:

        Aqudims();

        explicit Aqudims(const Deck& deck);

        static Aqudims serializeObject()
        {
            Aqudims result;
            result.m_mxnaqn = 1;
            result.m_mxnaqc = 2;
            result.m_niftbl = 3;
            result.m_nriftb = 4;
            result.m_nanaqu = 5;
            result.m_ncamax = 6;
            result.m_mxnali = 7;
            result.m_mxaaql = 8;

            return result;
        }

        size_t getNumAqunum() const
        {
            return m_mxnaqn;
        }

        size_t getNumConnectionNumericalAquifer() const
        {
            return m_mxnaqc;
        }

        size_t getNumInfluenceTablesCT() const
        {
            return m_niftbl;
        }

        size_t getNumRowsInfluenceTable() const
        {
            return m_nriftb;
        }

        size_t getNumAnalyticAquifers() const
        {
            return m_nanaqu;
        }
        
        size_t getNumRowsAquancon() const
        {
            return m_ncamax;
        }

        size_t getNumAquiferLists() const
        {
            return m_mxnali;
        }

        size_t getNumAnalyticAquifersSingleList() const
        {
            return m_mxaaql;
        }

        bool operator==(const Aqudims& data) const
        {
            return m_mxnaqn == data.m_mxnaqn &&
                   m_mxnaqc == data.m_mxnaqc &&
                   m_niftbl == data.m_niftbl &&
                   m_nriftb == data.m_nriftb &&
                   m_nanaqu == data.m_nanaqu &&
                   m_ncamax == data.m_ncamax &&
                   m_mxnali == data.m_mxnali &&
                   m_mxaaql == data.m_mxaaql;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_mxnaqn);
            serializer(m_mxnaqc);
            serializer(m_niftbl);
            serializer(m_nriftb);
            serializer(m_nanaqu);
            serializer(m_ncamax);
            serializer(m_mxnali);
            serializer(m_mxaaql);
        }

    private:
        size_t m_mxnaqn , m_mxnaqc , m_niftbl , m_nriftb , m_nanaqu , m_ncamax , m_mxnali , m_mxaaql;

    };
}


#endif
