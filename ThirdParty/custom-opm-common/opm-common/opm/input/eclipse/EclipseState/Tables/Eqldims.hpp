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

#ifndef EQLDIMS_HPP
#define EQLDIMS_HPP

#include <cstddef>

namespace Opm {

/*
  The Eqldims class is a small utility class designed to hold on to
  the values from the EQLDIMS keyword.
*/

    class Eqldims {
    public:

        Eqldims();

        Eqldims( size_t ntequl , size_t depth_nodes_p , size_t depth_nodes_tab , size_t nttrvd , size_t nstrvd) :
            m_ntequl( ntequl ),
            m_depth_nodes_p( depth_nodes_p ),
            m_depth_nodes_tab( depth_nodes_tab ),
            m_nttrvd( nttrvd ),
            m_nstrvd( nstrvd )
        { }

        static Eqldims serializeObject()
        {
            return Eqldims(1, 2, 3, 4, 5);
        }

        size_t getNumEquilRegions() const
        {
            return m_ntequl;
        }

        size_t getNumDepthNodesP() const
        {
            return m_depth_nodes_p;
        }


        size_t getNumDepthNodesTable() const
        {
            return m_depth_nodes_tab;
        }

        size_t getNumTracerTables() const
        {
            return m_nttrvd;
        }

        size_t getNumDepthNodesTracer() const
        {
            return m_nstrvd;
        }

        bool operator==(const Eqldims& data) const
        {
            return this->getNumEquilRegions() == data.getNumEquilRegions() &&
                   this->getNumDepthNodesP() == data.getNumDepthNodesP() &&
                   this->getNumDepthNodesTable() == data.getNumDepthNodesTable() &&
                   this->getNumTracerTables() == data.getNumTracerTables() &&
                   this->getNumDepthNodesTracer() == data.getNumDepthNodesTracer();
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_ntequl);
            serializer(m_depth_nodes_p);
            serializer(m_depth_nodes_tab);
            serializer(m_nttrvd);
            serializer(m_nstrvd);
        }

    private:
        size_t m_ntequl , m_depth_nodes_p , m_depth_nodes_tab , m_nttrvd , m_nstrvd;

    };
}


#endif
