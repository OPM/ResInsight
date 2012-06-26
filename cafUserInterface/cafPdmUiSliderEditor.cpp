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

#include "cafPdmUiSliderEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QIntValidator>

#include <assert.h>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiSliderEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_lineEdit.isNull());

    m_groupBox->setTitle(field()->uiName(uiConfigName));

    m_lineEdit->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_slider->setEnabled(!field()->isUiReadOnly(uiConfigName));

    field()->ownerObject()->editorAttribute(field(), uiConfigName, &m_attributes);

    m_lineEdit->setText(field()->uiValue().toString());
    updateSliderPosition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiSliderEditor::createEditorWidget(QWidget * parent)
{
    m_groupBox = new QGroupBox(parent);

    QVBoxLayout* layout = new QVBoxLayout(parent);
    m_groupBox->setLayout(layout);

    m_lineEdit = new QLineEdit(m_groupBox);
    m_lineEdit->setValidator(new QIntValidator(m_attributes.m_minimum, m_attributes.m_maximum, m_groupBox));
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    m_slider = new QSlider(Qt::Horizontal, m_groupBox);
    m_slider->setRange(m_attributes.m_minimum, m_attributes.m_maximum);
    
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_slider);


    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));
    
    return m_groupBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::slotEditingFinished()
{
    updateSliderPosition();

    QString textValue = m_lineEdit->text();
    QVariant v;
    v = textValue;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::slotSliderValueChanged(int position)
{
    m_lineEdit->setText(QString::number(position));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::updateSliderPosition()
{
    QString textValue = m_lineEdit->text();

    bool convertOk = false;
    int newSliderValue = textValue.toInt(&convertOk);
    if (convertOk)
    {
        newSliderValue = qBound(m_attributes.m_minimum, newSliderValue, m_attributes.m_maximum);
        m_slider->setValue(newSliderValue);
    }
}


} // end namespace caf
