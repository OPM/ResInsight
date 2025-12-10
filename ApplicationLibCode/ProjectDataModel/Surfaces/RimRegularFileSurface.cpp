/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimRegularFileSurface.h"

#include "RiaLogging.h"

#include "RifSurfio.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimRegularFileSurface, "RegularFileSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularFileSurface::RimRegularFileSurface()
{
    CAF_PDM_InitScriptableObject( "RegularFileSurface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceFilePath, "SurfaceFilePath", "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularFileSurface::setFilePath( const QString& filePath )
{
    m_surfaceFilePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRegularFileSurface::filePath() const
{
    return m_surfaceFilePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularFileSurface::onLoadData()
{
    auto surfaceData = RifSurfio::importSurfaceData( m_surfaceFilePath().path().toStdString() );
    if ( surfaceData.has_value() )
    {
        const auto [regularSurface, values] = surfaceData.value();

        setNx( regularSurface.nx );
        setNy( regularSurface.ny );
        setOriginX( regularSurface.originX );
        setOriginY( regularSurface.originY );
        setIncrementX( regularSurface.incrementX );
        setIncrementY( regularSurface.incrementY );
        setRotation( regularSurface.rotation );

        if ( !values.empty() )
        {
            const QString propertyName = "File Property Values";
            setProperty( propertyName, values );
        }

        RimRegularSurface::onLoadData();

        return true;
    }

    RiaLogging::error( QString::fromStdString( surfaceData.error() ) );

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularFileSurface::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_surfaceFilePath );

    setFieldsReadOnly( true );
    RimRegularSurface::defineUiOrdering( uiConfigName, uiOrdering );
}
