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

    CAF_PDM_InitField( &m_oneBasedSliceIndex, "SliceIndex", 1, "Slice Index (K)", "", "", "" );
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
void RimGridCaseSurface::setOneBasedIndex( int oneBasedSliceIndex )
{
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
    RimGridCaseSurface* newSurface = dynamic_cast<RimGridCaseSurface*>(
        xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    newSurface->setCase( m_case.value() ); // TODO: case seems to get lost in the xml copy, investigate later

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
        myAttr->m_maximum = static_cast<int>( grid->cellCountK() );
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

    if ( changedField == &m_case || changedField == &m_oneBasedSliceIndex )
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

    if ( m_case )
    {
        RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclCase && eclCase->mainGrid() )
        {
            const RigMainGrid* grid = eclCase->mainGrid();

            size_t minI      = 0;
            size_t minJ      = 0;
            size_t minK      = 0;
            size_t maxI      = grid->cellCountI();
            size_t maxJ      = grid->cellCountJ();
            size_t maxK      = grid->cellCountK();
            size_t row       = 0;
            size_t maxRow    = 0;
            size_t column    = 0;
            size_t maxColumn = 0;

            size_t zeroBasedLayerIndex = static_cast<size_t>( m_oneBasedSliceIndex ) - 1;

            cvf::StructGridInterface::FaceType faceType = cvf::StructGridInterface::NO_FACE;
            {
                auto sliceDirection = RiaDefines::GridCaseAxis::AXIS_K;

                if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
                {
                    faceType = cvf::StructGridInterface::NEG_K;

                    minK      = zeroBasedLayerIndex;
                    maxK      = zeroBasedLayerIndex;
                    maxRow    = maxJ;
                    maxColumn = maxI;
                }
                else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_J )
                {
                    faceType  = cvf::StructGridInterface::NEG_J;
                    minJ      = zeroBasedLayerIndex;
                    maxJ      = zeroBasedLayerIndex;
                    maxRow    = maxK;
                    maxColumn = maxI;
                }
                else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_I )
                {
                    faceType  = cvf::StructGridInterface::NEG_I;
                    minI      = zeroBasedLayerIndex;
                    maxI      = zeroBasedLayerIndex;
                    maxRow    = maxK;
                    maxColumn = maxJ;
                }
            }

            std::vector<unsigned>                      triangleIndices;
            std::vector<cvf::Vec3d>                    vertices;
            std::vector<std::pair<unsigned, unsigned>> structGridVertexIndices;

            for ( size_t i = minI; i <= maxI; i++ )
            {
                for ( size_t j = minJ; j <= maxJ; j++ )
                {
                    for ( size_t k = minK; k <= maxK; k++ )
                    {
                        switch ( faceType )
                        {
                            case cvf::StructGridInterface::NEG_I:
                                row    = k;
                                column = j;
                                break;
                            case cvf::StructGridInterface::NEG_J:
                                row    = k;
                                column = i;
                                break;
                            case cvf::StructGridInterface::NEG_K:
                                row    = j;
                                column = i;
                                break;
                        }

                        size_t cellIndex     = 0;
                        size_t cellFaceIndex = 0;
                        if ( !findValidCellIndex( grid, faceType, cellIndex, row, column, zeroBasedLayerIndex, cellFaceIndex ) )
                            return;

                        cvf::Vec3d cornerVerts[8];
                        grid->cellCornerVertices( cellIndex, cornerVerts );

                        cvf::ubyte faceConn[4];
                        grid->cellFaceVertexIndices( faceType, faceConn );

                        structGridVertexIndices.push_back(
                            std::make_pair( static_cast<cvf::uint>( column + 1 ), static_cast<cvf::uint>( row + 1 ) ) );

                        vertices.push_back( cornerVerts[faceConn[cellFaceIndex]] );

                        if ( row < maxRow && column < maxColumn )
                        {
                            cvf::uint triangleIndexLeft = static_cast<cvf::uint>( column * ( maxRow + 1 ) + row );
                            cvf::uint triangleIndexRight = static_cast<cvf::uint>( ( column + 1 ) * ( maxRow + 1 ) + row );

                            triangleIndices.push_back( triangleIndexLeft );
                            triangleIndices.push_back( triangleIndexLeft + 1 );
                            triangleIndices.push_back( triangleIndexRight + 1 );

                            triangleIndices.push_back( triangleIndexLeft );
                            triangleIndices.push_back( triangleIndexRight + 1 );
                            triangleIndices.push_back( triangleIndexRight );
                        }
                    }
                }
            }

            m_vertices          = vertices;
            m_tringleIndices    = triangleIndices;
            m_structGridIndices = structGridVertexIndices;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCaseSurface::findValidCellIndex( const RigMainGrid*                       grid,
                                             const cvf::StructGridInterface::FaceType faceType,
                                             size_t&                                  cellIndex,
                                             const size_t                             row,
                                             const size_t                             column,
                                             const size_t                             layer,
                                             size_t&                                  cellFaceIndex )
{
    auto getCellFromRowColumnLayer = [grid, faceType]( size_t row, size_t column, size_t layer ) -> size_t {
        if ( faceType == cvf::StructGridInterface::NEG_I ) return grid->cellIndexFromIJK( layer, column, row );
        if ( faceType == cvf::StructGridInterface::NEG_J ) return grid->cellIndexFromIJK( column, layer, row );
        return grid->cellIndexFromIJK( column, row, layer );
    };

    auto isCellValid = [grid, faceType]( size_t row, size_t column, size_t layer ) -> bool {
        if ( faceType == cvf::StructGridInterface::NEG_I )
        {
            return column < grid->cellCountJ() && row < grid->cellCountK() &&
                   !grid->cell( grid->cellIndexFromIJK( layer, column, row ) ).isInvalid();
        }
        if ( faceType == cvf::StructGridInterface::NEG_J )
        {
            return column < grid->cellCountI() && row < grid->cellCountK() &&
                   !grid->cell( grid->cellIndexFromIJK( column, layer, row ) ).isInvalid();
        }
        return column < grid->cellCountI() && row < grid->cellCountJ() &&
               !grid->cell( grid->cellIndexFromIJK( column, row, layer ) ).isInvalid();
    };

    if ( isCellValid( row, column, layer ) )
    {
        cellIndex     = getCellFromRowColumnLayer( row, column, layer );
        cellFaceIndex = 0;
        return true;
    }

    if ( isCellValid( row - 1, column, layer ) )
    {
        cellIndex     = getCellFromRowColumnLayer( row - 1, column, layer );
        cellFaceIndex = 1;
        return true;
    }

    if ( isCellValid( row, column - 1, layer ) )
    {
        cellIndex     = getCellFromRowColumnLayer( row, column - 1, layer );
        cellFaceIndex = 3;
        return true;
    }

    if ( isCellValid( row - 1, column - 1, layer ) )
    {
        cellIndex     = getCellFromRowColumnLayer( row - 1, column - 1, layer );
        cellFaceIndex = 2;
        return true;
    }

    return false;
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

    RiaDefines::GridCaseAxis sliceDirection = RiaDefines::GridCaseAxis::AXIS_K;

    if ( !tringleIndices.empty() )
    {
        {
            // Modify the z-value slightly to avoid geometrical numerical issues when the surface intersects
            // exactly at the cell face

            double delta = 1.0e-5;

            cvf::Vec3d offset = cvf::Vec3d::ZERO;

            if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_I )
            {
                offset.x() += delta;
            }
            else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_J )
            {
                offset.y() += delta;
            }
            if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
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

    RiaDefines::GridCaseAxis sliceDirection = RiaDefines::GridCaseAxis::AXIS_K;

    if ( !vertices->empty() )
    {
        // Permute z-value to avoid numerical issues when surface intersects exactly at cell face

        double delta = 1.0e-5;

        cvf::Vec3d offset = cvf::Vec3d::ZERO;

        if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_I )
        {
            offset.x() += delta;
        }
        else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_J )
        {
            offset.y() += delta;
        }
        else if ( sliceDirection == RiaDefines::GridCaseAxis::AXIS_K )
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
    retval += " - K:";
    retval += QString::number( m_oneBasedSliceIndex );
    return retval;
}
