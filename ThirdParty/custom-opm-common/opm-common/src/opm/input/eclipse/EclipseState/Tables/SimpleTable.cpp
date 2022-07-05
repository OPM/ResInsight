/*
  Copyright (C) 2013 by Andreas Lauser, 2016 Statoil ASA

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
#include <utility>
#include <iostream>

#include <opm/input/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableSchema.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>

#include <fmt/format.h>

namespace Opm {

    SimpleTable::SimpleTable( TableSchema schema, const DeckItem& deckItem,
                              const int tableID) :
        m_schema( std::move( schema ) ),
        m_jfunc (false)
    {
        init( deckItem, tableID );
    }


    SimpleTable::SimpleTable( TableSchema schema ) :
        m_schema( std::move( schema ) ),
        m_jfunc (false)
    {
        addColumns();
    }


    SimpleTable SimpleTable::serializeObject()
    {
        SimpleTable result;
        result.m_schema = Opm::TableSchema::serializeObject();
        result.m_columns.insert({"test3", Opm::TableColumn::serializeObject()});
        result.m_jfunc = true;

        return result;
    }

    void SimpleTable::addRow( const std::vector<double>& row) {
        if (row.size() == numColumns()) {
            for (size_t colIndex  = 0; colIndex < numColumns(); colIndex++) {
                auto& col = getColumn( colIndex );
                col.addValue( row[colIndex] );
            }
        } else
            throw std::invalid_argument("Size mismatch");
    }


    void SimpleTable::addColumns() {
        for (size_t colIdx = 0; colIdx < m_schema.size(); ++colIdx) {
            const auto& schemaColumn = m_schema.getColumn( colIdx );
            TableColumn column(schemaColumn); // Some move trickery here ...
            m_columns.insert( std::make_pair( schemaColumn.name() , column ));
        }
    }


    double SimpleTable::get(const std::string& column  , size_t row) const {
        const auto& col = getColumn( column );
        return col[row];
    }


    double SimpleTable::get(size_t column  , size_t row) const {
        const auto& col = getColumn( column );
        return col[row];
    }

    void SimpleTable::init( const DeckItem& deckItem,
                            const int tableID,
                            double scaling_factor) {
        this->addColumns();

        if ( (deckItem.data_size() % numColumns()) != 0) {
            throw std::runtime_error {
                fmt::format("For table with ID {}: "
                            "Number of input table elements ({}) is "
                            "not a multiple of table's specified number "
                            "of columns ({})",
                            tableID+1, deckItem.data_size(), this->numColumns())
            };
        }

        size_t rows = deckItem.data_size() / numColumns();
        for (size_t colIdx = 0; colIdx < numColumns(); ++colIdx) {
            auto& column = getColumn( colIdx );
            for (size_t rowIdx = 0; rowIdx < rows; rowIdx++) {
                size_t deckItemIdx = rowIdx*numColumns() + colIdx;
                if (deckItem.defaultApplied(deckItemIdx))
                    column.addDefault( );
                else if (m_jfunc) {
                    column.addValue( deckItem.getData<double>()[deckItemIdx] );
                }
                else {
                    if (scaling_factor > 0.0)
                        column.addValue( scaling_factor * deckItem.get<double>(deckItemIdx) );
                    else
                        column.addValue( deckItem.getSIDouble(deckItemIdx) );
                }
            }
            if (colIdx > 0)
                column.applyDefaults(getColumn( 0 ));
        }
    }

    size_t SimpleTable::numColumns() const {
        return m_schema.size();
    }

    size_t SimpleTable::numRows() const {
        return getColumn( 0 ).size();
    }

    const TableColumn& SimpleTable::getColumn( const std::string& name) const {
        if (!this->m_jfunc)
            return m_columns.get( name );

        if (name == "PCOW" || name == "PCOG")
            assertJFuncPressure(false); // this will throw since m_jfunc=true
        return m_columns.get( name );
    }

    const TableColumn& SimpleTable::getColumn( size_t columnIndex )  const {
        return m_columns.iget( columnIndex );
    }


    TableColumn& SimpleTable::getColumn( const std::string& name) {
        if (!this->m_jfunc)
            return m_columns.get( name );

        if (name == "PCOW" || name == "PCOG")
            assertJFuncPressure(false); // this will throw since m_jfunc=true
        return m_columns.get( name );
    }

    TableColumn& SimpleTable::getColumn( size_t columnIndex ) {
        return m_columns.iget( columnIndex );
    }


    bool SimpleTable::hasColumn(const std::string& name) const {
        return m_schema.hasColumn( name );
    }

    double SimpleTable::evaluate(const std::string& columnName, double xPos) const
    {
        const auto& argColumn = getColumn( 0 );
        const auto& valueColumn = getColumn( columnName );

        const auto index = argColumn.lookup( xPos );
        return valueColumn.eval( index );
    }

    void SimpleTable::assertJFuncPressure(const bool jf) const {
        if (jf == m_jfunc)
            return;
        // if we reach here, wrong values are read from the deck! (JFUNC is used
        // incorrectly.)  This function writes to std err for now, but will
        // after a grace period be rewritten to throw (std::invalid_argument).
        if (m_jfunc)
            std::cerr << "Developer warning: Pressure column is read with JFUNC in deck." << std::endl;
        else
            std::cerr << "Developer warning: Raw values from JFUNC column is read, but JFUNC not provided in deck." << std::endl;
    }


    bool SimpleTable::operator==(const SimpleTable& data) const {
        return this->m_schema == data.m_schema &&
               this->m_columns == data.m_columns &&
               this->m_jfunc == data.m_jfunc;
    }
}
