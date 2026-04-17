/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"

#include <vector>

namespace cvf
{
class ModelBasicList;
class Part;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class RimEclipseCase;
class RimEclipseCellColors;
class RimRefinementRegion;
class RimRefinementRegionCollection;

//==================================================================================================
///
/// Builds parts for previewing refinement regions in the 3D view. Each region is rendered as
/// solid polyhedrons (one per refined sub-cell) coloured by the parent cell's value of the view's
/// currently selected cell result, with mesh lines drawn on top.
///
/// Geometry and the per-sub-cell parent-cell index list are built once in buildGeometry() and
/// cached. Per-time-step recolouring updates only texture coordinates via updateCellResultColor().
///
//==================================================================================================
class RivRefinementRegionPartMgr : public cvf::Object
{
public:
    RivRefinementRegionPartMgr();
    ~RivRefinementRegionPartMgr() override;

    void clearGeometryCache();

    // Rebuild geometry + parent-cell index list for every active region in the collection.
    void buildGeometry( const RimRefinementRegionCollection* collection,
                        RimEclipseCase*                      eclipseCase,
                        const caf::DisplayCoordTransform*    coordTransform );

    // Append all cached parts (face + mesh, or outer-box fallback) to the given scene model.
    void appendStaticPartsToModel( cvf::ModelBasicList* model );

    // Recompute texture coordinates from the current result accessor and apply to cached face parts.
    // No-op for regions that fell back to the outer-box wireframe.
    void updateCellResultColor( size_t timeStepIndex, RimEclipseCellColors* cellResultColors );

private:
    struct RegionCache
    {
        cvf::ref<cvf::Part>       facePart; // PT_TRIANGLES, null when outer-box fallback or no geometry
        cvf::ref<cvf::Part>       meshPart; // PT_LINES, null when outer-box fallback
        cvf::ref<cvf::Part>       outerBoxPart; // 8-vertex wireframe box, used when refined cell count exceeds limit
        cvf::ref<cvf::Vec2fArray> textureCoords; // Reused per time step; sized to vertex count
        std::vector<size_t>       parentGlobalCellIdx; // One entry per sub-cell, row-major (i,j,k)
        cvf::Color3f              fallbackColor; // Region preview color (used when no result is selected)
        size_t                    gridIndex; // Source grid index (always 0 for main grid today)
    };

    static cvf::ref<cvf::Part>
        createOuterBoxPart( const RimRefinementRegion* region, RimEclipseCase* eclipseCase, const caf::DisplayCoordTransform* coordTransform );

    void buildRegionCache( const RimRefinementRegion* region, RimEclipseCase* eclipseCase, const caf::DisplayCoordTransform* coordTransform );

    void applyFlatColorToFacePart( RegionCache& cache );

    std::vector<RegionCache> m_regionCaches;
};
