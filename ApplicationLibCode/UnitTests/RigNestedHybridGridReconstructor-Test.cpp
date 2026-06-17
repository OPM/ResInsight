/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaResultNames.h"
#include "RiaTestDataDirectory.h"

#include "RifEclipseInputPropertyLoader.h"
#include "RifEclipseKeywordContent.h"
#include "RifEclipseTextFileReader.h"
#include "RifReaderEclipseOutput.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigHexIntersectionTools.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNestedHybridGridReconstructor.h"
#include "RigNncConnection.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseResultCase.h"

#include "cvfBoundingBox.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <cmath>
#include <iostream>

namespace
{
QString nestedHybridModelDir()
{
    QDir baseFolder( TEST_MODEL_DIR );
    baseFolder.cd( "NestedHybridGrid" );
    return baseFolder.absolutePath();
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Diagnostic: learn the structure of the supplied nested-hybrid test grid.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, DISABLED_InspectDrogonLayout )
{
    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_HIST_NESTED.EGRID";
    QString nestFile = dir + "/DROGON_HIST_NESTED_NEST_ID.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) ) << gridFile.toStdString();
    ASSERT_TRUE( QFile::exists( nestFile ) ) << nestFile.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );

    RigMainGrid* grid = reservoir->mainGrid();
    ASSERT_TRUE( grid != nullptr );
    grid->computeCachedData(); // build the cell search tree used by findReservoirCellIndexFromPoint

    const size_t nx = grid->cellCountI();
    const size_t ny = grid->cellCountJ();
    const size_t nz = grid->cellCountK();
    std::cout << "Main grid dims: " << nx << " x " << ny << " x " << nz << " = " << ( nx * ny * nz ) << " cells\n";
    std::cout << "mainGrid->cellCount()=" << grid->cellCount() << "  totalCellCount()=" << grid->totalCellCount()
              << "  gridCount()=" << grid->gridCount() << "\n";

    // Read NEST_ID raw values
    auto keywordContent = RifEclipseTextFileReader::readKeywordAndValues( nestFile.toStdString() );
    ASSERT_FALSE( keywordContent.empty() );
    const auto& nestKw = keywordContent.front();
    std::cout << "NEST_ID keyword='" << nestKw.keyword << "' values=" << nestKw.values.size() << "\n";
    ASSERT_EQ( nestKw.values.size(), nx * ny * nz );

    // Per-level IJK bounding boxes
    std::map<int, std::array<size_t, 6>> levelBounds; // level -> {iMin,iMax,jMin,jMax,kMin,kMax}
    std::map<int, size_t>                levelCounts;
    for ( size_t k = 0; k < nz; k++ )
    {
        for ( size_t j = 0; j < ny; j++ )
        {
            for ( size_t i = 0; i < nx; i++ )
            {
                size_t flat  = i + j * nx + k * nx * ny;
                int    level = (int)std::lround( nestKw.values[flat] );
                levelCounts[level]++;
                if ( !levelBounds.count( level ) )
                    levelBounds[level] = { i, i, j, j, k, k };
                else
                {
                    auto& b = levelBounds[level];
                    b[0]    = std::min( b[0], i );
                    b[1]    = std::max( b[1], i );
                    b[2]    = std::min( b[2], j );
                    b[3]    = std::max( b[3], j );
                    b[4]    = std::min( b[4], k );
                    b[5]    = std::max( b[5], k );
                }
            }
        }
    }

    for ( const auto& [level, b] : levelBounds )
    {
        std::cout << "Level " << level << ": count=" << levelCounts[level] << "  I[" << b[0] << ".." << b[1] << "] J[" << b[2] << ".."
                  << b[3] << "] K[" << b[4] << ".." << b[5] << "]\n";
    }

    // Spatial bboxes for level 0 vs level 1 (active cells only) to see if they overlap in space
    {
        cvf::BoundingBox bb0, bb1;
        for ( size_t idx = 0; idx < nestKw.values.size(); idx += 50 )
        {
            int  lvl = (int)std::lround( nestKw.values[idx] );
            auto c   = grid->cell( idx ).boundingBox();
            if ( c.isValid() )
            {
                if ( lvl == 0 )
                    bb0.add( c );
                else if ( lvl == 1 )
                    bb1.add( c );
            }
        }
        std::cout << "Level0 spatial bbox min=(" << bb0.min().x() << "," << bb0.min().y() << "," << bb0.min().z() << ") max=("
                  << bb0.max().x() << "," << bb0.max().y() << "," << bb0.max().z() << ")\n";
        std::cout << "Level1 spatial bbox min=(" << bb1.min().x() << "," << bb1.min().y() << "," << bb1.min().z() << ") max=("
                  << bb1.max().x() << "," << bb1.max().y() << "," << bb1.max().z() << ")\n";
    }

    RigActiveCellInfo* actInfo = reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    std::cout << "reservoirActiveCellCount=" << actInfo->reservoirActiveCellCount() << "\n";

