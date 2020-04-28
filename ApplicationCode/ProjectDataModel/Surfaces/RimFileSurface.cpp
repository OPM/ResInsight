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
#include "RigSurface.h"
#include "RimSurfaceCollection.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimFileSurface, "FileSurface" );

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
bool RimFileSurface::loadData()
{
    return updateSurfaceDataFromFile();
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
        updateSurfaceDataFromFile();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimFileSurface::updateSurfaceDataFromFile()
{
    QString filePath = this->surfaceFilePath();

    std::vector<unsigned>   tringleIndices;
    std::vector<cvf::Vec3d> vertices;

    if ( filePath.endsWith( "ptl", Qt::CaseInsensitive ) )
    {
        auto surface = RifSurfaceReader::readPetrelFile( filePath );

        vertices       = surface.first;
        tringleIndices = surface.second;
    }
    else if ( filePath.endsWith( "ts", Qt::CaseInsensitive ) )
    {
        auto surface = RifSurfaceReader::readGocadFile( filePath );

        vertices       = surface.first;
        tringleIndices = surface.second;
    }

    if ( !vertices.empty() && !tringleIndices.empty() )
    {
        auto surface = new RigSurface();
        surface->setTriangleData( tringleIndices, vertices );

        setSurfaceData( surface );

        return true;
    }

    return false;
}
