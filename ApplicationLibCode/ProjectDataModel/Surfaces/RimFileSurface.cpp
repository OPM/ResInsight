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

#include "RimFileSurface.h"

#include "RiaPreferences.h"

#include "RimSurfaceCollection.h"

#include "RifSurfaceImporter.h"
#include "RifVtkSurfaceImporter.h"

#include "Surface/RigSurface.h"
#include "Surface/RigTriangleMeshData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFileInfo>

#include <memory>

CAF_PDM_SOURCE_INIT( RimFileSurface, "Surface", "FileSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSurface::RimFileSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSurface::~RimFileSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSurface::setSurfaceFilePath( const QString& filePath )
{
    m_surfaceDefinitionFilePath = filePath;

    clearCachedNativeData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSurface::surfaceFilePath()
{
    return m_surfaceDefinitionFilePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFileSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimFileSurface::createCopy()
{
    auto newSurface = copyObject<RimFileSurface>();
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
void RimFileSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
bool RimFileSurface::updateSurfaceData()
{
    bool result = true;
    if ( !m_triangleMeshData || m_triangleMeshData->geometry().first.empty() )
    {
        result = loadDataFromFile();
    }

    if ( !m_triangleMeshData ) return false;

    auto [vertices, tringleIndices] = m_triangleMeshData->geometry();

    auto surface = new RigSurface;
    if ( !vertices.empty() && !tringleIndices.empty() )
    {
        RimSurface::applyDepthOffsetIfNeeded( &vertices );

        surface->setTriangleData( tringleIndices, vertices );
    }

    if ( m_triangleMeshData )
    {
        auto propertyNames = m_triangleMeshData->propertyNames();
        for ( const auto& name : propertyNames )
        {
            auto values = m_triangleMeshData->propertyValues( name );
            surface->addVertexResult( name, values );
        }
    }

    setSurfaceData( surface );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSurface::clearCachedNativeData()
{
    m_triangleMeshData.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFileSurface::loadDataFromFile()
{
    m_triangleMeshData = std::make_unique<RigTriangleMeshData>();

    QString filePath = surfaceFilePath();
    if ( filePath.endsWith( "ptl", Qt::CaseInsensitive ) )
    {
        auto surface = RifSurfaceImporter::readPetrelFile( filePath );
        m_triangleMeshData->setGeometryData( surface.first, surface.second );
    }
    else if ( filePath.endsWith( "ts", Qt::CaseInsensitive ) )
    {
        RifSurfaceImporter::readGocadFile( filePath, m_triangleMeshData.get() );
    }
    else if ( filePath.endsWith( "vtu", Qt::CaseInsensitive ) )
    {
        m_triangleMeshData = RifVtkSurfaceImporter::importFromFile( filePath.toStdString() );
    }
    else if ( filePath.endsWith( "dat", Qt::CaseInsensitive ) || filePath.endsWith( "xyz", Qt::CaseInsensitive ) )
    {
        double resamplingDistance = RiaPreferences::current()->surfaceImportResamplingDistance();
        auto   surface            = RifSurfaceImporter::readOpenWorksXyzFile( filePath, resamplingDistance );
        m_triangleMeshData->setGeometryData( surface.first, surface.second );
    }

    return !( m_triangleMeshData->geometry().first.empty() );
}
