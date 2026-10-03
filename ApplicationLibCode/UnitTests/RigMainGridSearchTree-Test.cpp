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

#include "RiaTestDataDirectory.h"

#include "RifReaderOpmCommon.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include <QDir>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// The search tree stores several cells in each leaf. Verify that the result is the same as when testing the bounding
/// box of every valid cell, including LGR cells.
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridSearchTree, FindIntersectingCellsMatchesAllCellsTest )
{
    QDir    baseFolder( TEST_MODEL_DIR );
    QString filename = baseFolder.absoluteFilePath( "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID" );
    ASSERT_TRUE( QFile::exists( filename ) );

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );
    cvf::ref<RifReaderOpmCommon> reader   = new RifReaderOpmCommon;
    ASSERT_TRUE( reader->open( filename, caseData.p() ) );

    RigMainGrid* mainGrid = caseData->mainGrid();
    mainGrid->computeCachedData();
    ASSERT_GT( mainGrid->gridCount(), 1u );

    std::vector<cvf::BoundingBox> cellBoundingBoxes( mainGrid->totalCellCount() );
    for ( size_t cellIdx = 0; cellIdx < mainGrid->totalCellCount(); cellIdx++ )
    {
        if ( mainGrid->cell( cellIdx ).isInvalid() ) continue;

        for ( size_t nodeIdx : mainGrid->cell( cellIdx ).cornerIndices() )
        {
            cellBoundingBoxes[cellIdx].add( mainGrid->nodes()[nodeIdx] );
        }
    }

    const std::vector<double> halfSizes         = { 0.1, 20.0, 200.0 };
    const size_t              queryCountPerGrid = 20;

    // Query around cells in the main grid and in each LGR
    size_t queryCount = 0;
    for ( size_t gridIdx = 0; gridIdx < mainGrid->gridCount(); gridIdx++ )
    {
        const RigGridBase* grid   = mainGrid->gridByIndex( gridIdx );
        const size_t       stride = std::max( size_t( 1 ), grid->cellCount() / queryCountPerGrid );

        for ( size_t localIdx = 0; localIdx < grid->cellCount(); localIdx += stride )
        {
            const size_t cellIdx = grid->reservoirCellIndex( localIdx );
            if ( mainGrid->cell( cellIdx ).isInvalid() ) continue;

            const cvf::Vec3d center   = mainGrid->cell( cellIdx ).center();
            const double     halfSize = halfSizes[queryCount++ % halfSizes.size()];

            cvf::BoundingBox queryBB( center - cvf::Vec3d( halfSize, halfSize, halfSize ), center + cvf::Vec3d( halfSize, halfSize, halfSize ) );

            std::vector<size_t> expected;
            for ( size_t i = 0; i < cellBoundingBoxes.size(); i++ )
            {
                if ( cellBoundingBoxes[i].isValid() && cellBoundingBoxes[i].intersects( queryBB ) ) expected.push_back( i );
            }

            std::vector<size_t> found = mainGrid->findIntersectingCells( queryBB );
            std::sort( found.begin(), found.end() );

            EXPECT_EQ( expected, found ) << "Query around cell " << cellIdx << " in grid " << gridIdx;
        }
    }

    EXPECT_GT( queryCount, 2 * queryCountPerGrid );
}
