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

#include "cafPdmUiSliderTools.h"

#include <QSlider>
#include <cmath>

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDoubleValidator::PdmDoubleValidator( QObject* parent /*= nullptr */ )
    : QDoubleValidator( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDoubleValidator::PdmDoubleValidator( double bottom, double top, int decimals, QObject* parent )
    : QDoubleValidator( bottom, top, decimals, parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDoubleValidator::fixup( QString& stringValue ) const
{
    double doubleValue = stringValue.toDouble();

    if ( std::isinf( doubleValue ) || std::isnan( doubleValue ) )
    {
        return;
    }

    doubleValue = qBound( bottom(), doubleValue, top() );
    stringValue = QString::number( doubleValue, 'g', decimals() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiSliderTools::updateSliderPosition( QSlider* slider, double value, const PdmUiDoubleSliderEditorAttribute& attributes )
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
int PdmUiSliderTools::convertToSliderValue( QSlider* slider, double value, const PdmUiDoubleSliderEditorAttribute& attributes )
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
double PdmUiSliderTools::convertFromSliderValue( QSlider*                                slider,
                                                 int                                     sliderValue,
                                                 const PdmUiDoubleSliderEditorAttribute& attributes )
{
    double clampedValue = attributes.m_minimum +
                          sliderValue * ( attributes.m_maximum - attributes.m_minimum ) / slider->maximum();
    clampedValue = qBound( attributes.m_minimum, clampedValue, attributes.m_maximum );

    return clampedValue;
}

} //namespace caf