    // Active count per level + level-1 footprint per K layer
    std::map<int, size_t>    activePerLevel;
    std::map<size_t, size_t> level1PerK;
    for ( size_t idx = 0; idx < nestKw.values.size(); idx++ )
    {
        int level = (int)std::lround( nestKw.values[idx] );
        if ( actInfo->isActive( ReservoirCellIndex( idx ) ) ) activePerLevel[level]++;
        if ( level == 1 )
        {
            size_t i, j, k;
            grid->ijkFromCellIndex( idx, &i, &j, &k );
            level1PerK[k]++;
        }
    }
    for ( const auto& [level, c] : activePerLevel )
        std::cout << "Level " << level << " ACTIVE count=" << c << "\n";
    std::cout << "Level1 cells in K=0: " << level1PerK[0] << "  K=1: " << level1PerK[1] << "  K=80: " << level1PerK[80]
              << "  K=159: " << level1PerK[159] << "\n";

    // Examine a sample active refined cell, its overlap with coarse cells, and volume ratio
    size_t sampleFlat = cvf::UNDEFINED_SIZE_T;
    for ( size_t idx = 0; idx < nestKw.values.size(); idx++ )
    {
        if ( (int)std::lround( nestKw.values[idx] ) == 1 && actInfo->isActive( ReservoirCellIndex( idx ) ) )
        {
            sampleFlat = idx;
            break;
        }
    }
    ASSERT_NE( sampleFlat, cvf::UNDEFINED_SIZE_T );
    {
        size_t si, sj, sk;
        grid->ijkFromCellIndex( sampleFlat, &si, &sj, &sk );
        double           refVol   = grid->cell( sampleFlat ).volume();
        cvf::BoundingBox cellBb   = grid->cell( sampleFlat ).boundingBox();
        cvf::Vec3d       centroid = cellBb.center();
        std::cout << "\nSample active refined cell flat=" << sampleFlat << " IJK(" << si << "," << sj << "," << sk << ") volume=" << refVol
                  << "\n";

        // Which cells actually CONTAIN the refined centroid (true geometric parents)?
        std::vector<size_t> overlapping = grid->findIntersectingCells( cellBb );
        std::cout << "bbox-overlap count=" << overlapping.size() << "; cells CONTAINING centroid:\n";
        for ( size_t ov : overlapping )
        {
            auto corners = grid->cellCornerVertices( ov );
            if ( !RigHexIntersectionTools::isPointInCell( centroid, corners ) ) continue;
            size_t oi, oj, ok;
            grid->ijkFromCellIndex( ov, &oi, &oj, &ok );
            std::cout << "  CONTAINS idx=" << ov << " IJK(" << oi << "," << oj << "," << ok
                      << ") nest=" << (int)std::lround( nestKw.values[ov] ) << " active=" << actInfo->isActive( ReservoirCellIndex( ov ) )
                      << " volume=" << grid->cell( ov ).volume() << "\n";
        }
    }

    // Aggregate: the set of true coarse (nest==0) parents over ALL level-1 cells (sample every Nth for speed)
    {
        std::array<size_t, 6> pBounds{ SIZE_MAX, 0, SIZE_MAX, 0, SIZE_MAX, 0 };
        size_t                parentsFound = 0, activeParents = 0, noParent = 0;
        size_t                step = 200; // sample for speed
        for ( size_t idx = 0; idx < nestKw.values.size(); idx += step )
        {
            if ( (int)std::lround( nestKw.values[idx] ) != 1 ) continue;
            if ( !actInfo->isActive( ReservoirCellIndex( idx ) ) ) continue;
            cvf::Vec3d c = grid->cell( idx ).boundingBox().center();
            // find containing nest==0 cell
            for ( size_t ov : grid->findIntersectingCells( grid->cell( idx ).boundingBox() ) )
            {
                if ( (int)std::lround( nestKw.values[ov] ) != 0 ) continue;
                if ( !RigHexIntersectionTools::isPointInCell( c, grid->cellCornerVertices( ov ) ) ) continue;
                size_t pi, pj, pk;
                grid->ijkFromCellIndex( ov, &pi, &pj, &pk );
                pBounds[0] = std::min( pBounds[0], pi );
                pBounds[1] = std::max( pBounds[1], pi );
                pBounds[2] = std::min( pBounds[2], pj );
                pBounds[3] = std::max( pBounds[3], pj );
                pBounds[4] = std::min( pBounds[4], pk );
                pBounds[5] = std::max( pBounds[5], pk );
                parentsFound++;
                if ( actInfo->isActive( ReservoirCellIndex( ov ) ) ) activeParents++;
                goto nextSample;
            }
            noParent++;
        nextSample:;
        }
        std::cout << "\nParent aggregation (sampled): parentsFound=" << parentsFound << " activeParents=" << activeParents
                  << " noParent=" << noParent << "\n";
        std::cout << "Parent (nest=0) IJK bbox: I[" << pBounds[0] << ".." << pBounds[1] << "] J[" << pBounds[2] << ".." << pBounds[3]
                  << "] K[" << pBounds[4] << ".." << pBounds[5] << "]\n";
    }

