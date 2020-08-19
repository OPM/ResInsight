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

    CAF_PDM_InitField( &m_oneBasedSliceIndex, "SliceIndex", 1, "Slice Index", "", "", "" );
    m_oneBasedSliceIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
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
void RimGridCaseSurface::setSliceTypeAndOneBasedIndex( RiaDefines::GridCaseAxis sliceType, int oneBasedSliceIndex )
{
    m_sliceDirection     = sliceType;
    m_oneBasedSliceIndex = oneBasedSliceIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimGridCaseSurface::createCopy()
{
    return nullptr;
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
    RimSurface::defineEditorAttribute( field, uiConfigName, attribute );

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

    if ( changedField == &m_case || changedField == &m_sliceDirection || changedField == &m_oneBasedSliceIndex )
    {
        clearCachedNativeData();
        updateSurfaceData();

        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( {this} );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCaseSurface::extractDataFromGrid()
{
    clearCachedNativeData();

    if ( m_sliceDirection() == RiaDefines::GridCaseAxis::UNDEFINED_AXIS ) return;

    if ( m_case )
    {
        RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclCase && eclCase->mainGrid() )
        {
            const RigMainGrid* grid = eclCase->mainGrid();

            size_t minI = 0;
            size_t minJ = 0;
            size_t minK = 0;
            size_t maxI = grid->cellCountI();
            size_t maxJ = grid->cellCountJ();
            size_t maxK = grid->cellCountK();

            size_t zeroBasedLayerIndex = static_cast<size_t>( m_oneBasedSliceIndex - 1 );

            cvf::StructGridInterface::FaceType faceType = cvf::StructGridInterface::NO_FACE;
            {
                if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_K )
                {
                    faceType = cvf::StructGridInterface::NEG_K;

                    minK = zeroBasedLayerIndex;
                    maxK = zeroBasedLayerIndex + 1;
                }
                else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_J )
                {
                    faceType = cvf::StructGridInterface::NEG_J;
                    minJ     = zeroBasedLayerIndex;
                    maxJ     = zeroBasedLayerIndex + 1;
                }
                else if ( m_sliceDirection() == RiaDefines::GridCaseAxis::AXIS_I )
                {
                    faceType = cvf::StructGridInterface::NEG_I;
                    minI     = zeroBasedLayerIndex;
                    maxI     = zeroBasedLayerIndex + 1;
                }
            }

            std::vector<unsigned>                      tringleIndices;
            std::vector<cvf::Vec3d>                    vertices;
            std::vector<std::pair<unsigned, unsigned>> structGridVertexIndices;

            for ( size_t i = minI; i < maxI; i++ )
            {
                for ( size_t j = minJ; j < maxJ; j++ )
                {
                    for ( size_t k = minK; k < maxK; k++ )
                    {
                        std::pair<unsigned, unsigned> quadIJIndices;

                        switch ( faceType )
                        {
                            case cvf::StructGridInterface::NEG_I:
                                quadIJIndices = std::make_pair( j, k );
                                break;
                            case cvf::StructGridInterface::NEG_J:
                                quadIJIndices = std::make_pair( i, k );
                                break;
                            case cvf::StructGridInterface::NEG_K:
                                quadIJIndices = std::make_pair( i, j );
                                break;
                        }

                        size_t cellIndex = grid->cellIndexFromIJK( i, j, k );

                        if ( grid->cell( cellIndex ).isInvalid() ) continue;

                        cvf::Vec3d cornerVerts[8];
                        grid->cellCornerVertices( cellIndex, cornerVerts );

                        cvf::ubyte faceConn[4];
                        grid->cellFaceVertexIndices( faceType, faceConn );

                        cvf::uint triangleIndex = static_cast<cvf::uint>( vertices.size() );

                        for ( int n = 0; n < 4; n++ )
                        {
                            auto localIndexPair = getStructGridIndex( faceType, faceConn[n] );

                            structGridVertexIndices.push_back(
                                std::make_pair( quadIJIndices.first + localIndexPair.first,
                                                quadIJIndices.second + localIndexPair.second ) );

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

            m_vertices          = vertices;
            m_tringleIndices    = tringleIndices;
            m_structGridIndices = structGridVertexIndices;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCaseSurface::clearCachedNativeData()
{
    m_vertices.clear();
    m_tringleIndices.clear();
    m_structGridIndices.clear();
}

//--------------------------------------------------------------------------------------------------
/// Return local column and row number for structured grid based on a given cell face.
/// Argument faceType may be superfluous depending on winding and particular NEG_I face may
/// need particular handling, see StructGridInterface::cellFaceVertexIndices().
//
//     7---------6
//    /|        /|     |k
//   / |       / |     | /j
//  4---------5  |     |/
//  |  3------|--2     *---i
//  | /       | /
//  |/        |/
//  0---------1
//--------------------------------------------------------------------------------------------------
std::pair<cvf::uint, cvf::uint> RimGridCaseSurface::getStructGridIndex( cvf::StructGridInterface::FaceType faceType,
                                                                        cvf::ubyte localVertexIndex )
{
    std::pair<unsigned, unsigned> localIndexPair;

    CVF_TIGHT_ASSERT( localVertexIndex <= 3 );

    if ( localVertexIndex == 0 ) localIndexPair = std::make_pair( 0, 0 );
    if ( localVertexIndex == 1 ) localIndexPair = std::make_pair( 1, 0 );
    if ( localVertexIndex == 2 ) localIndexPair = std::make_pair( 1, 1 );
    if ( localVertexIndex == 3 ) localIndexPair = std::make_pair( 0, 1 );

    return localIndexPair;
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::updateSurfaceData()
{
    if ( m_vertices.empty() || m_tringleIndices.empty() || m_structGridIndices.empty() )
    {
        extractDataFromGrid();
    }

    RigSurface* surfaceData = nullptr;

    std::vector<unsigned>   tringleIndices{m_tringleIndices};
    std::vector<cvf::Vec3d> vertices{m_vertices};

    if ( !tringleIndices.empty() )
    {
        {
            // Modify the z-value slightly to avoid geometrical numerical issues when the surface intersects
            // exactly at the cell face

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

        surfaceData = new RigSurface;
        surfaceData->setTriangleData( tringleIndices, vertices );
    }

    setSurfaceData( surfaceData );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::exportStructSurfaceFromGridCase( std::vector<cvf::Vec3d>*            vertices,
                                                          std::vector<std::pair<uint, uint>>* structGridVertexIndices )
{
    if ( m_vertices.empty() || m_tringleIndices.empty() || m_structGridIndices.empty() )
    {
        extractDataFromGrid();
    }

    if ( m_vertices.empty() ) return false;

    *vertices                = m_vertices;
    *structGridVertexIndices = m_structGridIndices;

    if ( !vertices->empty() )
    {
        // Permute z-value to avoid numerical issues when surface intersects exactly at cell face

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
        else if ( m_sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
        {
            offset.z() += delta;
        }

        // Include the potential depth offset in the base class
        offset.z() += depthOffset();

        RimSurface::applyDepthOffset( offset, vertices );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the tree selector, including the slice index
//--------------------------------------------------------------------------------------------------
QString RimGridCaseSurface::fullName() const
{
    QString retval = RimSurface::fullName();

    auto dirValue = m_sliceDirection().value();
    switch ( dirValue )
    {
        case RiaDefines::GridCaseAxis::AXIS_I:
            retval += " - I:";
            break;
        case RiaDefines::GridCaseAxis::AXIS_J:
            retval += " - J:";
            break;
        case RiaDefines::GridCaseAxis::AXIS_K:
            retval += " - K:";
            break;
        case RiaDefines::GridCaseAxis::UNDEFINED_AXIS:
        default:
            break;
    }

    retval += QString::number( m_oneBasedSliceIndex );
    return retval;
}
