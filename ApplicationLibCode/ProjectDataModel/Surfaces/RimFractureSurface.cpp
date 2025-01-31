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

#include "RimFractureSurface.h"

#include "RiaPreferences.h"

#include "RimSurfaceCollection.h"

#include "RifSurfaceImporter.h"
#include "RifVtkSurfaceImporter.h"

#include "RigGocadData.h"
#include "RigSurface.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFileInfo>
#include <memory>

CAF_PDM_SOURCE_INIT( RimFractureSurface, "RimFractureSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureSurface::RimFractureSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureSurface::~RimFractureSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::setSurfaceFilePath( const QString& filePath )
{
    m_surfaceDefinitionFilePath = filePath;

    clearCachedNativeData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureSurface::surfaceFilePath()
{
    return m_surfaceDefinitionFilePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimFractureSurface::createCopy()
{
    auto newSurface = copyObject<RimFractureSurface>();
    if ( !newSurface->onLoadData() )
    {
        delete newSurface;
        return nullptr;
    }

    return newSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::loadSurfaceDataForTimeStep( int timeStep )
{
    if ( m_surfacePerTimeStep.empty() )
    {
        loadDataFromFile();
    }

    if ( timeStep >= m_surfacePerTimeStep.size() ) return;

    auto surface = new RigSurface;

    auto gocadData                     = m_surfacePerTimeStep[timeStep];
    const auto& [coordinates, indices] = gocadData.gocadGeometry();

    surface->setTriangleData( indices, coordinates );
    auto propertyNames = gocadData.propertyNames();
    for ( const auto& name : propertyNames )
    {
        auto values = gocadData.propertyValues( name );
        surface->addVerticeResult( name, values );
    }
    setSurfaceData( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_surfaceDefinitionFilePath )
    {
        clearCachedNativeData();
        updateSurfaceData();

        auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
        surfColl->updateViews( { this } );
    }
}

//--------------------------------------------------------------------------------------------------
/// Regenerate the surface geometry, using the offset specified.
/// If the surface data hasn't been loaded from file yet, load it.
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::updateSurfaceData()
{
    loadSurfaceDataForTimeStep( 0 );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::clearCachedNativeData()
{
    m_secondsSinceSimulationStart.clear();
    m_surfacePerTimeStep.clear();

    /*
        m_vertices.clear();
        m_tringleIndices.clear();
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::loadDataFromFile()
{
    auto surfaceInfo = RifVtkSurfaceImporter::parsePvdDatasets( m_surfaceDefinitionFilePath().path().toStdString() );

    for ( const auto& s : surfaceInfo )
    {
        RigGocadData gocadData;
        if ( RifVtkSurfaceImporter::importFromFile( s.filename, &gocadData ) )
        {
            m_secondsSinceSimulationStart.push_back( s.timestep );
            m_surfacePerTimeStep.push_back( gocadData );
        }
    }

    return false;
}
