//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafPdmUiVec3dEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUniqueIdValidator.h"
#include "cafPickEventHandler.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QPushButton>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiVec3dEditor::createEditorWidget(QWidget* parent)
{
    QWidget* containerWidget = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    containerWidget->setLayout(layout);

    m_lineEdit = new QLineEdit(containerWidget);

    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    m_pickButton = new QPushButton(containerWidget);
    m_pickButton->setText("Pick");
    m_pickButton->setCheckable(true);

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_pickButton);

    connect(m_pickButton, SIGNAL(toggled(bool)), this, SLOT(pickButtonToggled(bool)));
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiVec3dEditor::createLabelWidget(QWidget* parent)
{
    m_label = new QLabel(parent);
    return m_label;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (!m_label.isNull())
    {
        PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);
    }

    PdmUiVec3dEditorAttribute attributes;
    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &attributes);
    }
    m_attribute = attributes;    

    bool isReadOnly = uiField()->isUiReadOnly(uiConfigName);

    m_pickButton->setVisible(attributes.pickEventHandler != nullptr);
    m_pickButton->setEnabled(!isReadOnly);

    m_lineEdit->setText(uiField()->uiValue().toString());
    m_lineEdit->setReadOnly(isReadOnly);
    if (isReadOnly)
    {
        m_lineEdit->setStyleSheet("QLineEdit {"
            "color: #808080;"
            "background-color: #F0F0F0;}");
    }
    else
    {
        m_lineEdit->setStyleSheet("");
    }
    pickButtonToggled(m_attribute.startPicking);

    m_lineEdit->setToolTip(uiField()->uiToolTip(uiConfigName));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins caf::PdmUiVec3dEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_lineEdit->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int   heightDiff = editorSize.height() - labelSize.height();

    QMargins contentMargins = m_label->contentsMargins();
    if (heightDiff > 0)
    {
        contentMargins.setTop(contentMargins.top() + heightDiff / 2);
        contentMargins.setBottom(contentMargins.bottom() + heightDiff / 2);
    }
    return contentMargins;    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::slotEditingFinished()
{
    QString  textValue = m_lineEdit->text();
    QVariant v         = textValue;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::pickButtonToggled(bool checked)
{
    if (!pickEventHandler())
    {
        return;
    }
    if (checked)
    {
        m_lineEdit->setStyleSheet("QLineEdit {"
            "color: #000000;"
            "background-color: #FFDCFF;}");
        m_pickButton->setText("Stop Picking");
        pickEventHandler()->registerAsPickEventHandler();
        m_pickButton->setChecked(true);
    }
    else
    {
        m_lineEdit->setStyleSheet("");
        m_pickButton->setText("Pick");
        pickEventHandler()->unregisterAsPickEventHandler();
        m_pickButton->setChecked(false);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiVec3dEditor::isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const
{
    std::vector<PdmFieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back(editorField);

    // For current selection, find all fields with same keyword
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems(items, SelectionManager::FIRST_LEVEL);

    for (size_t i = 0; i < items.size(); i++)
    {
        PdmUiFieldHandle* uiField = dynamic_cast<PdmUiFieldHandle*>(items[i]);
        if (!uiField) continue;

        PdmFieldHandle* field = uiField->fieldHandle();
        if (field && field != editorField && field->keyword() == editorField->keyword())
        {
            fieldsToUpdate.push_back(field);
        }
    }

    if (fieldsToUpdate.size() > 1)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* caf::PdmUiVec3dEditor::pickEventHandler() 
{
    return m_attribute.pickEventHandler.get();
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build. The
// variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiVec3dEditor);
