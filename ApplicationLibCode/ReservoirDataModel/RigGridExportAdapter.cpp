/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RigGridExportAdapter.h"

#include "RiaCellDividingTools.h"
#include "RiaDefines.h"

#include "RigActiveCellInfo.h"
#include "RigBoundingBoxIjk.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "cafVecIjk.h"
#include "cvfAssert.h"
#include "cvfStructGrid.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// Generate refined cell corners using non-uniform cumulative fractions
//--------------------------------------------------------------------------------------------------
static std::array<cvf::Vec3d, 8> generateRefinedCellCornersNonUniform( const std::array<cvf::Vec3d, 8>& originalCorners,
                                                                       const std::vector<double>&       cumulativeFractionsX,
                                                                       const std::vector<double>&       cumulativeFractionsY,
                                                                       const std::vector<double>&       cumulativeFractionsZ,
                                                                       size_t                           subI,
                                                                       size_t                           subJ,
                                                                       size_t                           subK )
{
    auto allRefinedCorners =
        RiaCellDividingTools::createHexCornerCoords( originalCorners, cumulativeFractionsX, cumulativeFractionsY, cumulativeFractionsZ );

    size_t nx = cumulativeFractionsX.size();
    size_t ny = cumulativeFractionsY.size();

    size_t subcellIndex     = subK * ( nx * ny ) + subJ * nx + subI;
    size_t cornerStartIndex = subcellIndex * 8;

    std::array<cvf::Vec3d, 8> refinedCorners;
    for ( size_t i = 0; i < 8; ++i )
    {
        refinedCorners[i] = allRefinedCorners[cornerStartIndex + i];
    }

    return refinedCorners;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridExportAdapter::RigGridExportAdapter( RigEclipseCaseData*    eclipseCase,
                                            const cvf::Vec3st&     min,
                                            const cvf::Vec3st&     max,
                                            const RigRefinement&   refinement,
                                            const cvf::UByteArray* cellVisibilityOverrideForActnum )
    : m_mainGrid( nullptr )
    , m_activeCellInfo( nullptr )
    , m_cellVisibilityOverride( cellVisibilityOverrideForActnum )
    , m_min( min )
    , m_max( max )
    , m_refinement( refinement.clone() )
    , m_refinedNI( 0 )
    , m_refinedNJ( 0 )
    , m_refinedNK( 0 )
{
    CVF_ASSERT( eclipseCase );

    m_mainGrid       = eclipseCase->mainGrid();
    m_activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    CVF_ASSERT( m_mainGrid );
    CVF_ASSERT( m_activeCellInfo );

    cvf::Vec3st maxActual =
        max.isUndefined() ? cvf::Vec3st( m_mainGrid->cellCountI() - 1, m_mainGrid->cellCountJ() - 1, m_mainGrid->cellCountK() - 1 ) : max;
    m_max = maxActual;

    m_refinedNI = m_refinement->totalRefinedCount( RigRefinement::DimI );
    m_refinedNJ = m_refinement->totalRefinedCount( RigRefinement::DimJ );
    m_refinedNK = m_refinement->totalRefinedCount( RigRefinement::DimK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> RigGridExportAdapter::getCellCorners( size_t i, size_t j, size_t k ) const
{
    // Map refined cell indices to original cell and subcell indices
    CellMapping mapping = mapRefinedToOriginal( i, j, k );

    std::array<cvf::Vec3d, 8> corners;

    if ( hasRefinement() )
    {
        corners = getRefinedCellCorners( mapping.originalI, mapping.originalJ, mapping.originalK, mapping.subI, mapping.subJ, mapping.subK );
    }
    else
    {
        corners = getOriginalCellCorners( mapping.originalI, mapping.originalJ, mapping.originalK );
    }

    // Apply coordinate transformations if needed
    applyCoordinateTransformation( corners );

    return corners;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 4> RigGridExportAdapter::getFaceCorners( size_t i, size_t j, size_t k, cvf::StructGridInterface::FaceType face ) const
{
    if ( hasRefinement() )
    {
        // For refined grids, create 8 cell corners from two face corner calls
        CellMapping mapping = mapRefinedToOriginal( i, j, k );

        size_t originalCellIndex = m_mainGrid->cellIndexFromIJK( mapping.originalI, mapping.originalJ, mapping.originalK );
        auto   cell              = m_mainGrid->cell( originalCellIndex );

        // Get top and bottom face corners to construct the full 8-corner cell
        auto topFaceCorners    = cell.faceCorners( cvf::StructGridInterface::NEG_K );
        auto bottomFaceCorners = cell.faceCorners( cvf::StructGridInterface::POS_K );

        // Construct the 8-corner array in the correct order
        std::array<cvf::Vec3d, 8> originalCorners;
        originalCorners[0] = topFaceCorners[0]; // (-I,-J,top)
        originalCorners[1] = topFaceCorners[3]; // (+I,-J,top)
        originalCorners[2] = topFaceCorners[2]; // (+I,+J,top)
        originalCorners[3] = topFaceCorners[1]; // (-I,+J,top)
        originalCorners[4] = bottomFaceCorners[0]; // (-I,-J,bottom)
        originalCorners[5] = bottomFaceCorners[1]; // (+I,-J,bottom)
        originalCorners[6] = bottomFaceCorners[2]; // (+I,+J,bottom)
        originalCorners[7] = bottomFaceCorners[3]; // (-I,+J,bottom)

        // Apply coordinate transformations if needed
        applyCoordinateTransformation( originalCorners );

        // Generate refined cell corners using the appropriate refinement algorithm
        auto refinedCorners = computeRefinedCorners( originalCorners,
                                                     mapping.originalI,
                                                     mapping.originalJ,
                                                     mapping.originalK,
                                                     mapping.subI,
                                                     mapping.subJ,
                                                     mapping.subK );

        // Extract the 4 corners for the requested face from the refined corners
        std::array<cvf::Vec3d, 4> faceCorners;

        switch ( face )
        {
            case cvf::StructGridInterface::NEG_K: // Top face (k-)
                faceCorners[0] = refinedCorners[0]; // (-I,-J,top)
                faceCorners[1] = refinedCorners[3]; // (+I,-J,top)
                faceCorners[2] = refinedCorners[2]; // (+I,+J,top)
                faceCorners[3] = refinedCorners[1]; // (-I,+J,top)
                break;

            case cvf::StructGridInterface::POS_K: // Bottom face (k+)
                faceCorners[0] = refinedCorners[4]; // (-I,-J,bottom)
                faceCorners[1] = refinedCorners[5]; // (+I,-J,bottom)
                faceCorners[2] = refinedCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = refinedCorners[7]; // (-I,+J,bottom)
                break;

            case cvf::StructGridInterface::NEG_I: // Left face (i-)
                faceCorners[0] = refinedCorners[0]; // (-I,-J,top)
                faceCorners[1] = refinedCorners[3]; // (-I,+J,top)
                faceCorners[2] = refinedCorners[7]; // (-I,+J,bottom)
                faceCorners[3] = refinedCorners[4]; // (-I,-J,bottom)
                break;

            case cvf::StructGridInterface::POS_I: // Right face (i+)
                faceCorners[0] = refinedCorners[1]; // (+I,-J,top)
                faceCorners[1] = refinedCorners[2]; // (+I,+J,top)
                faceCorners[2] = refinedCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = refinedCorners[5]; // (+I,-J,bottom)
                break;

            case cvf::StructGridInterface::NEG_J: // Front face (j-)
                faceCorners[0] = refinedCorners[0]; // (-I,-J,top)
                faceCorners[1] = refinedCorners[1]; // (+I,-J,top)
                faceCorners[2] = refinedCorners[5]; // (+I,-J,bottom)
                faceCorners[3] = refinedCorners[4]; // (-I,-J,bottom)
                break;

            case cvf::StructGridInterface::POS_J: // Back face (j+)
                faceCorners[0] = refinedCorners[3]; // (-I,+J,top)
                faceCorners[1] = refinedCorners[2]; // (+I,+J,top)
                faceCorners[2] = refinedCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = refinedCorners[7]; // (-I,+J,bottom)
                break;

            default:
                // Unsupported face type - return zeros
                faceCorners.fill( cvf::Vec3d::ZERO );
                break;
        }

        return faceCorners;
    }
    else
    {
        // For non-refined grids, go directly to RigCell::faceCorners method
        CellMapping mapping = mapRefinedToOriginal( i, j, k );

        size_t originalCellIndex = m_mainGrid->cellIndexFromIJK( mapping.originalI, mapping.originalJ, mapping.originalK );
        auto   cell              = m_mainGrid->cell( originalCellIndex );
        auto   faceCorners       = cell.faceCorners( face );

        // Apply coordinate transformations if needed
        applyCoordinateTransformation( faceCorners );

        return faceCorners;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridExportAdapter::isCellActive( size_t i, size_t j, size_t k ) const
{
    // Map refined cell indices to original cell
    CellMapping mapping = mapRefinedToOriginal( i, j, k );

    // Get original cell index
    size_t originalCellIndex = m_mainGrid->cellIndexFromIJK( mapping.originalI, mapping.originalJ, mapping.originalK );

    // Check if original cell is active
    bool isActive = m_activeCellInfo->isActive( originalCellIndex );

    // Apply visibility override if present
    if ( isActive && m_cellVisibilityOverride )
    {
        isActive = ( *m_cellVisibilityOverride )[originalCellIndex] != 0;
    }

    return isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridExportAdapter::useMapAxes() const
{
    return m_mainGrid->useMapAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RigGridExportAdapter::mapAxisTransform() const
{
    return m_mainGrid->mapAxisTransform();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<float, 6> RigGridExportAdapter::mapAxes() const
{
    return m_mainGrid->mapAxesF();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> RigGridExportAdapter::getOriginalCellCorners( size_t origI, size_t origJ, size_t origK ) const
{
    size_t cellIndex = m_mainGrid->cellIndexFromIJK( origI, origJ, origK );
    return m_mainGrid->cellCornerVertices( cellIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8>
    RigGridExportAdapter::getRefinedCellCorners( size_t origI, size_t origJ, size_t origK, size_t subI, size_t subJ, size_t subK ) const
{
    std::array<cvf::Vec3d, 8> originalCorners = getOriginalCellCorners( origI, origJ, origK );
    return computeRefinedCorners( originalCorners, origI, origJ, origK, subI, subJ, subK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> RigGridExportAdapter::computeRefinedCorners( const std::array<cvf::Vec3d, 8>& originalCorners,
                                                                       size_t                           origI,
                                                                       size_t                           origJ,
                                                                       size_t                           origK,
                                                                       size_t                           subI,
                                                                       size_t                           subJ,
                                                                       size_t                           subK ) const
{
    size_t sectorI = origI - m_min.x();
    size_t sectorJ = origJ - m_min.y();
    size_t sectorK = origK - m_min.z();

    return generateRefinedCellCornersNonUniform( originalCorners,
                                                 m_refinement->cumulativeFractions( RigRefinement::DimI, sectorI ),
                                                 m_refinement->cumulativeFractions( RigRefinement::DimJ, sectorJ ),
                                                 m_refinement->cumulativeFractions( RigRefinement::DimK, sectorK ),
                                                 subI,
                                                 subJ,
                                                 subK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridExportAdapter::applyCoordinateTransformation( std::array<cvf::Vec3d, 8>& corners ) const
{
    if ( useMapAxes() )
    {
        cvf::Mat4d transform = mapAxisTransform();
        for ( cvf::Vec3d& corner : corners )
        {
            corner.transformPoint( transform );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridExportAdapter::applyCoordinateTransformation( std::array<cvf::Vec3d, 4>& corners ) const
{
    if ( useMapAxes() )
    {
        cvf::Mat4d transform = mapAxisTransform();
        for ( cvf::Vec3d& corner : corners )
        {
            corner.transformPoint( transform );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridExportAdapter::CellMapping RigGridExportAdapter::mapRefinedToOriginal( size_t refinedI, size_t refinedJ, size_t refinedK ) const
{
    CellMapping mapping;

    auto [origI, subI] = m_refinement->mapRefinedToOriginal( RigRefinement::DimI, refinedI );
    auto [origJ, subJ] = m_refinement->mapRefinedToOriginal( RigRefinement::DimJ, refinedJ );
    auto [origK, subK] = m_refinement->mapRefinedToOriginal( RigRefinement::DimK, refinedK );

    mapping.originalI = m_min.x() + origI;
    mapping.originalJ = m_min.y() + origJ;
    mapping.originalK = m_min.z() + origK;
    mapping.subI      = subI;
    mapping.subJ      = subJ;
    mapping.subK      = subK;

    return mapping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RigGridExportAdapter::originalMin() const
{
    return m_min;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RigGridExportAdapter::originalMax() const
{
    return m_max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridExportAdapter::hasRefinement() const
{
    return m_refinement->hasRefinement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGridExportAdapter::cellCountI() const
{
    return m_refinedNI;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGridExportAdapter::cellCountJ() const
{
    return m_refinedNJ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGridExportAdapter::cellCountK() const
{
    return m_refinedNK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGridExportAdapter::totalCells() const
{
    return m_refinedNI * m_refinedNJ * m_refinedNK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigRefinement& RigGridExportAdapter::refinement() const
{
    return *m_refinement;
}

//--------------------------------------------------------------------------------------------------
/// Transform IJK coordinates from global grid space to sector-relative space with refinement
//--------------------------------------------------------------------------------------------------
std::expected<caf::VecIjk0, QString> RigGridExportAdapter::transformIjkToSectorCoordinates( const caf::VecIjk0&  ijk,
                                                                                            const caf::VecIjk0&  min,
                                                                                            const caf::VecIjk0&  max,
                                                                                            const RigRefinement& refinement,
                                                                                            bool                 applyRefinementCentering,
                                                                                            bool                 isBoxMaxCoordinate )
{
    RigBoundingBoxIjk<caf::VecIjk0> sectorBox( min, max );
    if ( !sectorBox.contains( ijk ) )
    {
        return std::unexpected( QString( "IJK coordinates (%1) are outside sector bounds [(%2), (%3)]" )
                                    .arg( QString::fromStdString( ijk.toString() ) )
                                    .arg( QString::fromStdString( min.toString() ) )
                                    .arg( QString::fromStdString( max.toString() ) ) );
    }

    size_t sectorI = ijk.x() - min.x();
    size_t sectorJ = ijk.y() - min.y();
    size_t sectorK = ijk.z() - min.z();

    if ( applyRefinementCentering )
    {
        return caf::VecIjk0( refinement.cumulativeOffset( RigRefinement::DimI, sectorI ) +
                                 refinement.subcellCount( RigRefinement::DimI, sectorI ) / 2,
                             refinement.cumulativeOffset( RigRefinement::DimJ, sectorJ ) +
                                 refinement.subcellCount( RigRefinement::DimJ, sectorJ ) / 2,
                             refinement.cumulativeOffset( RigRefinement::DimK, sectorK ) +
                                 refinement.subcellCount( RigRefinement::DimK, sectorK ) / 2 );
    }
    else if ( isBoxMaxCoordinate )
    {
        return caf::VecIjk0( refinement.cumulativeOffset( RigRefinement::DimI, sectorI + 1 ) - 1,
                             refinement.cumulativeOffset( RigRefinement::DimJ, sectorJ + 1 ) - 1,
                             refinement.cumulativeOffset( RigRefinement::DimK, sectorK + 1 ) - 1 );
    }
    else
    {
        return caf::VecIjk0( refinement.cumulativeOffset( RigRefinement::DimI, sectorI ),
                             refinement.cumulativeOffset( RigRefinement::DimJ, sectorJ ),
                             refinement.cumulativeOffset( RigRefinement::DimK, sectorK ) );
    }
}