    // Hypothesis: coarse parents were collapsed (degenerate geometry). Scan nest=0 cells for tiny/zero volume.
    {
        std::array<size_t, 6> dBounds{ SIZE_MAX, 0, SIZE_MAX, 0, SIZE_MAX, 0 };
        size_t                collapsed = 0, collapsedInactive = 0;
        cvf::BoundingBox      collapsedSpatial;
        size_t                sampleCollapsed = cvf::UNDEFINED_SIZE_T;
        for ( size_t idx = 0; idx < nestKw.values.size(); idx++ )
        {
            if ( (int)std::lround( nestKw.values[idx] ) != 0 ) continue;
            double vol = grid->cell( idx ).volume();
            if ( vol > 1.0e-6 ) continue; // not collapsed
            collapsed++;
            if ( !actInfo->isActive( ReservoirCellIndex( idx ) ) ) collapsedInactive++;
            if ( sampleCollapsed == cvf::UNDEFINED_SIZE_T ) sampleCollapsed = idx;
            size_t i, j, k;
            grid->ijkFromCellIndex( idx, &i, &j, &k );
            dBounds[0] = std::min( dBounds[0], i );
            dBounds[1] = std::max( dBounds[1], i );
            dBounds[2] = std::min( dBounds[2], j );
            dBounds[3] = std::max( dBounds[3], j );
            dBounds[4] = std::min( dBounds[4], k );
            dBounds[5] = std::max( dBounds[5], k );
            auto bb    = grid->cell( idx ).boundingBox();
            if ( bb.isValid() ) collapsedSpatial.add( bb );
        }
        std::cout << "\nCollapsed nest=0 cells (vol~0): " << collapsed << " (inactive=" << collapsedInactive << ")\n";
        std::cout << "Collapsed IJK bbox: I[" << dBounds[0] << ".." << dBounds[1] << "] J[" << dBounds[2] << ".." << dBounds[3] << "] K["
                  << dBounds[4] << ".." << dBounds[5] << "]\n";
        if ( sampleCollapsed != cvf::UNDEFINED_SIZE_T )
        {
            auto bb = grid->cell( sampleCollapsed ).boundingBox();
            std::cout << "Sample collapsed cell spatial center=(" << bb.center().x() << "," << bb.center().y() << "," << bb.center().z()
                      << ")\n";
        }
    }

    // NNC connectivity: do NNCs connect refined (nest=1) cells to coarse (nest=0) cells?
    {
        RigNNCData* nnc   = grid->nncData();
        size_t      total = nnc ? nnc->availableConnections().size() : 0;
        std::cout << "\nNNC connections total=" << total << "\n";
        size_t                crossLevel = 0, refToCoarse = 0;
        std::array<size_t, 6> coarsePartnerBounds{ SIZE_MAX, 0, SIZE_MAX, 0, SIZE_MAX, 0 };
        for ( size_t c = 0; c < total; c++ )
        {
            const RigConnection& conn = nnc->availableConnections()[c];
            size_t               a = conn.c1GlobIdx(), b = conn.c2GlobIdx();
            if ( a >= nestKw.values.size() || b >= nestKw.values.size() ) continue;
            int la = (int)std::lround( nestKw.values[a] );
            int lb = (int)std::lround( nestKw.values[b] );
            if ( la != lb )
            {
                crossLevel++;
                size_t coarseIdx = ( la == 0 ) ? a : b;
                refToCoarse++;
                size_t pi, pj, pk;
                grid->ijkFromCellIndex( coarseIdx, &pi, &pj, &pk );
                coarsePartnerBounds[0] = std::min( coarsePartnerBounds[0], pi );
                coarsePartnerBounds[1] = std::max( coarsePartnerBounds[1], pi );
                coarsePartnerBounds[2] = std::min( coarsePartnerBounds[2], pj );
                coarsePartnerBounds[3] = std::max( coarsePartnerBounds[3], pj );
                coarsePartnerBounds[4] = std::min( coarsePartnerBounds[4], pk );
                coarsePartnerBounds[5] = std::max( coarsePartnerBounds[5], pk );
            }
        }
        std::cout << "Cross-level NNCs (nest differs)=" << crossLevel << "\n";
        if ( refToCoarse )
            std::cout << "Coarse NNC-partner IJK bbox: I[" << coarsePartnerBounds[0] << ".." << coarsePartnerBounds[1] << "] J["
                      << coarsePartnerBounds[2] << ".." << coarsePartnerBounds[3] << "] K[" << coarsePartnerBounds[4] << ".."
                      << coarsePartnerBounds[5] << "]\n";
    }
}

