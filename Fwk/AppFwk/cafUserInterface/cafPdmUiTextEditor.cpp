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

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"

#include "cafPdmField.h"

#include <QTextEdit>
#include <QLabel>
#include <QIntValidator>
#include <QVBoxLayout>

#include <assert.h>
#include "cafFactory.h"



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiTextEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTextEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_textEdit.isNull());
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

    //m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));

    m_textEdit->setReadOnly(field()->isUiReadOnly(uiConfigName));
    //m_textEdit->setEnabled(!field()->isUiReadOnly(uiConfigName)); // Neccesary ?

    PdmUiTextEditorAttribute leab;
    field()->ownerObject()->editorAttribute(field(), uiConfigName, &leab);
    m_textMode = leab.textMode;

    if (leab.showSaveButton)
    {
        disconnect(m_textEdit, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
        m_saveButton->show();
    }
    else
    {
        connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
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

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTextEditor::createEditorWidget(QWidget * parent)
{
    QWidget* containerWidget = new QWidget(parent);

    m_textEdit = new QTextEdit(containerWidget);
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));

    m_saveButton = new QPushButton("Save changes", containerWidget);
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(slotSaveButtonClicked()));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_textEdit);

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
void PdmUiTextEditor::slotTextChanged()
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
void PdmUiTextEditor::slotSaveButtonClicked()
{
    slotTextChanged();
}


} // end namespace caf
