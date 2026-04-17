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

#include "RiaPreferences.h"

#include "RivBoxGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivScalarMapperUtils.h"

#include "ResultAccessors/RigResultAccessor.h"
#include "ResultAccessors/RigResultAccessorFactory.h"
#include "RigEclipseCaseData.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigRefinement.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimRefinementRegion.h"
#include "RimRefinementRegionCollection.h"
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfMatrix4.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <array>
#include <cmath>

namespace
{
// Above this refined-cell count, the per-cell solid + wireframe rendering is skipped in favour of the outer-box wireframe.
constexpr size_t MAX_CELLS_FOR_PER_CELL_GEOMETRY = 200000;

constexpr int VERTICES_PER_SUB_CELL    = 24; // 6 faces x 4 corners
constexpr int TRI_INDICES_PER_SUB_CELL = 36; // 6 faces x 2 triangles x 3 indices
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivRefinementRegionPartMgr::RivRefinementRegionPartMgr() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivRefinementRegionPartMgr::~RivRefinementRegionPartMgr() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::clearGeometryCache()
{
    m_regionCaches.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::buildGeometry( const RimRefinementRegionCollection* collection,
                                                RimEclipseCase*                      eclipseCase,
                                                const caf::DisplayCoordTransform*    coordTransform )
{
    clearGeometryCache();

    if ( !collection || !eclipseCase || !coordTransform ) return;
    if ( !collection->isActive() ) return;

    for ( auto* region : collection->regions() )
    {
        if ( !region || !region->isActive() ) continue;
        buildRegionCache( region, eclipseCase, coordTransform );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::appendStaticPartsToModel( cvf::ModelBasicList* model )
{
    if ( !model ) return;

    for ( auto& cache : m_regionCaches )
    {
        if ( cache.facePart.notNull() ) model->addPart( cache.facePart.p() );
        if ( cache.meshPart.notNull() ) model->addPart( cache.meshPart.p() );
        if ( cache.outerBoxPart.notNull() ) model->addPart( cache.outerBoxPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Build the cache entry for a single region: per-sub-cell vertex array, triangle index list for
/// the solid faces, line index list for the mesh overlay, and parent-cell index list used by
/// updateCellResultColor() to look up the result value of the original (un-refined) cell.
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::buildRegionCache( const RimRefinementRegion*        region,
                                                   RimEclipseCase*                   eclipseCase,
                                                   const caf::DisplayCoordTransform* coordTransform )
{
    auto* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;
    auto* mainGrid = caseData->mainGrid();
    if ( !mainGrid ) return;

    const size_t gridI = mainGrid->cellCountI();
    const size_t gridJ = mainGrid->cellCountJ();
    const size_t gridK = mainGrid->cellCountK();
    if ( gridI == 0 || gridJ == 0 || gridK == 0 ) return;

    const auto   rmin = region->ijkMin();
    const auto   rmax = region->ijkMax();
    const size_t minI = std::min( rmin.x(), gridI - 1 );
    const size_t minJ = std::min( rmin.y(), gridJ - 1 );
    const size_t minK = std::min( rmin.z(), gridK - 1 );
    const size_t maxI = std::min( rmax.x(), gridI - 1 );
    const size_t maxJ = std::min( rmax.y(), gridJ - 1 );
    const size_t maxK = std::min( rmax.z(), gridK - 1 );

    auto refinement = region->effectiveRefinement();
    if ( !refinement )
    {
        RegionCache cache;
        cache.fallbackColor = region->previewColor();
        cache.gridIndex     = 0;
        cache.outerBoxPart  = createOuterBoxPart( region, eclipseCase, coordTransform );
        if ( cache.outerBoxPart.notNull() ) m_regionCaches.push_back( cache );
        return;
    }

    cvf::Vec3st sectorMin( minI, minJ, minK );
    cvf::Vec3st sectorMax( maxI, maxJ, maxK );

    RigGridExportAdapter adapter( caseData, sectorMin, sectorMax, *refinement );

    const size_t refinedI     = adapter.cellCountI();
    const size_t refinedJ     = adapter.cellCountJ();
    const size_t refinedK     = adapter.cellCountK();
    const size_t totalRefined = refinedI * refinedJ * refinedK;
    if ( totalRefined == 0 ) return;

    if ( totalRefined > MAX_CELLS_FOR_PER_CELL_GEOMETRY )
    {
        RegionCache cache;
        cache.fallbackColor = region->previewColor();
        cache.gridIndex     = 0;
        cache.outerBoxPart  = createOuterBoxPart( region, eclipseCase, coordTransform );
        if ( cache.outerBoxPart.notNull() ) m_regionCaches.push_back( cache );
        return;
    }

    // RigGridExportAdapter applies MAPAXES to its output. The 3D view works in reservoir-native
    // (pre-MAPAXES) coords, so undo it for the preview if active.
    cvf::Mat4d invMapAxes   = cvf::Mat4d::IDENTITY;
    bool       applyInverse = adapter.useMapAxes();
    if ( applyInverse ) invMapAxes = adapter.mapAxisTransform().getInverted();

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( totalRefined * VERTICES_PER_SUB_CELL );

    std::vector<size_t> parentCellIdx;
    parentCellIdx.reserve( totalRefined );

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
                        c.transformPoint( invMapAxes );
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

                auto mapping = adapter.mapRefinedToOriginal( i, j, k );
                parentCellIdx.push_back( mainGrid->cellIndexFromIJK( mapping.originalI, mapping.originalJ, mapping.originalK ) );
            }
        }
    }

    if ( vertices.empty() ) return;

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray;
    vertexArray->assign( vertices );

    // Triangle indices: each face quad [a,b,c,d] becomes two triangles [a,b,c] and [a,c,d].
    cvf::ref<cvf::UIntArray> triIndices = new cvf::UIntArray;
    triIndices->resize( totalRefined * TRI_INDICES_PER_SUB_CELL );
    for ( size_t cell = 0; cell < totalRefined; ++cell )
    {
        const cvf::uint baseVertex = static_cast<cvf::uint>( cell * VERTICES_PER_SUB_CELL );
        const size_t    baseIndex  = cell * TRI_INDICES_PER_SUB_CELL;
        for ( int face = 0; face < 6; ++face )
        {
            const cvf::uint q = baseVertex + face * 4;
            const size_t    t = baseIndex + face * 6;
            triIndices->set( t + 0, q + 0 );
            triIndices->set( t + 1, q + 1 );
            triIndices->set( t + 2, q + 2 );
            triIndices->set( t + 3, q + 0 );
            triIndices->set( t + 4, q + 2 );
            triIndices->set( t + 5, q + 3 );
        }
    }

    cvf::ref<cvf::DrawableGeo> faceGeo = new cvf::DrawableGeo;
    faceGeo->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> trianglePrim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_TRIANGLES );
    trianglePrim->setIndices( triIndices.p() );
    faceGeo->addPrimitiveSet( trianglePrim.p() );
    faceGeo->computeNormals();

    cvf::ref<cvf::Part> facePart = new cvf::Part;
    facePart->setName( "RivRefinementRegionPartMgr - refined cell faces" );
    facePart->setDrawable( faceGeo.p() );
    facePart->setPriority( RivPartPriority::PartType::BaseLevel );

    // Mesh wireframe overlay.
    cvf::ref<cvf::UIntArray>   lineIndices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( vertexArray.p() );
    cvf::ref<cvf::DrawableGeo> meshGeo     = new cvf::DrawableGeo;
    meshGeo->setVertexArray( vertexArray.p() );
    cvf::ref<cvf::PrimitiveSetIndexedUInt> linePrim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    linePrim->setIndices( lineIndices.p() );
    meshGeo->addPrimitiveSet( linePrim.p() );

    cvf::ref<cvf::Part> meshPart = new cvf::Part;
    meshPart->setName( "RivRefinementRegionPartMgr - refined cell mesh" );
    meshPart->setDrawable( meshGeo.p() );
    meshPart->setPriority( RivPartPriority::PartType::MeshLines );

    caf::MeshEffectGenerator meshEffGen( RiaPreferences::current()->defaultGridLineColors() );
    meshPart->setEffect( meshEffGen.generateCachedEffect().p() );

    RegionCache cache;
    cache.facePart            = facePart;
    cache.meshPart            = meshPart;
    cache.parentGlobalCellIdx = std::move( parentCellIdx );
    cache.fallbackColor       = region->previewColor();
    cache.gridIndex           = 0;
    cache.textureCoords       = new cvf::Vec2fArray;
    cache.textureCoords->resize( vertices.size() );

    // Apply a flat fallback color so the part renders sensibly before updateCellResultColor() runs.
    applyFlatColorToFacePart( cache );

    m_regionCaches.push_back( std::move( cache ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::applyFlatColorToFacePart( RegionCache& cache )
{
    if ( cache.facePart.isNull() ) return;

    caf::SurfaceEffectGenerator surfEff( cvf::Color4f( cache.fallbackColor, 1.0f ), caf::PO_1 );
    cache.facePart->setEffect( surfEff.generateCachedEffect().p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivRefinementRegionPartMgr::updateCellResultColor( size_t timeStepIndex, RimEclipseCellColors* cellResultColors )
{
    if ( m_regionCaches.empty() ) return;
    if ( !cellResultColors ) return;

    auto* view = cellResultColors->reservoirView();
    if ( !view ) return;
    auto* eclipseCase = view->eclipseCase();
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return;

    const bool               noResult = !cellResultColors->hasResult();
    const cvf::ScalarMapper* mapper   = noResult ? nullptr : cellResultColors->legendConfig()->scalarMapper();
    const bool               ternary  = cellResultColors->isTernarySaturationSelected();
    const bool               lighting = view->isLightingDisabled();

    // Ternary saturations are not supported for the preview; fall back to a flat color so the
    // user still sees the region clearly.
    if ( noResult || !mapper || ternary )
    {
        for ( auto& cache : m_regionCaches )
            applyFlatColorToFacePart( cache );
        return;
    }

    cvf::ref<RigResultAccessor> accessor =
        RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(), 0, timeStepIndex, cellResultColors );
    if ( accessor.isNull() )
    {
        for ( auto& cache : m_regionCaches )
            applyFlatColorToFacePart( cache );
        return;
    }

    for ( auto& cache : m_regionCaches )
    {
        if ( cache.facePart.isNull() || cache.textureCoords.isNull() ) continue;
        if ( cache.parentGlobalCellIdx.empty() ) continue;

        const size_t numSubCells = cache.parentGlobalCellIdx.size();
        cvf::Vec2f*  rawTexCoord = cache.textureCoords->ptr();

        for ( size_t s = 0; s < numSubCells; ++s )
        {
            double     value = accessor->cellScalar( cache.parentGlobalCellIdx[s] );
            cvf::Vec2f texCoord;
            if ( value == HUGE_VAL || std::isnan( value ) || std::isinf( value ) )
            {
                // y=1.0 targets the "undefined" texel row of the legend texture, rendered in undefColor.
                texCoord = cvf::Vec2f( 0.0f, 1.0f );
            }
            else
            {
                texCoord = mapper->mapToTextureCoord( value );
            }

            const size_t base = s * VERTICES_PER_SUB_CELL;
            for ( int v = 0; v < VERTICES_PER_SUB_CELL; ++v )
            {
                rawTexCoord[base + v] = texCoord;
            }
        }

        RivScalarMapperUtils::applyTextureResultsToPart( cache.facePart.p(), cache.textureCoords.p(), mapper, 1.0f, caf::FC_NONE, lighting );
    }
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
