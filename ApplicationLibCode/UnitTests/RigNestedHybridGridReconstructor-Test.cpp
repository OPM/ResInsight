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
#include "RigNestedHybridGridResultTools.h"
#include "RigNncConnection.h"

#include "RimCellFilterTools.h"
#include "RimCellRangeFilter.h"
#include "RimEclipseResultCase.h"

#include "Well/RigSimWellData.h"
#include "Well/RigWellResultBranch.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include <QDir>
#include <QFile>

#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <tuple>

//==================================================================================================
// Tests for the "nested hybrid grid" LGR reconstruction from the REFINE + OLDIJK sidecars.
//
// The test model TestModels/NestedHybridGrid/DROGON_NESTED.* is a single flat 150x84x96 EGRID where
// the refined cells of each coarse (15x24x12) cell are appended in per-level I bands. The sidecars:
//   DROGON_NESTED_REFINE.grdecl  : per-cell nesting level (1 base, 2/3/4 refined)
//   DROGON_NESTED_OLDIJK.grdecl  : OLDI/OLDJ/OLDK (coarse parent IJK)
//
// Opening the ~1.2M cell EGRID, parsing the sidecars and reconstructing the LGR hierarchy is
// expensive (several seconds), so it is done once in SetUpTestSuite() and shared read-only by all
// tests (#14422). The aggregate tests recompute their GENERATED results from scratch on each call,
// so they do not interfere with each other.
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
/// Shared fixture: the reconstructed case is built once for the whole suite. The setup mirrors the
/// order the individual tests depend on: the first STATIC result and the REFINE input property are
/// loaded BEFORE reconstruction so they are extended onto the LGR cells, and the active cell count
/// before reconstruction is captured for the growth check.
//--------------------------------------------------------------------------------------------------
class RigNestedHybridGridReconstructorTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        QString dir      = nestedHybridModelDir();
        QString gridFile = dir + "/DROGON_NESTED.EGRID";
        if ( !QFile::exists( gridFile ) )
        {
            s_setupError = "Missing test model: " + gridFile;
            return;
        }

        s_resultCase = std::make_unique<RimEclipseResultCase>();
        s_reservoir  = new RigEclipseCaseData( s_resultCase.get() );
        s_reader     = new RifReaderEclipseOutput;
        if ( !s_reader->open( gridFile, s_reservoir.p() ) )
        {
            s_setupError = "Failed to open " + gridFile;
            return;
        }

        RigMainGrid* grid = s_reservoir->mainGrid();
        grid->computeCachedData();

        RigCaseCellResultsData* res = s_reservoir->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        res->setReaderInterface( s_reader.p() );
        res->createPlaceholderResultEntries();

        // Load the first STATIC result before reconstruction (verified to follow the LGR cells).
        QStringList staticResultNames = res->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        if ( staticResultNames.empty() )
        {
            s_setupError = "No static results in test model";
            return;
        }
        s_staticAddr = RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, staticResultNames.front() );
        if ( !res->ensureKnownResultLoaded( s_staticAddr ) )
        {
            s_setupError = "Failed to load static result " + staticResultNames.front();
            return;
        }

        // Load the full-length REFINE input property before reconstruction (extended onto LGR cells,
        // and used by the per-level aggregates to keep refinement levels apart).
        RifEclipseInputPropertyLoader::readProperties( dir + "/DROGON_NESTED_REFINE.grdecl", s_reservoir.p() );

        s_activeCellsBeforeReconstruct =
            s_reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirActiveCellCount();

        s_input = buildInput( dir );

        // The restart file of this model has no well connections, so add a synthetic well with
        // connections into a refined and an unrefined flat cell to check that they are re-pointed.
        for ( size_t i = 0; i < s_input.refine.size(); i++ )
        {
            if ( s_input.refine[i] > 1 && s_wellRefinedFlatCell == cvf::UNDEFINED_SIZE_T ) s_wellRefinedFlatCell = i;
            if ( s_input.refine[i] == 1 && s_wellUnrefinedFlatCell == cvf::UNDEFINED_SIZE_T ) s_wellUnrefinedFlatCell = i;
        }
        {
            auto makePoint = []( size_t cellIndex )
            {
                RigWellResultPoint point;
                point.setGridIndex( 0 );
                point.setGridCellIndex( cellIndex );
                return point;
            };

            RigWellResultBranch branch;
            branch.setBranchResultPoints( { makePoint( s_wellRefinedFlatCell ), makePoint( s_wellUnrefinedFlatCell ) } );

            RigWellResultFrame frame;
            frame.setWellHead( makePoint( s_wellRefinedFlatCell ) );
            frame.addWellResultBranch( branch );

            cvf::ref<RigSimWellData> well = new RigSimWellData;
            well->setWellName( "SYNTHETIC_NHG_WELL" );
            well->m_wellCellsTimeSteps.push_back( frame );

            cvf::Collection<RigSimWellData> wells = s_reservoir->wellResults();
            wells.push_back( well.p() );
            s_reservoir->setSimWellData( wells );
        }

        QString err;
        if ( !RigNestedHybridGridReconstructor::reconstruct( s_reservoir.p(), s_input, &err ) )
        {
            s_setupError = "Reconstruction failed: " + err;
            return;
        }
        grid->computeCachedData();

        s_setupOk = true;
    }

    static void TearDownTestSuite()
    {
        s_reservoir = nullptr;
        s_reader    = nullptr;
        s_resultCase.reset();
        s_input   = {};
        s_setupOk = false;
    }

    void SetUp() override { ASSERT_TRUE( s_setupOk ) << s_setupError.toStdString(); }

    static RigMainGrid*            grid() { return s_reservoir->mainGrid(); }
    static RigCaseCellResultsData* results() { return s_reservoir->results( RiaDefines::PorosityModelType::MATRIX_MODEL ); }
    static RigActiveCellInfo*      activeCellInfo() { return s_reservoir->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ); }

    static std::unique_ptr<RimEclipseResultCase>               s_resultCase;
    static cvf::ref<RigEclipseCaseData>                        s_reservoir;
    static cvf::ref<RifReaderEclipseOutput>                    s_reader;
    static RigNestedHybridGridReconstructor::NestedHybridInput s_input;
    static RigEclipseResultAddress                             s_staticAddr;
    static size_t                                              s_activeCellsBeforeReconstruct;
    static size_t                                              s_wellRefinedFlatCell;
    static size_t                                              s_wellUnrefinedFlatCell;
    static bool                                                s_setupOk;
    static QString                                             s_setupError;
};

