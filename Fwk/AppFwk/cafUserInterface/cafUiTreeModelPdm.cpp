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


#include "cafUiTreeModelPdm.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"



namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UiTreeModelPdm::UiTreeModelPdm(QObject* parent)
{
    m_treeItemRoot = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::setTreeItemRoot(PdmUiTreeItem* root)
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
QModelIndex UiTreeModelPdm::index(int row, int column, const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_treeItemRoot)
        return QModelIndex();

    if (!hasIndex(row, column, parentIndex))
        return QModelIndex();

    PdmUiTreeItem* parentItem = NULL;

    if (!parentIndex.isValid())
        parentItem = m_treeItemRoot;
    else
        parentItem = UiTreeModelPdm::getTreeItemFromIndex(parentIndex);


    PdmUiTreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeModelPdm::parent(const QModelIndex &childIndex) const
{
    if (!m_treeItemRoot) return QModelIndex();

    if (!childIndex.isValid()) return QModelIndex();

    PdmUiTreeItem* childItem = UiTreeModelPdm::getTreeItemFromIndex(childIndex);
    if (!childItem) return QModelIndex();

    PdmUiTreeItem* parentItem = childItem->parent();
    if (!parentItem) return QModelIndex();

    if (parentItem == m_treeItemRoot) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTreeModelPdm::rowCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_treeItemRoot)
        return 0;

    if (parentIndex.column() > 0)
        return 0;

    PdmUiTreeItem* parentItem;
    if (!parentIndex.isValid())
        parentItem = m_treeItemRoot;
    else
        parentItem = UiTreeModelPdm::getTreeItemFromIndex(parentIndex);

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTreeModelPdm::columnCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_treeItemRoot)
        return 0;

    if (parentIndex.isValid())
    {
         PdmUiTreeItem* parentItem = UiTreeModelPdm::getTreeItemFromIndex(parentIndex);
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
QVariant UiTreeModelPdm::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (!m_treeItemRoot)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    PdmUiTreeItem* treeItem = UiTreeModelPdm::getTreeItemFromIndex(index);
    assert(treeItem);

    PdmObjectHandle* obj = treeItem->dataObject();

    if (obj == NULL) return QVariant();

    // We try to find the context of the object first: The parent field
    // If found, use its data to describe the thing
    // Note: This code will only find first field pointing at the current object. Its valid for now,
    // but will not generally be valid if references is introduced in the pdm system

    PdmUiFieldHandle* parentField = 0;

    PdmUiTreeItem* parentTreeItem = treeItem->parent();
    if (parentTreeItem)
    {
        PdmObjectHandle* parentObj = parentTreeItem->dataObject();
        if (parentObj)
        {
            std::vector<PdmFieldHandle*> fields;
            parentObj->fields(fields);

            size_t i;
            for (i = 0; i < fields.size(); ++i)
            {
                std::vector<PdmObjectHandle*> children;
                if (fields[i]) fields[i]->childObjects(&children);
                size_t cIdx;
                for (cIdx = 0; cIdx < children.size(); ++ cIdx)
                {
                    if (children[cIdx] == obj)
                    {
                        caf::PdmUiFieldHandle* uiFieldHandle = fields[i]->uiCapability();
                        if (uiFieldHandle)
                        {
                            parentField = uiFieldHandle;
                            break;
                        }
                    }
                }
                if (parentField) break;
            }
        }
    }

    assert(obj);
    PdmUiObjectHandle* uiObject = uiObj(obj);

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (uiObject)
        {
            if (uiObject->userDescriptionField())
            {
                caf::PdmUiFieldHandle* uiFieldHandle = uiObject->userDescriptionField()->uiCapability();
                if (uiFieldHandle)
                {
                    return uiFieldHandle->uiValue();
                }
                else
                {
                    return QVariant();
                }
            }
            else
            {
                if (parentField && !parentField->uiName().isEmpty())
                    return parentField->uiName();
                else
                    return uiObject->uiName();
            }
        }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (parentField && !parentField->uiIcon().isNull())
            return parentField->uiIcon();
        else if (uiObject)
            return uiObject->uiIcon();
        else
            return QVariant();
    }
    else if (role == Qt::ToolTipRole)
    {
        if (parentField && !parentField->uiToolTip().isEmpty())
            return parentField->uiToolTip();
        else if (uiObject)
            return uiObject->uiToolTip();
        else
            return QVariant();
    }
    else if (role == Qt::WhatsThisRole)
    {
        if (parentField && !parentField->uiWhatsThis().isEmpty())
            return parentField->uiWhatsThis();
        else if (uiObject)
            return uiObject->uiWhatsThis();
        else
            return QVariant();
    }
    else if (role == Qt::CheckStateRole)
    {
        if (uiObject && uiObject->objectToggleField())
        {
            caf::PdmUiFieldHandle* uiFieldHandle = uiObject->objectToggleField()->uiCapability();
            if (uiFieldHandle)
            {
                bool isToggledOn = uiFieldHandle->uiValue().toBool();
                if (isToggledOn)
                {
                    return Qt::Checked;
                }
                else
                {
                    return Qt::Unchecked;
                }
            }
            else
            {
                return QVariant();
            }
        }
        else
        {
            return QVariant();
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::emitDataChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool UiTreeModelPdm::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
    {
        return false;
    }
    
    PdmUiTreeItem* treeItem = UiTreeModelPdm::getTreeItemFromIndex(index);
    assert(treeItem);
    
    PdmUiObjectHandle* uiObject = uiObj(treeItem->dataObject());
    if (uiObject)
    {
        if (role == Qt::EditRole && uiObject->userDescriptionField())
        {
            caf::PdmUiFieldHandle* uiFieldHandle = uiObject->userDescriptionField()->uiCapability();
            if (uiFieldHandle)
            {
                uiFieldHandle->setValueFromUi(value);
            }

            emitDataChanged(index);

            return true;
        }
        else if (role == Qt::CheckStateRole && uiObject->objectToggleField())
        {
            bool toggleOn = (value == Qt::Checked);

            caf::PdmUiFieldHandle* uiFieldHandle = uiObject->objectToggleField()->uiCapability();
            if (uiFieldHandle)
            {
                uiFieldHandle->setValueFromUi(toggleOn);
            }

            emitDataChanged(index);

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Enable edit of this item if we have a editable user description field for a pdmObject
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags UiTreeModelPdm::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index);

    PdmUiTreeItem* treeItem = getTreeItemFromIndex(index);
    if (treeItem)
    {
        PdmUiObjectHandle* uiObject = uiObj(treeItem->dataObject());
        if (uiObject)
        {
            if (uiObject->userDescriptionField() && !uiObject->userDescriptionField()->uiCapability()->isUiReadOnly())
            {
                flagMask = flagMask | Qt::ItemIsEditable;
            }

            if (uiObject->objectToggleField())
            {
                flagMask = flagMask | Qt::ItemIsUserCheckable;
            }

            if (uiObject->isUiReadOnly())
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
/// 
//--------------------------------------------------------------------------------------------------
bool UiTreeModelPdm::removeRows_special(int position, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
    if (count <= 0) return true;

    PdmUiTreeItem* parentItem = NULL;
    if (parent.isValid())
    {
        parentItem = getTreeItemFromIndex(parent);
    }
    else
    {
        parentItem = m_treeItemRoot;
    }

    if (!parentItem) return true;

    bool success = true;

    beginRemoveRows(parent, position, position + count - 1);
    success = parentItem->removeChildren(position, count);
    endRemoveRows();

    return success;
}

//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root PdmObject
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::updateUiSubTree(PdmObjectHandle* pdmRoot)
{
    // Build the new "Correct" Tree

    PdmUiTreeItem* tempUpdatedPdmTree = UiTreeItemBuilderPdm::buildViewItems(NULL, -1, pdmRoot);
    if (!tempUpdatedPdmTree) return;

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
void UiTreeModelPdm::updateModelSubTree(const QModelIndex& modelIdxOfDestinationSubTreeRoot, PdmUiTreeItem* destinationSubTreeRoot, PdmUiTreeItem* sourceSubTreeRoot)
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
PdmUiTreeItem* UiTreeModelPdm::treeItemRoot()
{
    return m_treeItemRoot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::notifyModelChanged()
{
    QModelIndex startModelIdx = index(0,0);
    QModelIndex endModelIdx = index(rowCount(startModelIdx), 0);

    emit dataChanged(startModelIdx, endModelIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant UiTreeModelPdm::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
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
void UiTreeModelPdm::setColumnHeaders(const QStringList& columnHeaders)
{
    m_columnHeaders = columnHeaders;
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeItem* caf::UiTreeModelPdm::getTreeItemFromIndex(const QModelIndex& index)
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
QModelIndex caf::UiTreeModelPdm::getModelIndexFromPdmObjectRecursive(const QModelIndex& currentIndex, const PdmObjectHandle * object) const
{
    if (currentIndex.internalPointer())
    {
        PdmUiTreeItem* treeItem = static_cast<PdmUiTreeItem*>(currentIndex.internalPointer());
        if (treeItem->dataObject() == object) return currentIndex;
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
QModelIndex caf::UiTreeModelPdm::getModelIndexFromPdmObject( const PdmObjectHandle * object) const
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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeItem* UiTreeItemBuilderPdm::buildViewItems(PdmUiTreeItem* parentTreeItem, int position, caf::PdmObjectHandle* object)
{
    if (object == NULL)
    {
        return NULL;
    }

    PdmUiTreeItem* objectTreeItem = new PdmUiTreeItem(parentTreeItem, position, object);

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    std::vector<caf::PdmFieldHandle*>::iterator it;
    for (it = fields.begin(); it != fields.end(); it++)
    {
        caf::PdmFieldHandle* field = *it;

        // Fix for hidden legend definitions. There is only one visible legend definition, the others reside in a hidden container
        // Todo: This is a Hack. Must be rewritten when a more general ui tree building method is in place.
        // See comment at top of this method.
        caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if (uiFieldHandle && uiFieldHandle->isUiChildrenHidden())
        {
            continue;
        }

        std::vector<caf::PdmObjectHandle*> children;
        field->childObjects(&children);
        size_t i;
        for (i = 0; i < children.size(); i++)
        {
            caf::PdmObjectHandle* childObj = children[i];
            if (childObj)
            {
                // NOTE: -1 as second argument indicates that child objects will be appended to collection
                UiTreeItemBuilderPdm::buildViewItems(objectTreeItem, -1, childObj);
            }
        }
    }

    return objectTreeItem;
}



} // end namespace caf
