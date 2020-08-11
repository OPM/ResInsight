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

#include "RifSurfaceReader.h"
#include "RigGocadData.h"
#include "RigSurface.h"
#include "RimSurfaceCollection.h"

#include <QFileInfo>

// TODO: Use the alias concept prototyped below when the alias concept for class is ready
// CAF_PDM_SOURCE_INIT( RimFileSurface, "FileSurface", "Surface" );
// CAF_PDM_SOURCE_INIT( <class>       ,  <keyword>   , <alias>);
CAF_PDM_SOURCE_INIT( RimFileSurface, "Surface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSurface::RimFileSurface()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File", "", "", "" );
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
    if ( userDescription().isEmpty() )
    {
        setUserDescription( QFileInfo( filePath ).fileName() );
    }

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
void RimFileSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_surfaceDefinitionFilePath )
    {
        clearCachedNativeData();
        updateSurfaceData();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
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
    if ( m_vertices.empty() )
    {
        result = loadDataFromFile();
    }

    std::vector<cvf::Vec3d> vertices{m_vertices};
    std::vector<unsigned>   tringleIndices{m_tringleIndices};

    auto surface = new RigSurface;
    if ( !vertices.empty() && !tringleIndices.empty() )
    {
        RimSurface::applyDepthOffsetIfNeeded( &vertices );

        surface->setTriangleData( tringleIndices, vertices );
    }

    if ( m_gocadData )
    {
        auto propertyNames = m_gocadData->propertyNames();
        for ( const auto& name : propertyNames )
        {
            auto values = m_gocadData->propertyValues( name );
            surface->addVerticeResult( name, values );
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
    m_vertices.clear();
    m_tringleIndices.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFileSurface::loadDataFromFile()
{
    std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> surface;

    QString filePath = this->surfaceFilePath();
    if ( filePath.endsWith( "ptl", Qt::CaseInsensitive ) )
    {
        surface = RifSurfaceReader::readPetrelFile( filePath );
    }
    else if ( filePath.endsWith( "ts", Qt::CaseInsensitive ) )
    {
        m_gocadData.reset( new RigGocadData );

        RifSurfaceReader::readGocadFile( filePath, m_gocadData.get() );

        surface = m_gocadData->gocadGeometry();
    }

    m_vertices       = surface.first;
    m_tringleIndices = surface.second;

    if ( m_vertices.empty() || m_tringleIndices.empty() ) return false;

    return true;
}
