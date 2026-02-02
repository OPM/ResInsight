/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>

//==================================================================================================
///
/// Settings for model padding (adding Z-direction layers to sector models)
///
//==================================================================================================
class RigModelPaddingSettings
{
public:
    RigModelPaddingSettings();

    // Enable/disable
    bool isEnabled() const;
    void setEnabled( bool enabled );

    // Upper padding
    int    nzUpper() const;
    void   setNzUpper( int value );
    double topUpper() const;
    void   setTopUpper( double value );
    double upperPorosity() const;
    void   setUpperPorosity( double value );
    int    upperEquilnum() const;
    void   setUpperEquilnum( int value );

    // Lower padding
    int    nzLower() const;
    void   setNzLower( int value );
    double bottomLower() const;
    void   setBottomLower( double value );

    // Geometry options
    double minLayerThickness() const;
    void   setMinLayerThickness( double value );
    bool   fillGaps() const;
    void   setFillGaps( bool value );
    bool   monotonicZcorn() const;
    void   setMonotonicZcorn( bool value );
    bool   verticalPillars() const;
    void   setVerticalPillars( bool value );

    // Validation
    QString validate() const;

private:
    bool   m_enabled;
    int    m_nzUpper;
    double m_topUpper;
    double m_upperPorosity;
    int    m_upperEquilnum;
    int    m_nzLower;
    double m_bottomLower;
    double m_minLayerThickness;
    bool   m_fillGaps;
    bool   m_monotonicZcorn;
    bool   m_verticalPillars;
};
