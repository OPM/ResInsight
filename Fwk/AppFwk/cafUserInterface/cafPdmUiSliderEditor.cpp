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


#include "cafPdmUiSliderEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"
#include "cafQShortenedLabel.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QIntValidator>



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiSliderEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_spinBox.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    m_spinBox->setEnabled(!uiField()->isUiReadOnly(uiConfigName));
    m_spinBox->setToolTip(uiField()->uiToolTip(uiConfigName));

    m_slider->setEnabled(!uiField()->isUiReadOnly(uiConfigName));
    m_slider->setToolTip(uiField()->uiToolTip(uiConfigName));

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }

    {
        m_spinBox->blockSignals(true);
        m_spinBox->setMinimum(m_attributes.m_minimum);
        m_spinBox->setMaximum(m_attributes.m_maximum);
        
        QString textValue = uiField()->uiValue().toString();
        m_spinBox->setValue(textValue.toInt());
        m_spinBox->blockSignals(false);
    }

    {
        m_slider->blockSignals(true);
        m_slider->setRange(m_attributes.m_minimum, m_attributes.m_maximum);
        updateSliderPosition();
        m_slider->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiSliderEditor::createEditorWidget(QWidget * parent)
{
    QWidget* containerWidget = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    containerWidget->setLayout(layout);

    m_spinBox = new QSpinBox(containerWidget);
    m_spinBox->setMaximumWidth(60);
    m_spinBox->setKeyboardTracking(false);
    connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(slotSpinBoxValueChanged(int)));

    m_slider = new QSlider(Qt::Horizontal, containerWidget);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_slider);

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));
    
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiSliderEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QShortenedLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::slotSliderValueChanged(int position)
{
    m_spinBox->setValue(position);

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::slotSpinBoxValueChanged(int spinBoxValue)
{
    updateSliderPosition();

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::updateSliderPosition()
{
    QString textValue = m_spinBox->text();

    bool convertOk = false;
    int newSliderValue = textValue.toInt(&convertOk);
    if (convertOk)
    {
        newSliderValue = qBound(m_attributes.m_minimum, newSliderValue, m_attributes.m_maximum);
        m_slider->setValue(newSliderValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSliderEditor::writeValueToField()
{
    QString textValue = m_spinBox->text();
    QVariant v;
    v = textValue;
    this->setValueToField(v);
}


} // end namespace caf