std::unique_ptr<RimEclipseResultCase>               RigNestedHybridGridReconstructorTest::s_resultCase;
cvf::ref<RigEclipseCaseData>                        RigNestedHybridGridReconstructorTest::s_reservoir;
cvf::ref<RifReaderEclipseOutput>                    RigNestedHybridGridReconstructorTest::s_reader;
RigNestedHybridGridReconstructor::NestedHybridInput RigNestedHybridGridReconstructorTest::s_input;
RigEclipseResultAddress                             RigNestedHybridGridReconstructorTest::s_staticAddr;
size_t                                              RigNestedHybridGridReconstructorTest::s_activeCellsBeforeReconstruct = 0;
size_t                                              RigNestedHybridGridReconstructorTest::s_wellRefinedFlatCell   = cvf::UNDEFINED_SIZE_T;
size_t                                              RigNestedHybridGridReconstructorTest::s_wellUnrefinedFlatCell = cvf::UNDEFINED_SIZE_T;
bool                                                RigNestedHybridGridReconstructorTest::s_setupOk               = false;
QString                                             RigNestedHybridGridReconstructorTest::s_setupError;

//--------------------------------------------------------------------------------------------------
/// Reconstruct the LGR hierarchy from the REFINE + OLDIJK sidecars and verify the structure.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, ReconstructFromOldIjk )
{
    RigMainGrid* mainGrid = grid();

    EXPECT_EQ( mainGrid->cellCountI(), 150u );
    EXPECT_EQ( mainGrid->cellCountJ(), 84u );
    EXPECT_EQ( mainGrid->cellCountK(), 96u );

    ASSERT_EQ( s_input.refine.size(), mainGrid->cellCount() );
    ASSERT_EQ( s_input.oldI.size(), mainGrid->cellCount() );

    // Distinct coarse parents that own a level-4 region (one nested LGR is built per such parent).
    std::set<std::tuple<int, int, int>> level4Parents;
    for ( size_t f = 0; f < s_input.refine.size(); f++ )
    {
        if ( s_input.refine[f] == 4 && s_input.oldI[f] >= 1 )
            level4Parents.insert( std::make_tuple( s_input.oldI[f], s_input.oldJ[f], s_input.oldK[f] ) );
    }
    ASSERT_GT( level4Parents.size(), 0u );

    // Count the level-4 LGRs. Adjacent coarse parents with equal refinement dimensions are merged.
    size_t numLevel4Lgrs = 0;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        ASSERT_NE( lgr, nullptr );
        EXPECT_TRUE( lgr->isReconstructedGrid() );
        EXPECT_EQ( lgr->parentGrid(), mainGrid );
        if ( QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) ) numLevel4Lgrs++;
    }
    EXPECT_GT( numLevel4Lgrs, 0u );
    EXPECT_LT( numLevel4Lgrs, level4Parents.size() ); // merged, not one-per-parent

    // Every refinement level is reconstructed as sibling LGRs below the main grid. None is counted as
    // an on-file grid.
    EXPECT_EQ( mainGrid->gridCount(), 1u + 2u + numLevel4Lgrs );
    EXPECT_EQ( mainGrid->gridCountOnFile(), 1u );

    // Level 2: one LGR over coarse block 13x22x12 refined 2x2x4 -> 26x44x48.
    RigLocalGrid* lgr2 = findLgrByName( mainGrid, "LGR_NHG_L2" );
    ASSERT_TRUE( lgr2 != nullptr );
    EXPECT_EQ( lgr2->cellCountI(), 26u );
    EXPECT_EQ( lgr2->cellCountJ(), 44u );
    EXPECT_EQ( lgr2->cellCountK(), 24u );
    EXPECT_TRUE( lgr2->isReconstructedGrid() );
    EXPECT_EQ( lgr2->parentGrid(), mainGrid );

    // Level 3: one LGR over coarse block 12x19x12 refined 4x4x4 -> 48x76x48.
    RigLocalGrid* lgr3 = findLgrByName( mainGrid, "LGR_NHG_L3" );
    ASSERT_TRUE( lgr3 != nullptr );
    EXPECT_EQ( lgr3->cellCountI(), 48u );
    EXPECT_EQ( lgr3->cellCountJ(), 76u );
    EXPECT_EQ( lgr3->cellCountK(), 48u );

    // Level 4 is also reconstructed directly below the main grid.
    RigLocalGrid* lgr4 = nullptr;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) )
        {
            lgr4 = lgr;
            break;
        }
    }
    ASSERT_TRUE( lgr4 != nullptr );
    EXPECT_EQ( lgr4->parentGrid(), mainGrid );
    EXPECT_TRUE( lgr4->isReconstructedGrid() );

    {
        // Pick a real (sourced) level-4 cell and verify that OLDIJK linked it to a coarse cell.
        const size_t                    l4Begin     = lgr4->reservoirCellIndex( 0 );
        const size_t                    l4End       = l4Begin + lgr4->cellCount();
        const std::map<size_t, size_t>& srcAll      = mainGrid->nestedHybridLgrSourceCells();
        bool                            checkedNest = false;
        for ( const auto& [lgrGlobal, flat] : srcAll )
        {
            if ( lgrGlobal < l4Begin || lgrGlobal >= l4End ) continue;
            size_t parentLocal  = mainGrid->cell( lgrGlobal ).parentCellIndex();
            size_t parentGlobal = mainGrid->reservoirCellIndex( parentLocal );
            EXPECT_EQ( mainGrid->cell( parentGlobal ).subGrid(), lgr4 );
            EXPECT_EQ( mainGrid->cell( parentGlobal ).hostGrid(), mainGrid );
            checkedNest = true;
            break;
        }
        EXPECT_TRUE( checkedNest );
    }

    // Active count grew (LGR cells added as new active cells; source flat cells stay active).
    EXPECT_GT( activeCellInfo()->reservoirActiveCellCount(), s_activeCellsBeforeReconstruct );

    // Each reconstructed LGR cell carries its source flat cell's geometry; the original flat cell is
    // hidden; and the parent coarse cell it subdivides points back to the LGR it belongs to.
    const size_t                    l2Begin     = lgr2->reservoirCellIndex( 0 );
    const size_t                    l2End       = l2Begin + lgr2->cellCount();
    const std::map<size_t, size_t>& sourceCells = mainGrid->nestedHybridLgrSourceCells();
    ASSERT_FALSE( sourceCells.empty() );
    int checkedL2 = 0;
    for ( const auto& [lgrGlobal, flat] : sourceCells )
    {
        EXPECT_TRUE( mainGrid->cell( flat ).isInvalid() );

        if ( lgrGlobal >= l2Begin && lgrGlobal < l2End && checkedL2++ < 50 )
        {
            // The parent coarse cell of an L2 cell is refined by the L2 LGR.
            size_t parent = mainGrid->cell( lgrGlobal ).parentCellIndex();
            EXPECT_EQ( mainGrid->cell( parent ).subGrid(), lgr2 );
            EXPECT_EQ( mainGrid->cell( lgrGlobal ).hostGrid(), lgr2 );

            auto lgrCorners = mainGrid->cellCornerVertices( lgrGlobal );
            for ( int k = 0; k < 8; k++ )
                EXPECT_FALSE( std::isnan( lgrCorners[k].x() ) );
        }
    }
    EXPECT_GT( checkedL2, 0 );
}

