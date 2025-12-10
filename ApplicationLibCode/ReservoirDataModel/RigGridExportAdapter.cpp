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
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "cafVecIjk.h"
#include "cvfAssert.h"
#include "cvfStructGrid.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// Generate refined cell corners using trilinear interpolation within the original cell
/// This ensures refined cells are strictly contained within the original cell bounds
//--------------------------------------------------------------------------------------------------
static std::array<cvf::Vec3d, 8> generateRefinedCellCorners( const std::array<cvf::Vec3d, 8>& originalCorners,
                                                             size_t                           refinementI,
                                                             size_t                           refinementJ,
                                                             size_t                           refinementK,
                                                             size_t                           subI,
                                                             size_t                           subJ,
                                                             size_t                           subK )
{
    // Use ResInsight's proven face-based refinement approach
    auto allRefinedCorners = RiaCellDividingTools::createHexCornerCoords( originalCorners, refinementI, refinementJ, refinementK );

    // Calculate the linear index of the specific subcell we want
    // createHexCornerCoords uses for(z) for(y) for(x) loops, so X is fastest changing (X-major)
    // Linear index = z * (nx * ny) + y * nx + x
    size_t subcellIndex     = subK * ( refinementI * refinementJ ) + subJ * refinementI + subI;
    size_t cornerStartIndex = subcellIndex * 8;

    // Extract the 8 corners for this specific subcell
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
                                            const cvf::Vec3st&     refinement,
                                            const cvf::UByteArray* cellVisibilityOverrideForActnum )
    : m_mainGrid( nullptr )
    , m_activeCellInfo( nullptr )
    , m_cellVisibilityOverride( cellVisibilityOverrideForActnum )
    , m_min( min )
    , m_max( max )
    , m_refinement( refinement )
    , m_refinedNI( 0 )
    , m_refinedNJ( 0 )
    , m_refinedNK( 0 )
{
    CVF_ASSERT( eclipseCase );

    m_mainGrid       = eclipseCase->mainGrid();
    m_activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    CVF_ASSERT( m_mainGrid );
    CVF_ASSERT( m_activeCellInfo );

    // Calculate actual max if undefined
    cvf::Vec3st maxActual =
        max.isUndefined() ? cvf::Vec3st( m_mainGrid->cellCountI() - 1, m_mainGrid->cellCountJ() - 1, m_mainGrid->cellCountK() - 1 ) : max;
    m_max = maxActual;

    // Calculate refined grid dimensions
    m_refinedNI = ( maxActual.x() - min.x() + 1 ) * refinement.x();
    m_refinedNJ = ( maxActual.y() - min.y() + 1 ) * refinement.y();
    m_refinedNK = ( maxActual.z() - min.z() + 1 ) * refinement.z();
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

        // Generate refined cell corners using ResInsight's refinement algorithm
        auto refinedCorners = generateRefinedCellCorners( originalCorners,
                                                          m_refinement.x(),
                                                          m_refinement.y(),
                                                          m_refinement.z(),
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
    // Get original cell corners first
    std::array<cvf::Vec3d, 8> originalCorners = getOriginalCellCorners( origI, origJ, origK );

    // Generate refined subcell corners
    return generateRefinedCellCorners( originalCorners, m_refinement.x(), m_refinement.y(), m_refinement.z(), subI, subJ, subK );
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

    // Calculate which original cell this refined cell belongs to
    mapping.originalI = m_min.x() + refinedI / m_refinement.x();
    mapping.originalJ = m_min.y() + refinedJ / m_refinement.y();
    mapping.originalK = m_min.z() + refinedK / m_refinement.z();

    // Calculate subcell indices within the original cell
    mapping.subI = refinedI % m_refinement.x();
    mapping.subJ = refinedJ % m_refinement.y();
    mapping.subK = refinedK % m_refinement.z();

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
cvf::Vec3st RigGridExportAdapter::refinement() const
{
    return m_refinement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridExportAdapter::hasRefinement() const
{
    return m_refinement.x() > 1 || m_refinement.y() > 1 || m_refinement.z() > 1;
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
/// Transform IJK coordinates from global grid space to sector-relative space with refinement
/// Returns 1-based Eclipse coordinates
//--------------------------------------------------------------------------------------------------
std::expected<caf::VecIjk1, QString> RigGridExportAdapter::transformIjkToSectorCoordinates( const caf::VecIjk0& originalIjk,
                                                                                            const caf::VecIjk0& min,
                                                                                            const caf::VecIjk0& max,
                                                                                            const cvf::Vec3st&  refinement,
                                                                                            bool                applyRefinementCentering,
                                                                                            bool                isBoxMaxCoordinate )
{
    // Check if original IJK is within the sector bounds
    if ( originalIjk.x() < min.x() || originalIjk.x() > max.x() || originalIjk.y() < min.y() || originalIjk.y() > max.y() ||
         originalIjk.z() < min.z() || originalIjk.z() > max.z() )
    {
        return std::unexpected( QString( "IJK coordinates (%1, %2, %3) are outside sector bounds [(%4, %5, %6), (%7, %8, %9)]" )
                                    .arg( originalIjk.x() )
                                    .arg( originalIjk.y() )
                                    .arg( originalIjk.z() )
                                    .arg( min.x() )
                                    .arg( min.y() )
                                    .arg( min.z() )
                                    .arg( max.x() )
                                    .arg( max.y() )
                                    .arg( max.z() ) );
    }

    // Transform to sector-relative coordinates with refinement
    // Eclipse uses 1-based indexing, so we'll return 1-based coordinates
    size_t sectorI, sectorJ, sectorK;

    if ( applyRefinementCentering )
    {
        // Center the coordinate in the refined cell block (for point coordinates like WELSPECS)
        sectorI = ( originalIjk.x() - min.x() ) * refinement.x() + ( refinement.x() + 1 ) / 2;
        sectorJ = ( originalIjk.y() - min.y() ) * refinement.y() + ( refinement.y() + 1 ) / 2;
        sectorK = ( originalIjk.z() - min.z() ) * refinement.z() + ( refinement.z() + 1 ) / 2;
    }
    else if ( isBoxMaxCoordinate )
    {
        // Box maximum coordinate: map to end of last refined cell
        // For a cell at index i (0-based), after refinement it becomes cells [i*r, i*r+r-1]
        // So the max index of a box [min, max] becomes: (max-sector_min+1)*r (1-based)
        sectorI = ( originalIjk.x() - min.x() + 1 ) * refinement.x();
        sectorJ = ( originalIjk.y() - min.y() + 1 ) * refinement.y();
        sectorK = ( originalIjk.z() - min.z() + 1 ) * refinement.z();
    }
    else
    {
        // Box minimum coordinate: map to start of first refined cell
        sectorI = ( originalIjk.x() - min.x() ) * refinement.x() + 1;
        sectorJ = ( originalIjk.y() - min.y() ) * refinement.y() + 1;
        sectorK = ( originalIjk.z() - min.z() ) * refinement.z() + 1;
    }

    return caf::VecIjk1( sectorI, sectorJ, sectorK );
}
