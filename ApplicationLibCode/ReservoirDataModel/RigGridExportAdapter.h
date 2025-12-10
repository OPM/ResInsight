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

#pragma once

#include "cafVecIjk.h"

#include "cvfArray.h"
#include "cvfMatrix4.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <array>
#include <expected>

class RigEclipseCaseData;
class RigMainGrid;
class RigActiveCellInfo;
class QString;

//==================================================================================================
//
// Abstraction layer for grid export that hides mainGrid complexity and refinement details
//
// This adapter provides a unified interface for accessing grid cells regardless of whether
// the grid is refined or not. It handles:
// - Original grid cell access
// - Refined subcell generation via trilinear interpolation
// - Coordinate transformations (MAPAXES)
// - Cell visibility and activity logic
//
// Usage:
//   RigGridExportAdapter adapter(eclipseCase, min, max, refinement, cellVisibilityOverride);
//   for (size_t k = 0; k < adapter.cellCountK(); ++k) {
//       for (size_t j = 0; j < adapter.cellCountJ(); ++j) {
//           for (size_t i = 0; i < adapter.cellCountI(); ++i) {
//               auto corners = adapter.getCellCorners(i, j, k);
//               bool active = adapter.isCellActive(i, j, k);
//               // ... use corners and active flag
//           }
//       }
//   }
//
//==================================================================================================
class RigGridExportAdapter
{
public:
    RigGridExportAdapter( RigEclipseCaseData*    eclipseCase,
                          const cvf::Vec3st&     min,
                          const cvf::Vec3st&     max,
                          const cvf::Vec3st&     refinement,
                          const cvf::UByteArray* cellVisibilityOverrideForActnum = nullptr );

    // Grid dimensions (after refinement)
    size_t cellCountI() const;
    size_t cellCountJ() const;
    size_t cellCountK() const;
    size_t totalCells() const;

    // Unified cell access interface - works for both refined and non-refined grids
    std::array<cvf::Vec3d, 8> getCellCorners( size_t i, size_t j, size_t k ) const;
    std::array<cvf::Vec3d, 4> getFaceCorners( size_t i, size_t j, size_t k, cvf::StructGridInterface::FaceType face ) const;
    bool                      isCellActive( size_t i, size_t j, size_t k ) const;

    // Coordinate transformation info
    bool                 useMapAxes() const;
    cvf::Mat4d           mapAxisTransform() const;
    std::array<float, 6> mapAxes() const;

    // Grid bounds and refinement info
    cvf::Vec3st originalMin() const;
    cvf::Vec3st originalMax() const;
    cvf::Vec3st refinement() const;
    bool        hasRefinement() const;

    // Coordinate transformation utilities
    static std::expected<caf::VecIjk1, QString> transformIjkToSectorCoordinates( const caf::VecIjk0& originalIjk,
                                                                                 const caf::VecIjk0& min,
                                                                                 const caf::VecIjk0& max,
                                                                                 const cvf::Vec3st&  refinement,
                                                                                 bool                applyRefinementCentering = false,
                                                                                 bool                isBoxMaxCoordinate       = false );

private:
    // Internal methods to handle original vs refined cell access
    std::array<cvf::Vec3d, 8> getOriginalCellCorners( size_t origI, size_t origJ, size_t origK ) const;
    std::array<cvf::Vec3d, 8> getRefinedCellCorners( size_t origI, size_t origJ, size_t origK, size_t subI, size_t subJ, size_t subK ) const;
    void applyCoordinateTransformation( std::array<cvf::Vec3d, 8>& corners ) const;
    void applyCoordinateTransformation( std::array<cvf::Vec3d, 4>& corners ) const;

    // Helper to calculate original cell indices and subcell indices from refined indices
    struct CellMapping
    {
        size_t originalI, originalJ, originalK; // Original cell indices
        size_t subI, subJ, subK; // Subcell indices within original cell
    };
    CellMapping mapRefinedToOriginal( size_t refinedI, size_t refinedJ, size_t refinedK ) const;

    const RigMainGrid*       m_mainGrid;
    const RigActiveCellInfo* m_activeCellInfo;
    const cvf::UByteArray*   m_cellVisibilityOverride;

    cvf::Vec3st m_min;
    cvf::Vec3st m_max;
    cvf::Vec3st m_refinement;

    // Cached refined grid dimensions
    size_t m_refinedNI;
    size_t m_refinedNJ;
    size_t m_refinedNK;
};
