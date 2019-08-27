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


#include "cafPdmUiTableViewQModel.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiFieldEditorHelper.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableRowEditor.h"
#include "cafPdmUiTableView.h"
#include "cafSelectionManager.h"

#include <QTableView>




namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTableViewQModel::PdmUiTableViewQModel(QWidget* parent)
    : QAbstractTableModel(parent)
{
    m_pdmList = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::rowCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    auto childArrayField = childArrayFieldHandle();
    if (childArrayField)
    {
        size_t itemCount = childArrayField->size();
        return static_cast<int>(itemCount);
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::columnCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    return static_cast<int>(m_modelColumnIndexToFieldIndex.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTableViewQModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            PdmUiFieldHandle* uiFieldHandle = getUiFieldHandle(createIndex(0, section));
            if (uiFieldHandle)
            {
                return uiFieldHandle->uiName(m_currentConfigName);
            }
        }
        else if (orientation == Qt::Vertical)
        {
            return section + 1;
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags PdmUiTableViewQModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index);

    if (isRepresentingBoolean(index))
    {
        flagMask = flagMask | Qt::ItemIsUserCheckable;
    }
    else
    {
        flagMask = flagMask | Qt::ItemIsEditable;
    }

    PdmUiFieldHandle* uiFieldHandle = getUiFieldHandle(index);
    if (uiFieldHandle)
    {
        if ( uiFieldHandle->isUiReadOnly(m_currentConfigName) )
        {
            if ( flagMask & Qt::ItemIsUserCheckable )
            {
                flagMask = flagMask & (~Qt::ItemIsEnabled);
            }
            else
            {
                flagMask = flagMask ^ Qt::ItemIsEditable; // To make it selectable, but not editable
            }
        }
    }
    return flagMask;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewQModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::CheckStateRole)
    {
        if (isRepresentingBoolean(index))
        {

            bool toggleOn = (value == Qt::Checked);

            PdmUiFieldHandle* uiFieldHandle = getUiFieldHandle(index);
            if (uiFieldHandle)
            {
                PdmUiCommandSystemProxy::instance()->setUiValueToField(uiFieldHandle, toggleOn);

                return true;
            }
        }
    }   

     return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTableViewQModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (role == Qt::ForegroundRole)
    {
        PdmFieldHandle* fieldHandle = getField(index);
        if (fieldHandle && fieldHandle->uiCapability())
        {
            QColor textColor = fieldHandle->uiCapability()->uiContentTextColor(m_currentConfigName);

            if (fieldHandle->uiCapability()->isUiReadOnly(m_currentConfigName))
            {
                if (textColor.isValid())
                {
                    return textColor.lighter(150);
                }
                else
                {
                    return QColor(Qt::lightGray);
                }
            }
            else if (textColor.isValid())
            {
                return textColor;
            }
        }
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        PdmFieldHandle* fieldHandle = getField(index);

        PdmUiFieldHandle* uiFieldHandle = fieldHandle->uiCapability();
        if (uiFieldHandle)
        {
            QVariant fieldValue = uiFieldHandle->uiValue();
            if (fieldValue.type() == QVariant::List)
            {
                QString displayText;
                QList<QVariant> valuesSelectedInField = fieldValue.toList();

                if (!valuesSelectedInField.empty())
                {
                    QList<PdmOptionItemInfo> options;
                    bool useOptionsOnly = true;
                    options = uiFieldHandle->valueOptions(&useOptionsOnly);
                    CAF_ASSERT(useOptionsOnly); // Not supported

                    for (const QVariant& v : valuesSelectedInField)
                    {
                        int optionIndex = v.toInt();
                        if (optionIndex != -1)
                        {
                            if (!displayText.isEmpty()) displayText += ", ";

                            if (optionIndex < options.size())
                            {
                                displayText += options.at(optionIndex).optionUiText();
                            }
                        }
                    }
                }

                return displayText;
            }

            bool useOptionsOnly = true;
            QList<PdmOptionItemInfo> valueOptions = uiFieldHandle->valueOptions(&useOptionsOnly);
            CAF_ASSERT(useOptionsOnly); // Not supported

            if (!valueOptions.isEmpty())
            {
                int listIndex = uiFieldHandle->uiValue().toInt();
                if (listIndex == -1)
                {
                    return "";
                }

                return valueOptions[listIndex].optionUiText();
            }

            QVariant val;

            PdmObjectHandle* objForRow = this->pdmObjectForRow(index.row());
            PdmUiObjectHandle* uiObjForRow = uiObj(objForRow);
            if (uiObjForRow)
            {
                // NOTE: Redesign
                // To be able to get formatted string, an editor attribute concept is used
                // TODO: Create a function in pdmObject like this
                // virtual void            defineDisplayString(const PdmFieldHandle* field, QString uiConfigName) {}

                {
                    PdmUiLineEditorAttributeUiDisplayString leab;
                    uiObjForRow->editorAttribute(fieldHandle, m_currentConfigName, &leab);

                    if (!leab.m_displayString.isEmpty())
                    {
                        val = leab.m_displayString;
                    }
                }

                if (val.isNull())
                {
                    PdmUiDateEditorAttribute leab;
                    uiObjForRow->editorAttribute(fieldHandle, m_currentConfigName, &leab);

                    QString dateFormat = leab.dateFormat;
                    if (!dateFormat.isEmpty())
                    {
                        QDate date = uiFieldHandle->uiValue().toDate();
                        if (date.isValid())
                        {
                            QString displayString = date.toString(dateFormat);
                            val = displayString;
                        }
                    }
                }

                if (val.isNull())
                {
                    val = uiFieldHandle->uiValue();
                }
            }
            else
            {
                val = uiFieldHandle->uiValue();
            }

            return val;
        }
        else
        {
            CAF_ASSERT(false);
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (isRepresentingBoolean(index))
        {
            PdmUiFieldHandle* uiFieldHandle = getField(index)->uiCapability();
            if (uiFieldHandle)
            {
                QVariant val = uiFieldHandle->uiValue();
                bool isToggledOn = val.toBool();
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
    }
    else if ( role == Qt::ToolTipRole )
    {
        PdmUiFieldHandle* uiFieldHandle = getField(index)->uiCapability();
        if ( uiFieldHandle )
        {
            return uiFieldHandle->uiToolTip();
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
void PdmUiTableViewQModel::setArrayFieldAndBuildEditors(PdmChildArrayFieldHandle* listField, const QString& configName)
{
    beginResetModel();

    {
        PdmObjectHandle* ownerObject = nullptr;
        if (listField)
        {
            ownerObject = listField->ownerObject();
        }

        if (ownerObject)
        {
            m_pdmList = listField;
            m_ownerObject = ownerObject;
        }
        else
        {
            m_ownerObject = nullptr;
            m_pdmList = nullptr;
        }
    }

    m_currentConfigName = configName;

    PdmUiOrdering configForFirstObject;

    if (m_pdmList && !m_pdmList->empty())
    {
        PdmObjectHandle* firstObject = m_pdmList->at(0);
        PdmUiObjectHandle* uiHandleForFirstObject = firstObject->uiCapability();
        if (uiHandleForFirstObject)
        {
            uiHandleForFirstObject->uiOrdering(configName, configForFirstObject);
            uiHandleForFirstObject->objectEditorAttribute(m_currentConfigName, &m_pushButtonEditorAttributes);
        }
    }

    const std::vector<PdmUiItem*>& uiItems = configForFirstObject.uiItems();

 
    std::set<QString> usedFieldKeywords;

    m_modelColumnIndexToFieldIndex.clear();

    for (auto uiItem : uiItems)
    {
        if (uiItem->isUiHidden(configName)) continue;

        if (uiItem->isUiGroup()) continue;

        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItem);
            PdmUiFieldEditorHandle* fieldEditor = nullptr;

            // Find or create FieldEditor
            auto it = m_fieldEditors.find(field->fieldHandle()->keyword());

            if (it == m_fieldEditors.end())
            {
                fieldEditor = PdmUiFieldEditorHelper::createFieldEditorForField(field, configName);

                if (fieldEditor)
                {
                    fieldEditor->setUiField(field);
                    m_fieldEditors[field->fieldHandle()->keyword()] = fieldEditor;
                }
            }
            else
            {
                fieldEditor = it->second;
            }

            if (fieldEditor)
            {
                usedFieldKeywords.insert(field->fieldHandle()->keyword());

                //TODO: Create/update is not required at this point, as UI is recreated in getEditorWidgetAndTransferOwnership()
                // Can be moved, but a move will require changes in PdmUiFieldEditorHandle
                fieldEditor->createWidgets(nullptr);
                fieldEditor->updateUi(configName);

                int fieldIndex = getFieldIndex(field->fieldHandle());
                m_modelColumnIndexToFieldIndex.push_back(fieldIndex);
            }
        }
    }

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector< QString > fvhToRemoveFromMap;
    for (auto it = m_fieldEditors.begin(); it != m_fieldEditors.end(); ++it)
    {
        if (usedFieldKeywords.count(it->first) == 0)
        {
            PdmUiFieldEditorHandle* fvh = it->second;
            delete fvh;
            fvhToRemoveFromMap.push_back(it->first);               
        }
    }

    for (const auto& fieldEditorName : fvhToRemoveFromMap)
    {
        m_fieldEditors.erase(fieldEditorName);
    }

    recreateTableItemEditors();

    endResetModel();

    if (m_pdmList)
    {
        // Update UI for all cells, as the content potentially has changed
        // This will probably cause performance issues for large tables

        for (auto tableItemEditor : m_tableRowEditors)
        {
            tableItemEditor->updateUi(configName);
        }

    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmUiTableViewQModel::getField(const QModelIndex &index) const
{
    auto childArrayField = childArrayFieldHandle();

    if (childArrayField && index.row() < static_cast<int>(childArrayField->size()))
    {
        PdmObjectHandle* pdmObject = childArrayField->at(index.row());
        if (pdmObject)
        {
            std::vector<PdmFieldHandle*> fields;
            pdmObject->fields(fields);

            int fieldIndex = m_modelColumnIndexToFieldIndex[index.column()];
            if (fieldIndex < static_cast<int>(fields.size()))
            {
                return fields[fieldIndex];
            }
            else
            {
                CAF_ASSERT(false);
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldEditorHandle* PdmUiTableViewQModel::getEditor(const QModelIndex &index)
{
    PdmFieldHandle* field = getField(index);
    if (!field)
    {
        return nullptr;
    }

    PdmUiFieldEditorHandle* editor = nullptr;
    
    std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
    it = m_fieldEditors.find(field->keyword());

    if (it != m_fieldEditors.end())
    {
        editor = it->second;
        if (editor)
        {
            if (field)
            {
                editor->setUiField(field->uiCapability());
            }
        }
    }
    
    return editor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewQModel::getEditorWidgetAndTransferOwnership(QWidget* parent, const QModelIndex &index)
{
    PdmUiFieldEditorHandle* editor = getEditor(index);
    if (editor)
    {
        // Recreate editor widget, as the delegate takes ownership of the QWidget and destroys it when
        // edit is completed. This will cause the editor widget pointer to be NULL, as it is a guarded pointer
        // using QPointer
        editor->createWidgets(parent);
        QWidget* editorWidget = editor->editorWidget();
        editorWidget->setParent(parent);

        return editorWidget;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::notifyDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    emit dataChanged(topLeft, bottomRight);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::recreateTableItemEditors()
{
    for (auto tableItemEditor : m_tableRowEditors)
    {
        delete tableItemEditor;
    }
    m_tableRowEditors.clear();

    auto childArrayField = childArrayFieldHandle();
    if (childArrayField)
    {
        for (size_t i = 0; i < childArrayField->size(); i++)
        {
            PdmObjectHandle* pdmObject = childArrayField->at(i);
            m_tableRowEditors.push_back(new PdmUiTableRowEditor(this, pdmObject, static_cast<int>(i)));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldHandle* PdmUiTableViewQModel::getUiFieldHandle(const QModelIndex& index) const
{
    auto fieldHandle = getField(index);
    if (fieldHandle)
    {
        return fieldHandle->uiCapability();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiTableViewQModel::pdmObjectForRow(int row) const
{
    auto childArrayField = childArrayFieldHandle();
    if (childArrayField && row < static_cast<int>(childArrayField->size()))
    {
        return childArrayField->at(row);
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewQModel::isRepresentingBoolean(const QModelIndex &index) const
{
    PdmFieldHandle* fieldHandle = getField(index);
    if (fieldHandle)
    {
        if (m_pushButtonEditorAttributes.showPushButtonForFieldKeyword(fieldHandle->keyword()))
        {
            return false;
        }

        QVariant val = fieldHandle->uiCapability()->uiValue();
        if (val.type() == QVariant::Bool)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::createPersistentPushButtonWidgets(QTableView* tableView)
{
    if (rowCount() > 0)
    {
        for (int col = 0; col < columnCount(); col++)
        {
            PdmFieldHandle* fieldHandle = getField(createIndex(0, col));
            if (m_pushButtonEditorAttributes.showPushButtonForFieldKeyword(fieldHandle->keyword()))
            {
                for (int row = 0; row < rowCount(); row++)
                {
                    QModelIndex mi = createIndex(row, col);

                    tableView->setIndexWidget(mi, new TableViewPushButton(getField(mi)->uiCapability(), m_pushButtonEditorAttributes.pushButtonText(fieldHandle->keyword())));
                    tableView->openPersistentEditor(mi);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QItemSelection PdmUiTableViewQModel::modelIndexFromPdmObject(PdmObjectHandle* pdmObject)
{
    QItemSelection itemSelection;

    for (int i = 0; i < this->rowCount(); i++)
    {
        PdmObjectHandle* obj = this->pdmObjectForRow(i);
        if (obj == pdmObject)
        {
            // Select whole row
            itemSelection.select(this->createIndex(i, 0), this->createIndex(i, this->columnCount()));
        }
    }

    return itemSelection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* PdmUiTableViewQModel::childArrayFieldHandle() const
{
    // Required to have a PdmPointer to the owner object. Used to guard access to a field inside this object. It is not
    // possible to use a PdmPointer on a field pointer
    if (m_ownerObject.isNull())
    {
        return nullptr;
    }

    return m_pdmList;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::getFieldIndex(PdmFieldHandle* field) const
{
    auto childArrayField = childArrayFieldHandle();

    if (childArrayField && !childArrayField->empty())
    {
        PdmObjectHandle* pdmObject = childArrayField->at(0);
        if (pdmObject)
        {
            std::vector<PdmFieldHandle*> fields;
            pdmObject->fields(fields);

            for (size_t i = 0; i < fields.size(); i++)
            {
                if (fields[i]->keyword() == field->keyword())
                {
                    return static_cast<int>(i);
                }
            }
        }
    }

    return -1;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TableViewPushButton::TableViewPushButton(caf::PdmUiFieldHandle* field, const QString& text, QWidget* parent /*= 0*/)
    : QPushButton(text, parent),
    m_fieldHandle(field)
{
    connect(this, SIGNAL(pressed()), SLOT(slotPressed()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TableViewPushButton::slotPressed()
{
    if (m_fieldHandle)
    {
        QVariant val = m_fieldHandle->uiValue();
        if (val.type() == QVariant::Bool)
        {
            bool currentValue = val.toBool();
            caf::PdmUiCommandSystemProxy::instance()->setUiValueToField(m_fieldHandle, !currentValue);
        }
    }
}

} // end namespace caf
