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

#pragma once

#include "cafPdmUiFieldEditorHandle.h"

#include <QValidator>

class QSlider;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmDoubleValidator : public QDoubleValidator
{
public:
    explicit PdmDoubleValidator( QObject* parent = nullptr );
    explicit PdmDoubleValidator( double bottom, double top, int decimals, QObject* parent );

    void fixup( QString& stringValue ) const override;
};

//==================================================================================================
///
//==================================================================================================
class PdmUiDoubleSliderEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiDoubleSliderEditorAttribute()
    {
        m_minimum                       = 0;
        m_maximum                       = 10;
        m_decimals                      = 6;
        m_sliderTickCount               = 2000;
        m_delaySliderUpdateUntilRelease = false;
    }

public:
    double m_minimum;
    double m_maximum;
    int    m_decimals;
    int    m_sliderTickCount;
    bool   m_delaySliderUpdateUntilRelease;
};

class PdmUiSliderTools
{
public:
    static void updateSliderPosition( QSlider* slider, double value, const PdmUiDoubleSliderEditorAttribute& attributes );
    static int convertToSliderValue( QSlider* slider, double value, const PdmUiDoubleSliderEditorAttribute& attributes );
    static double
        convertFromSliderValue( QSlider* slider, int sliderValue, const PdmUiDoubleSliderEditorAttribute& attributes );
};

} //namespace caf
