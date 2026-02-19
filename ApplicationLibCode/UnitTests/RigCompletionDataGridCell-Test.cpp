/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RigCompletionDataGridCell.h"

#include "RifReaderMockModel.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCompletionDataGridCellTest, LocalCellIndexK_StandardModel )
{
    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 3 ) );
    mockReader->enableWellData( false );
    mockReader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();

    RigMainGrid* mainGrid = caseData->mainGrid();
    ASSERT_FALSE( mainGrid->isDualPorosity() );
    ASSERT_EQ( 3u, mainGrid->cellCountK() );

    // Cell at (0, 0, 0) - first K-layer
    RigCompletionDataGridCell cell0( 0, mainGrid );
    EXPECT_EQ( 0u, cell0.localCellIndexK() );

    // Cell at (0, 0, 2) - last K-layer (global index = 2 * 2*2 = 8)
    RigCompletionDataGridCell cell2( 8, mainGrid );
    EXPECT_EQ( 2u, cell2.localCellIndexK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCompletionDataGridCellTest, LocalCellIndexK_DualPorosityModel )
{
    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 3 ) );
    mockReader->enableWellData( false );
    mockReader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();

    RigMainGrid* mainGrid = caseData->mainGrid();
    mainGrid->setDualPorosity( true );
    ASSERT_TRUE( mainGrid->isDualPorosity() );

    size_t cellCountK = mainGrid->cellCountK();
    ASSERT_EQ( 3u, cellCountK );

    // Cell at (0, 0, 0) - first K-layer: should be offset by cellCountK
    RigCompletionDataGridCell cell0( 0, mainGrid );
    EXPECT_EQ( 0u + cellCountK, cell0.localCellIndexK() );

    // Cell at (0, 0, 2) - last K-layer: should be offset by cellCountK
    RigCompletionDataGridCell cell2( 8, mainGrid );
    EXPECT_EQ( 2u + cellCountK, cell2.localCellIndexK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCompletionDataGridCellTest, OneBasedString_DualPorosityModel )
{
    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 3 ) );
    mockReader->enableWellData( false );
    mockReader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();

    RigMainGrid* mainGrid = caseData->mainGrid();

    // Without dual porosity: cell (0,0,0) should give "[1, 1, 1]"
    RigCompletionDataGridCell cellStandard( 0, mainGrid );
    EXPECT_EQ( QString( "[1, 1, 1]" ), cellStandard.oneBasedLocalCellIndexString() );

    // With dual porosity: cell (0,0,0) should give "[1, 1, 4]" (K offset by 3)
    mainGrid->setDualPorosity( true );
    RigCompletionDataGridCell cellDual( 0, mainGrid );
    EXPECT_EQ( QString( "[1, 1, 4]" ), cellDual.oneBasedLocalCellIndexString() );
}
