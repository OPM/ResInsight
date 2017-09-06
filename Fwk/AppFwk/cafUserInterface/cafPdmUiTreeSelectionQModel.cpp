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
    m_uiFieldHandle = nullptr;
    m_zeroLevelRowCount = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::setOptions(caf::PdmUiFieldEditorHandle* field, const QList<caf::PdmOptionItemInfo>& options)
{
    m_uiFieldHandle = field;
    m_options = options;

    computeOptionItemTreeData();
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

    int optionItemIndex = -1;
    if (parent.isValid())
    {
        optionItemIndex = toOptionItemIndex(parent) + row + 1;
    }
    else
    {
        optionItemIndex = m_zeroLevelRowToOptionIndex.at(row);
    }

    CAF_ASSERT(optionItemIndex > -1);
    CAF_ASSERT(optionItemIndex < m_options.size());
 
    return createIndex(row, column, optionItemIndex);
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

    return m_optionsTreeData[toOptionItemIndex(child)].parentModelIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return m_zeroLevelRowCount;
    }

    return m_optionsTreeData[toOptionItemIndex(parent)].childCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant caf::PdmUiTreeSelectionQModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (index.isValid())
    {
        CAF_ASSERT(index.internalId() < m_options.size());

        if (role == Qt::DisplayRole)
        {
            return m_options[toOptionItemIndex(index)].optionUiText();
        }
        else if (role == Qt::CheckStateRole)
        {
            CAF_ASSERT(m_uiFieldHandle);

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
        bool isSelected = value.toBool();

        emit signalSelectionStateForIndexHasChanged(toOptionItemIndex(index), isSelected);

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeSelectionQModel::computeOptionItemTreeData()
{
    m_optionsTreeData.clear();
    m_zeroLevelRowToOptionIndex.clear();

    if (m_options.size() == 0) return;

    m_optionsTreeData.resize(m_options.size());

    m_zeroLevelRowCount = 0;

    for (int i = 0; i < m_options.size(); i++)
    {
        if (m_options[i].level() == 0)
        {
            m_zeroLevelRowToOptionIndex[m_zeroLevelRowCount] = i;
            
            m_zeroLevelRowCount++;

            m_optionsTreeData[i].parentModelIndex = QModelIndex();
        }
        else if (m_options[i].level() > 0)
        {
            // Compute parent model index

            int childLevel = m_options[i].level();

            int parentOptionIndex = i - 1;
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

                    m_optionsTreeData[i].parentModelIndex = createIndex(parentRow, 0, parentOptionIndex);
                    break;
                }

                parentOptionIndex--;
            }
        }

        int childCount = 0;
        {
            int parentLevel = m_options[i].level();
            int currentOptionIndex = i + 1;

            while (currentOptionIndex < m_options.size() && m_options[currentOptionIndex].level() > parentLevel)
            {
                if (m_options[currentOptionIndex].level() == parentLevel + 1)
                {
                    childCount++;
                }

                currentOptionIndex++;
            }
        }

        m_optionsTreeData[i].childCount = childCount;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int caf::PdmUiTreeSelectionQModel::toOptionItemIndex(const QModelIndex& modelIndex) const
{
    CAF_ASSERT(modelIndex.isValid());
    CAF_ASSERT(modelIndex.internalId() < m_options.size());

    return modelIndex.internalId();
}
