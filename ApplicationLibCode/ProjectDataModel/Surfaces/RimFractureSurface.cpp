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
    bool result = true;
    /*
        if ( m_vertices.empty() )
        {
            result = loadDataFromFile();
        }

        std::vector<cvf::Vec3d> vertices{ m_vertices };
        std::vector<unsigned>   tringleIndices{ m_tringleIndices };

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
    */

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::clearCachedNativeData()
{
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
    return false;

    /*
    std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> surface;

    QString filePath = surfaceFilePath();
    if ( filePath.endsWith( "ptl", Qt::CaseInsensitive ) )
    {
        surface = RifSurfaceImporter::readPetrelFile( filePath );
    }
    else if ( filePath.endsWith( "ts", Qt::CaseInsensitive ) )
    {
        m_gocadData = std::make_unique<RigGocadData>();

        RifSurfaceImporter::readGocadFile( filePath, m_gocadData.get() );

        surface = m_gocadData->gocadGeometry();
    }
    else if ( filePath.endsWith( "vtu", Qt::CaseInsensitive ) )
    {
        m_gocadData = std::make_unique<RigGocadData>();
        RifVtkSurfaceImporter::importFromFile( filePath.toStdString(), m_gocadData.get() );

        surface = m_gocadData->gocadGeometry();
    }
    else if ( filePath.endsWith( "dat", Qt::CaseInsensitive ) || filePath.endsWith( "xyz", Qt::CaseInsensitive ) )
    {
        double resamplingDistance = RiaPreferences::current()->surfaceImportResamplingDistance();
        surface                   = RifSurfaceImporter::readOpenWorksXyzFile( filePath, resamplingDistance );
    }

    m_vertices       = surface.first;
    m_tringleIndices = surface.second;

    return !( m_vertices.empty() || m_tringleIndices.empty() );
*/
}