//--------------------------------------------------------------------------------------------------
/// LGR naming (#14611): a level with a single LGR keeps the bare level name, while a level with
/// several LGRs numbers all of them 1..N with no bare-named sibling. All names are unique.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, LgrNamesAreConsistent )
{
    RigMainGrid* mainGrid = grid();

    std::set<QString>              uniqueNames;
    std::map<QString, QStringList> namesByLevel;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        const QString name = QString::fromStdString( mainGrid->gridByIndex( i )->gridName() );
        EXPECT_TRUE( uniqueNames.insert( name ).second ) << "Duplicate LGR name: " << name.toStdString();
        namesByLevel[name.section( '_', 0, 2 )].push_back( name ); // "LGR_NHG_L4_2" -> "LGR_NHG_L4"
    }

    for ( const auto& [levelName, names] : namesByLevel )
    {
        if ( names.size() == 1 )
        {
            EXPECT_EQ( names.front(), levelName );
        }
        else
        {
            QStringList expected;
            for ( int n = 1; n <= names.size(); n++ )
                expected.push_back( QString( "%1_%2" ).arg( levelName ).arg( n ) );
            EXPECT_EQ( names, expected );
        }
    }

    // The Drogon model has one LGR on levels 2 and 3 and several on level 4.
    EXPECT_EQ( namesByLevel["LGR_NHG_L2"].size(), 1 );
    EXPECT_EQ( namesByLevel["LGR_NHG_L3"].size(), 1 );
    EXPECT_GT( namesByLevel["LGR_NHG_L4"].size(), 1 );
}

