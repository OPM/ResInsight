/*
  Copyright 2015 Statoil ASA.

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



#include <opm/input/eclipse/EclipseState/Tables/TableIndex.hpp>

namespace Opm {

    TableIndex::TableIndex( size_t index1 , double weight1) :
        m_index1( index1 ),
        m_weight1( weight1 )
    {

    }


    TableIndex::TableIndex(const TableIndex& tableIndex)
        : m_index1( tableIndex.m_index1 ),
          m_weight1(tableIndex.m_weight1 )
    {
    }

    
    size_t TableIndex::getIndex1( ) const {
        return m_index1;
    }

    size_t TableIndex::getIndex2( ) const {
        return m_index1 + 1;
    }


    double TableIndex::getWeight1( ) const {
        return m_weight1;
    }


    double TableIndex::getWeight2( ) const {
        return 1 - m_weight1;
    }


}
