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


#ifndef OPM_TABLE_SCHEMA_HPP
#define OPM_TABLE_SCHEMA_HPP

#include <string>

#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>
#include <opm/input/eclipse/EclipseState/Util/OrderedMap.hpp>

namespace Opm {

    class TableSchema {
    public:
        static TableSchema serializeObject();

        void addColumn( ColumnSchema );
        const ColumnSchema& getColumn( const std::string& name ) const;
        const ColumnSchema& getColumn( size_t columnIndex ) const;
        bool hasColumn(const std::string&) const;

        /* Number of columns */
        size_t size() const;

        bool operator==(const TableSchema& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_columns.serializeOp(serializer);
        }

    private:
        OrderedMap<ColumnSchema> m_columns;
    };
}

#endif