namespace
{
struct CellExtent
{
    double dx, dy, dz;
};
CellExtent cellExtent( const RigMainGrid* grid, size_t idx )
{
    auto bb = grid->cell( idx ).boundingBox();
    return { bb.max().x() - bb.min().x(), bb.max().y() - bb.min().y(), bb.max().z() - bb.min().z() };
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Offline generator: detect the refined region(s), parent region and refinement factor for the
/// supplied DROGON nested-hybrid grid, then write an example parent-mapping sidecar (HOSTNUM).
///
/// Sidecar format (minimal, one GRDECL keyword over the full flat IJK array):
///   HOSTNUM <natural 1-based index of parent coarse cell for each refined cell, else 0> /
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, DISABLED_GenerateHostNumSidecar )
{
    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_HIST_NESTED.EGRID";
    QString nestFile = dir + "/DROGON_HIST_NESTED_NEST_ID.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );
    ASSERT_TRUE( QFile::exists( nestFile ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();
    RigActiveCellInfo* actInfo = reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const size_t nx = grid->cellCountI(), ny = grid->cellCountJ(), nz = grid->cellCountK();

    auto kw = RifEclipseTextFileReader::readKeywordAndValues( nestFile.toStdString() );
    ASSERT_FALSE( kw.empty() );
    const std::vector<float>& nest = kw.front().values;
    ASSERT_EQ( nest.size(), nx * ny * nz );

    auto natIndex = []( size_t i, size_t j, size_t k, size_t nx, size_t ny ) { return i + j * nx + k * nx * ny; };

    // 1) Refined block index bbox (cells with nest >= 1)
    size_t oi = SIZE_MAX, oi1 = 0, oj = SIZE_MAX, oj1 = 0, ok = SIZE_MAX, ok1 = 0;
    size_t refSample = SIZE_MAX;
    for ( size_t k = 0; k < nz; k++ )
        for ( size_t j = 0; j < ny; j++ )
            for ( size_t i = 0; i < nx; i++ )
            {
                if ( (int)std::lround( nest[natIndex( i, j, k, nx, ny )] ) < 1 ) continue;
                oi          = std::min( oi, i );
                oi1         = std::max( oi1, i );
                oj          = std::min( oj, j );
                oj1         = std::max( oj1, j );
                ok          = std::min( ok, k );
                ok1         = std::max( ok1, k );
                size_t flat = natIndex( i, j, k, nx, ny );
                if ( refSample == SIZE_MAX && actInfo->isActive( ReservoirCellIndex( flat ) ) ) refSample = flat;
            }
    cvf::Vec3st blockDims( oi1 - oi + 1, oj1 - oj + 1, ok1 - ok + 1 );
    std::cout << "Refined block index origin=(" << oi << "," << oj << "," << ok << ") dims=(" << blockDims.x() << "," << blockDims.y()
              << "," << blockDims.z() << ")\n";

    // 2) Refinement factor per axis from cell-size ratio (coarse frame neighbor vs refined cell)
    // Frame = coarse partners of cross-level NNCs
    std::array<size_t, 6> frame{ SIZE_MAX, 0, SIZE_MAX, 0, SIZE_MAX, 0 };
    size_t                coarseSample = SIZE_MAX;
    RigNNCData*           nnc          = grid->nncData();
    for ( size_t c = 0; c < nnc->availableConnections().size(); c++ )
    {
        const RigConnection& conn = nnc->availableConnections()[c];
        size_t               a = conn.c1GlobIdx(), b = conn.c2GlobIdx();
        if ( a >= nest.size() || b >= nest.size() ) continue;
        int la = (int)std::lround( nest[a] ), lb = (int)std::lround( nest[b] );
        if ( la == lb ) continue;
        size_t coarse = ( la == 0 ) ? a : b;
        size_t ci, cj, ck;
        grid->ijkFromCellIndex( coarse, &ci, &cj, &ck );
        frame[0] = std::min( frame[0], ci );
        frame[1] = std::max( frame[1], ci );
        frame[2] = std::min( frame[2], cj );
        frame[3] = std::max( frame[3], cj );
        frame[4] = std::min( frame[4], ck );
        frame[5] = std::max( frame[5], ck );
        if ( coarseSample == SIZE_MAX && grid->cell( coarse ).volume() > 1.0 ) coarseSample = coarse;
    }
    ASSERT_NE( refSample, SIZE_MAX );
    ASSERT_NE( coarseSample, SIZE_MAX );

    // Areal factor (I,J) from cell-size ratio is reliable (areal cells are regular). Vertical thickness
    // varies per cell, so derive the K factor from the NNC frame K-span instead (lateral neighbours
    // share the parent's coarse K-layers, so the coarse-partner K-span == parent K extent).
    CellExtent re = cellExtent( grid, refSample ), ce = cellExtent( grid, coarseSample );
    size_t     factorI = (size_t)std::lround( ce.dx / re.dx );
    size_t     factorJ = (size_t)std::lround( ce.dy / re.dy );
    size_t     pExtK   = frame[5] - frame[4] + 1;
    ASSERT_GT( factorI, 0u );
    ASSERT_GT( factorJ, 0u );
    ASSERT_GT( pExtK, 0u );
    ASSERT_EQ( blockDims.x() % factorI, 0u );
    ASSERT_EQ( blockDims.y() % factorJ, 0u );
    ASSERT_EQ( blockDims.z() % pExtK, 0u );
    cvf::Vec3st factor( factorI, factorJ, blockDims.z() / pExtK );
    cvf::Vec3st pDims( blockDims.x() / factorI, blockDims.y() / factorJ, pExtK );
    std::cout << "Refined cell extent=(" << re.dx << "," << re.dy << "," << re.dz << ") coarse extent=(" << ce.dx << "," << ce.dy << ","
              << ce.dz << ")\n";
    std::cout << "Detected factor=(" << factor.x() << "," << factor.y() << "," << factor.z() << ")\n";
    std::cout << "NNC frame I[" << frame[0] << ".." << frame[1] << "] J[" << frame[2] << ".." << frame[3] << "] K[" << frame[4] << ".."
              << frame[5] << "]\n";

    // 3) Parent origin: lateral axes (I,J) use the frame-extent rule; K uses the frame min directly.
    auto originForAxis = [&]( size_t fMin, size_t fMax, size_t pDim, const char* axis ) -> size_t
    {
        size_t fExt = fMax - fMin + 1;
        if ( fExt == pDim + 2 ) return fMin + 1; // bounded on both lateral sides
        if ( fExt == pDim + 1 ) return fMin; // bounded on one side (touches grid edge)
        if ( fExt == pDim ) return fMin; // frame coincides
        std::cerr << "Ambiguous parent origin on " << axis << ": frameExt=" << fExt << " pDim=" << pDim << "\n";
        return fMin;
    };
    cvf::Vec3st pOrigin( originForAxis( frame[0], frame[1], pDims.x(), "I" ), originForAxis( frame[2], frame[3], pDims.y(), "J" ), frame[4] );
    std::cout << "Parent region origin=(" << pOrigin.x() << "," << pOrigin.y() << "," << pOrigin.z() << ") dims=(" << pDims.x() << ","
              << pDims.y() << "," << pDims.z() << ")\n";

    // 4) Build HOSTNUM over full flat array
    std::vector<int>         hostnum( nx * ny * nz, 0 );
    std::map<size_t, size_t> groupSize;
    for ( size_t k = ok; k <= ok1; k++ )
        for ( size_t j = oj; j <= oj1; j++ )
            for ( size_t i = oi; i <= oi1; i++ )
            {
                size_t flat = natIndex( i, j, k, nx, ny );
                if ( (int)std::lround( nest[flat] ) < 1 ) continue; // inactive remnant inside block
                size_t pi         = pOrigin.x() + ( i - oi ) / factor.x();
                size_t pj         = pOrigin.y() + ( j - oj ) / factor.y();
                size_t pk         = pOrigin.z() + ( k - ok ) / factor.z();
                size_t parentFlat = natIndex( pi, pj, pk, nx, ny );
                hostnum[flat]     = (int)( parentFlat + 1 ); // 1-based natural index
                groupSize[parentFlat]++;
            }

    // Validate grouping: each parent should host at most factor product children
    size_t prod     = factor.x() * factor.y() * factor.z();
    size_t maxGroup = 0, minGroup = SIZE_MAX, overfull = 0;
    for ( auto& [p, s] : groupSize )
    {
        maxGroup = std::max( maxGroup, s );
        minGroup = std::min( minGroup, s );
        if ( s > prod ) overfull++;
    }
    std::cout << "Parent groups=" << groupSize.size() << " expectedFullSize=" << prod << " minGroup=" << minGroup
              << " maxGroup=" << maxGroup << " overfull=" << overfull << "\n";
    EXPECT_EQ( overfull, 0u );
    EXPECT_LE( maxGroup, prod );

    // 5) Write the sidecar with simple run-length compression
    QString outPath = dir + "/DROGON_HIST_NESTED_HOSTNUM.grdecl";
    QFile   f( outPath );
    ASSERT_TRUE( f.open( QIODevice::WriteOnly | QIODevice::Text ) );
    QTextStream ts( &f );
    ts << "-- Generated by ResInsight: parent host cell (1-based natural index) for nested hybrid grid\n";
    ts << "HOSTNUM\n";
    size_t idx = 0, total = hostnum.size(), perLine = 0;
    while ( idx < total )
    {
        int    v   = hostnum[idx];
        size_t run = 1;
        while ( idx + run < total && hostnum[idx + run] == v )
            run++;
        if ( run >= 2 )
            ts << run << "*" << v << " ";
        else
            ts << v << " ";
        if ( ++perLine % 20 == 0 ) ts << "\n";
        idx += run;
    }
    ts << "\n/\n";
    f.close();
    std::cout << "Wrote " << outPath.toStdString() << "\n";
}

namespace
{
std::vector<int> readIntKeyword( const QString& filePath, const QString& keyword )
{
    auto content = RifEclipseTextFileReader::readKeywordAndValues( filePath.toStdString() );
    for ( const auto& kw : content )
    {
        if ( QString::fromStdString( kw.keyword ).compare( keyword, Qt::CaseInsensitive ) == 0 )
        {
            std::vector<int> values;
            values.reserve( kw.values.size() );
            for ( float v : kw.values )
                values.push_back( (int)std::lround( v ) );
            return values;
        }
    }
    return {};
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Reconstruct the LGR hierarchy from the explicit HOSTNUM parent mapping and verify the structure.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, ReconstructFromHostNum )
{
    QString dir         = nestedHybridModelDir();
    QString gridFile    = dir + "/DROGON_HIST_NESTED.EGRID";
    QString hostNumFile = dir + "/DROGON_HIST_NESTED_HOSTNUM.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );
    if ( !QFile::exists( hostNumFile ) )
    {
        GTEST_SKIP() << "HOSTNUM sidecar not present (run DISABLED_GenerateHostNumSidecar to create it): " << hostNumFile.toStdString();
    }

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigActiveCellInfo* actInfo         = reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const size_t       activeBefore    = actInfo->reservoirActiveCellCount();
    const size_t       flatCellCount   = grid->cellCount();
    const size_t       gridCountBefore = grid->gridCount();

    std::vector<int> hostNum = readIntKeyword( hostNumFile, "HOSTNUM" );
    ASSERT_EQ( hostNum.size(), flatCellCount );

    // Capture geometry of a known refined flat cell before reconstruction (IJK 47,0,0 -> parent 29,23,0)
    const size_t sampleFlat = 47; // i=47, j=0, k=0
    ASSERT_GT( hostNum[sampleFlat], 0 );
    std::array<cvf::Vec3d, 8> flatCornersBefore = grid->cellCornerVertices( sampleFlat );

    QString err;
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), hostNum, &err ) ) << err.toStdString();

    // Mirror the RimEclipseResultCase pipeline: rebuild grid caches over the new cell set.
    grid->computeCachedData();

    // One LGR was added
    EXPECT_EQ( grid->gridCount(), gridCountBefore + 1 );
    RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( 1 ) );
    ASSERT_TRUE( lgr != nullptr );

