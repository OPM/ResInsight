/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "cvfAssert.h"
#include "cvfStructGrid.h"

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
    : m_eclipseCase( eclipseCase )
    , m_mainGrid( nullptr )
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
    cvf::Vec3st maxActual = max.isUndefined() ? cvf::Vec3st( m_mainGrid->cellCountI() - 1, m_mainGrid->cellCountJ() - 1, m_mainGrid->cellCountK() - 1 ) : max;
    m_max                 = maxActual;

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
        // For refined grids, get all 8 cell corners first, then extract face corners
        auto cellCorners = getCellCorners( i, j, k );
        
        // Extract the 4 corners for the requested face
        // This follows the same convention as cvf::StructGridInterface::FaceType
        std::array<cvf::Vec3d, 4> faceCorners;
        
        switch ( face )
        {
            case cvf::StructGridInterface::NEG_K: // Top face (k-)
                faceCorners[0] = cellCorners[0]; // (-I,-J,top)
                faceCorners[1] = cellCorners[1]; // (+I,-J,top)
                faceCorners[2] = cellCorners[2]; // (+I,+J,top)
                faceCorners[3] = cellCorners[3]; // (-I,+J,top)
                break;
                
            case cvf::StructGridInterface::POS_K: // Bottom face (k+)
                faceCorners[0] = cellCorners[4]; // (-I,-J,bottom)
                faceCorners[1] = cellCorners[5]; // (+I,-J,bottom)
                faceCorners[2] = cellCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = cellCorners[7]; // (-I,+J,bottom)
                break;
                
            case cvf::StructGridInterface::NEG_I: // Left face (i-)
                faceCorners[0] = cellCorners[0]; // (-I,-J,top)
                faceCorners[1] = cellCorners[3]; // (-I,+J,top)
                faceCorners[2] = cellCorners[7]; // (-I,+J,bottom)
                faceCorners[3] = cellCorners[4]; // (-I,-J,bottom)
                break;
                
            case cvf::StructGridInterface::POS_I: // Right face (i+)
                faceCorners[0] = cellCorners[1]; // (+I,-J,top)
                faceCorners[1] = cellCorners[2]; // (+I,+J,top)
                faceCorners[2] = cellCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = cellCorners[5]; // (+I,-J,bottom)
                break;
                
            case cvf::StructGridInterface::NEG_J: // Front face (j-)
                faceCorners[0] = cellCorners[0]; // (-I,-J,top)
                faceCorners[1] = cellCorners[1]; // (+I,-J,top)
                faceCorners[2] = cellCorners[5]; // (+I,-J,bottom)
                faceCorners[3] = cellCorners[4]; // (-I,-J,bottom)
                break;
                
            case cvf::StructGridInterface::POS_J: // Back face (j+)
                faceCorners[0] = cellCorners[3]; // (-I,+J,top)
                faceCorners[1] = cellCorners[2]; // (+I,+J,top)
                faceCorners[2] = cellCorners[6]; // (+I,+J,bottom)
                faceCorners[3] = cellCorners[7]; // (-I,+J,bottom)
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
        // For non-refined grids, use the original cell face extraction
        CellMapping mapping = mapRefinedToOriginal( i, j, k );
        
        size_t originalCellIndex = m_mainGrid->cellIndexFromIJK( mapping.originalI, mapping.originalJ, mapping.originalK );
        auto   cell              = m_mainGrid->cell( originalCellIndex );
        auto   faceCorners       = cell.faceCorners( face );
        
        // Apply coordinate transformations if needed
        if ( useMapAxes() )
        {
            cvf::Mat4d transform = mapAxisTransform();
            for ( cvf::Vec3d& corner : faceCorners )
            {
                corner.transformPoint( transform );
            }
        }
        
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
std::array<cvf::Vec3d, 8> RigGridExportAdapter::getRefinedCellCorners( size_t origI, size_t origJ, size_t origK, size_t subI, size_t subJ, size_t subK ) const
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