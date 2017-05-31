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


#include "cafPdmUiToolButtonEditor.h"

#include "cafPdmUiFieldHandle.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiObjectHandle.h"


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiToolButtonEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiToolButtonEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_toolButton.isNull());

    QIcon ic = field()->uiIcon(uiConfigName);
    if (!ic.isNull())
    {
        m_toolButton->setIcon(ic);
    }

    QString buttonText = field()->uiName(uiConfigName);
    m_toolButton->setText(buttonText);

    m_toolButton->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_toolButton->setToolTip(field()->uiToolTip(uiConfigName));

    PdmUiToolButtonEditorAttribute attributes;
    
    PdmUiObjectHandle* pdmUiOjectHandle = uiObj(field()->fieldHandle()->ownerObject());
    if (pdmUiOjectHandle)
    {
        pdmUiOjectHandle->editorAttribute(field()->fieldHandle(), uiConfigName, &attributes);
    }
    bool isCheckable = attributes.m_checkable;
    m_toolButton->setCheckable(isCheckable);

    QVariant variantFieldValue = field()->uiValue();
    if (isCheckable)
    {
        m_toolButton->setChecked(field()->uiValue().toBool());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiToolButtonEditor::createEditorWidget(QWidget * parent)
{
    m_toolButton = new QToolButton(parent);
    connect(m_toolButton, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)));
    return m_toolButton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiToolButtonEditor::createLabelWidget(QWidget * parent)
{
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiToolButtonEditor::slotClicked(bool checked)
{
    QVariant v;
    v = checked;
    this->setValueToField(v);
}


} // end namespace caf