    // Cell counts: 20 x 40 x 160
    EXPECT_EQ( lgr->cellCountI(), 20u );
    EXPECT_EQ( lgr->cellCountJ(), 40u );
    EXPECT_EQ( lgr->cellCountK(), 160u );
    EXPECT_EQ( lgr->cellCount(), 20u * 40u * 160u );

    // Parent of LGR local cell 0 is coarse cell (29,23,0)
    size_t expectedParent = 29 + 23 * grid->cellCountI();
    EXPECT_EQ( lgr->cell( 0 ).parentCellIndex(), expectedParent );
    EXPECT_EQ( grid->cell( expectedParent ).subGrid(), lgr );
    EXPECT_EQ( lgr->cell( 0 ).hostGrid(), lgr );

    // computeCachedData resolved the parent-chain: the LGR cell maps back to its main-grid parent
    EXPECT_EQ( lgr->cell( 0 ).mainGridCellIndex(), expectedParent );
    EXPECT_EQ( lgr->parentGrid(), grid );

    // The original scattered cell is now hidden (invalid), and the LGR cell carries its geometry
    EXPECT_TRUE( grid->cell( sampleFlat ).isInvalid() );
    std::array<cvf::Vec3d, 8> lgrCorners = grid->cellCornerVertices( lgr->reservoirCellIndex( 0 ) );
    for ( int c = 0; c < 8; c++ )
    {
        EXPECT_NEAR( lgrCorners[c].x(), flatCornersBefore[c].x(), 1e-6 );
        EXPECT_NEAR( lgrCorners[c].y(), flatCornersBefore[c].y(), 1e-6 );
        EXPECT_NEAR( lgrCorners[c].z(), flatCornersBefore[c].z(), 1e-6 );
    }

