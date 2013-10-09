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


#include "cafPdmUiListViewEditor.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmUiEditorHandle.h"
#include "cafUiTreeModelPdm.h"
#include "cafPdmDocument.h"

#include <QWidget>
#include <QGridLayout>
#include <QTableView>



namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UiTableModelPdm::UiTableModelPdm(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_columnCount = 0;
    m_pdmObjectGroup = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTableModelPdm::rowCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    if (!m_pdmObjectGroup)
    {
        return 0;
    }

    return static_cast<int>(m_pdmObjectGroup->objects.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UiTableModelPdm::columnCount(const QModelIndex &parent /*= QModelIndex( ) */) const
{
    if (!m_pdmObjectGroup)
    {
        return 0;
    }

    if (m_columnCount < 0)
    {
        return 0;
    }

    return m_columnCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UiTableModelPdm::computeColumnCount()
{
    if (m_editorAttribute.fieldNames.size() > 0)
    {
        m_columnCount = m_editorAttribute.fieldNames.size();
    }
    else if (m_pdmObjectGroup)
    {
        m_columnCount = 0;

        // Loop over all objects and find the object with largest number of fields
        for (size_t i = 0; i < m_pdmObjectGroup->objects.size(); i++)
        {
            std::vector<PdmFieldHandle*> fields;
            m_pdmObjectGroup->objects[i]->fields(fields);

            if (m_columnCount < static_cast<int>(fields.size()))
            {
                m_columnCount = static_cast<int>(fields.size());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant UiTableModelPdm::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    return QVariant(QString("Header %1").arg(section));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant caf::UiTableModelPdm::data(const QModelIndex &index, int role /*= Qt::DisplayRole */) const
{
    if (m_pdmObjectGroup && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        if (index.row() < static_cast<int>(m_pdmObjectGroup->objects.size()))
        {
            PdmObject* pdmObject = m_pdmObjectGroup->objects[index.row()];
            if (pdmObject)
            {
                std::vector<PdmFieldHandle*> fields;
                pdmObject->fields(fields);

                if (index.column() < static_cast<int>(fields.size()))
                {
                    size_t fieldIndex = 0;

                    if (m_editorAttribute.fieldNames.size() > 0)
                    {
                        QString fieldName = m_editorAttribute.fieldNames[index.column()];
                        for (size_t i = 0; i < fields.size(); i++)
                        {
                            if (fields[i]->keyword() == fieldName)
                            {
                                fieldIndex = i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        fieldIndex = index.column();
                    }

                    return fields[fieldIndex]->uiValue();
                }
            }
        }
    }

    return QVariant();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::UiTableModelPdm::setPdmData(PdmObjectGroup* objectGroup, const QString& configName)
{
    m_pdmObjectGroup = objectGroup;
    m_configName = configName;

    if (m_pdmObjectGroup)
    {
        m_pdmObjectGroup->objectEditorAttribute(m_configName, &m_editorAttribute);
    }

    computeColumnCount();

    reset();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListViewEditor::PdmUiListViewEditor()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListViewEditor::~PdmUiListViewEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListViewEditor::createWidget(QWidget* parent)
{
    m_mainWidget = new QWidget(parent);
    m_layout     = new QVBoxLayout();
    m_mainWidget->setLayout(m_layout);

    m_tableModelPdm = new UiTableModelPdm(m_mainWidget);

    m_tableView = new QTableView(m_mainWidget);
    m_tableView->setShowGrid(false);
    m_tableView->setModel(m_tableModelPdm);

    m_layout->addWidget(m_tableView);

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiListViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmObjectGroup* objectGroup = dynamic_cast<PdmObjectGroup*>(pdmObject());
    m_tableModelPdm->setPdmData(objectGroup, uiConfigName);

    m_tableView->resizeColumnsToContents();
}


} // end namespace caf

