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

#pragma once
 
#include <QModelIndex>

class QTreeView;
class QString;
class QAbstractItemModel;

namespace caf
{

class QTreeViewStateSerializer
{
public:
    static void applyTreeViewStateFromString(QTreeView* treeView, const QString& treeViewState);
    static void storeTreeViewStateToString  (const QTreeView* treeView, QString& treeViewState);

    static QModelIndex  getModelIndexFromString(QAbstractItemModel* model, const QString& currentIndexString);
    static void         encodeStringFromModelIndex(const QModelIndex mi, QString& currentIndexString);
};

} // End namespace caf
