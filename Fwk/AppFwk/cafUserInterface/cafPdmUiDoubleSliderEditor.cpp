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


#include "cafPdmUiDoubleSliderEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleValidator>

#include <QDebug>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmDoubleValidator : public QDoubleValidator
{
public:
    explicit PdmDoubleValidator(QObject * parent = 0) : QDoubleValidator(parent)
    {
    }

    PdmDoubleValidator(double bottom, double top, int decimals, QObject * parent)
        : QDoubleValidator(bottom, top, decimals, parent)
    {
    }

    ~PdmDoubleValidator()
    {
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void fixup(QString& stringValue) const override
    {
        double doubleValue = stringValue.toDouble();
        doubleValue = qBound(bottom(), doubleValue, top());

        stringValue = QString::number(doubleValue, 'g', decimals());
    }
};

namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiDoubleSliderEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_lineEdit.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    m_lineEdit->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_slider->setEnabled(!field()->isUiReadOnly(uiConfigName));

    caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &m_attributes);
    }
    
    QString textValue = field()->uiValue().toString();

    m_slider->blockSignals(true);
    m_slider->setMaximum(m_attributes.m_sliderTickCount);
    m_slider->blockSignals(false);

    PdmDoubleValidator* pdmValidator = new PdmDoubleValidator(m_attributes.m_minimum, m_attributes.m_maximum, m_attributes.m_decimals, this);
    pdmValidator->fixup(textValue);
    
    m_lineEdit->setValidator(pdmValidator);
    m_lineEdit->setText(textValue);

    updateSliderPosition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleSliderEditor::createEditorWidget(QWidget * parent)
{
    QWidget* containerWidget = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    containerWidget->setLayout(layout);

    m_lineEdit = new QLineEdit(containerWidget);
    m_lineEdit->setMaximumWidth(100);
    connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    m_slider = new QSlider(Qt::Horizontal, containerWidget);

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_slider);

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));
    
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleSliderEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::slotEditingFinished()
{
    updateSliderPosition();

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::slotSliderValueChanged(int value)
{
    double newDoubleValue = convertFromSliderValue(value);
    m_lineEdit->setText(QString::number(newDoubleValue));

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::updateSliderPosition()
{
    QString textValue = m_lineEdit->text();

    bool convertOk = false;
    double newSliderValue = textValue.toDouble(&convertOk);

    int newSliderPosition = convertToSliderValue(newSliderValue);
    if (m_slider->value() != newSliderPosition)
    {
        m_slider->blockSignals(true);
        m_slider->setValue(newSliderPosition);
        m_slider->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::writeValueToField()
{
    QString textValue = m_lineEdit->text();
    QVariant v;
    v = textValue;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiDoubleSliderEditor::convertToSliderValue(double value)
{
    double exactSliderValue = m_slider->maximum() * (value - m_attributes.m_minimum) / (m_attributes.m_maximum - m_attributes.m_minimum);

    int sliderValue = static_cast<int>( exactSliderValue + 0.5);
    sliderValue = qBound(m_slider->minimum(), sliderValue, m_slider->maximum());

    return sliderValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double PdmUiDoubleSliderEditor::convertFromSliderValue(int sliderValue)
{
    double newDoubleValue = m_attributes.m_minimum + sliderValue * (m_attributes.m_maximum - m_attributes.m_minimum) / m_slider->maximum();
    newDoubleValue = qBound(m_attributes.m_minimum, newDoubleValue, m_attributes.m_maximum);
    
    return newDoubleValue;
}


} // end namespace caf
