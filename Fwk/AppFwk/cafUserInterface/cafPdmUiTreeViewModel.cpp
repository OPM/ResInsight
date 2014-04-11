//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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


#include "cafPdmUiTreeViewModel.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeOrdering.h"



namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewModel::PdmUiTreeViewModel(QObject* parent)
{
    m_treeItemRoot = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setTreeItemRoot(PdmUiTreeItem* root)
{
    beginResetModel();
    
    if (m_treeItemRoot)
    {
        delete m_treeItemRoot;
    }

    m_treeItemRoot = root;
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::index(int row, int column, const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
//     if (!m_treeItemRoot)
//         return QModelIndex();

    if (!hasIndex(row, column, parentIndex))
        return QModelIndex();

    PdmUiTreeItem* parentItem = NULL;

    if (!parentIndex.isValid())
        parentItem = m_treeItemRoot;
    else
        parentItem = static_cast<PdmUiTreeItem*>(parentIndex.internalPointer());

    PdmUiTreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::parent(const QModelIndex &childIndex) const
{
//    if (!m_treeItemRoot) return QModelIndex();

    if (!childIndex.isValid()) return QModelIndex();

    PdmUiTreeItem* childItem = static_cast<PdmUiTreeItem*>(childIndex.internalPointer());
    if (!childItem) return QModelIndex();

    PdmUiTreeItem* parentItem = childItem->parent();
    if (!parentItem) return QModelIndex();

    if (parentItem == m_treeItemRoot) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::rowCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_treeItemRoot)
        return 0;

    if (parentIndex.column() > 0)
        return 0;

    PdmUiTreeItem* parentItem;
    if (!parentIndex.isValid())
        parentItem = m_treeItemRoot;
    else
        parentItem = PdmUiTreeViewModel::getTreeItemFromIndex(parentIndex);

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::columnCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_treeItemRoot)
        return 0;

    if (parentIndex.isValid())
    {
         PdmUiTreeItem* parentItem = PdmUiTreeViewModel::getTreeItemFromIndex(parentIndex);
         if (parentItem)
         {
             return parentItem->columnCount();
         }
         else
         {
             return 0;
         }
    }
    else
        return m_treeItemRoot->columnCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (!index.isValid())
        return QVariant();

     PdmUiTreeOrdering* uitreeOrdering = static_cast<PdmUiTreeOrdering*>(index.internalPointer());
     if (!uitreeOrdering)
     {
         return QVariant();
     }
    PdmFieldHandle* pdmField = uitreeOrdering->field();
    PdmObject* pdmObj = uitreeOrdering->object();

     if (role == Qt::DisplayRole || role == Qt::EditRole)
     {
         if (pdmField && !pdmField->uiName().isEmpty())
         {
             return pdmField->uiName();
         }
         else if (pdmObj)
         {
             if (pdmObj->userDescriptionField())
                return pdmObj->userDescriptionField()->uiValue();
             else
                 return pdmObj->uiName();
         }
         else if (uitreeOrdering->uiItem())
         {
            return uitreeOrdering->uiItem()->uiName();
         }
         else
         {
             // Should not get here
             assert(0);
         }
     }
     else if (role == Qt::DecorationRole)
     {
         if (pdmField && !pdmField->uiIcon().isNull())
         {
             return pdmField->uiIcon();
         }
         else if (pdmObj)
         {
             return pdmObj->uiIcon();
         }
         else if (uitreeOrdering->uiItem())
         {
             return uitreeOrdering->uiItem()->uiIcon();
         }
         else
         {
             // Should not get here
             assert(0);
         }
     }
     else if (role == Qt::ToolTipRole)
     {
         if (pdmField && !pdmField->uiToolTip().isEmpty())
             return pdmField->uiToolTip();
         else if (pdmObj)
         {
             return pdmObj->uiToolTip();
         }
         else if (uitreeOrdering->uiItem())
         {
             return uitreeOrdering->uiItem()->uiToolTip();
         }
         else
         {
             // Should not get here
             assert(0);
         }
     }
     else if (role == Qt::WhatsThisRole)
     {
         if (pdmField && !pdmField->uiWhatsThis().isEmpty())
             return pdmField->uiWhatsThis();
         else if (pdmObj)
         {
             return pdmObj->uiWhatsThis();
         }
         else if (uitreeOrdering->uiItem())
         {
             return uitreeOrdering->uiItem()->uiWhatsThis();
         }
         else
         {
             // Should not get here
             assert(0);
         }
     }
     else if (role == Qt::CheckStateRole)
     {
         if (pdmObj && pdmObj->objectToggleField())
         {
             bool isToggledOn = pdmObj->objectToggleField()->uiValue().toBool();
             if (isToggledOn)
             {
                 return Qt::Checked;
             }
             else
             {
                 return Qt::Unchecked;
             }
         }
     }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::emitDataChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
    {
        return false;
    }
    
    PdmUiTreeItem* treeItem = PdmUiTreeViewModel::getTreeItemFromIndex(index);
    assert(treeItem);
    
    PdmObject* obj = treeItem->dataObject()->object();
    if (!obj)
    {
        return false;
    }
            
    if (role == Qt::EditRole && obj->userDescriptionField())
    {
        obj->userDescriptionField()->setValueFromUi(value);

        emitDataChanged(index);
        
        return true;
    }
    else if (role == Qt::CheckStateRole && obj->objectToggleField())
    {
        bool toggleOn = (value == Qt::Checked);
        
        obj->objectToggleField()->setValueFromUi(toggleOn);

        emitDataChanged(index);

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Enable edit of this item if we have a editable user description field for a pdmObject
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags PdmUiTreeViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index);

    PdmUiTreeItem* treeItem = getTreeItemFromIndex(index);
    if (treeItem)
    {
        PdmObject* pdmObject = treeItem->dataObject()->object();
        if (pdmObject)
        {
            if (pdmObject->userDescriptionField() && !pdmObject->userDescriptionField()->isUiReadOnly())
            {
                flagMask = flagMask | Qt::ItemIsEditable;
            }

            if (pdmObject->objectToggleField())
            {
                flagMask = flagMask | Qt::ItemIsUserCheckable;
            }

            if (pdmObject->isUiReadOnly())
            {
                flagMask = flagMask & (~Qt::ItemIsEnabled);
            }

        }
    }
    else
    {
        flagMask = flagMask & (~Qt::ItemIsEditable);
    }

    return flagMask;
}


//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root PdmObject
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateUiSubTree(PdmObject* pdmRoot)
{
    // Build the new "Correct" Tree

    PdmUiTreeOrdering* tempUpdatedPdmTree = pdmRoot->uiTreeOrdering();

    // Find the corresponding entry for "root" in the existing Ui tree

    QModelIndex uiSubTreeRootModelIdx = getModelIndexFromPdmObject(pdmRoot);

    PdmUiTreeItem* uiModelSubTreeRoot = NULL;
    if (uiSubTreeRootModelIdx.isValid())
    {
        uiModelSubTreeRoot = getTreeItemFromIndex(uiSubTreeRootModelIdx);
    }
    else
    {
        uiModelSubTreeRoot = m_treeItemRoot;
    }

 
    updateModelSubTree(uiSubTreeRootModelIdx, uiModelSubTreeRoot, tempUpdatedPdmTree);
 
    delete tempUpdatedPdmTree;
}

//--------------------------------------------------------------------------------------------------
/// Makes the destinationSubTreeRoot tree become identical to the tree in sourceSubTreeRoot, 
/// calling begin..() end..() to make the UI update accordingly.
/// This assumes that all the items have a pointer an unique PdmObject 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateModelSubTree(const QModelIndex& modelIdxOfDestinationSubTreeRoot, PdmUiTreeItem* destinationSubTreeRoot, PdmUiTreeItem* sourceSubTreeRoot)
{
    // First loop over children in the old ui tree, deleting the ones not present in 
    // the newUiTree

    for (int resultChildIdx = 0; resultChildIdx < destinationSubTreeRoot->childCount() ; ++resultChildIdx)
    {
        PdmUiTreeItem* oldChild = destinationSubTreeRoot->child(resultChildIdx);
        int childIndex = sourceSubTreeRoot->findChildItemIndex(oldChild->dataObject());

        if (childIndex == -1) // Not found
        {
            this->beginRemoveRows(modelIdxOfDestinationSubTreeRoot, resultChildIdx, resultChildIdx);
            destinationSubTreeRoot->removeChildren(resultChildIdx, 1);
            this->endRemoveRows();
            resultChildIdx--;
        }
    }

    // Then loop over the children in the new ui tree, finding the corresponding items in the old tree. 
    // If they are found, we move them to the correct position. 
    // If not found, we pulls the item out of the old ui tree, inserting it into the new tree to avoid the default delete operation in ~UiTreeItem()

    int sourceChildCount = sourceSubTreeRoot->childCount();
    int sourceChildIdx = 0;

    for (int resultChildIdx = 0; resultChildIdx < sourceChildCount; ++resultChildIdx, ++sourceChildIdx)
    {
        PdmUiTreeItem* newChild = sourceSubTreeRoot->child(sourceChildIdx);
        int childIndex = destinationSubTreeRoot->findChildItemIndex(newChild->dataObject());

        if (childIndex == -1) // Not found
        {
            this->beginInsertRows(modelIdxOfDestinationSubTreeRoot, resultChildIdx, resultChildIdx);
            destinationSubTreeRoot->insertChild(resultChildIdx, newChild);
            this->endInsertRows();
            sourceSubTreeRoot->removeChildrenNoDelete(sourceChildIdx, 1);
            sourceChildIdx--;
        }
        else if (childIndex != resultChildIdx) // Found, but must be moved
        {
            assert(childIndex > resultChildIdx);

            PdmUiTreeItem* oldChild = destinationSubTreeRoot->child(childIndex);
            this->beginMoveRows(modelIdxOfDestinationSubTreeRoot, childIndex, childIndex, modelIdxOfDestinationSubTreeRoot, resultChildIdx);
            destinationSubTreeRoot->removeChildrenNoDelete(childIndex, 1);
            destinationSubTreeRoot->insertChild(resultChildIdx, oldChild);
            this->endMoveRows();
            updateModelSubTree( index(resultChildIdx, 0, modelIdxOfDestinationSubTreeRoot), oldChild, newChild);
        }
        else // Found the corresponding item in the right place.
        {
            PdmUiTreeItem* oldChild = destinationSubTreeRoot->child(childIndex);
            updateModelSubTree( index(resultChildIdx, 0, modelIdxOfDestinationSubTreeRoot), oldChild, newChild);
        }
    }


}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeItem* PdmUiTreeViewModel::treeItemRoot()
{
    return m_treeItemRoot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::notifyModelChanged()
{
    QModelIndex startModelIdx = index(0,0);
    QModelIndex endModelIdx = index(rowCount(startModelIdx), 0);

    emit dataChanged(startModelIdx, endModelIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (section < m_columnHeaders.size())
    {
        return m_columnHeaders[section];
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setColumnHeaders(const QStringList& columnHeaders)
{
    m_columnHeaders = columnHeaders;
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeItem* caf::PdmUiTreeViewModel::getTreeItemFromIndex(const QModelIndex& index)
{
    if (index.isValid())
    {
        assert(index.internalPointer());

        PdmUiTreeItem* treeItem = static_cast<PdmUiTreeItem*>(index.internalPointer());
        return treeItem;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::getModelIndexFromPdmObjectRecursive(const QModelIndex& currentIndex, const PdmObject * object) const
{
    if (currentIndex.internalPointer())
    {
        PdmUiTreeItem* treeItem = static_cast<PdmUiTreeItem*>(currentIndex.internalPointer());
        if (treeItem->dataObject()->object() == object) return currentIndex;
    }

   int row;
   for (row = 0; row < rowCount(currentIndex); ++row)
   {
       QModelIndex foundIndex = getModelIndexFromPdmObjectRecursive(index(row, 0, currentIndex), object);
       if (foundIndex.isValid()) return foundIndex;
   }
   return QModelIndex();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::getModelIndexFromPdmObject( const PdmObject * object) const
{
   QModelIndex foundIndex;
   int numRows = rowCount(QModelIndex());
   int r = 0;
   while (r < numRows && !foundIndex.isValid())
   {
      foundIndex = getModelIndexFromPdmObjectRecursive(index(r, 0, QModelIndex()), object);
      ++r;
   }
   return foundIndex;
}






} // end namespace caf
