//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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

    m_label->setVisible(!field()->isUiHidden(uiConfigName));
    m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));

    m_lineEdit->setEnabled(!field()->isUiReadOnly(uiConfigName));

    field()->ownerObject()->editorAttribute(field(), uiConfigName, &m_attributes);

    m_lineEdit->setText(field()->uiValue().toString());
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
    QToolButton* button = new QToolButton(parent);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    button->setText(QLatin1String("..."));
    layout->addWidget(m_lineEdit);
    layout->addWidget(button);

    connect(button, SIGNAL(clicked()), this, SLOT(fileSelectionClicked()));
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
            m_lineEdit->setText(directoryPath);
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