    // The LGR cells are added as new active cells; the source flat cells remain active (but hidden),
    // so the active-cell count grows by the number of active LGR cells.
    EXPECT_GT( actInfo->reservoirActiveCellCount(), activeBefore );
    EXPECT_TRUE( actInfo->isActive( ReservoirCellIndex( lgr->reservoirCellIndex( 0 ) ) ) );
    EXPECT_TRUE( actInfo->isActive( ReservoirCellIndex( sampleFlat ) ) );

    // Result index == position in the active-cell list (invariant required by index calculators).
    EXPECT_EQ( actInfo->cellResultIndex( ReservoirCellIndex( lgr->reservoirCellIndex( 0 ) ) ).value(), activeBefore );

    // NNCs that referenced the scattered refined cells were re-pointed to the LGR cells. No
    // connection should reference a cell inside the refined block anymore; some should now
    // reference LGR cells (global index >= old flat cell count).
    RigNNCData*  nnc         = grid->nncData();
    const size_t lgrFirstIdx = lgr->reservoirCellIndex( 0 );
    size_t       refToBlock  = 0;
    size_t       refToLgr    = 0;
    for ( size_t c = 0; c < nnc->availableConnections().size(); c++ )
    {
        const RigConnection& conn = nnc->availableConnections()[c];
        for ( size_t idx : { conn.c1GlobIdx(), conn.c2GlobIdx() } )
        {
            if ( idx < flatCellCount && hostNum[idx] > 0 ) refToBlock++;
            if ( idx >= lgrFirstIdx ) refToLgr++;
        }
    }
    EXPECT_EQ( refToBlock, 0u );
    EXPECT_GT( refToLgr, 0u );

