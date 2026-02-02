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

#include "RigModelPaddingSettings.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigModelPaddingSettings::RigModelPaddingSettings()
    : m_enabled( false )
    , m_nzUpper( 0 )
    , m_topUpper( 0.0 )
    , m_upperPorosity( 0.0 )
    , m_upperEquilnum( 1 )
    , m_nzLower( 0 )
    , m_bottomLower( 0.0 )
    , m_minLayerThickness( 0.1 )
    , m_fillGaps( false )
    , m_monotonicZcorn( false )
    , m_verticalPillars( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigModelPaddingSettings::isEnabled() const
{
    return m_enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setEnabled( bool enabled )
{
    m_enabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigModelPaddingSettings::nzUpper() const
{
    return m_nzUpper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setNzUpper( int value )
{
    m_nzUpper = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigModelPaddingSettings::topUpper() const
{
    return m_topUpper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setTopUpper( double value )
{
    m_topUpper = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigModelPaddingSettings::upperPorosity() const
{
    return m_upperPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setUpperPorosity( double value )
{
    m_upperPorosity = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigModelPaddingSettings::upperEquilnum() const
{
    return m_upperEquilnum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setUpperEquilnum( int value )
{
    m_upperEquilnum = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigModelPaddingSettings::nzLower() const
{
    return m_nzLower;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setNzLower( int value )
{
    m_nzLower = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigModelPaddingSettings::bottomLower() const
{
    return m_bottomLower;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setBottomLower( double value )
{
    m_bottomLower = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigModelPaddingSettings::minLayerThickness() const
{
    return m_minLayerThickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setMinLayerThickness( double value )
{
    m_minLayerThickness = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigModelPaddingSettings::fillGaps() const
{
    return m_fillGaps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setFillGaps( bool value )
{
    m_fillGaps = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigModelPaddingSettings::monotonicZcorn() const
{
    return m_monotonicZcorn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setMonotonicZcorn( bool value )
{
    m_monotonicZcorn = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigModelPaddingSettings::verticalPillars() const
{
    return m_verticalPillars;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigModelPaddingSettings::setVerticalPillars( bool value )
{
    m_verticalPillars = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigModelPaddingSettings::validate() const
{
    if ( !m_enabled )
    {
        return {};
    }

    QStringList errors;

    // Check upper padding
    if ( m_nzUpper < 0 )
    {
        errors << "Number of upper padding layers cannot be negative.";
    }

    if ( m_nzUpper > 0 )
    {
        if ( m_upperPorosity < 0.0 || m_upperPorosity > 1.0 )
        {
            errors << "Upper porosity must be between 0.0 and 1.0.";
        }

        if ( m_upperEquilnum < 1 )
        {
            errors << "Upper EQUILNUM must be at least 1.";
        }
    }

    // Check lower padding
    if ( m_nzLower < 0 )
    {
        errors << "Number of lower padding layers cannot be negative.";
    }

    // Check geometry options
    if ( m_minLayerThickness <= 0.0 )
    {
        errors << "Minimum layer thickness must be positive.";
    }

    // Check that at least one padding direction is specified
    if ( m_nzUpper == 0 && m_nzLower == 0 )
    {
        errors << "At least one padding direction (upper or lower) must have layers > 0.";
    }

    return errors.join( "\n" );
}
