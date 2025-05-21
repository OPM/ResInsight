/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimSurfaceResultDefinition.h"

#include "RiaResultNames.h"

#include "Surface/RigSurface.h"

#include "Rim3dView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"

CAF_PDM_SOURCE_INIT( RimSurfaceResultDefinition, "SurfaceResultDefinition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceResultDefinition::RimSurfaceResultDefinition()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_propertyName, "PropertyName", "Property Name" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "Legend" );
    m_legendConfig.uiCapability()->setUiTreeChildrenHidden( false );
    m_legendConfig = new RimRegularLegendConfig;

    setName( "Surface Result" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceInView, "SurfaceInView", "Surface In View" );
    m_surfaceInView.uiCapability()->setUiHidden( true );
    m_surfaceInView.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceResultDefinition::~RimSurfaceResultDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceResultDefinition::setSurfaceInView( RimSurfaceInView* surfaceInView )
{
    m_surfaceInView = surfaceInView;

    assignDefaultProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceResultDefinition::propertyName() const
{
    return m_propertyName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSurfaceResultDefinition::legendConfig()
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceResultDefinition::updateMinMaxValues( int currentTimeStep )
{
    if ( !m_surfaceInView ) return;

    if ( currentTimeStep < 0 ) currentTimeStep = 0;

    if ( auto surface = m_surfaceInView->surface() ) surface->updateMinMaxValues( m_legendConfig, m_propertyName, currentTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceResultDefinition::assignDefaultProperty()
{
    if ( auto surface = surfaceData() )
    {
        auto propNames = surface->propertyNames();
        if ( !propNames.empty() )
        {
            m_propertyName = propNames.front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceResultDefinition::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_propertyName )
    {
        updateMinMaxValues( -1 );
    }

    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( view )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceResultDefinition::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_propertyName );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSurfaceResultDefinition::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_propertyName )
    {
        options.push_back( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), RiaResultNames::undefinedResultName() ) );

        RigSurface* surfData = surfaceData();
        if ( surfData )
        {
            auto propertyNames = surfData->propertyNames();
            for ( auto name : propertyNames )
            {
                options.push_back( caf::PdmOptionItemInfo( name, name ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface* RimSurfaceResultDefinition::surfaceData()
{
    if ( m_surfaceInView && m_surfaceInView->surface() && m_surfaceInView->surface()->surfaceData() )
    {
        return m_surfaceInView->surface()->surfaceData();
    }

    return nullptr;
}
