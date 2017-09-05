//##################################################################################################
//
//   Custom Visualization Core library
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


#include "cafPdmUiTreeSelectionQModel.h"

#include "cafPdmUiTreeViewModel.h"
#include "cafPdmObject.h"

#include <QAbstractItemModel>
#include <QLabel>
#include <QTreeView>




  
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeSelectionQModel::PdmUiTreeSelectionQModel(QObject *parent /*= 0*/) : QAbstractItemModel(parent)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::setOptions(caf::PdmUiFieldEditorHandle* field, const QList<caf::PdmOptionItemInfo>& options)
{
    m_uiFieldHandle = field;
    m_options = options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags caf::PdmUiTreeSelectionQModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeSelectionQModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (parent.isValid())
    {
        int optionItemIndex = parent.internalId() + 1 + row;

        return createIndex(row, column, optionItemIndex);
    }
    else
    {
        int optionItemIndex = 0;
        int zeroLevelItemCount = 0;

        while (optionItemIndex < m_options.size())
        {
            if (m_options[optionItemIndex].level() == 0)
            {
                zeroLevelItemCount++;
                if (zeroLevelItemCount == row + 1)
                {
                    return createIndex(row, column, optionItemIndex);
                }
            }
            optionItemIndex++;
        }
    }

    return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeSelectionQModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    int childOptionIndex = toOptionItemIndex(child);
    int childLevel = m_options[childOptionIndex].level();
    if (childLevel < 1)
    {
        return QModelIndex();
    }

    int parentOptionIndex = childOptionIndex - 1;
    while (parentOptionIndex > -1)
    {
        if (m_options[parentOptionIndex].level() == childLevel - 1)
        {
            int parentRow = 0;

            int parentLevelOptionIndex = parentOptionIndex - 1;
            while (parentLevelOptionIndex > -1 && m_options[parentLevelOptionIndex].level() > childLevel - 2)
            {
                if (m_options[parentLevelOptionIndex].level() == childLevel - 1)
                {
                    parentRow++;
                }

                parentLevelOptionIndex--;
            }

            return createIndex(parentRow, 0, parentOptionIndex);
        }

        parentOptionIndex--;
    }

    CAF_ASSERT(false);

    return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        int childCount = 0;
        for (auto o : m_options)
        {
            if (o.level() == 0)
            {
                childCount++;
            }
        }

        return childCount;
    }
    else
    {
        int childCount = 0;

        int parentOptionIndex = toOptionItemIndex(parent);
        int parentLevel = m_options[parentOptionIndex].level();
        int currentOptionIndex = parentOptionIndex + 1;

        while (currentOptionIndex < m_options.size() && m_options[currentOptionIndex].level() > parentLevel)
        {
            if (m_options[currentOptionIndex].level() == parentLevel + 1)
            {
                childCount++;
            }

            currentOptionIndex++;
        }

        return childCount;
    }

    CAF_ASSERT(false);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant caf::PdmUiTreeSelectionQModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    CAF_ASSERT(index.internalId() < m_options.size());

    if (role == Qt::DisplayRole)
    {
        return m_options[toOptionItemIndex(index)].optionUiText();
    }
    else if (role == Qt::CheckStateRole)
    {
        QVariant fieldValue = m_uiFieldHandle->field()->uiValue();
        QList<QVariant> valuesSelectedInField = fieldValue.toList();

        for (QVariant v : valuesSelectedInField)
        {
            int indexInField = v.toInt();
            if (indexInField == toOptionItemIndex(index))
            {
                return Qt::Checked;
            }
        }

        return Qt::Unchecked;
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiTreeSelectionQModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::CheckStateRole)
    {
        // TODO: wire up signal to editor

/*
        QVariant fieldValue = m_uiFieldHandle->field()->uiValue();
        QList<QVariant> valuesSelectedInField = fieldValue.toList();

        bool isPresent = false;
        for (QVariant v : valuesSelectedInField)
        {
            unsigned int indexInField = v.toUInt();
            if (indexInField == toOptionItemIndex(index))
            {
                valuesSelectedInField.removeAll(v);
                isPresent = true;
            }
        }

        if (!isPresent)
        {
            // Use unsigned int as all values communicated from UI to field is assumed to be unsigned int

            unsigned int value = static_cast<unsigned int>(toOptionItemIndex(index));
            valuesSelectedInField.push_back(QVariant(value));
        }

        m_uiFieldHandle->setValueToField(valuesSelectedInField);
*/

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::toOptionItemIndex(const QModelIndex& modelIndex)
{
    CAF_ASSERT(modelIndex.isValid());

    return modelIndex.internalId();
}
