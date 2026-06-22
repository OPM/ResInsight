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
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNestedHybridGridReconstructor.h"
#include "RigNncConnection.h"

#include "RimEclipseResultCase.h"

#include <QDir>
#include <QFile>

#include <array>
#include <cmath>
#include <map>
#include <set>
#include <tuple>

//==================================================================================================
// Tests for the "nested hybrid grid" LGR reconstruction from the REFINE + OLDIJK sidecars.
//
// The test model TestModels/NestedHybridGrid/DROGON_NESTED.* is a single flat 150x84x96 EGRID where
// the refined cells of each coarse (15x24x12) cell are appended in per-level I bands. The sidecars:
//   DROGON_NESTED_REFINE.grdecl  : per-cell nesting level (1 base, 2/3/4 refined)
//   DROGON_NESTED_OLDIJK.grdecl  : OLDI/OLDJ/OLDK (coarse parent IJK) + TMPI/TMPJ/TMPK (local coords)
//==================================================================================================

namespace
{
QString nestedHybridModelDir()
{
    QDir baseFolder( TEST_MODEL_DIR );
    baseFolder.cd( "NestedHybridGrid" );
    return baseFolder.absolutePath();
}

// Read all integer keywords of a GRDECL file in a single pass.
std::map<QString, std::vector<int>> readIntKeywords( const QString& filePath )
{
    std::map<QString, std::vector<int>> result;
    auto                                content = RifEclipseTextFileReader::readKeywordAndValues( filePath.toStdString() );
    for ( const auto& kw : content )
    {
        std::vector<int> values;
        values.reserve( kw.values.size() );
        for ( float v : kw.values )
            values.push_back( (int)std::lround( v ) );
        result[QString::fromStdString( kw.keyword )] = std::move( values );
    }
    return result;
}

RigNestedHybridGridReconstructor::NestedHybridInput buildInput( const QString& dir )
{
    RigNestedHybridGridReconstructor::NestedHybridInput input;
    auto                                                refine = readIntKeywords( dir + "/DROGON_NESTED_REFINE.grdecl" );
    auto                                                oldIjk = readIntKeywords( dir + "/DROGON_NESTED_OLDIJK.grdecl" );
    input.refine                                               = refine["REFINE"];
    input.oldI                                                 = oldIjk["OLDI"];
    input.oldJ                                                 = oldIjk["OLDJ"];
    input.oldK                                                 = oldIjk["OLDK"];
    input.tmpI                                                 = oldIjk["TMPI"];
    input.tmpJ                                                 = oldIjk["TMPJ"];
    input.tmpK                                                 = oldIjk["TMPK"];
    return input;
}

RigLocalGrid* findLgrByName( RigMainGrid* grid, const QString& name )
{
    for ( size_t i = 1; i < grid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ) == name ) return lgr;
    }
    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Reconstruct the LGR hierarchy from the REFINE + OLDIJK sidecars and verify the structure.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, ReconstructFromOldIjk )
{
    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_NESTED.EGRID";
    ASSERT_TRUE( QFile::exists( gridFile ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    EXPECT_EQ( grid->cellCountI(), 150u );
    EXPECT_EQ( grid->cellCountJ(), 84u );
    EXPECT_EQ( grid->cellCountK(), 96u );

    RigActiveCellInfo* actInfo      = reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const size_t       activeBefore = actInfo->reservoirActiveCellCount();

    auto input = buildInput( dir );
    ASSERT_EQ( input.refine.size(), grid->cellCount() );
    ASSERT_EQ( input.oldI.size(), grid->cellCount() );

    // Distinct coarse parents that own a level-4 region (one nested LGR is built per such parent).
    std::set<std::tuple<int, int, int>> level4Parents;
    for ( size_t f = 0; f < input.refine.size(); f++ )
    {
        if ( input.refine[f] == 4 && input.oldI[f] >= 1 )
            level4Parents.insert( std::make_tuple( input.oldI[f], input.oldJ[f], input.oldK[f] ) );
    }
    ASSERT_GT( level4Parents.size(), 0u );

    QString err;
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), input, &err ) ) << err.toStdString();
    grid->computeCachedData();

    // Count the level-4 LGRs (named "LGR_NHG_L4_<component>"). They are merged into connected regions,
    // so there are far fewer than the number of coarse parents that own level-4 cells.
    size_t numLevel4Lgrs = 0;
    for ( size_t i = 1; i < grid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4_" ) ) numLevel4Lgrs++;
    }
    EXPECT_GT( numLevel4Lgrs, 0u );
    EXPECT_LT( numLevel4Lgrs, level4Parents.size() ); // merged, not one-per-parent

    // Level 2 and level 3 each become one LGR refining the coarse grid; level 4 nests inside level 3
    // as merged regions. None is counted as an on-file grid.
    EXPECT_EQ( grid->gridCount(), 1u + 2u + numLevel4Lgrs );
    EXPECT_EQ( grid->gridCountOnFile(), 1u );

    // Level 2: one LGR over coarse block 13x22x12 refined 2x2x4 -> 26x44x48.
    RigLocalGrid* lgr2 = findLgrByName( grid, "LGR_NHG_L2" );
    ASSERT_TRUE( lgr2 != nullptr );
    EXPECT_EQ( lgr2->cellCountI(), 26u );
    EXPECT_EQ( lgr2->cellCountJ(), 44u );
    EXPECT_EQ( lgr2->cellCountK(), 48u );
    EXPECT_TRUE( lgr2->isReconstructedGrid() );
    EXPECT_EQ( lgr2->parentGrid(), grid );

    // Level 3: one LGR over coarse block 12x19x12 refined 4x4x4 -> 48x76x48.
    RigLocalGrid* lgr3 = findLgrByName( grid, "LGR_NHG_L3" );
    ASSERT_TRUE( lgr3 != nullptr );
    EXPECT_EQ( lgr3->cellCountI(), 48u );
    EXPECT_EQ( lgr3->cellCountJ(), 76u );
    EXPECT_EQ( lgr3->cellCountK(), 48u );

    // Level 4 nests inside the level-3 LGR (true LGR-in-LGR). Find a level-4 LGR and verify its parent
    // grid is the level-3 LGR, and the level-3 cell it subdivides points back to it.
    RigLocalGrid* lgr4 = nullptr;
    for ( size_t i = 1; i < grid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( grid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4_" ) )
        {
            lgr4 = lgr;
            break;
        }
    }
    ASSERT_TRUE( lgr4 != nullptr );
    EXPECT_EQ( lgr4->parentGrid(), lgr3 );
    EXPECT_TRUE( lgr4->isReconstructedGrid() );

    {
        // Pick a real (sourced) level-4 cell and verify its parent is a level-3 LGR cell whose subgrid
        // is this level-4 LGR (parentCellIndex must be local to the level-3 grid).
        const size_t                    l4Begin     = lgr4->reservoirCellIndex( 0 );
        const size_t                    l4End       = l4Begin + lgr4->cellCount();
        const std::map<size_t, size_t>& srcAll      = grid->nestedHybridLgrSourceCells();
        bool                            checkedNest = false;
        for ( const auto& [lgrGlobal, flat] : srcAll )
        {
            if ( lgrGlobal < l4Begin || lgrGlobal >= l4End ) continue;
            size_t parentLocal  = grid->cell( lgrGlobal ).parentCellIndex();
            size_t parentGlobal = lgr3->reservoirCellIndex( parentLocal );
            EXPECT_EQ( grid->cell( parentGlobal ).subGrid(), lgr4 );
            EXPECT_EQ( grid->cell( parentGlobal ).hostGrid(), lgr3 );
            checkedNest = true;
            break;
        }
        EXPECT_TRUE( checkedNest );
    }

    // Active count grew (LGR cells added as new active cells; source flat cells stay active).
    EXPECT_GT( actInfo->reservoirActiveCellCount(), activeBefore );

    // Each reconstructed LGR cell carries its source flat cell's geometry; the original flat cell is
    // hidden; and the parent coarse cell it subdivides points back to the LGR it belongs to.
    const size_t                    l2Begin     = lgr2->reservoirCellIndex( 0 );
    const size_t                    l2End       = l2Begin + lgr2->cellCount();
    const std::map<size_t, size_t>& sourceCells = grid->nestedHybridLgrSourceCells();
    ASSERT_FALSE( sourceCells.empty() );
    int checkedL2 = 0;
    for ( const auto& [lgrGlobal, flat] : sourceCells )
    {
        EXPECT_TRUE( grid->cell( flat ).isInvalid() );

        if ( lgrGlobal >= l2Begin && lgrGlobal < l2End && checkedL2++ < 50 )
        {
            // The parent coarse cell of an L2 cell is refined by the L2 LGR.
            size_t parent = grid->cell( lgrGlobal ).parentCellIndex();
            EXPECT_EQ( grid->cell( parent ).subGrid(), lgr2 );
            EXPECT_EQ( grid->cell( lgrGlobal ).hostGrid(), lgr2 );

            auto lgrCorners = grid->cellCornerVertices( lgrGlobal );
            for ( int k = 0; k < 8; k++ )
                EXPECT_FALSE( std::isnan( lgrCorners[k].x() ) );
        }
    }
    EXPECT_GT( checkedL2, 0 );
}

