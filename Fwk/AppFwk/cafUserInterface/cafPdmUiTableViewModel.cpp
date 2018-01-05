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


#include "cafPdmUiTableViewModel.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiFieldEditorHelper.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableItemEditor.h"
#include "cafPdmUiTableView.h"
#include "cafSelectionManager.h"

#include <QTableView>




namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTableViewModel::PdmUiTableViewModel(QWidget* parent)
    : QAbstractTableModel(parent)
{
    m_pdmList = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewModel::rowCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    if (!m_pdmList) return 0;

    size_t itemCount = m_pdmList->size();
    return static_cast<int>(itemCount);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewModel::columnCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    return static_cast<int>(m_modelColumnIndexToFieldIndex.size());

    // SIG_CAF_HACK
    // Magne hack to comment out code that crashed
    /*
    std::vector<PdmObject*> listObjects;
    if (m_pdmList)
    {
        m_pdmList->childObjects(&listObjects);

        if (listObjects.size() > 0)
        {
            PdmObject* pdmObject = listObjects[0];
            if (pdmObject)
            {
                std::vector<PdmFieldHandle*> fields;
                pdmObject->fields(fields);

                return static_cast<int>(fields.size());
            }
        }
    }

    return 0;
    */
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTableViewModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
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
Qt::ItemFlags PdmUiTableViewModel::flags(const QModelIndex &index) const
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
        if (uiFieldHandle->isUiReadOnly(m_currentConfigName))
        {
            flagMask = flagMask ^ Qt::ItemIsEditable;
        }
    }
    return flagMask;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::CheckStateRole)
    {
        if (isRepresentingBoolean(index))
        {
            // Clear current selection, UI does not behave well for multiple selection
            SelectionManager::instance()->clear(SelectionManager::CURRENT);

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
QVariant PdmUiTableViewModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (role == Qt::TextColorRole)
    {
        PdmFieldHandle* fieldHandle = getField(index);
        if (fieldHandle && fieldHandle->uiCapability())
        {
            if (fieldHandle->uiCapability()->isUiReadOnly(m_currentConfigName))
            {
                return Qt::lightGray;
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

                if (valuesSelectedInField.size() > 0)
                {
                    QList<PdmOptionItemInfo> options;
                    bool useOptionsOnly = true;
                    options = uiFieldHandle->valueOptions(&useOptionsOnly);
                    CAF_ASSERT(useOptionsOnly); // Not supported

                    for (QVariant v : valuesSelectedInField)
                    {
                        int index = v.toInt();
                        if (index != -1)
                        {
                            if (!displayText.isEmpty()) displayText += ", ";

                            if (index < options.size())
                            {
                                displayText += options.at(index).optionUiText();
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

                PdmUiLineEditorAttributeUiDisplayString leab;
                uiObjForRow->editorAttribute(fieldHandle, m_currentConfigName, &leab);

                if (!leab.m_displayString.isEmpty())
                {
                    val = leab.m_displayString;
                }
                else
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

    return QVariant();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewModel::setPdmData(PdmChildArrayFieldHandle* listField, const QString& configName)
{
    beginResetModel();

    m_pdmList = listField;
    m_currentConfigName = configName;

    PdmUiOrdering configForFirstObject;

    if (m_pdmList && m_pdmList->size() > 0)
    {
        PdmObjectHandle* pdmObjHandle = m_pdmList->at(0);
        PdmUiObjectHandle* uiObject = pdmObjHandle->uiCapability();
        if (uiObject)
        {
            uiObject->uiOrdering(configName, configForFirstObject);
            uiObject->objectEditorAttribute(m_currentConfigName, &m_attributes);
        }
    }

    const std::vector<PdmUiItem*>& uiItems = configForFirstObject.uiItems();

    // Set all fieldViews to be unvisited
    std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldEditors.begin(); it != m_fieldEditors.end(); ++it)
    {
        it->second->setField(NULL);
    }

    m_modelColumnIndexToFieldIndex.clear();

    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(configName)) continue;

        if (uiItems[i]->isUiGroup()) continue;

        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItems[i]);
            PdmUiFieldEditorHandle* fieldEditor = NULL;

            // Find or create FieldEditor
            std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
            it = m_fieldEditors.find(field->fieldHandle()->keyword());

            if (it == m_fieldEditors.end())
            {
                fieldEditor = PdmUiFieldEditorHelper::fieldEditorForField(field, configName);

                if (fieldEditor)
                {
                    m_fieldEditors[field->fieldHandle()->keyword()] = fieldEditor;
                }
            }
            else
            {
                fieldEditor = it->second;
            }

            if (fieldEditor)
            {
                fieldEditor->setField(field); 

                //TODO: Create/update is not required at this point, as UI is recreated in getEditorWidgetAndTransferOwnership()
                // Can be moved, but a move will require changes in PdmUiFieldEditorHandle
                fieldEditor->createWidgets(NULL);
                fieldEditor->updateUi(configName);

                int fieldIndex = getFieldIndex(field->fieldHandle());
                m_modelColumnIndexToFieldIndex.push_back(fieldIndex);
            }
        }
    }


    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector< QString > fvhToRemoveFromMap;
    for (it = m_fieldEditors.begin(); it != m_fieldEditors.end(); ++it)
    {
        if (it->second->field() == 0)
        {
            PdmUiFieldEditorHandle* fvh = it->second;
            delete fvh;
            fvhToRemoveFromMap.push_back(it->first);               
        }
    }

    for (size_t i = 0; i < fvhToRemoveFromMap.size(); ++i)
    {
        m_fieldEditors.erase(fvhToRemoveFromMap[i]);
    }

    recreateTableItemEditors();

    endResetModel();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmUiTableViewModel::getField(const QModelIndex &index) const
{
    if (m_pdmList && index.row() < static_cast<int>(m_pdmList->size()))
    {
        PdmObjectHandle* pdmObject = m_pdmList->at(index.row());
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

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldEditorHandle* PdmUiTableViewModel::getEditor(const QModelIndex &index)
{
    PdmFieldHandle* field = getField(index);
    if (!field)
    {
        return NULL;
    }

    PdmUiFieldEditorHandle* editor = NULL;
    
    std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
    it = m_fieldEditors.find(field->keyword());

    if (it != m_fieldEditors.end())
    {
        editor = it->second;
        if (editor)
        {
            if (field)
            {
                editor->setField(field->uiCapability());
            }
        }
    }
    
    return editor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewModel::getEditorWidgetAndTransferOwnership(QWidget* parent, const QModelIndex &index)
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

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewModel::notifyDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    emit dataChanged(topLeft, bottomRight);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewModel::recreateTableItemEditors()
{
    for (size_t i = 0; i < m_tableItemEditors.size(); i++)
    {
        delete m_tableItemEditors[i];
    }
    m_tableItemEditors.clear();

    if (m_pdmList)
    {
        for (size_t i = 0; i < m_pdmList->size(); i++)
        {
            PdmObjectHandle* pdmObject = m_pdmList->at(i);
            m_tableItemEditors.push_back(new PdmUiTableItemEditor(this, pdmObject, static_cast<int>(i)));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldHandle* PdmUiTableViewModel::getUiFieldHandle(const QModelIndex& index) const
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
PdmObjectHandle* PdmUiTableViewModel::pdmObjectForRow(int row) const
{
    if (m_pdmList && row < static_cast<int>(m_pdmList->size()))
    {
        return m_pdmList->at(row);
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewModel::isRepresentingBoolean(const QModelIndex &index) const
{
    PdmFieldHandle* fieldHandle = getField(index);
    if (fieldHandle)
    {
        if (m_attributes.showPushButtonForFieldKeyword(fieldHandle->keyword()))
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
void PdmUiTableViewModel::createPersistentPushButtonWidgets(QTableView* tableView)
{
    if (rowCount() > 0)
    {
        for (int col = 0; col < columnCount(); col++)
        {
            PdmFieldHandle* fieldHandle = getField(createIndex(0, col));
            if (m_attributes.showPushButtonForFieldKeyword(fieldHandle->keyword()))
            {
                for (int row = 0; row < rowCount(); row++)
                {
                    QModelIndex mi = createIndex(row, col);

                    tableView->setIndexWidget(mi, new TableViewPushButton(getField(mi)->uiCapability(), m_attributes.pushButtonText(fieldHandle->keyword())));
                    tableView->openPersistentEditor(mi);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QItemSelection PdmUiTableViewModel::modelIndexFromPdmObject(PdmObjectHandle* pdmObject)
{
    QItemSelection itemSelection;

    for (int i = 0; i < this->rowCount(); i++)
    {
        PdmObjectHandle* obj = this->pdmObjectForRow(i);
        if (obj == pdmObject)
        {
            // Currently selection only on model index, can be exteded to select whole row
            itemSelection.select(this->createIndex(i, 0), this->createIndex(i, 0));
        }
    }

    return itemSelection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewModel::getFieldIndex(PdmFieldHandle* field) const
{
    if (m_pdmList && m_pdmList->size() > 0)
    {
        PdmObjectHandle* pdmObject = m_pdmList->at(0);
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