//--------------------------------------------------------------------------------------------------
/// Tree grouping: numbered component names collapse to their refinement level name.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridReconstructorNameTest, LevelGroupName )
{
    EXPECT_EQ( RigNestedHybridGridReconstructor::levelGroupName( "LGR_NHG_L4_12" ), QString( "LGR_NHG_L4" ) );
    EXPECT_EQ( RigNestedHybridGridReconstructor::levelGroupName( "LGR_NHG_L2" ), QString( "LGR_NHG_L2" ) );
    EXPECT_EQ( RigNestedHybridGridReconstructor::levelGroupName( "LGR1" ), QString( "LGR1" ) );
}

//--------------------------------------------------------------------------------------------------
/// Results propagate to the reconstructed LGR cells: an active-cell-indexed STATIC result and the
/// full-length REFINE input property (both loaded before reconstruction) must both carry, on an LGR
/// cell, the source flat cell's value.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, ResultsFollowLgrCells )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid      = grid();
    RigCaseCellResultsData* matrixResults = results();
    RigActiveCellInfo*      actInfo       = activeCellInfo();

    RigEclipseResultAddress refineAddr( ResultCatType::INPUT_PROPERTY, ResultDataType::INTEGER, RiaResultNames::refine() );
    ASSERT_GT( matrixResults->cellScalarResults( refineAddr ).size(), 0u );

    const std::map<size_t, size_t>& sourceCells = mainGrid->nestedHybridLgrSourceCells();
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
    const std::vector<double>& staticValues = matrixResults->cellScalarResults( s_staticAddr, 0 );
    size_t                     la           = actInfo->cellResultIndex( ReservoirCellIndex( lgrGlobal ) ).value();
    size_t                     fa           = actInfo->cellResultIndex( ReservoirCellIndex( flat ) ).value();
    ASSERT_GT( staticValues.size(), la );
    EXPECT_DOUBLE_EQ( staticValues[la], staticValues[fa] );

    // (b) Full-length REFINE array was extended so the LGR cell carries the refined level.
    const std::vector<std::vector<double>>& refineValues = matrixResults->cellScalarResults( refineAddr );
    ASSERT_EQ( refineValues[0].size(), mainGrid->totalCellCount() );
    EXPECT_DOUBLE_EQ( refineValues[0][lgrGlobal], refineValues[0][flat] );
    EXPECT_GT( refineValues[0][lgrGlobal], 1.0 ); // refined cell
}

//--------------------------------------------------------------------------------------------------
/// Dynamic + computed results on LGR cells: the reconstructed LGRs must not be counted as on-file
/// grids (else the reader reads phantom grids). SWAT/SGAS load with all time steps, and computed
/// SOIL (= 1 - SWAT - SGAS) is finite on the LGR cells.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, DynamicAndComputedResultsOnLgr )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid = grid();
    RigCaseCellResultsData* res      = results();
    RigActiveCellInfo*      ai       = activeCellInfo();

    EXPECT_EQ( mainGrid->gridCountOnFile(), 1u );
    EXPECT_GT( mainGrid->gridCount(), 2u );

    const std::map<size_t, size_t>& sourceCells = mainGrid->nestedHybridLgrSourceCells();
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
/// QC: the pore-volume-weighted aggregate onto each coarse parent must equal the hand-computed
/// weighted average and be identical for every refined cell of the same parent.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, CoarsePoreVolumeWeightedAggregate )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid = grid();
    RigCaseCellResultsData* res      = results();
    RigActiveCellInfo*      ai       = activeCellInfo();

    RigEclipseResultAddress aggAddr = RigNestedHybridGridResultTools::computeCoarseAggregate( res, s_staticAddr );
    ASSERT_TRUE( aggAddr.isValid() );

    const std::vector<double>& src = res->cellScalarResults( s_staticAddr, 0 );
    const std::vector<double>& agg = res->cellScalarResults( aggAddr, 0 );
    RigEclipseResultAddress    volAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( volAddr ) );
    const std::vector<double>& vol = res->cellScalarResults( volAddr, 0 );

    // Pore volume is the weight (active-cell indexed via the shared helper).
    std::vector<double>        porvTemp;
    const std::vector<double>* porv = RigCaseCellResultsData::getResultIndexableStaticResult( ai, res, RiaResultNames::porv(), porvTemp );
    ASSERT_TRUE( porv != nullptr );

    // The flat refined cells are hidden (zero volume) once moved into an LGR; their geometry/value
    // lives on the LGR copy. Map each flat cell to its geometry-bearing cell, and group by parent.
    std::map<size_t, size_t> flatToGeom;
    for ( const auto& [lgrCell, flatCell] : mainGrid->nestedHybridLgrSourceCells() )
        flatToGeom[flatCell] = lgrCell;

    std::map<size_t, std::vector<size_t>> byParent; // parent -> geometry cells
    for ( const auto& [flatCell, parent] : mainGrid->nestedHybridCoarseParents() )
    {
        size_t geom = flatToGeom.count( flatCell ) ? flatToGeom[flatCell] : flatCell;
        if ( ai->isActive( ReservoirCellIndex( geom ) ) ) byParent[parent].push_back( geom );
    }

    // Find a parent with at least two active refined cells and verify its aggregate. Only cells with
    // a positive bulk volume contribute, weighted by their pore volume (mirrors the implementation).
    int verified = 0;
    for ( const auto& [parent, cells] : byParent )
    {
        if ( cells.size() < 2 ) continue;

        double sumVW = 0.0, sumV = 0.0;
        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            if ( vol[ri] <= 0.0 ) continue;
            double w = ( *porv )[ri];
            if ( w <= 0.0 || w == HUGE_VAL ) continue;
            sumVW += src[ri] * w;
            sumV += w;
        }
        if ( sumV <= 0.0 ) continue;
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

