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


#include "cafPdmUiPushButtonEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiPushButtonEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_pushButton.isNull());
    CAF_ASSERT(!m_label.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    m_pushButton->setCheckable(true);
    m_pushButton->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_pushButton->setToolTip(field()->uiToolTip(uiConfigName));

    PdmUiPushButtonEditorAttribute attributes;
    caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &attributes);
    }

    QVariant variantFieldValue = field()->uiValue();

    if (!attributes.m_buttonIcon.isNull())
    {
        m_pushButton->setIcon(attributes.m_buttonIcon);
    }
    
    if (!attributes.m_buttonText.isEmpty())
    {
        m_pushButton->setText(attributes.m_buttonText);
    }
    else
    {
        if (variantFieldValue.type() == QVariant::Bool)
        {
            m_pushButton->setText(variantFieldValue.toBool() ? "On" : "Off" );
        }
        else
        {
             m_pushButton->setText(variantFieldValue.toString());
        }
    }

    if (variantFieldValue.type() == QVariant::Bool)
    {
        m_pushButton->setChecked(field()->uiValue().toBool());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureEditorForField(PdmFieldHandle* fieldHandle)
{
    if (fieldHandle)
    {
        if (fieldHandle->xmlCapability())
        {
            fieldHandle->xmlCapability()->disableIO();
        }

        if (fieldHandle->uiCapability())
        {
            fieldHandle->uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
            fieldHandle->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LEFT);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createEditorWidget(QWidget * parent)
{
    m_pushButton = new QPushButton("", parent);
    connect(m_pushButton, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)));
    return m_pushButton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::slotClicked(bool checked)
{
    if (field() && dynamic_cast<PdmField<bool> *> (field()->fieldHandle()))
    {
        QVariant v;
        v = checked;
        this->setValueToField(v);
    }
}


} // end namespace caf
