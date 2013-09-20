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


#include "cafPdmUiComboBoxEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QComboBox>
#include <QLabel>

#include <assert.h>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiComboBoxEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_comboBox.isNull());
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
    m_comboBox->setEnabled(!field()->isUiReadOnly(uiConfigName));

    // Demo code for attribute retreival when becoming relevant
    // PdmUiComboBoxEditorAttribute attributes;
    // field()->ownerObject()->editorAttribute(field(), uiConfigName, &attributes);

    bool fromMenuOnly = false;
    QList<PdmOptionItemInfo> options = field()->valueOptions(&fromMenuOnly);
    m_comboBox->blockSignals(true);
    m_comboBox->clear();
    if (!options.isEmpty())
    {
        m_comboBox->addItems(PdmOptionItemInfo::extractUiTexts(options));
        m_comboBox->setCurrentIndex(field()->uiValue().toInt());
    }
    else
    {
        m_comboBox->addItem(field()->uiValue().toString());
        m_comboBox->setCurrentIndex(0);
    }
    m_comboBox->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiComboBoxEditor::createEditorWidget(QWidget * parent)
{
    m_comboBox = new QComboBox(parent);
    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
    return m_comboBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiComboBoxEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotCurrentIndexChanged(int index)
{
    QVariant v;
    v = index;

    QVariant uintValue(v.toUInt());
    this->setValueToField(uintValue);
}



} // end namespace caf
