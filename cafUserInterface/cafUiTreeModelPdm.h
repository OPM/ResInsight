//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cafUiTreeItem.h"

#include <QAbstractItemModel>
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

    void                    setRoot(PdmUiTreeItem* root);
    void                    emitDataChanged(const QModelIndex& index);

    static PdmUiTreeItem*  getTreeItemFromIndex(const QModelIndex& index);
    QModelIndex            getModelIndexFromPdmObject( const PdmObject * object) const;
    void                   rebuildUiSubTree(PdmObject* root);

public:
    // Overrides from QAbstractItemModel
    virtual QModelIndex     index(int row, int column, const QModelIndex &parentIndex = QModelIndex( )) const;
    virtual QModelIndex     parent(const QModelIndex &index) const;
    virtual int             rowCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;
    virtual int             columnCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // TO BE DELETED, NOT USED
    virtual bool            insertRows_special(int position, int rows, const QModelIndex &parent = QModelIndex());

    virtual bool            removeRows_special(int position, int rows, const QModelIndex &parent = QModelIndex());

protected:
    QModelIndex             getModelIndexFromPdmObjectRecursive(const QModelIndex& currentIndex, const PdmObject * object) const;

private:
    PdmUiTreeItem* m_root;
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