    // Regression: per-grid well-cell cache must cover the new LGR grid (was sized for the flat grid,
    // causing "gridIndex < m_wellCellsInGrid.size()" assertion during view creation).
    EXPECT_TRUE( reservoir->wellCellsInGrid( 0 ) != nullptr );
    ASSERT_TRUE( reservoir->wellCellsInGrid( 1 ) != nullptr );
    EXPECT_EQ( reservoir->wellCellsInGrid( 1 )->size(), lgr->cellCount() );
}

//--------------------------------------------------------------------------------------------------
/// Verify that results propagate to the reconstructed LGR cells: both an active-cell-indexed INIT
/// result and a full-length input property must read, on an LGR cell, the value of the source flat
/// refined cell it was built from.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, ResultsFollowLgrCells )
{
    using namespace RiaDefines;

    QString dir         = nestedHybridModelDir();
    QString gridFile    = dir + "/DROGON_HIST_NESTED.EGRID";
    QString hostNumFile = dir + "/DROGON_HIST_NESTED_HOSTNUM.grdecl";
    QString nestFile    = dir + "/DROGON_HIST_NESTED_NEST_ID.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );
    if ( !QFile::exists( hostNumFile ) || !QFile::exists( nestFile ) )
    {
        GTEST_SKIP() << "Sidecar(s) not present.";
    }

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigCaseCellResultsData* matrixResults = reservoir->results( PorosityModelType::MATRIX_MODEL );
    matrixResults->setReaderInterface( reader.p() );

    const size_t sampleFlat    = 47; // main-grid local index = global index for the main grid
    const size_t mainCellCount = grid->cellCount();

    // (a) Active-cell-indexed INIT result: load before reconstruction and capture the source value.
    QStringList statics = matrixResults->resultNames( ResultCatType::STATIC_NATIVE );
    ASSERT_FALSE( statics.empty() );
    RigEclipseResultAddress staticAddr( ResultCatType::STATIC_NATIVE, statics.front() );
    ASSERT_TRUE( matrixResults->ensureKnownResultLoaded( staticAddr ) );
    double sourceStaticValue =
        RigResultAccessorFactory::createFromResultAddress( reservoir.p(), 0, PorosityModelType::MATRIX_MODEL, 0, staticAddr )->cellScalar( sampleFlat );

    // (b) Full-length input property (NEST_ID): load before reconstruction.
    RifEclipseInputPropertyLoader::readProperties( nestFile, reservoir.p() );
    RigEclipseResultAddress nestAddr( ResultCatType::INPUT_PROPERTY, ResultDataType::INTEGER, "NEST_ID" );
    ASSERT_TRUE( matrixResults->cellScalarResults( nestAddr ).size() > 0 );
    double sourceNestValue = matrixResults->cellScalarResults( nestAddr )[0][sampleFlat];
    EXPECT_DOUBLE_EQ( sourceNestValue, 1.0 ); // sample cell is a level-1 refined cell

    // Reconstruct.
    std::vector<int> hostNum = readIntKeyword( hostNumFile, "HOSTNUM" );
    ASSERT_EQ( hostNum.size(), mainCellCount );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), hostNum ) );
    grid->computeCachedData();

    RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( 1 ) );
    ASSERT_TRUE( lgr != nullptr );

    // (a) LGR local cell 0 reads the source flat cell's INIT value via the inherited result index.
    double lgrStaticValue =
        RigResultAccessorFactory::createFromResultAddress( reservoir.p(), 1, PorosityModelType::MATRIX_MODEL, 0, staticAddr )->cellScalar( 0 );
    EXPECT_DOUBLE_EQ( lgrStaticValue, sourceStaticValue );

    // (b) The full-length NEST_ID array was extended so the LGR cell carries the refined level.
    const std::vector<std::vector<double>>& nestValues = matrixResults->cellScalarResults( nestAddr );
    ASSERT_EQ( nestValues[0].size(), grid->totalCellCount() );
    EXPECT_DOUBLE_EQ( nestValues[0][lgr->reservoirCellIndex( 0 )], sourceNestValue );
}

