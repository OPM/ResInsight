//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUiDoubleValueEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"

#include "cafFactory.h"
#include "cafQShortenedLabel.h"

#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiDoubleValueEditor);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDoubleValueEditor::PdmUiDoubleValueEditor() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDoubleValueEditor::~PdmUiDoubleValueEditor() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_lineEdit.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    m_lineEdit->setEnabled(!uiField()->isUiReadOnly(uiConfigName));

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
        if (m_attributes.m_validator)
        {
            m_lineEdit->setValidator(m_attributes.m_validator);
        }


    }

    bool    valueOk = false;
    double  value   = uiField()->uiValue().toDouble(&valueOk);
    QString textValue;
    if (valueOk)
    {
        if (m_attributes.m_numberFormat == PdmUiDoubleValueEditorAttribute::NumberFormat::FIXED)
            textValue = QString::number(value, 'f', m_attributes.m_decimals);
        else if (m_attributes.m_numberFormat == PdmUiDoubleValueEditorAttribute::NumberFormat::SCIENTIFIC)
            textValue = QString::number(value, 'e', m_attributes.m_decimals);
        else
            textValue = QString::number(value, 'g', m_attributes.m_decimals);        
    }
    else
    {
        textValue = uiField()->uiValue().toString();
    }

    m_lineEdit->setText(textValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleValueEditor::createEditorWidget(QWidget* parent)
{
    QWidget* containerWidget = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    containerWidget->setLayout(layout);

    m_lineEdit = new QLineEdit(containerWidget);
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    layout->addWidget(m_lineEdit);

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleValueEditor::createLabelWidget(QWidget* parent)
{
    m_label = new QShortenedLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::slotEditingFinished()
{
    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::writeValueToField()
{
    QString  textValue = m_lineEdit->text();
    QVariant v;
    v = textValue;
    this->setValueToField(v);
}

} // end namespace caf
