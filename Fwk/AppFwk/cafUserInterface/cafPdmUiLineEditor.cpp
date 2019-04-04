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

#include "cafFactory.h"
#include "cafShortenedQLabel.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUniqueIdValidator.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QKeyEvent>
#include <QIntValidator>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>

namespace caf
{


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createEditorWidget(QWidget * parent)
{
    m_lineEdit = new PdmUiLineEdit(parent);

    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    return m_lineEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createLabelWidget(QWidget * parent)
{
    m_label = new cafShortenedQLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (!m_label.isNull())
    {
        PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);
    }

    if (!m_lineEdit.isNull())
    {
        bool isReadOnly = uiField()->isUiReadOnly(uiConfigName);
        if (isReadOnly)
        {
            m_lineEdit->setReadOnly(true);

            m_lineEdit->setStyleSheet("QLineEdit {"
                "color: #808080;"
                "background-color: #F0F0F0;}");
        }
        else
        {
            m_lineEdit->setReadOnly(false);
            m_lineEdit->setStyleSheet("");
        }

        m_lineEdit->setToolTip(uiField()->uiToolTip(uiConfigName));

        {
            PdmUiLineEditorAttribute leab;
            caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
            if (uiObject)
            {
                uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &leab);
            }

            if (leab.validator)
            {
                m_lineEdit->setValidator(leab.validator);
            }

            m_lineEdit->setAvoidSendingEnterEventToParentWidget(leab.avoidSendingEnterEventToParentWidget);
        }

        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> enumNames = uiField()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        if (!enumNames.isEmpty() && fromMenuOnly == true)
        {
            int enumValue = uiField()->uiValue().toInt();

            if (enumValue < enumNames.size() && enumValue > -1)
            {
                m_lineEdit->setText(enumNames[enumValue].optionUiText());
            }
        }
        else
        {
            PdmUiLineEditorAttributeUiDisplayString leab;
            caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
            if (uiObject)
            {
                uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &leab);
            }

            QString displayString;
            if (leab.m_displayString.isEmpty())
            {
                displayString = uiField()->uiValue().toString();
            }
            else
            {
                displayString = leab.m_displayString;
            }

            m_lineEdit->setText(displayString);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins PdmUiLineEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_lineEdit->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int heightDiff   = editorSize.height() - labelSize.height();

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
void PdmUiLineEditor::slotEditingFinished()
{
    QVariant v;
    QString textValue = m_lineEdit->text();
    v = textValue;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiLineEditor::isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const
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
PdmUiLineEdit::PdmUiLineEdit(QWidget* parent)
    : QLineEdit(parent), m_avoidSendingEnterEvent(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEdit::setAvoidSendingEnterEventToParentWidget(bool avoidSendingEnterEvent)
{
    m_avoidSendingEnterEvent = avoidSendingEnterEvent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEdit::keyPressEvent(QKeyEvent * event)
{
    QLineEdit::keyPressEvent(event);
    if (m_avoidSendingEnterEvent)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            // accept enter/return events so they won't
            // be ever propagated to the parent dialog..
            event->accept();
        }
    }
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build. The
// variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiLineEditor);

} // end namespace caf
