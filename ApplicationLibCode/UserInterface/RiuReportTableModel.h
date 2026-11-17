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

#pragma once

#include "RifReportTableFormatter.h"

#include <QAbstractTableModel>
#include <QColor>
#include <QFont>

#include <memory>

//==================================================================================================
/// Table model for one report table. Large tables show the first and last rows, separated by a
/// row telling how many rows are omitted. Rows are sorted before rows are omitted.
/// The table data is shared, not copied, as tables can be large.
//==================================================================================================
class RiuReportTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static constexpr int UnitRole = Qt::UserRole + 1;

    RiuReportTableModel( std::shared_ptr<const RiaReportTable> table,
                         const RifReportFormatOptions&         options,
                         const QColor&                         mutedTextColor,
                         QObject*                              parent = nullptr );

    int      rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int      columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    // A negative column restores the original row order
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;

    // Row index in the model for the row telling how many rows are omitted, or -1 if no rows are omitted
    int omittedRowIndex() const;

    bool anyColumnHasUnit() const;

    const RiaReportTable& table() const;

private:
    QVariant cellData( size_t tableRowIndex, int column, int role ) const;
    int      tableRowIndex( int modelRow ) const;
    void     updateRowOrder();

private:
    std::shared_ptr<const RiaReportTable> m_table;
    RifReportFormatOptions                m_options;
    RifReportRowRange                     m_rowRange;
    std::vector<size_t>                   m_rowOrder;
    int                                   m_sortColumn;
    Qt::SortOrder                         m_sortOrder;
    QColor                                m_mutedTextColor;
    QFont                                 m_numberFont;
    bool                                  m_anyColumnHasUnit;
};