//--------------------------------------------------------------------------------------------------
/// QC per refinement level: <RESULT>_COARSE_L4 holds, on each level-4 cell, the pore-volume-weighted
/// average over the level-4 cells of its coarse parent.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, PerLevelPoreVolumeWeightedAggregate )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid = grid();
    RigCaseCellResultsData* res      = results();
    RigActiveCellInfo*      ai       = activeCellInfo();

    // Sanity: REFINE is loaded full-length (per the request, it drives the level separation).
    RigEclipseResultAddress refineAddr( ResultCatType::INPUT_PROPERTY, ResultDataType::INTEGER, RiaResultNames::refine() );
    ASSERT_TRUE( res->cellScalarResults( refineAddr ).size() > 0 &&
                 res->cellScalarResults( refineAddr )[0].size() == mainGrid->totalCellCount() );

    std::vector<RigEclipseResultAddress> created = RigNestedHybridGridResultTools::computePerLevelAggregate( res, s_staticAddr );
    ASSERT_FALSE( created.empty() );

    RigEclipseResultAddress l4Addr( ResultCatType::GENERATED, s_staticAddr.resultName() + "_COARSE_L4" );
    ASSERT_TRUE( res->hasResultEntry( l4Addr ) );

    const std::vector<double>& src = res->cellScalarResults( s_staticAddr, 0 );
    const std::vector<double>& agg = res->cellScalarResults( l4Addr, 0 );
    RigEclipseResultAddress    volAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( volAddr ) );
    const std::vector<double>& vol = res->cellScalarResults( volAddr, 0 );

    // Pore volume is the weight (active-cell indexed via the shared helper).
    std::vector<double>        porvTemp;
    const std::vector<double>* porv = RigCaseCellResultsData::getResultIndexableStaticResult( ai, res, RiaResultNames::porv(), porvTemp );
    ASSERT_TRUE( porv != nullptr );

    // Blank-elsewhere: the _COARSE_L4 result is defined only on active level-4 cells.
    size_t activeLevel4 = 0;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        if ( !lgr || !QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) ) continue;
        for ( size_t c = 0; c < lgr->cellCount(); c++ )
            if ( ai->isActive( ReservoirCellIndex( lgr->reservoirCellIndex( c ) ) ) ) activeLevel4++;
    }
    size_t definedCount = 0;
    for ( double x : agg )
        if ( x != HUGE_VAL ) definedCount++;
    EXPECT_GT( activeLevel4, 0u );
    EXPECT_EQ( definedCount, activeLevel4 );

    // Find a level-4 LGR and group its cells by their coarse parent cell.
    RigLocalGrid* lgr4 = nullptr;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) )
        {
            lgr4 = lgr;
            break;
        }
    }
    ASSERT_TRUE( lgr4 != nullptr );

    std::map<size_t, std::vector<size_t>> byParent; // coarse parent cell -> level-4 global cells
    for ( size_t c = 0; c < lgr4->cellCount(); c++ )
    {
        size_t global = lgr4->reservoirCellIndex( c );
        if ( ai->isActive( ReservoirCellIndex( global ) ) ) byParent[lgr4->cell( c ).parentCellIndex()].push_back( global );
    }

    int verified = 0;
    for ( const auto& [parent, cells] : byParent )
    {
        if ( cells.size() < 2 ) continue;

        double sumVW = 0.0, sumV = 0.0;
        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            if ( vol[ri] <= 0.0 ) continue;
            double w = ( *porv )[ri];
            if ( w <= 0.0 || w == HUGE_VAL ) continue;
            sumVW += src[ri] * w;
            sumV += w;
        }
        if ( sumV <= 0.0 ) continue;
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

