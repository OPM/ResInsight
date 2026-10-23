/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RiaTestDataDirectory.h"

#include "RifReaderEclipseOutput.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimulationInputNncTools.h"
#include "RigUniformRefinement.h"

#include "RimEclipseResultCase.h"

#include <QDir>
#include <QFile>
#include <QString>

#include <memory>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Helper to load model5 test case
//--------------------------------------------------------------------------------------------------
static cvf::ref<RigEclipseCaseData> loadModel5TestCase()
{
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    if ( !subFolderExists ) return nullptr;

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    if ( !QFile::exists( egridFilePath ) ) return nullptr;

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    if ( !loadResult ) return nullptr;

    return caseData;
}

//--------------------------------------------------------------------------------------------------
/// Test filterInternalSectorConnections - both cells inside
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputNncTools, FilterNNCConnections_BothInside )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    RigMainGrid* mainGrid = caseData->mainGrid();

    // Create test connections using the real grid
    std::vector<RigSimulationInputNncTools::NNCConnection> allConnections;

    // Connection 1: (2,3,4) to (3,3,4) - both inside sector [1,5] x [1,5] x [1,5]
    size_t c1Idx = mainGrid->cellIndexFromIJK( 2, 3, 4 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 3, 3, 4 );
    allConnections.push_back( { c1Idx, c2Idx, 1.5 } );

    // Connection 2: (10,20,8) to (11,20,8) - both outside sector
    c1Idx = mainGrid->cellIndexFromIJK( 10, 20, 8 );
    c2Idx = mainGrid->cellIndexFromIJK( 11, 20, 8 );
    allConnections.push_back( { c1Idx, c2Idx, 2.5 } );

    // Connection 3: (1,2,3) to (15,15,8) - one inside, one outside
    c1Idx = mainGrid->cellIndexFromIJK( 1, 2, 3 );
    c2Idx = mainGrid->cellIndexFromIJK( 15, 15, 8 );
    allConnections.push_back( { c1Idx, c2Idx, 3.5 } );

    // Filter for sector [1,5] x [1,5] x [1,5]
    caf::VecIjk0 sectorMin( 1, 1, 1 );
    caf::VecIjk0 sectorMax( 5, 5, 5 );

    auto filtered = RigSimulationInputNncTools::filterInternalSectorConnections( allConnections, *mainGrid, sectorMin, sectorMax );

    // Only connection 1 should remain
    ASSERT_EQ( 1u, filtered.size() );
    EXPECT_EQ( 1.5, filtered[0].transmissibility );
}

//--------------------------------------------------------------------------------------------------
/// Test transformNNCToSectorCoordinates - no refinement
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputNncTools, TransformNNCToSectorCoordinates_NoRefinement )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    RigMainGrid* mainGrid = caseData->mainGrid();

    // Connection between (10,15,5) and (11,15,5) - I-face neighbors
    size_t c1Idx = mainGrid->cellIndexFromIJK( 10, 15, 5 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 11, 15, 5 );

    RigSimulationInputNncTools::NNCConnection connection{ c1Idx, c2Idx, 4.5 };

    // Sector starts at (5,10,2), grid is 20x30x10
    caf::VecIjk0 sectorMin( 5, 10, 2 );
    auto         refinement = RigUniformRefinement( cvf::Vec3st( 1, 1, 1 ), cvf::Vec3st( 20, 30, 10 ) );

    auto result = RigSimulationInputNncTools::transformNNCToSectorCoordinates( connection, *mainGrid, sectorMin, refinement );

    ASSERT_TRUE( result.has_value() );

    // Cell 1: (10,15,5) - sector min (5,10,2) = (5,5,3) in 0-based sector coords
    EXPECT_EQ( 5, result->cell1.i() );
    EXPECT_EQ( 5, result->cell1.j() );
    EXPECT_EQ( 3, result->cell1.k() );

    // Cell 2: (11,15,5) - sector min (5,10,2) = (6,5,3) in 0-based sector coords
    EXPECT_EQ( 6, result->cell2.i() );
    EXPECT_EQ( 5, result->cell2.j() );
    EXPECT_EQ( 3, result->cell2.k() );

    EXPECT_EQ( 4.5, result->transmissibility );
}

//--------------------------------------------------------------------------------------------------
/// Test refineEDITNNCConnection - corresponding subcells with refinement (2,2,1)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputNncTools, RefineEDITNNCConnection_CorrespondingSubcells )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    RigMainGrid* mainGrid = caseData->mainGrid();

    // Connection between cells (3,4,5) and (6,8,7) - non-neighbors
    size_t c1Idx = mainGrid->cellIndexFromIJK( 3, 4, 5 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 6, 8, 7 );

    RigSimulationInputNncTools::NNCConnection connection{ c1Idx, c2Idx, 2.5 }; // TRAN_MULT = 2.5

    caf::VecIjk0 sectorMin( 0, 0, 0 );
    auto refinement = RigUniformRefinement( cvf::Vec3st( 2, 2, 1 ), cvf::Vec3st( 20, 30, 10 ) ); // Refine by 2x2x1, grid is 20x30x10

    auto refined = RigSimulationInputNncTools::refineEditNncConnection( connection, *mainGrid, sectorMin, refinement );

    // For EDITNNC: creates connections between corresponding subcells
    // Should create 2*2*1 = 4 connections (one per subcell pair at same relative position)
    ASSERT_EQ( 4u, refined.size() );

    // All connections should have same TRAN_MULT (no distribution)
    for ( const auto& conn : refined )
    {
        EXPECT_DOUBLE_EQ( 2.5, conn.transmissibility );
    }

    // Verify corresponding subcell connections
    // Cell1 starts at sector coordinates (3*2, 4*2, 5*1) = (6, 8, 5)
    // Cell2 starts at sector coordinates (6*2, 8*2, 7*1) = (12, 16, 7)

    // Connection 0: (0,0,0) -> (0,0,0): (6,8,5) -> (12,16,7)
    EXPECT_EQ( caf::VecIjk0( 6, 8, 5 ), refined[0].cell1 );
    EXPECT_EQ( caf::VecIjk0( 12, 16, 7 ), refined[0].cell2 );

    // Connection 1: (0,1,0) -> (0,1,0): (6,9,5) -> (12,17,7)
    EXPECT_EQ( caf::VecIjk0( 6, 9, 5 ), refined[1].cell1 );
    EXPECT_EQ( caf::VecIjk0( 12, 17, 7 ), refined[1].cell2 );

    // Connection 2: (1,0,0) -> (1,0,0): (7,8,5) -> (13,16,7)
    EXPECT_EQ( caf::VecIjk0( 7, 8, 5 ), refined[2].cell1 );
    EXPECT_EQ( caf::VecIjk0( 13, 16, 7 ), refined[2].cell2 );

    // Connection 3: (1,1,0) -> (1,1,0): (7,9,5) -> (13,17,7)
    EXPECT_EQ( caf::VecIjk0( 7, 9, 5 ), refined[3].cell1 );
    EXPECT_EQ( caf::VecIjk0( 13, 17, 7 ), refined[3].cell2 );
}