//--------------------------------------------------------------------------------------------------
/// Results propagate to the reconstructed LGR cells: an active-cell-indexed STATIC result and the
/// full-length REFINE input property must both carry, on an LGR cell, the source flat cell's value.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, ResultsFollowLgrCells )
{
    using namespace RiaDefines;

    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_NESTED.EGRID";
    QString refineF  = dir + "/DROGON_NESTED_REFINE.grdecl";
    ASSERT_TRUE( QFile::exists( gridFile ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigCaseCellResultsData* matrixResults = reservoir->results( PorosityModelType::MATRIX_MODEL );
    matrixResults->setReaderInterface( reader.p() );
    RigActiveCellInfo* actInfo = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );

    // Load a STATIC result and the full-length REFINE property before reconstruction.
    QStringList staticResultNames = matrixResults->resultNames( ResultCatType::STATIC_NATIVE );
    ASSERT_FALSE( staticResultNames.empty() );
    RigEclipseResultAddress staticAddr( ResultCatType::STATIC_NATIVE, staticResultNames.front() );
    ASSERT_TRUE( matrixResults->ensureKnownResultLoaded( staticAddr ) );

    RifEclipseInputPropertyLoader::readProperties( refineF, reservoir.p() );
    RigEclipseResultAddress refineAddr( ResultCatType::INPUT_PROPERTY, ResultDataType::INTEGER, RiaResultNames::refine() );
    ASSERT_GT( matrixResults->cellScalarResults( refineAddr ).size(), 0u );

    auto input = buildInput( dir );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), input ) );
    grid->computeCachedData();

    const std::map<size_t, size_t>& sourceCells = grid->nestedHybridLgrSourceCells();
    ASSERT_FALSE( sourceCells.empty() );

    // Pick an LGR cell whose source flat cell is active.
    size_t lgrGlobal = cvf::UNDEFINED_SIZE_T, flat = cvf::UNDEFINED_SIZE_T;
    for ( const auto& [g, f] : sourceCells )
    {
        if ( actInfo->isActive( ReservoirCellIndex( f ) ) )
        {
            lgrGlobal = g;
            flat      = f;
            break;
        }
    }
    ASSERT_NE( lgrGlobal, cvf::UNDEFINED_SIZE_T );

    // (a) Active-cell-indexed STATIC result: LGR cell reads the source flat cell's value.
    const std::vector<double>& staticValues = matrixResults->cellScalarResults( staticAddr, 0 );
    size_t                     la           = actInfo->cellResultIndex( ReservoirCellIndex( lgrGlobal ) ).value();
    size_t                     fa           = actInfo->cellResultIndex( ReservoirCellIndex( flat ) ).value();
    ASSERT_GT( staticValues.size(), la );
    EXPECT_DOUBLE_EQ( staticValues[la], staticValues[fa] );

    // (b) Full-length REFINE array was extended so the LGR cell carries the refined level.
    const std::vector<std::vector<double>>& refineValues = matrixResults->cellScalarResults( refineAddr );
    ASSERT_EQ( refineValues[0].size(), grid->totalCellCount() );
    EXPECT_DOUBLE_EQ( refineValues[0][lgrGlobal], refineValues[0][flat] );
    EXPECT_GT( refineValues[0][lgrGlobal], 1.0 ); // refined cell
}

