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

#include "RigMainGrid.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"

#include "cafPdmUiSliderEditor.h"

#include "RigReservoirGridTools.h"
#include "cvfVector3.h"

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

    CAF_PDM_InitField( &m_sliceIndex, "SliceIndex", 1, "Slice Index", "", "", "" );
    m_sliceIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
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
bool RimGridCaseSurface::onLoadData()
{
    return updateSurfaceDataFromGridCase();
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
void RimGridCaseSurface::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
    if ( myAttr && m_case )
    {
        const cvf::StructGridInterface* grid = RigReservoirGridTools::mainGrid( m_case );
        if ( !grid ) return;

        myAttr->m_minimum = 1;

        if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_I )
        {
            myAttr->m_maximum = static_cast<int>( grid->cellCountI() );
        }
        else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_J )
        {
            myAttr->m_maximum = static_cast<int>( grid->cellCountJ() );
        }
        else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_K )
        {
            myAttr->m_maximum = static_cast<int>( grid->cellCountK() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCaseSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case || changedField == &m_sliceDirection || changedField == &m_sliceIndex )
    {
        updateSurfaceDataFromGridCase();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::updateSurfaceDataFromGridCase()
{
    RigSurface* surfaceData = nullptr;

    std::vector<unsigned>   tringleIndices;
    std::vector<cvf::Vec3d> vertices;

    cvf::StructGridInterface::FaceType faceType = cvf::StructGridInterface::NO_FACE;
    {
        if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_K )
        {
            faceType = cvf::StructGridInterface::NEG_K;
        }
        else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_J )
        {
            faceType = cvf::StructGridInterface::NEG_J;
        }
        else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_I )
        {
            faceType = cvf::StructGridInterface::NEG_I;
        }
    }

    if ( m_case && faceType != cvf::StructGridInterface::NO_FACE )
    {
        RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclCase && eclCase->mainGrid() )
        {
            {
                const RigMainGrid* grid = eclCase->mainGrid();

                size_t zeroBasedLayerIndex = static_cast<size_t>( m_sliceIndex - 1 );
                for ( size_t i = 0; i < grid->cellCountI(); i++ )
                {
                    for ( size_t j = 0; j < grid->cellCountJ(); j++ )
                    {
                        size_t cellIndex = grid->cellIndexFromIJK( i, j, zeroBasedLayerIndex );

                        if ( grid->cell( cellIndex ).isInvalid() ) continue;

                        cvf::Vec3d cornerVerts[8];
                        grid->cellCornerVertices( cellIndex, cornerVerts );

                        cvf::ubyte faceConn[4];
                        grid->cellFaceVertexIndices( faceType, faceConn );

                        cvf::uint triangleIndex = static_cast<cvf::uint>( vertices.size() );

                        for ( int n = 0; n < 4; n++ )
                        {
                            vertices.push_back( cornerVerts[faceConn[n]] );
                        }

                        tringleIndices.push_back( triangleIndex + 0 );
                        tringleIndices.push_back( triangleIndex + 1 );
                        tringleIndices.push_back( triangleIndex + 2 );

                        tringleIndices.push_back( triangleIndex + 0 );
                        tringleIndices.push_back( triangleIndex + 2 );
                        tringleIndices.push_back( triangleIndex + 3 );
                    }
                }
            }
        }
    }

    if ( !tringleIndices.empty() )
    {
        surfaceData = new RigSurface;

        {
            // Modify the z-value slightly to avoid geometrical numerical issues when the surface intersects exactly at
            // the cell face

            double delta = 1.0e-5;

            cvf::Vec3d offset = cvf::Vec3d::ZERO;

            if ( m_sliceDirection == RiaDefines::GridCaseAxis::AXIS_I )
            {
                offset.x() += delta;
            }
            else if ( m_sliceDirection == RiaDefines::GridCaseAxis::AXIS_J )
            {
                offset.y() += delta;
            }
            if ( m_sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
            {
                offset.z() += delta;
            }

            // Include the potential depth offset in the base class
            offset.z() += depthOffset();

            RimSurface::applyDepthOffset( offset, &vertices );
        }

        surfaceData->setTriangleData( tringleIndices, vertices );
    }

    setSurfaceData( surfaceData );

    return true;
}