//--------------------------------------------------------------------------------------------------
/// QC for extensive quantities (e.g. FIP): the SUM aggregate onto each coarse parent must equal the
/// plain sum over the parent's geometry-bearing cells and be identical for every refined cell of the
/// same parent.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, CoarseSumAggregate )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid = grid();
    RigCaseCellResultsData* res      = results();
    RigActiveCellInfo*      ai       = activeCellInfo();

    RigEclipseResultAddress aggAddr =
        RigNestedHybridGridResultTools::computeCoarseAggregate( res, s_staticAddr, RigNestedHybridGridResultTools::AggregationMode::SUM );
    ASSERT_TRUE( aggAddr.isValid() );

    const std::vector<double>& src = res->cellScalarResults( s_staticAddr, 0 );
    const std::vector<double>& agg = res->cellScalarResults( aggAddr, 0 );
    RigEclipseResultAddress    volAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( volAddr ) );
    const std::vector<double>& vol = res->cellScalarResults( volAddr, 0 );

    // Map each flat cell to its geometry-bearing cell (its LGR copy if moved), and group by parent.
    std::map<size_t, size_t> flatToGeom;
    for ( const auto& [lgrCell, flatCell] : mainGrid->nestedHybridLgrSourceCells() )
        flatToGeom[flatCell] = lgrCell;

    std::map<size_t, std::vector<size_t>> byParent; // parent -> geometry cells
    for ( const auto& [flatCell, parent] : mainGrid->nestedHybridCoarseParents() )
    {
        size_t geom = flatToGeom.count( flatCell ) ? flatToGeom[flatCell] : flatCell;
        if ( ai->isActive( ReservoirCellIndex( geom ) ) ) byParent[parent].push_back( geom );
    }

    // Find a parent with at least two active refined cells and verify its sum. Only cells with a
    // positive volume contribute (the zero-volume mask excludes hidden duplicates).
    int verified = 0;
    for ( const auto& [parent, cells] : byParent )
    {
        if ( cells.size() < 2 ) continue;

        double expected = 0.0;
        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            if ( vol[ri] > 0.0 ) expected += src[ri];
        }

        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            EXPECT_NEAR( agg[ri], expected, std::max( 1e-6, std::abs( expected ) * 1e-9 ) );
        }

        if ( ++verified >= 20 ) break;
    }
    EXPECT_GE( verified, 1 );
}

//--------------------------------------------------------------------------------------------------
/// Per-level SUM aggregate: <RESULT>_COARSE_L4 holds, on each level-4 cell, the sum over the level-4
/// cells of its coarse parent, and is blank everywhere else.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, PerLevelSumAggregate )
{
    using namespace RiaDefines;

    RigMainGrid*            mainGrid = grid();
    RigCaseCellResultsData* res      = results();
    RigActiveCellInfo*      ai       = activeCellInfo();

    std::vector<RigEclipseResultAddress> created =
        RigNestedHybridGridResultTools::computePerLevelAggregate( res, s_staticAddr, RigNestedHybridGridResultTools::AggregationMode::SUM );
    ASSERT_FALSE( created.empty() );

    RigEclipseResultAddress l4Addr( ResultCatType::GENERATED, s_staticAddr.resultName() + "_COARSE_L4" );
    ASSERT_TRUE( res->hasResultEntry( l4Addr ) );

    const std::vector<double>& src = res->cellScalarResults( s_staticAddr, 0 );
    const std::vector<double>& agg = res->cellScalarResults( l4Addr, 0 );
    RigEclipseResultAddress    volAddr( ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    ASSERT_TRUE( res->ensureKnownResultLoaded( volAddr ) );
    const std::vector<double>& vol = res->cellScalarResults( volAddr, 0 );

    // Blank-elsewhere: defined only on active level-4 cells.
    size_t activeLevel4 = 0;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        if ( !lgr || !QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) ) continue;
        for ( size_t c = 0; c < lgr->cellCount(); c++ )
            if ( ai->isActive( ReservoirCellIndex( lgr->reservoirCellIndex( c ) ) ) ) activeLevel4++;
    }
    size_t definedCount = 0;
    for ( double x : agg )
        if ( x != HUGE_VAL ) definedCount++;
    EXPECT_GT( activeLevel4, 0u );
    EXPECT_EQ( definedCount, activeLevel4 );

    // Find a level-4 LGR and group its cells by their coarse parent cell.
    RigLocalGrid* lgr4 = nullptr;
    for ( size_t i = 1; i < mainGrid->gridCount(); i++ )
    {
        RigLocalGrid* lgr = dynamic_cast<RigLocalGrid*>( mainGrid->gridByIndex( i ) );
        if ( lgr && QString::fromStdString( lgr->gridName() ).startsWith( "LGR_NHG_L4" ) )
        {
            lgr4 = lgr;
            break;
        }
    }
    ASSERT_TRUE( lgr4 != nullptr );

    std::map<size_t, std::vector<size_t>> byParent; // coarse parent cell -> level-4 global cells
    for ( size_t c = 0; c < lgr4->cellCount(); c++ )
    {
        size_t global = lgr4->reservoirCellIndex( c );
        if ( ai->isActive( ReservoirCellIndex( global ) ) ) byParent[lgr4->cell( c ).parentCellIndex()].push_back( global );
    }

    int verified = 0;
    for ( const auto& [parent, cells] : byParent )
    {
        if ( cells.size() < 2 ) continue;

        double expected = 0.0;
        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            if ( vol[ri] > 0.0 ) expected += src[ri];
        }

        for ( size_t cell : cells )
        {
            size_t ri = ai->cellResultIndex( ReservoirCellIndex( cell ) ).value();
            EXPECT_NEAR( agg[ri], expected, std::max( 1e-6, std::abs( expected ) * 1e-9 ) );
        }

        if ( ++verified >= 20 ) break;
    }
    EXPECT_GE( verified, 1 );
}

