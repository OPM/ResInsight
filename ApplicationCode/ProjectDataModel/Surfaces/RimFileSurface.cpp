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

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SurfaceUserDecription", "Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File", "", "", "" );
    CAF_PDM_InitField( &m_color, "SurfaceColor", cvf::Color3f( 0.5f, 0.3f, 0.2f ), "Color", "", "", "" );
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
    if ( m_userDescription().isEmpty() )
    {
        m_userDescription = QFileInfo( filePath ).fileName();
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
void RimFileSurface::setColor( const cvf::Color3f& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFileSurface::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSurface::userDescription()
{
    return m_userDescription();
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
RigSurface* RimFileSurface::surfaceData()
{
    return m_surfaceData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFileSurface::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( changedField == &m_surfaceDefinitionFilePath )
    {
        updateSurfaceDataFromFile();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
    else if ( changedField == &m_color )
    {
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
        m_surfaceData = new RigSurface();
        m_surfaceData->setTriangleData( tringleIndices, vertices );

        return true;
    }

    return false;
}
