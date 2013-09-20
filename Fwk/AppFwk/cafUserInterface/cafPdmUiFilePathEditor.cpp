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


#include "cafPdmUiFilePathEditor.h"

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
#include "qboxlayout.h"
#include "qtoolbutton.h"
#include "qfiledialog.h"



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiFilePathEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFilePathEditor::configureAndUpdateUi(const QString& uiConfigName)
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

    field()->ownerObject()->editorAttribute(field(), uiConfigName, &m_attributes);

    m_lineEdit->setText(field()->uiValue().toString());

    if (m_attributes.m_appendUiSelectedFolderToText)
    {
        m_label->setToolTip("Define multiple directories separated by ';'");
        m_button->setText(QLatin1String("Append"));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiFilePathEditor::createEditorWidget(QWidget * parent)
{
    QWidget* placeholder = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout(placeholder);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_lineEdit = new QLineEdit(parent);
    m_button = new QToolButton(parent);
    m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    m_button->setText(QLatin1String("..."));

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_button);

    connect(m_button, SIGNAL(clicked()), this, SLOT(fileSelectionClicked()));
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    return placeholder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiFilePathEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFilePathEditor::slotEditingFinished()
{
    QVariant v;
    QString textValue = m_lineEdit->text();
    v = textValue;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFilePathEditor::fileSelectionClicked()
{
    QString defaultPath;
    if ( m_lineEdit->text().isEmpty())
    {
        defaultPath = QDir::homePath();
    }
    else
    {
        defaultPath = m_lineEdit->text();
    }

    if (m_attributes.m_selectDirectory)
    {
        QString directoryPath = QFileDialog::getExistingDirectory(m_lineEdit, 
                                                tr("Get existing directory"),
                                                defaultPath,
                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
        if (!directoryPath.isEmpty())
        {
            QString filePathString;
            if (m_attributes.m_appendUiSelectedFolderToText)
            {
                filePathString = m_lineEdit->text();
                if (!filePathString.isEmpty() && !filePathString.endsWith(m_attributes.m_multipleItemSeparator, Qt::CaseInsensitive))
                {
                    filePathString += m_attributes.m_multipleItemSeparator;
                }
                filePathString += directoryPath;
            }
            else
            {
                filePathString = directoryPath;
            }

            m_lineEdit->setText(filePathString);
            slotEditingFinished();
        }
    }
    else
    {
        QString filePath;
        if (m_attributes.m_selectSaveFileName)
        {
            filePath = QFileDialog::getSaveFileName(m_lineEdit, tr("Save File"), defaultPath, m_attributes.m_fileSelectionFilter);
        }
        else
        {
            filePath = QFileDialog::getOpenFileName(m_lineEdit, tr("Choose a file"), defaultPath, m_attributes.m_fileSelectionFilter);
        }

        if (!filePath.isEmpty())
        {
            m_lineEdit->setText(filePath);
            slotEditingFinished();
        }
    }
}


} // end namespace caf