//--------------------------------------------------------------------------------------------------
/// The reconstruction records the original (coarse) grid layout: 15x24x12 coarse cells, with each
/// coarse K layer spanning 8 flat K layers (96 / 12).
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, NestedHybridLayout )
{
    const auto& layout = grid()->nestedHybridLayout();
    ASSERT_TRUE( layout.has_value() );

    EXPECT_EQ( layout->originalCellCounts(), cvf::Vec3st( 15, 24, 12 ) );
    EXPECT_EQ( layout->kFactor(), 8u );

    auto host = layout->flatIjkFromOriginalIjk( caf::VecIjk0( 3, 4, 5 ) );
    EXPECT_EQ( host, caf::VecIjk0( 3, 4, 40 ) );

    auto original = layout->originalIjkFromFlatIjk( caf::VecIjk0( 3, 4, 47 ) );
    ASSERT_TRUE( original.has_value() );
    EXPECT_EQ( *original, caf::VecIjk0( 3, 4, 5 ) );

    // Flat columns outside the original I/J extent hold refined cells or padding
    EXPECT_FALSE( layout->originalIjkFromFlatIjk( caf::VecIjk0( 15, 0, 0 ) ).has_value() );
    EXPECT_FALSE( layout->originalIjkFromFlatIjk( caf::VecIjk0( 0, 24, 0 ) ).has_value() );
}

//--------------------------------------------------------------------------------------------------
/// The original IJK computed on demand (layout + parent cell chain) must agree with the OLDIJK
/// sidecar: for every reconstructed LGR cell, for every unrefined coarse cell, and the scattered
/// flat refined cells must have no original IJK.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, OriginalIjkFollowsOldIjk )
{
    RigMainGrid* mainGrid = grid();

    auto oldIjk0 = []( size_t flat )
    { return caf::VecIjk0( (size_t)( s_input.oldI[flat] - 1 ), (size_t)( s_input.oldJ[flat] - 1 ), (size_t)( s_input.oldK[flat] - 1 ) ); };

    // LGR cells: original IJK equals OLDIJK of the flat cell they were built from
    size_t lgrChecked = 0, lgrMismatch = 0;
    for ( const auto& [lgrGlobal, flat] : mainGrid->nestedHybridLgrSourceCells() )
    {
        size_t localIdx = 0;
        auto   lgr      = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( lgrGlobal, &localIdx );
        ASSERT_FALSE( lgr->isMainGrid() );

        auto original = mainGrid->nestedHybridOriginalIjk( lgr, localIdx );
        if ( !original || *original != oldIjk0( flat ) ) lgrMismatch++;
        lgrChecked++;
    }
    EXPECT_GT( lgrChecked, 0u );
    EXPECT_EQ( lgrMismatch, 0u );

    // Main grid cells
    const size_t nx = mainGrid->cellCountI();
    const size_t ny = mainGrid->cellCountJ();

    size_t coarseChecked = 0, coarseMismatch = 0, refinedWithIjk = 0;
    for ( size_t f = 0; f < mainGrid->cellCount(); f++ )
    {
        auto original = mainGrid->nestedHybridOriginalIjk( mainGrid, f );

        if ( s_input.refine[f] > 1 )
        {
            // Scattered refined cells (now represented by LGR cells) have no original IJK
            if ( original ) refinedWithIjk++;
            continue;
        }

        if ( s_input.oldI[f] < 1 ) continue;

        const caf::VecIjk0 flatIjk( f % nx, ( f / nx ) % ny, f / ( nx * ny ) );
        const caf::VecIjk0 expected = oldIjk0( f );
        if ( flatIjk != grid()->nestedHybridLayout()->flatIjkFromOriginalIjk( expected ) ) continue; // padding

        if ( !original || *original != expected ) coarseMismatch++;
        coarseChecked++;
    }
    EXPECT_GT( coarseChecked, 0u );
    EXPECT_EQ( coarseMismatch, 0u );
    EXPECT_EQ( refinedWithIjk, 0u );
}

