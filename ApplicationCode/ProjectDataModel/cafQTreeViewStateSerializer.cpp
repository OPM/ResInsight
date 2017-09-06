/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafQTreeViewStateSerializer.h"


#include <QTreeView>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void setExpandedState(QStringList& nodes, QTreeView* view, QAbstractItemModel* model,
                      const QModelIndex startIndex, QString path)
{
    path += QString::number(startIndex.row()) + QString::number(startIndex.column());
    for (int i = 0; i < model->rowCount(startIndex); ++i)
    {
        QModelIndex nextIndex = model->index(i, 0, startIndex);
        QString nextPath = path + QString::number(nextIndex.row()) + QString::number(nextIndex.column());
        if(!nodes.contains(nextPath))
            continue;
        
        setExpandedState(nodes, view, model, model->index(i, 0, startIndex), path);
    }
    
    if (nodes.contains(path))
    {
        view->setExpanded( startIndex.sibling(startIndex.row(), 0), true );
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void storeExpandedState(QStringList & nodes, const QTreeView * view, QAbstractItemModel * model,
                        const QModelIndex startIndex, QString path)
{
    path += QString::number(startIndex.row()) + QString::number(startIndex.column());
    for (int i = 0; i < model->rowCount(startIndex); ++i)
    {
        if(!view->isExpanded(model->index(i, 0, startIndex)))
            continue;

        storeExpandedState(nodes, view, model, model->index(i, 0, startIndex), path);
    }

    if (view->isExpanded(startIndex))
    {
        nodes << path;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::QTreeViewStateSerializer::applyTreeViewStateFromString( QTreeView* treeView, const QString& treeViewState)
{
    if (treeView->model())
    {
        treeView->collapseAll();

        QStringList nodes = treeViewState.split(";");

        QString path;
        setExpandedState(nodes, treeView, treeView->model(), QModelIndex(), path);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::QTreeViewStateSerializer::storeTreeViewStateToString(const QTreeView* treeView, QString& treeViewState)
{
    if (treeView->model())
    {
        QStringList nodes;
        QString path;

        storeExpandedState(nodes, treeView, treeView->model(), QModelIndex(), path);

        treeViewState = nodes.join(";");
    }
}

//--------------------------------------------------------------------------------------------------
/// Find index based of an encode QString <row> <column>;<row> <column>;...;<row> <column>
/// Set the decoded index as current index in the QAbstractItemView
//--------------------------------------------------------------------------------------------------
QModelIndex caf::QTreeViewStateSerializer::getModelIndexFromString(QAbstractItemModel* model, const QString& currentIndexString)
{
    QStringList modelIndexStringList = currentIndexString.split(";");

    QModelIndex mi;

    foreach (QString modelIndexString, modelIndexStringList)
    {
        QStringList items = modelIndexString.split(" ");

        if (items.size() != 2) continue;

        int row = items[0].toInt();
        int col = items[1].toInt();

        mi = model->index(row, col, mi);
    }

    return mi;
}

//--------------------------------------------------------------------------------------------------
/// Store path to model index in item view using follwoing encoding into a QString <row> <column>;<row> <column>;...;<row> <column>
//--------------------------------------------------------------------------------------------------
void caf::QTreeViewStateSerializer::encodeStringFromModelIndex(const QModelIndex mi, QString& encodedModelIndex)
{
    if (!mi.isValid()) return;

    QModelIndex localModelIdx = mi;

    while (localModelIdx.isValid())
    {
        if (encodedModelIndex.isEmpty())
        {
            encodedModelIndex = QString("%1 %2").arg(localModelIdx.row()).arg(localModelIdx.column()) + encodedModelIndex;
        }
        else
        {
            encodedModelIndex = QString("%1 %2;").arg(localModelIdx.row()).arg(localModelIdx.column()) + encodedModelIndex;
        }
        localModelIdx = localModelIdx.parent();
    }
}

