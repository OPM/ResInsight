/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimFishbonesPipeProperties.h"

#include "RiaEclipseUnitTools.h"

#include "RimWellPath.h"

#include <cstdlib>

CAF_PDM_SOURCE_INIT( RimFishbonesPipeProperties, "FishbonesPipeProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesPipeProperties::RimFishbonesPipeProperties()
{
    CAF_PDM_InitObject( "FishbonesPipeProperties" );

    CAF_PDM_InitField( &m_lateralHoleDiameter, "LateralHoleDiameter", 12.5, "Hole Diameter [mm]" );
    CAF_PDM_InitField( &m_skinFactor, "SkinFactor", 0.0, "Skin Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesPipeProperties::setHoleDiameter( const double& diameter )
{
    m_lateralHoleDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesPipeProperties::skinFactor() const
{
    return m_skinFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesPipeProperties::setSkinFactor( const double& skinFactor )
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesPipeProperties::holeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            return RiaEclipseUnitTools::inchToMeter( m_lateralHoleDiameter() );
        }
        else
        {
            return m_lateralHoleDiameter() / 1000;
        }
    }
    else if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            return RiaEclipseUnitTools::meterToFeet( m_lateralHoleDiameter() / 1000 );
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet( m_lateralHoleDiameter() );
        }
    }
    CVF_ASSERT( false );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesPipeProperties::holeDiameter() const
{
    return m_lateralHoleDiameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesPipeProperties::setUnitSystemSpecificDefaults()
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_lateralHoleDiameter = 12.5;
        }
        else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_lateralHoleDiameter = 0.5;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesPipeProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_lateralHoleDiameter.uiCapability()->setUiName( "Hole Diameter [mm]" );
        }
        else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_lateralHoleDiameter.uiCapability()->setUiName( "Hole Diameter [in]" );
        }
    }

    uiOrdering.add( &m_lateralHoleDiameter );
    uiOrdering.add( &m_skinFactor );
}