//--------------------------------------------------------------------------------------------------
/// IJK range filters on a nested hybrid grid refer to the original grid IJK. A slice selects every
/// cell mapped to the original slice - unrefined cells and the cells of all refinement levels.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, RangeFilterUsesOriginalIjk )
{
    RigMainGrid* mainGrid = grid();
    s_resultCase->setReservoirData( s_reservoir.p() );

    RimCellRangeFilter rangeFilter;
    rangeFilter.setCase( s_resultCase.get() );
    EXPECT_TRUE( rangeFilter.usesNestedHybridOriginalIjk() );

    // A filter targeting an LGR is redirected to the original grid
    rangeFilter.setGridIndex( 2 );
    EXPECT_EQ( rangeFilter.gridIndex(), 0 );

    // Original IJK level of every reconstructed LGR cell (via its source flat cell)
    std::map<size_t, int> lgrLevel; // LGR global cell index -> refinement level
    for ( const auto& [lgrGlobal, flat] : mainGrid->nestedHybridLgrSourceCells() )
        lgrLevel[lgrGlobal] = s_input.refine[flat];

    struct Slice
    {
        int axis;
        int index; // 1-based original index
    };

    for ( const Slice& slice : { Slice{ 0, 10 }, Slice{ 1, 12 }, Slice{ 2, 5 } } )
    {
        rangeFilter.startIndexI = 1;
        rangeFilter.startIndexJ = 1;
        rangeFilter.startIndexK = 1;
        rangeFilter.cellCountI  = 15;
        rangeFilter.cellCountJ  = 24;
        rangeFilter.cellCountK  = 12;
        if ( slice.axis == 0 ) std::tie( rangeFilter.startIndexI.v(), rangeFilter.cellCountI.v() ) = std::make_pair( slice.index, 1 );
        if ( slice.axis == 1 ) std::tie( rangeFilter.startIndexJ.v(), rangeFilter.cellCountJ.v() ) = std::make_pair( slice.index, 1 );
        if ( slice.axis == 2 ) std::tie( rangeFilter.startIndexK.v(), rangeFilter.cellCountK.v() ) = std::make_pair( slice.index, 1 );

        auto visibility = RimCellFilterTools::computeReservoirCellVisibility( &rangeFilter, s_resultCase.get(), 0 );
        ASSERT_TRUE( visibility.notNull() );
        ASSERT_EQ( visibility->size(), mainGrid->totalCellCount() );

        // Every LGR cell is visible exactly when its OLDIJK lies in the slice
        size_t           mismatch = 0;
        std::set<int>    visibleLevels;
        const std::array oldIjk = { &s_input.oldI, &s_input.oldJ, &s_input.oldK };
        for ( const auto& [lgrGlobal, flat] : mainGrid->nestedHybridLgrSourceCells() )
        {
            const bool expected = ( *oldIjk[slice.axis] )[flat] == slice.index;
            const bool visible  = visibility->val( lgrGlobal ) != 0;
            if ( expected != visible ) mismatch++;
            if ( visible ) visibleLevels.insert( lgrLevel[lgrGlobal] );
        }
        EXPECT_EQ( mismatch, 0u ) << "axis " << slice.axis;

        // Main grid cells: only cells within the original columns, mapping to the slice, are visible
        size_t mainVisible = 0, mainWrong = 0;
        for ( size_t f = 0; f < mainGrid->cellCount(); f++ )
        {
            if ( !visibility->val( f ) ) continue;
            mainVisible++;
            auto original = mainGrid->nestedHybridOriginalIjk( mainGrid, f );
            if ( !original || ( *original )[slice.axis] != (size_t)( slice.index - 1 ) ) mainWrong++;
        }
        EXPECT_GT( mainVisible, 0u ) << "axis " << slice.axis;
        EXPECT_EQ( mainWrong, 0u ) << "axis " << slice.axis;

        // The slices are chosen to cross more than one refinement level
        EXPECT_GE( visibleLevels.size(), 2u ) << "axis " << slice.axis;
    }
}

//--------------------------------------------------------------------------------------------------
/// Simulation well connections into refined flat cells follow the cells into the reconstructed
/// LGRs, so the hidden flat cells are not shown as well cells in the main grid. Connections into
/// unrefined cells stay in the main grid.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridReconstructorTest, WellConnectionsFollowLgrCells )
{
    ASSERT_NE( s_wellRefinedFlatCell, cvf::UNDEFINED_SIZE_T );
    ASSERT_NE( s_wellUnrefinedFlatCell, cvf::UNDEFINED_SIZE_T );

    const RigSimWellData* well = nullptr;
    for ( const auto& w : s_reservoir->wellResults() )
        if ( w->m_wellName == "SYNTHETIC_NHG_WELL" ) well = w.p();
    ASSERT_NE( well, nullptr );
    ASSERT_EQ( well->m_wellCellsTimeSteps.size(), 1u );

    const RigWellResultFrame& frame = well->m_wellCellsTimeSteps.front();

    auto expectInLgrCellFrom = [&]( const RigWellResultPoint& point, size_t flatCell )
    {
        ASSERT_TRUE( point.isCell() );
        ASSERT_GT( point.gridIndex(), 0u );
        const RigGridBase* lgr = grid()->gridByIndex( point.gridIndex() );
        ASSERT_LT( point.cellIndex(), lgr->cellCount() );

        const auto& sourceCells = grid()->nestedHybridLgrSourceCells();
        auto        it          = sourceCells.find( lgr->reservoirCellIndex( point.cellIndex() ) );
        ASSERT_NE( it, sourceCells.end() );
        EXPECT_EQ( it->second, flatCell );
    };

    expectInLgrCellFrom( frame.wellHead(), s_wellRefinedFlatCell );

    const auto points = frame.allResultPoints();
    ASSERT_EQ( points.size(), 2u );
    expectInLgrCellFrom( points[0], s_wellRefinedFlatCell );
    EXPECT_EQ( points[1].gridIndex(), 0u );
    EXPECT_EQ( points[1].cellIndex(), s_wellUnrefinedFlatCell );

    // The well cells used for visualization no longer include the hidden flat cell
    const cvf::UByteArray* mainGridWellCells = s_reservoir->wellCellsInGrid( 0 );
    ASSERT_NE( mainGridWellCells, nullptr );
    EXPECT_FALSE( ( *mainGridWellCells )[s_wellRefinedFlatCell] );
    EXPECT_TRUE( ( *mainGridWellCells )[s_wellUnrefinedFlatCell] );
}
