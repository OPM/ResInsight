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

#include "RimGridCaseSurface.h"

#include "RigSurface.h"
#include "RimCase.h"
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimGridCaseSurface, "GridCaseSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCaseSurface::RimGridCaseSurface()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "SourceCase", "Source Case", "", "", "" );

    CAF_PDM_InitField( &m_sliceDirection,
                       "SnapShotDirection",
                       caf::AppEnum<RiaDefines::GridCaseAxis>( RiaDefines::GridCaseAxis::UNDEFINED_AXIS ),
                       "Range Filter Slice",
                       "",
                       "",
                       "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCaseSurface::~RimGridCaseSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCaseSurface::setCase( RimCase* sourceCase )
{
    m_case = sourceCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::loadData()
{
    return updateSurfaceDataFromFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCaseSurface::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_case )
    {
        RimTools::caseOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCaseSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    /*
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
    */
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::updateSurfaceDataFromFile()
{
    /*
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
    */

    return false;
}
