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


#include "cafPdmUiCheckBoxEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <assert.h>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiCheckBoxEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_checkBox.isNull());
    assert(!m_label.isNull());

    QIcon ic = field()->uiIcon(uiConfigName);
    if (!ic.isNull())
    {
        m_label->setPixmap(ic.pixmap(ic.actualSize(QSize(64, 64))));
    }
    else
    {
        m_label->setText(field()->uiName(uiConfigName));
    }

    m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_label->setToolTip(field()->uiToolTip(uiConfigName));

    m_checkBox->setEnabled(!field()->isUiReadOnly(uiConfigName));

    PdmUiCheckBoxEditorAttribute attributes;
    field()->ownerObject()->editorAttribute(field(), uiConfigName, &attributes);

    m_checkBox->setChecked(field()->uiValue().toBool());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxEditor::createEditorWidget(QWidget * parent)
{
    m_checkBox = new QCheckBox(parent);
    connect(m_checkBox, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)));
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxEditor::slotClicked(bool checked)
{
    QVariant v;
    v = checked;
    this->setValueToField(v);
}


} // end namespace caf
