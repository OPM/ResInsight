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
    m_root = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::setRoot(PdmUiTreeItem* root)
{
    beginResetModel();
    
    if (m_root)
    {
        delete m_root;
    }

    m_root = root;
    endResetModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeModelPdm::index(int row, int column, const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_root)
        return QModelIndex();

    if (!hasIndex(row, column, parentIndex))
        return QModelIndex();

    PdmUiTreeItem* parentItem = NULL;

    if (!parentIndex.isValid())
        parentItem = m_root;
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
    if (!m_root) return QModelIndex();

    if (!childIndex.isValid()) return QModelIndex();

    PdmUiTreeItem* childItem = UiTreeModelPdm::getTreeItemFromIndex(childIndex);
    if (!childItem) return QModelIndex();

    PdmUiTreeItem* parentItem = childItem->parent();
    if (!parentItem) return QModelIndex();

    if (parentItem == m_root) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTreeModelPdm::rowCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_root)
        return 0;

    if (parentIndex.column() > 0)
        return 0;

    PdmUiTreeItem* parentItem;
    if (!parentIndex.isValid())
        parentItem = m_root;
    else
        parentItem = UiTreeModelPdm::getTreeItemFromIndex(parentIndex);

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTreeModelPdm::columnCount(const QModelIndex &parentIndex /*= QModelIndex( ) */) const
{
    if (!m_root)
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
        return m_root->columnCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant UiTreeModelPdm::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (!m_root)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    PdmUiTreeItem* treeItem = UiTreeModelPdm::getTreeItemFromIndex(index);
    assert(treeItem);

    PdmObject* obj = treeItem->dataObject();

    if (obj == NULL) return QVariant();

    // We try to find the context of the object first: The parent field
    // If found, use its data to describe the thing
    // Note: This code will only find first field pointing at the current object. Its valid for now,
    // but will not generally be valid if references is introduced in the pdm system

    PdmFieldHandle* parentField = 0;

    PdmUiTreeItem* parentTreeItem = treeItem->parent();
    if (parentTreeItem)
    {
        PdmObject* parentObj = parentTreeItem->dataObject();
        if (parentObj)
        {
            std::vector<PdmFieldHandle*> fields;
            parentObj->fields(fields);

            size_t i;
            for (i = 0; i < fields.size(); ++i)
            {
                std::vector<PdmObject*> children;
                if (fields[i]) fields[i]->childObjects(&children);
                size_t cIdx;
                for (cIdx = 0; cIdx < children.size(); ++ cIdx)
                {
                    if (children[cIdx] == obj)
                    { 
                        parentField = fields[i];
                        break;
                    }
                }
                if (parentField) break;
            }
        }
    }

    assert(obj);

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (obj->userDescriptionField())
        {
            return obj->userDescriptionField()->uiValue();
        }
        else
        {
            if (parentField && !parentField->uiName().isEmpty())
                return parentField->uiName();
            else
                return  obj->uiName();
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (parentField && !parentField->uiIcon().isNull())
            return parentField->uiIcon();
        else
            return obj->uiIcon();
    }
    else if (role == Qt::ToolTipRole)
    {
        if (parentField && !parentField->uiToolTip().isEmpty())
            return parentField->uiToolTip();
        else
            return  obj->uiToolTip();
    }
    else if (role == Qt::WhatsThisRole)
    {
        if (parentField && !parentField->uiWhatsThis().isEmpty())
            return parentField->uiWhatsThis();
        else
            return  obj->uiWhatsThis();
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
    if (index.isValid() && role == Qt::EditRole)
    {
        PdmUiTreeItem* treeItem = UiTreeModelPdm::getTreeItemFromIndex(index);
        assert(treeItem);

        PdmObject* obj = treeItem->dataObject();

        if (obj->userDescriptionField())
        {
            obj->userDescriptionField()->setValueFromUi(value);
        }

        emitDataChanged(index);
        
        return true;
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

    PdmUiTreeItem* treeItem = getTreeItemFromIndex(index);
    if (treeItem)
    {
        PdmObject* pdmObject = treeItem->dataObject();
        if (pdmObject && pdmObject->userDescriptionField() && !pdmObject->userDescriptionField()->isUiReadOnly())
        {
            Qt::ItemFlags flagMask = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
            return flagMask;
        }
    }

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index) & (~Qt::ItemIsEditable);
    return flagMask;
}

//--------------------------------------------------------------------------------------------------
/// TO BE DELETED
//--------------------------------------------------------------------------------------------------
bool UiTreeModelPdm::insertRows_special(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
{
    PdmUiTreeItem* parentItem = getTreeItemFromIndex(parent);
    
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows);
    endInsertRows();

    return success;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool UiTreeModelPdm::removeRows_special(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
{
    if (rows <= 0) return true;

    PdmUiTreeItem* parentItem = NULL;
    if (parent.isValid())
    {
        parentItem = getTreeItemFromIndex(parent);
    }
    else
    {
        parentItem = m_root;
    }

    if (!parentItem) return true;

    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTreeModelPdm::rebuildUiSubTree(PdmObject* root)
{
    QModelIndex item = getModelIndexFromPdmObject(root);
    if (item.isValid())
    {
       this->removeRows_special(0, rowCount(item), item);
       PdmUiTreeItem* treeItem = getTreeItemFromIndex(item);


       PdmUiTreeItem* fakeRoot = UiTreeItemBuilderPdm::buildViewItems(NULL, -1, root);
       
       this->beginInsertRows(item, 0, fakeRoot->childCount());
       for(int i = 0; i < fakeRoot->childCount(); ++i)
       {
           treeItem->appendChild(fakeRoot->child(i));
       }
       this->endInsertRows();
       fakeRoot->removeAllChildrenNoDelete();
       delete fakeRoot;
    }
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
QModelIndex caf::UiTreeModelPdm::getModelIndexFromPdmObjectRecursive(const QModelIndex& root, const PdmObject * object) const
{
    if (root.internalPointer())
    {
        PdmUiTreeItem* treeItem = static_cast<PdmUiTreeItem*>(root.internalPointer());
        if (treeItem->dataObject() == object) return root;
    }

   int row;
   for (row = 0; row < rowCount(root); ++row)
   {
       QModelIndex foundIndex = getModelIndexFromPdmObjectRecursive(index(row, 0, root), object);
       if (foundIndex.isValid()) return foundIndex;
   }
   return QModelIndex();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::UiTreeModelPdm::getModelIndexFromPdmObject( const PdmObject * object) const
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
PdmUiTreeItem* UiTreeItemBuilderPdm::buildViewItems(PdmUiTreeItem* parentTreeItem, int position, caf::PdmObject* object)
{
    if (object == NULL)
    {
        return NULL;
    }

    // NOTE: if position is -1, the item is appended to the parent tree item
    PdmUiTreeItem* objectTreeItem = new PdmUiTreeItem(parentTreeItem, position, object);

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    std::vector<caf::PdmFieldHandle*>::iterator it;
    for (it = fields.begin(); it != fields.end(); it++)
    {
        caf::PdmFieldHandle* field = *it;
        if (field->isUiHidden()) continue;

        std::vector<caf::PdmObject*> children;
        field->childObjects(&children);
        size_t i;
        for (i = 0; i < children.size(); i++)
        {
            caf::PdmObject* childObj = children[i];
            assert(childObj);

            // NOTE: -1 as second argument indicates that child objects will be appended to collection
            UiTreeItemBuilderPdm::buildViewItems(objectTreeItem, -1, childObj);
        }
    }

    return objectTreeItem;
}



} // end namespace caf
