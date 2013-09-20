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


#include "cafPdmUiLineEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"

#include "cafPdmField.h"

#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>

#include <assert.h>
#include "cafFactory.h"



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiLineEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_lineEdit.isNull());
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
    m_lineEdit->setEnabled(!field()->isUiReadOnly(uiConfigName));

    PdmUiLineEditorAttribute leab;
    field()->ownerObject()->editorAttribute(field(), uiConfigName, &leab);
    if (leab.useRangeValidator)
    {
        m_lineEdit->setValidator(new QIntValidator(leab.minValue, leab.maxValue, this));
    }

    bool fromMenuOnly = false;
    QList<PdmOptionItemInfo> enumNames = field()->valueOptions(&fromMenuOnly);
    if (!enumNames.isEmpty() && fromMenuOnly == true)
    {
        QStringList uiTexts = PdmOptionItemInfo::extractUiTexts(enumNames);
        int enumValue = field()->uiValue().toInt();
        m_lineEdit->setText(uiTexts.at(enumValue));
    }
    else
    {
        m_lineEdit->setText(field()->uiValue().toString());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createEditorWidget(QWidget * parent)
{
    m_lineEdit = new QLineEdit(parent);
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    return m_lineEdit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::slotEditingFinished()
{
    QVariant v;
    QString textValue = m_lineEdit->text();
    v = textValue;
    this->setValueToField(v);
}


} // end namespace caf
