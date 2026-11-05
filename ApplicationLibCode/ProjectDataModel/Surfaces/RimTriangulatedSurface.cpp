/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimTriangulatedSurface.h"

#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include "RimProject.h"
#include "RimSurfaceCollection.h"

#include "RifSurfaceExporter.h"
#include "RifSurfaceImporter.h"

#include "Surface/RigSurface.h"
#include "Surface/RigTriangleMeshData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimTriangulatedSurface, "TriangulatedSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTriangulatedSurface::RimTriangulatedSurface()
{
    CAF_PDM_InitScriptableObject( "Triangulated Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceFilePath, "SurfaceFilePath", "File" );
    m_surfaceFilePath.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTriangulatedSurface::~RimTriangulatedSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTriangulatedSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimTriangulatedSurface::createCopy()
{
    auto newSurface = copyObject<RimTriangulatedSurface>();
    if ( m_triangleMeshData ) newSurface->m_triangleMeshData = std::make_unique<RigTriangleMeshData>( *m_triangleMeshData );

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
void RimTriangulatedSurface::setGeometry( const std::vector<cvf::Vec3d>& vertices, const std::vector<unsigned>& triangleIndices )
{
    // Properties are per vertex, so a new geometry removes them
    m_triangleMeshData = std::make_unique<RigTriangleMeshData>();
    m_triangleMeshData->setGeometryData( vertices, triangleIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTriangulatedSurface::setProperty( const QString& key, const std::vector<float>& values )
{
    CAF_ASSERT( values.size() == vertexCount() );
    if ( !m_triangleMeshData ) return;

    auto                            names = m_triangleMeshData->propertyNames();
    std::vector<std::vector<float>> propertyValues;
    for ( const auto& name : names )
    {
        propertyValues.push_back( m_triangleMeshData->propertyValues( name ) );
    }

    auto it = std::find( names.begin(), names.end(), key );
    if ( it != names.end() )
    {
        propertyValues[std::distance( names.begin(), it )] = values;
    }
    else
    {
        names.push_back( key );
        propertyValues.push_back( values );
    }

    m_triangleMeshData->setPropertyData( names, propertyValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimTriangulatedSurface::vertexCount() const
{
    if ( !m_triangleMeshData ) return 0;

    return m_triangleMeshData->geometry().first.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RimTriangulatedSurface::getProperty( const QString& key ) const
{
    if ( !m_triangleMeshData ) return {};

    return m_triangleMeshData->propertyValues( key );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimTriangulatedSurface::propertyNames() const
{
    if ( !m_triangleMeshData ) return {};

    return m_triangleMeshData->propertyNames();
}

//--------------------------------------------------------------------------------------------------
/// Write the surface to the project folder before the project file is written
//--------------------------------------------------------------------------------------------------
void RimTriangulatedSurface::onProjectBeingSaved()
{
    const QString fileName = fileNameInProjectFolder();
    if ( fileName.isEmpty() ) return;

    if ( !m_triangleMeshData ) loadDataFromFile();
    if ( !m_triangleMeshData || m_triangleMeshData->geometry().first.empty() ) return;

    auto [vertices, triangleIndices] = m_triangleMeshData->geometry();

    auto                            names = m_triangleMeshData->propertyNames();
    std::vector<std::vector<float>> propertyValues;
    for ( const auto& name : names )
    {
        propertyValues.push_back( m_triangleMeshData->propertyValues( name ) );
    }

    if ( !RifSurfaceExporter::writeGocadTSurfFile( fileName, m_userDescription(), vertices, triangleIndices, names, propertyValues ) )
    {
        RiaLogging::error( std::format( "Could not write surface '{}' to {}", m_userDescription(), fileName ) );
        return;
    }

    m_surfaceFilePath = fileName;
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimTriangulatedSurface::updateSurfaceData()
{
    bool result = true;
    if ( !m_triangleMeshData )
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

    for ( const auto& name : m_triangleMeshData->propertyNames() )
    {
        auto values = m_triangleMeshData->propertyValues( name );
        surface->addVertexResult( name, values );
    }

    setSurfaceData( surface );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTriangulatedSurface::clearCachedNativeData()
{
    // The geometry only exists in memory until the project is saved
    if ( !m_surfaceFilePath().path().isEmpty() ) m_triangleMeshData.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimTriangulatedSurface::loadDataFromFile()
{
    QString path = m_surfaceFilePath().path();
    if ( path.isEmpty() ) return false;

    m_triangleMeshData = std::make_unique<RigTriangleMeshData>();
    RifSurfaceImporter::readGocadFile( path, m_triangleMeshData.get() );

    return !( m_triangleMeshData->geometry().first.empty() );
}

//--------------------------------------------------------------------------------------------------
/// The surface name, prefixed with the folder name for surfaces in a sub folder
//--------------------------------------------------------------------------------------------------
QString RimTriangulatedSurface::fileNameInProjectFolder() const
{
    const QString projectFileName = RimProject::current() ? RimProject::current()->fileName() : QString();
    if ( projectFileName.isEmpty() ) return {};

    QString name = m_userDescription();
    if ( auto folder = firstAncestorOrThisOfType<RimSurfaceCollection>() )
    {
        if ( folder->firstAncestorOfType<RimSurfaceCollection>() ) name = folder->collectionName() + "_" + name;
    }
    name.replace( QRegularExpression( "[^A-Za-z0-9._-]" ), "_" );

    return QFileInfo( projectFileName ).dir().absoluteFilePath( name + ".ts" );
}