//--------------------------------------------------------------------------------------------------
/// INDEX_I/J/K on LGR cells must reflect the LGR-local IJK, not the source flat cell's indices.
/// Regression for: cells 782560 and 782580 (same local I) reported different INDEX_I.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, IndexIjkOnLgrCells )
{
    using namespace RiaDefines;

    QString dir         = nestedHybridModelDir();
    QString gridFile    = dir + "/DROGON_HIST_NESTED.EGRID";
    QString hostNumFile = dir + "/DROGON_HIST_NESTED_HOSTNUM.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );
    if ( !QFile::exists( hostNumFile ) ) GTEST_SKIP() << "HOSTNUM sidecar not present.";

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();
    reservoir->results( PorosityModelType::MATRIX_MODEL )->setReaderInterface( reader.p() );

    std::vector<int> hostNum = readIntKeyword( hostNumFile, "HOSTNUM" );
    ASSERT_EQ( hostNum.size(), grid->cellCount() );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), hostNum ) );
    grid->computeCachedData();

    RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( 1 ) );
    ASSERT_TRUE( lgr != nullptr );

    // Register the computed INDEX_I/J/K result entries (normally done during case setup), then load.
    reservoir->results( PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();
    RigEclipseResultAddress indexIAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::indexIResultName( false ) );
    ASSERT_TRUE( reservoir->results( PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( indexIAddr ) );
    auto acc = RigResultAccessorFactory::createFromResultAddress( reservoir.p(), 1, PorosityModelType::MATRIX_MODEL, 0, indexIAddr );
    ASSERT_TRUE( acc.notNull() );

    // LGR is 20 x 40 x 160. Local cell 0 = (i0,j0), 1 = (i1,j0), 20 = (i0,j1). INDEX_I is 1-based.
    double idxLocal0  = acc->cellScalar( 0 ); // i = 0 -> 1
    double idxLocal1  = acc->cellScalar( 1 ); // i = 1 -> 2
    double idxLocal20 = acc->cellScalar( 20 ); // i = 0 -> 1 (next J row)

    EXPECT_DOUBLE_EQ( idxLocal0, 1.0 );
    EXPECT_DOUBLE_EQ( idxLocal1, 2.0 );
    EXPECT_DOUBLE_EQ( idxLocal20, 1.0 );
    EXPECT_DOUBLE_EQ( idxLocal0, idxLocal20 ); // the originally reported bug
}

//--------------------------------------------------------------------------------------------------
/// Dynamic + computed results on LGR cells: the reconstructed LGR must not be counted as an on-file
/// grid (else the reader reads a phantom grid, corrupting dynamic results and wiping computed ones).
/// SWAT/SGAS load with all time steps, and computed SOIL (= 1 - SWAT - SGAS) is finite on the LGR.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, DynamicAndComputedResultsOnLgr )
{
    using namespace RiaDefines;
    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_HIST_NESTED.EGRID";
    QString hostNum  = dir + "/DROGON_HIST_NESTED_HOSTNUM.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );
    if ( !QFile::exists( hostNum ) ) GTEST_SKIP() << "HOSTNUM sidecar not present.";

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigCaseCellResultsData* res = reservoir->results( PorosityModelType::MATRIX_MODEL );
    res->setReaderInterface( reader.p() );
    res->createPlaceholderResultEntries();

    std::vector<int> hn = readIntKeyword( hostNum, "HOSTNUM" );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), hn ) );
    grid->computeCachedData();

    // The reconstructed LGR must not inflate the on-file grid count.
    EXPECT_EQ( grid->gridCountOnFile(), 1u );
    EXPECT_EQ( grid->gridCount(), 2u );

    RigActiveCellInfo* ai = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );
    RigLocalGrid*      lg = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( 1 ) );
    ASSERT_TRUE( lg != nullptr );
    size_t la = ai->cellResultIndex( ReservoirCellIndex( lg->reservoirCellIndex( 0 ) ) ).value();
    size_t fa = ai->cellResultIndex( ReservoirCellIndex( 47 ) ).value(); // LGR cell 0 was built from flat cell 47

    auto loadTs0 = [&]( const char* name ) -> const std::vector<double>&
    {
        RigEclipseResultAddress a( ResultCatType::DYNAMIC_NATIVE, QString( name ) );
        EXPECT_TRUE( res->ensureKnownResultLoaded( a ) ) << name;
        EXPECT_GT( res->timeStepCount( a ), 1u ) << name; // all time steps present, not wiped to 0/1
        return res->cellScalarResults( a, 0 );
    };

    const std::vector<double>& swat = loadTs0( "SWAT" );
    const std::vector<double>& sgas = loadTs0( "SGAS" );
    const std::vector<double>& soil = loadTs0( "SOIL" );

    ASSERT_GT( swat.size(), la );
    ASSERT_GT( soil.size(), la );

    // LGR cell carries the source flat cell's value, and SOIL is finite and consistent.
    EXPECT_DOUBLE_EQ( swat[la], swat[fa] );
    EXPECT_DOUBLE_EQ( soil[la], soil[fa] );
    EXPECT_FALSE( std::isnan( soil[la] ) );
    EXPECT_FALSE( std::isinf( soil[la] ) );
    EXPECT_NEAR( soil[la], 1.0 - swat[la] - sgas[la], 1e-6 );
}
