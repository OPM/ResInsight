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

#include "cafPdmField.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafQShortenedLabel.h"

#include <QDoubleValidator>
#include <QHBoxLayout>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiDoubleSliderEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDoubleSliderEditor::PdmUiDoubleSliderEditor()
    : m_sliderValue( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDoubleSliderEditor::~PdmUiDoubleSliderEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_lineEdit.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_lineEdit->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_slider->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );

    caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &m_attributes );
    }

    double  doubleValue = uiField()->uiValue().toDouble();
    QString textValue   = uiField()->uiValue().toString();

    m_slider->blockSignals( true );
    m_slider->setMaximum( m_attributes.m_sliderTickCount );
    m_slider->blockSignals( false );

    PdmDoubleValidator* pdmValidator =
        new PdmDoubleValidator( m_attributes.m_minimum, m_attributes.m_maximum, m_attributes.m_decimals, this );
    pdmValidator->fixup( textValue );

    m_lineEdit->setValidator( pdmValidator );
    m_lineEdit->setText( textValue );

    m_sliderValue = doubleValue;
    PdmUiSliderTools::updateSliderPosition( m_slider, doubleValue, m_attributes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleSliderEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    containerWidget->setLayout( layout );

    m_lineEdit = new QLineEdit( containerWidget );
    m_lineEdit->setMaximumWidth( 100 );
    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    m_slider = new QSlider( Qt::Horizontal, containerWidget );

    layout->addWidget( m_lineEdit );
    layout->addWidget( m_slider );

    connect( m_slider, SIGNAL( valueChanged( int ) ), this, SLOT( slotSliderValueChanged( int ) ) );
    connect( m_slider, SIGNAL( sliderReleased() ), this, SLOT( slotSliderReleased() ) );
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleSliderEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::slotEditingFinished()
{
    QString textValue = m_lineEdit->text();

    double doubleVal = textValue.toDouble();
    doubleVal        = qBound( m_attributes.m_minimum, doubleVal, m_attributes.m_maximum );
    m_sliderValue    = doubleVal;

    writeValueToField( doubleVal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::slotSliderValueChanged( int value )
{
    double newDoubleValue = PdmUiSliderTools::convertFromSliderValue( m_slider, value, m_attributes );
    m_sliderValue         = newDoubleValue;

    if ( m_attributes.m_delaySliderUpdateUntilRelease )
    {
        m_lineEdit->setText( QString( "%1" ).arg( m_sliderValue ) );
    }
    else
    {
        writeValueToField( m_sliderValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::slotSliderReleased()
{
    writeValueToField( m_sliderValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleSliderEditor::writeValueToField( double value )
{
    QVariant v = value;
    this->setValueToField( v );
}

} // end namespace caf