//--------------------------------------------------------------------------------------------------
/// Dynamic + computed results on LGR cells: the reconstructed LGRs must not be counted as on-file
/// grids (else the reader reads phantom grids). SWAT/SGAS load with all time steps, and computed
/// SOIL (= 1 - SWAT - SGAS) is finite on the LGR cells.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, DynamicAndComputedResultsOnLgr )
{
    using namespace RiaDefines;

    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_NESTED.EGRID";
    ASSERT_TRUE( QFile::exists( gridFile ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigCaseCellResultsData* res = reservoir->results( PorosityModelType::MATRIX_MODEL );
    res->setReaderInterface( reader.p() );
    res->createPlaceholderResultEntries();

    auto input = buildInput( dir );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), input ) );
    grid->computeCachedData();

    EXPECT_EQ( grid->gridCountOnFile(), 1u );
    EXPECT_GT( grid->gridCount(), 2u );

    RigActiveCellInfo* ai = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );

    const std::map<size_t, size_t>& sourceCells = grid->nestedHybridLgrSourceCells();
    size_t                          lgrGlobal = cvf::UNDEFINED_SIZE_T, flat = cvf::UNDEFINED_SIZE_T;
    for ( const auto& [g, f] : sourceCells )
    {
        if ( ai->isActive( ReservoirCellIndex( f ) ) )
        {
            lgrGlobal = g;
            flat      = f;
            break;
        }
    }
    ASSERT_NE( lgrGlobal, cvf::UNDEFINED_SIZE_T );
    size_t la = ai->cellResultIndex( ReservoirCellIndex( lgrGlobal ) ).value();
    size_t fa = ai->cellResultIndex( ReservoirCellIndex( flat ) ).value();

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

