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

#include "cafPdmUiToolBarEditor.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldEditorHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"

#include <QAction>
#include <QMainWindow>
#include <QToolBar>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::PdmUiToolBarEditor(const QString& title, QMainWindow* mainWindow)
{
    m_toolbar = new QToolBar(mainWindow);
    m_toolbar->setObjectName(title);
    m_toolbar->setWindowTitle(title);

    mainWindow->addToolBar(m_toolbar);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::~PdmUiToolBarEditor()
{
    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiToolBarEditor::isEditorDataValid(const std::vector<caf::PdmFieldHandle*>& fields) const
{
    if (m_fields.size() == fields.size() && m_fieldViews.size() == fields.size())
    {
        bool equalContent = true;

        for (size_t i = 0; i < m_fields.size(); i++)
        {
            if (m_fields[i] != fields[i])
            {
                equalContent = false;
            }
        }

        if (equalContent)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    {
        // Find set of owner objects. Can be several objects, make a set to avoid calling uiOrdering more than once for an object

        std::set<caf::PdmUiObjectHandle*> ownerUiObjects;

        for (PdmFieldHandle* field : m_fields)
        {
            caf::PdmUiObjectHandle* ownerUiObject = field->ownerObject()->uiCapability();
            if (ownerUiObject)
            {
                ownerUiObjects.insert(ownerUiObject);
            }
        }

        PdmUiOrdering config;
        for (caf::PdmUiObjectHandle* ownerUiObject : ownerUiObjects)
        {
            ownerUiObject->uiOrdering(uiConfigName, config);
        }
    }

    for (PdmFieldHandle* field : m_fields)
    {
        PdmUiFieldEditorHandle* fieldEditor = nullptr;

        // Find or create FieldEditor
        std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
        it = m_fieldViews.find(field->keyword());
        if (it == m_fieldViews.end())
        {
            caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();

            bool addSpace = false;
            if (uiFieldHandle)
            {
                if (uiFieldHandle->uiValue().type() == QVariant::Bool)
                {
                    QString editorTypeName = caf::PdmUiToolButtonEditor::uiEditorTypeName();

                    fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(editorTypeName);
                }
                else
                {
                    fieldEditor = caf::PdmUiFieldEditorHelper::createFieldEditorForField(field->uiCapability(), uiConfigName);

                    addSpace = true;
                }
            }

            if (fieldEditor)
            {
                m_fieldViews[field->keyword()] = fieldEditor;
                fieldEditor->setUiField(uiFieldHandle);
                fieldEditor->createWidgets(nullptr);
                m_actions.push_back(m_toolbar->addWidget(fieldEditor->editorWidget()));

                if (addSpace)
                {
                    QWidget* widget = new QWidget;
                    widget->setMinimumWidth(5);
                    m_toolbar->addWidget(widget);
                }

                fieldEditor->updateUi(uiConfigName);
            }
        }
        else
        {
            if (it->second)
            {
                it->second->updateUi(uiConfigName);
            }
        }
    }

    CAF_ASSERT(m_fields.size() == m_fieldViews.size());
    CAF_ASSERT(static_cast<int>(m_fields.size()) == m_actions.size());

    for (size_t i = 0; i < m_fields.size(); i++)
    {
        caf::PdmFieldHandle* field = m_fields[i];

        // Enabled state of a tool button is controlled by the QAction associated with a tool button
        // Changing the state of a widget directly has no effect
        // See Qt doc for QToolBar::insertWidget
        QAction* action = m_actions[static_cast<int>(i)];

        caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if (uiFieldHandle)
        {
            action->setEnabled(!uiFieldHandle->isUiReadOnly(uiConfigName));
        }

        // TODO: Show/hide of tool bar items can be done by
        // action->setVisible(!field->isUiHidden(uiConfigName));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::setFields(std::vector<caf::PdmFieldHandle*>& fields)
{
    clear();

    m_fields = fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::clear()
{
    for (const auto& it : m_fieldViews)
    {
        delete it.second;
    }

    m_fieldViews.clear();

    if (m_toolbar)
    {
        m_toolbar->clear();
    }

    m_actions.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::show()
{
    if (m_toolbar)
    {
        m_toolbar->show();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::hide()
{
    if (m_toolbar)
    {
        m_toolbar->hide();
    }
}

} // end namespace caf
