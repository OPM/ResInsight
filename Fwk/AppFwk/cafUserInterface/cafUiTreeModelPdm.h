//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cafUiTreeItem.h"

#include <QAbstractItemModel>
#include <QStringList>

#include <assert.h>
#include "cafPdmPointer.h"


namespace caf
{

class PdmObject;

typedef UiTreeItem<PdmPointer<PdmObject> > PdmUiTreeItem;

//==================================================================================================
/// 
//==================================================================================================
class UiTreeModelPdm : public QAbstractItemModel
{
    Q_OBJECT

public:
    UiTreeModelPdm(QObject* parent);

    void                    setTreeItemRoot(PdmUiTreeItem* root);
    PdmUiTreeItem*          treeItemRoot();

    void                    emitDataChanged(const QModelIndex& index);

    static PdmUiTreeItem*   getTreeItemFromIndex(const QModelIndex& index);
    QModelIndex             getModelIndexFromPdmObject(const PdmObject* object) const;
    void                    updateUiSubTree(PdmObject* root);

    void                    notifyModelChanged();
    void                    setColumnHeaders(const QStringList& columnHeaders);

public:
    // Overrides from QAbstractItemModel
    virtual QModelIndex     index(int row, int column, const QModelIndex &parentIndex = QModelIndex( )) const;
    virtual QModelIndex     parent(const QModelIndex &index) const;
    virtual int             rowCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;
    virtual int             columnCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant        headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);


    virtual bool            removeRows_special(int position, int rows, const QModelIndex &parent = QModelIndex());

protected:
    QModelIndex             getModelIndexFromPdmObjectRecursive(const QModelIndex& currentIndex, const PdmObject * object) const;
private:
    void                    updateModelSubTree(const QModelIndex& uiSubTreeRootModelIdx, PdmUiTreeItem* uiModelSubTreeRoot, PdmUiTreeItem* updatedPdmSubTreeRoot);

    PdmUiTreeItem*          m_treeItemRoot;
    QStringList             m_columnHeaders;
};



//==================================================================================================
/// 
//==================================================================================================
class UiTreeItemBuilderPdm
{
public:
    static PdmUiTreeItem* buildViewItems(PdmUiTreeItem* parentTreeItem, int position, caf::PdmObject* object);
};

} // End of namespace caf