//--------------------------------------------------------------------------------------------------
/// QC: the volume-weighted aggregate onto each coarse parent must equal the hand-computed weighted
/// average and be identical for every refined cell of the same parent.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorTest, CoarseVolumeWeightedAggregate )
{
    using namespace RiaDefines;

    QString dir      = nestedHybridModelDir();
    QString gridFile = dir + "/DROGON_NESTED.EGRID";
    ASSERT_TRUE( QFile::exists( gridFile ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );
    cvf::ref<RifReaderEclipseOutput>      reader    = new RifReaderEclipseOutput;
    ASSERT_TRUE( reader->open( gridFile, reservoir.p() ) );
    RigMainGrid* grid = reservoir->mainGrid();
    grid->computeCachedData();

    RigCaseCellResultsData* res = reservoir->results( PorosityModelType::MATRIX_MODEL );
    res->setReaderInterface( reader.p() );
    res->createPlaceholderResultEntries();

    auto input = buildInput( dir );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( reservoir.p(), input ) );
    grid->computeCachedData();

    RigActiveCellInfo* ai = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );

    // Use a static result as the QC source.
    QStringList staticResultNames = res->resultNames( ResultCatType::STATIC_NATIVE );
    ASSERT_FALSE( staticResultNames.empty() );
    RigEclipseResultAddress sourceAddr( ResultCatType::STATIC_NATIVE, staticResultNames.front() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( sourceAddr ) );

    RigEclipseResultAddress aggAddr = res->computeNestedHybridCoarseAggregate( sourceAddr );
    ASSERT_TRUE( aggAddr.isValid() );

    const std::vector<double>& src = res->cellScalarResults( sourceAddr, 0 );
    const std::vector<double>& agg = res->cellScalarResults( aggAddr, 0 );
    RigEclipseResultAddress    volAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( volAddr ) );
    const std::vector<double>& vol = res->cellScalarResults( volAddr, 0 );

    // The flat refined cells are hidden (zero volume) once moved into an LGR; their geometry/value
    // lives on the LGR copy. Map each flat cell to its geometry-bearing cell, and group by parent.
    std::map<size_t, size_t> flatToGeom;
    for ( const auto& [lgrCell, flatCell] : grid->nestedHybridLgrSourceCells() )
        flatToGeom[flatCell] = lgrCell;

    std::map<size_t, std::vector<size_t>> byParent; // parent -> geometry cells
    for ( const auto& [flatCell, parent] : grid->nestedHybridCoarseParents() )
    {
        size_t geom = flatToGeom.count( flatCell ) ? flatToGeom[flatCell] : flatCell;
        if ( ai->isActive( ReservoirCellIndex( geom ) ) ) byParent[parent].push_back( geom );
    }

    // Find a parent with at least two active refined cells and verify its aggregate.
    int verified = 0;
    for ( const auto& [parent, cells] : byParent )
    {
        if ( cells.size() < 2 ) continue;

        double sumVW = 0.0, sumV = 0.0;
        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            sumVW += src[ri] * vol[ri];
            sumV += vol[ri];
        }
        ASSERT_GT( sumV, 0.0 );
        double expected = sumVW / sumV;

        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            EXPECT_NEAR( agg[ri], expected, std::max( 1e-6, std::abs( expected ) * 1e-9 ) );
        }

        if ( ++verified >= 20 ) break;
    }
    EXPECT_GE( verified, 1 );
}
