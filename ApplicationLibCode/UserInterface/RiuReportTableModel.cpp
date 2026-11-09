/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuReportTableModel.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuReportTableModel::RiuReportTableModel( const RiaReportTable&         table,
                                          const RifReportFormatOptions& options,
                                          const QColor&                 mutedTextColor,
                                          QObject*                      parent )
    : QAbstractTableModel( parent )
    , m_table( table )
    , m_options( options )
    , m_mutedTextColor( mutedTextColor )
    , m_sortColumn( -1 )
    , m_sortOrder( Qt::AscendingOrder )
{
    updateRowOrder();

#if QT_VERSION >= QT_VERSION_CHECK( 6, 7, 0 )
    // Tabular digits make numbers in a column line up
    m_numberFont.setFeature( QFont::Tag( "tnum" ), 1 );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuReportTableModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) return 0;

    const size_t omittedRow = m_rowRange.omittedRowCount > 0 ? 1 : 0;
    return static_cast<int>( m_rowRange.headRowCount + omittedRow + m_rowRange.tailRowCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuReportTableModel::columnCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) return 0;

    return static_cast<int>( m_table.columns.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuReportTableModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() ) return {};

    if ( index.row() == omittedRowIndex() )
    {
        switch ( role )
        {
            case Qt::DisplayRole:
                return index.column() == 0 ? RifReportTableFormatter::omittedRowsText( m_rowRange.omittedRowCount ) : QString();
            case Qt::TextAlignmentRole:
                return QVariant::fromValue( Qt::AlignCenter );
            case Qt::ForegroundRole:
                return m_mutedTextColor;
            case Qt::FontRole:
            {
                QFont font;
                font.setItalic( true );
                return font;
            }
            case Qt::ToolTipRole:
                return QString( "Use Plain text or CSV format to see all rows" );
            default:
                return {};
        }
    }

    const int tableRow = tableRowIndex( index.row() );
    if ( tableRow < 0 ) return {};

    return cellData( static_cast<size_t>( tableRow ), index.column(), role );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuReportTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation != Qt::Horizontal || section < 0 || section >= columnCount() ) return {};

    const auto& column = m_table.columns[section];
    switch ( role )
    {
        case Qt::DisplayRole:
            return column.title;
        case UnitRole:
            return column.unit;
        case Qt::ToolTipRole:
            return RifReportTableFormatter::columnHeaderText( column );
        case Qt::TextAlignmentRole:
            return QVariant::fromValue( column.alignment == RIGHT ? Qt::AlignRight | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignVCenter );
        default:
            return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportTableModel::sort( int column, Qt::SortOrder order )
{
    if ( column == m_sortColumn && ( column < 0 || order == m_sortOrder ) ) return;

    beginResetModel();
    m_sortColumn = column;
    m_sortOrder  = order;
    updateRowOrder();
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportTableModel::setFilterText( const QString& filterText )
{
    if ( filterText == m_filterText ) return;

    beginResetModel();
    m_filterText = filterText;
    updateRowOrder();
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiuReportTableModel::filteredRowCount() const
{
    return m_rowOrder.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuReportTableModel::omittedRowIndex() const
{
    if ( m_rowRange.omittedRowCount == 0 ) return -1;

    return static_cast<int>( m_rowRange.headRowCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RiaReportTable& RiuReportTableModel::table() const
{
    return m_table;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuReportTableModel::cellData( size_t tableRowIndex, int column, int role ) const
{
    if ( column < 0 || column >= columnCount() || tableRowIndex >= m_table.rows.size() ) return {};

    const auto&              row         = m_table.rows[tableRowIndex];
    const auto&              columnDef   = m_table.columns[column];
    const RiaReportCellValue value       = static_cast<size_t>( column ) < row.size() ? row[column] : RiaReportCellValue();
    const bool               isUndefined = RifReportTableFormatter::isUndefined( value );

    switch ( role )
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return RifReportTableFormatter::formatCell( value, columnDef, m_options );
        case Qt::TextAlignmentRole:
            return QVariant::fromValue( columnDef.alignment == RIGHT ? Qt::AlignRight | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignVCenter );
        case Qt::ForegroundRole:
            if ( isUndefined ) return m_mutedTextColor;
            return {};
#if QT_VERSION >= QT_VERSION_CHECK( 6, 7, 0 )
        case Qt::FontRole:
            if ( RifReportTableFormatter::isNumeric( value ) ) return m_numberFont;
            return {};
#endif
        default:
            return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuReportTableModel::tableRowIndex( int modelRow ) const
{
    if ( modelRow < 0 ) return -1;

    const size_t row = static_cast<size_t>( modelRow );
    if ( row < m_rowRange.headRowCount ) return static_cast<int>( m_rowOrder[row] );

    const size_t tailFirstModelRow = m_rowRange.headRowCount + ( m_rowRange.omittedRowCount > 0 ? 1 : 0 );
    if ( row >= tailFirstModelRow && row < tailFirstModelRow + m_rowRange.tailRowCount )
    {
        return static_cast<int>( m_rowOrder[m_rowRange.tailStartRowIndex + row - tailFirstModelRow] );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportTableModel::updateRowOrder()
{
    m_rowOrder =
        RifReportTableFormatter::filteredAndSortedRows( m_table, m_filterText, m_sortColumn, m_sortOrder == Qt::AscendingOrder, m_options );
    m_rowRange = RifReportTableFormatter::htmlRowRange( m_rowOrder.size(), m_options );
}
