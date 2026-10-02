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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

class QLineEdit;
class QStandardItemModel;
class QSortFilterProxyModel;
class QTreeView;
class QModelIndex;

//==================================================================================================
/// The task types published by installed packages, grouped by package. Tasks are added to the
/// workflow by dragging them onto the graph or by double-clicking.
//==================================================================================================
class RiuWorkflowTaskPalette : public QWidget
{
    Q_OBJECT

public:
    explicit RiuWorkflowTaskPalette( QWidget* parent = nullptr );

    void reload();

signals:
    void taskActivated( const QString& taskId );

private:
    void onDoubleClicked( const QModelIndex& index );

private:
    QLineEdit*             m_filter;
    QTreeView*             m_treeView;
    QStandardItemModel*    m_model;
    QSortFilterProxyModel* m_proxyModel;
};
