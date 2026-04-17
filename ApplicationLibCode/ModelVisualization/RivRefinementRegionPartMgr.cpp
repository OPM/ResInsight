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

#include "RivRefinementRegionPartMgr.h"

#include "RivBoxGeometryGenerator.h"

#include "RigEclipseCaseData.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigRefinement.h"

#include "RimEclipseCase.h"
#include "RimRefinementRegion.h"
#include "RimRefinementRegionCollection.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfMatrix4.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <array>

namespace
{
// Above this refined-cell count, the per-cell wireframe is skipped in favour of the outer-box wireframe.
constexpr size_t MAX_CELLS_FOR_PER_CELL_WIREFRAME = 200000;
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::appendPartsToModel( cvf::ModelBasicList*                 model,
                                                     const RimRefinementRegionCollection* collection,
                                                     RimEclipseCase*                      eclipseCase,
                                                     const caf::DisplayCoordTransform*    coordTransform )
{
    if ( !model || !collection || !eclipseCase || !coordTransform ) return;
    if ( !collection->isActive() ) return;

    for ( auto* region : collection->regions() )
    {
        if ( !region || !region->isActive() ) continue;

        cvf::ref<cvf::Part> part = createRegionPart( region, eclipseCase, coordTransform );
        if ( part.notNull() )
        {
            model->addPart( part.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Build a wireframe drawable covering every refined sub-cell in the region.
/// Uses RigGridExportAdapter for corner computation so refinement logic (uniform and non-uniform)
/// is shared with the sector-model exporter.
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivRefinementRegionPartMgr::createRegionPart( const RimRefinementRegion*        region,
                                                                  RimEclipseCase*                   eclipseCase,
                                                                  const caf::DisplayCoordTransform* coordTransform )
{
    auto* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return nullptr;
    auto* mainGrid = caseData->mainGrid();
    if ( !mainGrid ) return nullptr;

    const size_t gridI = mainGrid->cellCountI();
    const size_t gridJ = mainGrid->cellCountJ();
    const size_t gridK = mainGrid->cellCountK();
    if ( gridI == 0 || gridJ == 0 || gridK == 0 ) return nullptr;

    const auto   rmin = region->ijkMin();
    const auto   rmax = region->ijkMax();
    const size_t minI = std::min( rmin.x(), gridI - 1 );
    const size_t minJ = std::min( rmin.y(), gridJ - 1 );
    const size_t minK = std::min( rmin.z(), gridK - 1 );
    const size_t maxI = std::min( rmax.x(), gridI - 1 );
    const size_t maxJ = std::min( rmax.y(), gridJ - 1 );
    const size_t maxK = std::min( rmax.z(), gridK - 1 );

    auto refinement = region->effectiveRefinement();
    if ( !refinement ) return createOuterBoxPart( region, eclipseCase, coordTransform );

    cvf::Vec3st sectorMin( minI, minJ, minK );
    cvf::Vec3st sectorMax( maxI, maxJ, maxK );

    RigGridExportAdapter adapter( caseData, sectorMin, sectorMax, *refinement );

    const size_t refinedI     = adapter.cellCountI();
    const size_t refinedJ     = adapter.cellCountJ();
    const size_t refinedK     = adapter.cellCountK();
    const size_t totalRefined = refinedI * refinedJ * refinedK;
    if ( totalRefined == 0 ) return nullptr;

    if ( totalRefined > MAX_CELLS_FOR_PER_CELL_WIREFRAME )
    {
        return createOuterBoxPart( region, eclipseCase, coordTransform );
    }

    // RigGridExportAdapter applies MAPAXES to its output. The 3D view works in reservoir-native
    // (pre-MAPAXES) coords, so undo it for the preview if active.
    cvf::Mat4d invMapAxes     = cvf::Mat4d::IDENTITY;
    bool       applyInverse   = adapter.useMapAxes();
    if ( applyInverse )
    {
        invMapAxes = adapter.mapAxisTransform().getInverted();
    }

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( totalRefined * 24 ); // 6 faces × 4 corners per cell

    for ( size_t k = 0; k < refinedK; ++k )
    {
        for ( size_t j = 0; j < refinedJ; ++j )
        {
            for ( size_t i = 0; i < refinedI; ++i )
            {
                auto corners = adapter.getCellCorners( i, j, k );

                if ( applyInverse )
                {
                    for ( auto& c : corners )
                    {
                        c.transformPoint( invMapAxes );
                    }
                }

                for ( int faceEnum = cvf::StructGridInterface::POS_I; faceEnum < cvf::StructGridInterface::NO_FACE; ++faceEnum )
                {
                    auto       face = static_cast<cvf::StructGridInterface::FaceType>( faceEnum );
                    cvf::ubyte faceConn[4];
                    cvf::StructGridInterface::cellFaceVertexIndices( face, faceConn );

                    for ( int n = 0; n < 4; ++n )
                    {
                        auto display = coordTransform->transformToDisplayCoord( corners[faceConn[n]] );
                        vertices.push_back( cvf::Vec3f( display ) );
                    }
                }
            }
        }
    }

    if ( vertices.empty() ) return nullptr;

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray;
    vertexArray->assign( vertices );

    cvf::ref<cvf::UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( vertexArray.p() );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( indices.p() );
    geo->addPrimitiveSet( prim.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( "RivRefinementRegionPartMgr - refined cell wireframe" );
    part->setDrawable( geo.p() );

    caf::MeshEffectGenerator effGen( region->previewColor() );
    part->setEffect( effGen.generateUnCachedEffect().p() );

    return part;
}

//--------------------------------------------------------------------------------------------------
/// Fallback for very large regions: an 8-vertex wireframe box at the region's extremes.
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivRefinementRegionPartMgr::createOuterBoxPart( const RimRefinementRegion*        region,
                                                                    RimEclipseCase*                   eclipseCase,
                                                                    const caf::DisplayCoordTransform* coordTransform )
{
    auto* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return nullptr;
    auto* mainGrid = caseData->mainGrid();
    if ( !mainGrid ) return nullptr;

    const size_t gridI = mainGrid->cellCountI();
    const size_t gridJ = mainGrid->cellCountJ();
    const size_t gridK = mainGrid->cellCountK();

    const auto   rmin = region->ijkMin();
    const auto   rmax = region->ijkMax();
    const size_t minI = std::min( rmin.x(), gridI - 1 );
    const size_t minJ = std::min( rmin.y(), gridJ - 1 );
    const size_t minK = std::min( rmin.z(), gridK - 1 );
    const size_t maxI = std::min( rmax.x(), gridI - 1 );
    const size_t maxJ = std::min( rmax.y(), gridJ - 1 );
    const size_t maxK = std::min( rmax.z(), gridK - 1 );

    struct CornerPick
    {
        size_t i;
        size_t j;
        size_t k;
        size_t localCorner;
    };

    const std::array<CornerPick, 8> picks = { {
        { minI, minJ, minK, 0 },
        { maxI, minJ, minK, 1 },
        { maxI, maxJ, minK, 2 },
        { minI, maxJ, minK, 3 },
        { minI, minJ, maxK, 4 },
        { maxI, minJ, maxK, 5 },
        { maxI, maxJ, maxK, 6 },
        { minI, maxJ, maxK, 7 },
    } };

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 8 );
    for ( const auto& p : picks )
    {
        size_t cellIdx = mainGrid->cellIndexFromIJK( p.i, p.j, p.k );
        auto   corners = mainGrid->cellCornerVertices( cellIdx );
        auto   domain  = corners[p.localCorner];
        auto   display = coordTransform->transformToDisplayCoord( domain );
        vertices.push_back( cvf::Vec3f( display ) );
    }

    return RivBoxGeometryGenerator::createBoxFromVertices( vertices, region->previewColor() );
}
