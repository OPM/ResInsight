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


#include "cafPdmUiTextEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"

#include <QTextEdit>
#include <QLabel>
#include <QIntValidator>
#include <QVBoxLayout>




namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiTextEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextEdit::TextEdit(QWidget *parent /*= 0*/) : QTextEdit(parent)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    emit editingFinished();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTextEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_textEdit.isNull());
    CAF_ASSERT(!m_label.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    m_textEdit->setReadOnly(field()->isUiReadOnly(uiConfigName));
    //m_textEdit->setEnabled(!field()->isUiReadOnly(uiConfigName)); // Neccesary ?
    m_textEdit->setToolTip(field()->uiToolTip(uiConfigName));

    PdmUiTextEditorAttribute leab;
    
    caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &leab);
    }
    
    m_textMode = leab.textMode;

    if (leab.showSaveButton)
    {
        disconnect(m_textEdit, SIGNAL(editingFinished()), this, SLOT(slotSetValueToField()));
        m_saveButton->show();
    }
    else
    {
        connect(m_textEdit, SIGNAL(editingFinished()), this, SLOT(slotSetValueToField()));
        m_saveButton->hide();
    }

    m_textEdit->blockSignals(true);
    switch (leab.textMode)
    {
    case PdmUiTextEditorAttribute::PLAIN:
        m_textEdit->setPlainText(field()->uiValue().toString());
        break;
    case PdmUiTextEditorAttribute::HTML:
        m_textEdit->setHtml(field()->uiValue().toString());
        break;
    }
    m_textEdit->blockSignals(false);

    m_textEdit->setWordWrapMode(toQTextOptionWrapMode(leab.wrapMode));

    m_textEdit->setFont(leab.font);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTextEditor::createEditorWidget(QWidget * parent)
{
    QWidget* containerWidget = new QWidget(parent);

    m_textEdit = new TextEdit(containerWidget);
    connect(m_textEdit, SIGNAL(editingFinished()), this, SLOT(slotSetValueToField()));

    m_saveButton = new QPushButton("Save changes", containerWidget);
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(slotSetValueToField()));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_textEdit);
    layout->setMargin(0);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->insertStretch(0, 10);
    buttonLayout->addWidget(m_saveButton);

    layout->addLayout(buttonLayout);
    containerWidget->setLayout(layout);

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTextEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTextEditor::slotSetValueToField()
{
    QVariant v;
    QString textValue;

    switch (m_textMode)
    {
    case PdmUiTextEditorAttribute::PLAIN:
        textValue = m_textEdit->toPlainText();
        break;
    case PdmUiTextEditorAttribute::HTML:
        textValue = m_textEdit->toHtml();
        break;
    }

    v = textValue;

    this->setValueToField(v);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextOption::WrapMode PdmUiTextEditor::toQTextOptionWrapMode(PdmUiTextEditorAttribute::WrapMode wrapMode)
{
    switch (wrapMode)
    {
    case PdmUiTextEditorAttribute::NoWrap:                      return QTextOption::WrapMode::NoWrap;
    case PdmUiTextEditorAttribute::WordWrap:                    return QTextOption::WrapMode::WordWrap;
    case PdmUiTextEditorAttribute::ManualWrap:                  return QTextOption::WrapMode::ManualWrap;
    case PdmUiTextEditorAttribute::WrapAnywhere:                return QTextOption::WrapMode::WrapAnywhere;
    case PdmUiTextEditorAttribute::WrapAtWordBoundaryOrAnywhere:
    default:                                                    return QTextOption::WrapMode::WrapAtWordBoundaryOrAnywhere;
    }
}

} // end namespace caf
