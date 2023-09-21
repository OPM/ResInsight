//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2023 Ceetron Solutions AS
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

#include "cafPdmUiValueRangeEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafQShortenedLabel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmDoubleValidator : public QDoubleValidator
{
public:
    explicit PdmDoubleValidator( QObject* parent = nullptr )
        : QDoubleValidator( parent )
    {
    }

    PdmDoubleValidator( double bottom, double top, int decimals, QObject* parent )
        : QDoubleValidator( bottom, top, decimals, parent )
    {
    }

    ~PdmDoubleValidator() override {}

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void fixup( QString& stringValue ) const override
    {
        double doubleValue = stringValue.toDouble();
        doubleValue        = qBound( bottom(), doubleValue, top() );

        stringValue = QString::number( doubleValue, 'g', decimals() );
    }
};

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiValueRangeEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_lineEditMin->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_sliderMin->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_lineEditMax->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_sliderMax->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );

    caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &m_attributes );
    }

    double firstValue  = 0.0;
    double secondValue = 0.0;
    {
        // A pair is converted into a list of QVariant in PdmValueFieldSpecialization<std::pair<T, U>>
        auto variantValue = uiField()->uiValue();
        if ( variantValue.canConvert<QList<QVariant>>() )
        {
            QList<QVariant> lst = variantValue.toList();
            if ( lst.size() == 2 )
            {
                firstValue  = lst[0].toDouble();
                secondValue = lst[1].toDouble();
            }
        }
    }

    {
        m_sliderMin->blockSignals( true );
        m_sliderMin->setMaximum( m_attributes.m_sliderTickCount );
        m_sliderMin->blockSignals( false );

        QString textValueMin = QString( "%1" ).arg( firstValue );

        PdmDoubleValidator* pdmValidator =
            new PdmDoubleValidator( m_attributes.m_minimum, m_attributes.m_maximum, m_attributes.m_decimals, this );
        pdmValidator->fixup( textValueMin );

        m_lineEditMin->setValidator( pdmValidator );
        m_lineEditMin->setText( textValueMin );

        m_sliderValueMin = firstValue;
        updateSliderPosition( m_sliderMin, firstValue, m_attributes );
    }

    {
        m_sliderMax->blockSignals( true );
        m_sliderMax->setMaximum( m_attributes.m_sliderTickCount );
        m_sliderMax->blockSignals( false );

        QString textValueMax = QString( "%1" ).arg( secondValue );

        PdmDoubleValidator* pdmValidator =
            new PdmDoubleValidator( m_attributes.m_minimum, m_attributes.m_maximum, m_attributes.m_decimals, this );
        pdmValidator->fixup( textValueMax );

        m_lineEditMax->setValidator( pdmValidator );
        m_lineEditMax->setText( textValueMax );

        m_sliderValueMax = secondValue;
        updateSliderPosition( m_sliderMax, secondValue, m_attributes );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotMinEditingFinished()
{
    QString minText = m_lineEditMin->text();
    QString maxText = m_lineEditMax->text();

    double minValue = minText.toDouble();
    double maxValue = maxText.toDouble();

    clampAndWriteValues( minValue, maxValue, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotMaxEditingFinished()
{
    QString minText = m_lineEditMin->text();
    QString maxText = m_lineEditMax->text();

    double minValue = minText.toDouble();
    double maxValue = maxText.toDouble();

    clampAndWriteValues( minValue, maxValue, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotMinSliderValueChanged( int value )
{
    double newDoubleValue = convertFromSliderValue( m_sliderMin, value, m_attributes );
    m_sliderValueMin      = newDoubleValue;

    if ( m_attributes.m_delaySliderUpdateUntilRelease )
    {
        m_lineEditMin->setText( QString( "%1" ).arg( m_sliderValueMin ) );
    }
    else
    {
        clampAndWriteValues( m_sliderValueMin, m_sliderValueMax, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotMaxSliderValueChanged( int value )
{
    double newDoubleValue = convertFromSliderValue( m_sliderMax, value, m_attributes );
    m_sliderValueMax      = newDoubleValue;

    if ( m_attributes.m_delaySliderUpdateUntilRelease )
    {
        m_lineEditMax->setText( QString( "%1" ).arg( m_sliderValueMax ) );
    }
    else
    {
        clampAndWriteValues( m_sliderValueMin, m_sliderValueMax, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotSliderReleasedMin()
{
    clampAndWriteValues( m_sliderValueMin, m_sliderValueMax, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::slotSliderReleasedMax()
{
    clampAndWriteValues( m_sliderValueMin, m_sliderValueMax, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::updateSliderPosition( QSlider*                                slider,
                                                  double                                  value,
                                                  const PdmUiDoubleSliderEditorAttribute& attributes )
{
    if ( !slider ) return;

    int newSliderPosition = convertToSliderValue( slider, value, attributes );
    if ( slider->value() != newSliderPosition )
    {
        slider->blockSignals( true );
        slider->setValue( newSliderPosition );
        slider->blockSignals( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::clampAndWriteValues( double valueMin, double valueMax, bool isMinChanged )
{
    if ( isMinChanged )
    {
        if ( valueMin > valueMax ) valueMax = valueMin;
    }
    else
    {
        if ( valueMax < valueMin ) valueMin = valueMax;
    }

    valueMin         = qBound( m_attributes.m_minimum, valueMin, valueMax );
    valueMax         = qBound( valueMin, valueMax, m_attributes.m_maximum );
    m_sliderValueMin = valueMin;
    m_sliderValueMax = valueMax;

    clampAndWriteValues( valueMin, valueMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiValueRangeEditor::clampAndWriteValues( double valueMin, double valueMax )
{
    auto pairValue = std::make_pair( valueMin, valueMax );

    QVariant v = caf::PdmValueFieldSpecialization<std::pair<double, double>>::convert( pairValue );
    setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiValueRangeEditor::convertToSliderValue( QSlider*                                slider,
                                                 double                                  value,
                                                 const PdmUiDoubleSliderEditorAttribute& attributes )
{
    double exactSliderValue =
        slider->maximum() * ( value - attributes.m_minimum ) / ( attributes.m_maximum - attributes.m_minimum );

    int sliderValue = static_cast<int>( exactSliderValue + 0.5 );
    sliderValue     = qBound( slider->minimum(), sliderValue, slider->maximum() );

    return sliderValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double PdmUiValueRangeEditor::convertFromSliderValue( QSlider*                                slider,
                                                      int                                     sliderValue,
                                                      const PdmUiDoubleSliderEditorAttribute& attributes )
{
    double clampedValue = attributes.m_minimum +
                          sliderValue * ( attributes.m_maximum - attributes.m_minimum ) / slider->maximum();
    clampedValue = qBound( attributes.m_minimum, clampedValue, attributes.m_maximum );

    return clampedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiValueRangeEditor::PdmUiValueRangeEditor()
    : m_sliderValueMin( 0.0 )
    , m_sliderValueMax( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiValueRangeEditor::~PdmUiValueRangeEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiValueRangeEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    auto layout = new QGridLayout();
    layout->setMargin( 0 );
    containerWidget->setLayout( layout );

    {
        m_lineEditMin = new QLineEdit( containerWidget );
        m_lineEditMin->setMaximumWidth( 100 );
        connect( m_lineEditMin, SIGNAL( editingFinished() ), this, SLOT( slotMinEditingFinished() ) );

        m_sliderMin = new QSlider( Qt::Horizontal, containerWidget );

        layout->addWidget( m_lineEditMin, 0, 0 );
        layout->addWidget( m_sliderMin, 0, 1 );

        connect( m_sliderMin, SIGNAL( valueChanged( int ) ), this, SLOT( slotMinSliderValueChanged( int ) ) );
        connect( m_sliderMin, SIGNAL( sliderReleased() ), this, SLOT( slotSliderReleasedMin() ) );
    }

    {
        m_lineEditMax = new QLineEdit( containerWidget );
        m_lineEditMax->setMaximumWidth( 100 );
        connect( m_lineEditMax, SIGNAL( editingFinished() ), this, SLOT( slotMaxEditingFinished() ) );

        m_sliderMax = new QSlider( Qt::Horizontal, containerWidget );

        layout->addWidget( m_lineEditMax, 1, 0 );
        layout->addWidget( m_sliderMax, 1, 1 );

        connect( m_sliderMax, SIGNAL( valueChanged( int ) ), this, SLOT( slotMaxSliderValueChanged( int ) ) );
        connect( m_sliderMax, SIGNAL( sliderReleased() ), this, SLOT( slotSliderReleasedMax() ) );
    }
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiValueRangeEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

} // end namespace caf
