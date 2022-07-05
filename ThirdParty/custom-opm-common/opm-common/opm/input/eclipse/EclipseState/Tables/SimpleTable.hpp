/*
  Copyright (C) 2013 by Andreas Lauser

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
#ifndef OPM_PARSER_SIMPLE_TABLE_HPP
#define	OPM_PARSER_SIMPLE_TABLE_HPP

#include <opm/input/eclipse/EclipseState/Tables/TableColumn.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableSchema.hpp>
#include <opm/input/eclipse/EclipseState/Util/OrderedMap.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Opm {

    class DeckItem;

    class SimpleTable {

    public:
        SimpleTable() = default;
        SimpleTable(TableSchema, const DeckItem& deckItem, const int tableID);
        explicit SimpleTable( TableSchema );

        static SimpleTable serializeObject();

        void addColumns();
        //! \brief Initialize deck item.
        //! \param deckItem item to initialize
        //! \param tableID Index for table to initialize
        //! \param scaling_factor If zero use SI value, else use value scaled by scaling factor
        void init( const DeckItem& deckItem,
                   const int tableID,
                   double scaling_factor = 0.0);
        size_t numColumns() const;
        size_t numRows() const;
        void addRow( const std::vector<double>& row);
        const TableColumn& getColumn(const std::string &name) const;
        const TableColumn& getColumn(size_t colIdx) const;
        bool hasColumn(const std::string& name) const;

        TableColumn& getColumn(const std::string &name);
        TableColumn& getColumn(size_t colIdx);

        double get(const std::string& column  , size_t row) const;
        double get(size_t column  , size_t row) const;
        /*!
         * \brief Evaluate a column of the table at a given position.
         *
         * This method uses linear interpolation and always uses the first column as the
         * X coordinate.
         */
        double evaluate(const std::string& columnName, double xPos) const;

        /// throws std::invalid_argument if jf != m_jfunc
        void assertJFuncPressure(const bool jf) const;

        bool operator==(const SimpleTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_schema.serializeOp(serializer);
            m_columns.serializeOp(serializer);
            serializer(m_jfunc);
        }

    protected:
        TableSchema m_schema;
        OrderedMap<TableColumn> m_columns;
        bool m_jfunc = false;
    };
}

#endif
