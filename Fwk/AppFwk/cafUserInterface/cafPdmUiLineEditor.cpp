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
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>




class PdmUniqueIdValidator : public QValidator
{
public:
    PdmUniqueIdValidator(const std::set<int>& usedIds, bool multipleSelectionOfSameFieldsSelected, const QString& errorMessage, QObject* parent)
        : QValidator(parent),
        m_usedIds(usedIds),
        m_nextValidValue(0),
        m_multipleSelectionOfSameFieldsSelected(multipleSelectionOfSameFieldsSelected),
        m_errorMessage(errorMessage)
    {
        computeNextValidId();
    }

    virtual State validate(QString& currentString, int &) const
    {
        if (m_multipleSelectionOfSameFieldsSelected)
        {
            return QValidator::Invalid;
        }

        if (currentString.isEmpty())
        {
            return QValidator::Intermediate;
        }

        bool isValidInteger = false;
        int currentValue = currentString.toInt(&isValidInteger);

        if (!isValidInteger)
        {
            return QValidator::Invalid;
        }

        if (currentValue < 0)
        {
            return QValidator::Invalid;
        }

        if (m_usedIds.find(currentValue) != m_usedIds.end())
        {
            QApplication* qapplication = qobject_cast<QApplication*>(qApp);

            foreach(QWidget* widget, qapplication->topLevelWidgets())
            {
                if (widget->inherits("QMainWindow"))
                {
                    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
                    if (mainWindow && mainWindow->statusBar())
                    {
                        mainWindow->statusBar()->showMessage(m_errorMessage, 3000);
                    }
                }
            }

            return QValidator::Intermediate;
        }

        return QValidator::Acceptable;
    }

    virtual void fixup(QString& editorText) const
    {
        editorText = QString::number(m_nextValidValue);
    }

private:
    int computeNextValidId()
    {
        if (!m_usedIds.empty())
        {
            m_nextValidValue = *m_usedIds.rbegin();
        }
        else
        {
            m_nextValidValue = 1;
        }

        return m_nextValidValue;
    }

private:
    std::set<int> m_usedIds;

    int m_nextValidValue;
    bool m_multipleSelectionOfSameFieldsSelected;
    QString m_errorMessage;
};



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiLineEditor);


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
        bool isReadOnly = field()->isUiReadOnly(uiConfigName);
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

        m_lineEdit->setToolTip(field()->uiToolTip(uiConfigName));

        {
            PdmUiLineEditorAttribute leab;
            caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
            if (uiObject)
            {
                uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &leab);
            }

            if (leab.useRangeValidator)
            {
                m_lineEdit->setValidator(new QIntValidator(leab.minValue, leab.maxValue, this));
            }
        }

        {
            PdmUiLineEditorAttributeUniqueValues leab;
            caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
            if (uiObject)
            {
                uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &leab);
            }
            if (leab.usedIds.size() > 0)
            {
                if (isMultipleFieldsWithSameKeywordSelected(field()->fieldHandle()))
                {
                    QMessageBox::information(nullptr, "Invalid operation", "The field you are manipulating is defined to have unique values. A selection of multiple fields is detected. Please select a single item.");
                }

                m_lineEdit->setValidator(new PdmUniqueIdValidator(leab.usedIds, isMultipleFieldsWithSameKeywordSelected(field()->fieldHandle()), leab.errorMessage, this));
            }
        }


        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> enumNames = field()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        if (!enumNames.isEmpty() && fromMenuOnly == true)
        {
            int enumValue = field()->uiValue().toInt();

            if (enumValue < enumNames.size() && enumValue > -1)
            {
                m_lineEdit->setText(enumNames[enumValue].optionUiText());
            }
        }
        else
        {
            PdmUiLineEditorAttributeUiDisplayString leab;
            caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
            if (uiObject)
            {
                uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &leab);
            }

            QString displayString;
            if (leab.m_displayString.isEmpty())
            {
                displayString = field()->uiValue().toString();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiLineEditor::isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const
{
    std::vector<PdmFieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back(editorField);

    // For current selection, find all fields with same keyword
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems(items, SelectionManager::CURRENT);

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


} // end namespace caf
