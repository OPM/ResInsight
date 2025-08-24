/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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
#include "RifEclipseInputFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"

#include "RimEclipseResultCase.h"

#include "cvfStructGrid.h"

#include <QDir>
#include <QFile>

#include <memory>

//--------------------------------------------------------------------------------------------------
/// Helper function to load test grid
//--------------------------------------------------------------------------------------------------
static cvf::ref<RigEclipseCaseData> loadTestGrid()
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    if ( !subFolderExists ) return nullptr;

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    if ( !QFile::exists( inputFilePath ) ) return nullptr;

    std::unique_ptr<RimEclipseResultCase> testCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( testCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, caseData.p(), false, &errorMessages );
    if ( !loadResult ) return nullptr;

    return caseData;
}

//--------------------------------------------------------------------------------------------------
/// Test basic adapter construction and properties
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, BasicConstruction )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() ) << "Failed to load test grid";

    const RigMainGrid* mainGrid = caseData->mainGrid();
    ASSERT_NE( mainGrid, nullptr );

    // Test with no refinement
    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 2, 2, 2 ); // Small 3x3x3 subset
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    EXPECT_EQ( 3, adapter.cellCountI() );
    EXPECT_EQ( 3, adapter.cellCountJ() );
    EXPECT_EQ( 3, adapter.cellCountK() );
    EXPECT_EQ( 27, adapter.totalCells() );
    EXPECT_FALSE( adapter.hasRefinement() );
    EXPECT_EQ( min, adapter.originalMin() );
    EXPECT_EQ( max, adapter.originalMax() );
    EXPECT_EQ( refinement, adapter.refinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test adapter with undefined max (should use full grid)
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, UndefinedMax )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max = cvf::Vec3st::UNDEFINED;
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    EXPECT_EQ( mainGrid->cellCountI(), adapter.cellCountI() );
    EXPECT_EQ( mainGrid->cellCountJ(), adapter.cellCountJ() );
    EXPECT_EQ( mainGrid->cellCountK(), adapter.cellCountK() );
    EXPECT_EQ( mainGrid->cellCount(), adapter.totalCells() );
}

//--------------------------------------------------------------------------------------------------
/// Test adapter with 2x2x2 refinement
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, RefinementConstruction )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 1, 1, 1 ); // 2x2x2 original cells
    cvf::Vec3st refinement( 2, 2, 2 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    EXPECT_EQ( 4, adapter.cellCountI() ); // 2 * 2
    EXPECT_EQ( 4, adapter.cellCountJ() ); // 2 * 2
    EXPECT_EQ( 4, adapter.cellCountK() ); // 2 * 2
    EXPECT_EQ( 64, adapter.totalCells() ); // 4 * 4 * 4 = 64
    EXPECT_TRUE( adapter.hasRefinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test cell corner access without refinement
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, CellCornersNoRefinement )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 0, 0, 0 ); // Just first cell
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    EXPECT_EQ( 1, adapter.cellCountI() );
    EXPECT_EQ( 1, adapter.cellCountJ() );
    EXPECT_EQ( 1, adapter.cellCountK() );

    // Get corners from adapter
    auto adapterCorners = adapter.getCellCorners( 0, 0, 0 );

    // Get corners directly from main grid for comparison
    size_t mainGridCellIndex = mainGrid->cellIndexFromIJK( 0, 0, 0 );
    auto   mainGridCorners   = mainGrid->cellCornerVertices( mainGridCellIndex );

    // Apply same coordinate transformation if needed
    if ( adapter.useMapAxes() )
    {
        cvf::Mat4d transform = adapter.mapAxisTransform();
        for ( cvf::Vec3d& corner : mainGridCorners )
        {
            corner.transformPoint( transform );
        }
    }

    // Compare corners
    for ( size_t i = 0; i < 8; ++i )
    {
        EXPECT_NEAR( mainGridCorners[i].x(), adapterCorners[i].x(), 0.001 ) << "Corner " << i << " X mismatch";
        EXPECT_NEAR( mainGridCorners[i].y(), adapterCorners[i].y(), 0.001 ) << "Corner " << i << " Y mismatch";
        EXPECT_NEAR( mainGridCorners[i].z(), adapterCorners[i].z(), 0.001 ) << "Corner " << i << " Z mismatch";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test cell activity status
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, CellActivity )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid*       mainGrid       = caseData->mainGrid();
    const RigActiveCellInfo* activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 2, 2, 2 ); // Small 3x3x3 subset
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    // Test a few cells
    for ( size_t k = 0; k < adapter.cellCountK(); ++k )
    {
        for ( size_t j = 0; j < adapter.cellCountJ(); ++j )
        {
            for ( size_t i = 0; i < adapter.cellCountI(); ++i )
            {
                bool adapterActive = adapter.isCellActive( i, j, k );

                // Get corresponding activity from main grid
                size_t origI          = min.x() + i;
                size_t origJ          = min.y() + j;
                size_t origK          = min.z() + k;
                size_t mainIndex      = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                bool   expectedActive = activeCellInfo->isActive( mainIndex );

                EXPECT_EQ( expectedActive, adapterActive ) << "Activity mismatch at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test cell activity with visibility override
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, CellActivityWithOverride )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    // Create a visibility override array that marks all cells as inactive
    cvf::UByteArray visibilityOverride;
    visibilityOverride.resize( mainGrid->cellCount() );
    visibilityOverride.setAll( 0 ); // All cells inactive

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 1, 1, 1 ); // 2x2x2 subset
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement, &visibilityOverride );

    // All cells should be inactive due to override
    for ( size_t k = 0; k < adapter.cellCountK(); ++k )
    {
        for ( size_t j = 0; j < adapter.cellCountJ(); ++j )
        {
            for ( size_t i = 0; i < adapter.cellCountI(); ++i )
            {
                EXPECT_FALSE( adapter.isCellActive( i, j, k ) )
                    << "Cell (" << i << "," << j << "," << k << ") should be inactive due to override";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test refined cell corner generation
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, RefinedCellCorners )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 0, 0, 0 ); // Just first cell
    cvf::Vec3st refinement( 2, 2, 2 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    EXPECT_EQ( 2, adapter.cellCountI() );
    EXPECT_EQ( 2, adapter.cellCountJ() );
    EXPECT_EQ( 2, adapter.cellCountK() );
    EXPECT_EQ( 8, adapter.totalCells() );

    // Get original cell corners
    size_t originalCellIndex = mainGrid->cellIndexFromIJK( 0, 0, 0 );
    auto   originalCorners   = mainGrid->cellCornerVertices( originalCellIndex );

    // Apply coordinate transformation if needed
    if ( adapter.useMapAxes() )
    {
        cvf::Mat4d transform = adapter.mapAxisTransform();
        for ( cvf::Vec3d& corner : originalCorners )
        {
            corner.transformPoint( transform );
        }
    }

    // Find bounding box of original cell
    cvf::Vec3d originalMin = originalCorners[0];
    cvf::Vec3d originalMax = originalCorners[0];
    for ( const auto& corner : originalCorners )
    {
        originalMin.x() = std::min( originalMin.x(), corner.x() );
        originalMin.y() = std::min( originalMin.y(), corner.y() );
        originalMin.z() = std::min( originalMin.z(), corner.z() );
        originalMax.x() = std::max( originalMax.x(), corner.x() );
        originalMax.y() = std::max( originalMax.y(), corner.y() );
        originalMax.z() = std::max( originalMax.z(), corner.z() );
    }

    // Test all 8 refined subcells
    for ( size_t k = 0; k < 2; ++k )
    {
        for ( size_t j = 0; j < 2; ++j )
        {
            for ( size_t i = 0; i < 2; ++i )
            {
                auto refinedCorners = adapter.getCellCorners( i, j, k );

                // All refined cell corners should be within original cell bounds
                for ( const auto& corner : refinedCorners )
                {
                    EXPECT_GE( corner.x(), originalMin.x() - 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original X min";
                    EXPECT_LE( corner.x(), originalMax.x() + 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original X max";

                    EXPECT_GE( corner.y(), originalMin.y() - 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original Y min";
                    EXPECT_LE( corner.y(), originalMax.y() + 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original Y max";

                    EXPECT_GE( corner.z(), originalMin.z() - 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original Z min";
                    EXPECT_LE( corner.z(), originalMax.z() + 0.001 )
                        << "Refined cell (" << i << "," << j << "," << k << ") corner outside original Z max";
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test that corner vertices are reasonable (not NaN, not all zeros)
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, ReasonableCornerValues )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 1, 1, 1 );
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    bool hasNonZeroValues = false;

    for ( size_t k = 0; k < adapter.cellCountK(); ++k )
    {
        for ( size_t j = 0; j < adapter.cellCountJ(); ++j )
        {
            for ( size_t i = 0; i < adapter.cellCountI(); ++i )
            {
                auto corners = adapter.getCellCorners( i, j, k );

                for ( const auto& corner : corners )
                {
                    EXPECT_FALSE( std::isnan( corner.x() ) ) << "Corner X is NaN at (" << i << "," << j << "," << k << ")";
                    EXPECT_FALSE( std::isnan( corner.y() ) ) << "Corner Y is NaN at (" << i << "," << j << "," << k << ")";
                    EXPECT_FALSE( std::isnan( corner.z() ) ) << "Corner Z is NaN at (" << i << "," << j << "," << k << ")";

                    if ( corner.x() != 0.0 || corner.y() != 0.0 || corner.z() != 0.0 )
                    {
                        hasNonZeroValues = true;
                    }
                }
            }
        }
    }

    EXPECT_TRUE( hasNonZeroValues ) << "All corner coordinates are zero - this is suspicious";
}

//--------------------------------------------------------------------------------------------------
/// Test MAPAXES functionality
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, MapAxes )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 1, 1, 1 );
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    // Should match main grid MAPAXES settings
    EXPECT_EQ( mainGrid->useMapAxes(), adapter.useMapAxes() );

    if ( adapter.useMapAxes() )
    {
        auto adapterMapAxes  = adapter.mapAxes();
        auto mainGridMapAxes = mainGrid->mapAxesF();

        for ( size_t i = 0; i < 6; ++i )
        {
            EXPECT_NEAR( mainGridMapAxes[i], adapterMapAxes[i], 0.001 ) << "MAPAXES value " << i << " mismatch";
        }

        // Transform matrices should also match
        auto adapterTransform  = adapter.mapAxisTransform();
        auto mainGridTransform = mainGrid->mapAxisTransform();

        for ( int i = 0; i < 4; ++i )
        {
            for ( int j = 0; j < 4; ++j )
            {
                EXPECT_NEAR( mainGridTransform.rowCol( i, j ), adapterTransform.rowCol( i, j ), 0.001 )
                    << "Transform matrix[" << i << "][" << j << "] mismatch";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test face corners access without refinement
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, FaceCornersNoRefinement )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    const RigMainGrid* mainGrid = caseData->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 0, 0, 0 ); // Just first cell
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    // Get face corners from adapter
    auto adapterTopFace    = adapter.getFaceCorners( 0, 0, 0, cvf::StructGridInterface::NEG_K );
    auto adapterBottomFace = adapter.getFaceCorners( 0, 0, 0, cvf::StructGridInterface::POS_K );

    // Get face corners directly from main grid for comparison
    size_t mainGridCellIndex  = mainGrid->cellIndexFromIJK( 0, 0, 0 );
    auto   cell               = mainGrid->cell( mainGridCellIndex );
    auto   mainGridTopFace    = cell.faceCorners( cvf::StructGridInterface::NEG_K );
    auto   mainGridBottomFace = cell.faceCorners( cvf::StructGridInterface::POS_K );

    // Apply same coordinate transformation if needed
    if ( adapter.useMapAxes() )
    {
        cvf::Mat4d transform = adapter.mapAxisTransform();
        for ( cvf::Vec3d& corner : mainGridTopFace )
        {
            corner.transformPoint( transform );
        }
        for ( cvf::Vec3d& corner : mainGridBottomFace )
        {
            corner.transformPoint( transform );
        }
    }

    // Compare top face corners
    for ( size_t i = 0; i < 4; ++i )
    {
        EXPECT_NEAR( mainGridTopFace[i].x(), adapterTopFace[i].x(), 0.001 ) << "Top face corner " << i << " X mismatch";
        EXPECT_NEAR( mainGridTopFace[i].y(), adapterTopFace[i].y(), 0.001 ) << "Top face corner " << i << " Y mismatch";
        EXPECT_NEAR( mainGridTopFace[i].z(), adapterTopFace[i].z(), 0.001 ) << "Top face corner " << i << " Z mismatch";
    }

    // Compare bottom face corners
    for ( size_t i = 0; i < 4; ++i )
    {
        EXPECT_NEAR( mainGridBottomFace[i].x(), adapterBottomFace[i].x(), 0.001 ) << "Bottom face corner " << i << " X mismatch";
        EXPECT_NEAR( mainGridBottomFace[i].y(), adapterBottomFace[i].y(), 0.001 ) << "Bottom face corner " << i << " Y mismatch";
        EXPECT_NEAR( mainGridBottomFace[i].z(), adapterBottomFace[i].z(), 0.001 ) << "Bottom face corner " << i << " Z mismatch";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test that face corners are consistent with cell corners for refined grids
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, FaceCornersRefinedConsistency )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max = cvf::Vec3st::UNDEFINED;
    cvf::Vec3st refinement( 2, 2, 2 );
    cvf::Vec3st noRefinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );
    RigGridExportAdapter adapterNoRefinement( caseData.p(), min, max, noRefinement );

    auto makeVector = []( const std::array<cvf::Vec3d, 4>& input1, const std::array<cvf::Vec3d, 4>& input2 )
    {
        std::vector<cvf::Vec3d> output;
        for ( auto c : input1 )
            output.push_back( c );
        for ( auto c : input2 )
            output.push_back( c );
        return output;
    };

    // Get both cell corners and face corners
    auto topFaceCorners    = adapterNoRefinement.getFaceCorners( 0, 0, 0, cvf::StructGridInterface::NEG_K );
    auto bottomFaceCorners = adapterNoRefinement.getFaceCorners( 0, 0, 0, cvf::StructGridInterface::POS_K );

    // Track which original corners we've found in refined cells
    std::vector<bool> cornerFound( 8, false );
    const double      tolerance = 0.001;

    // Test a few refined subcells
    for ( size_t k = 0; k < 2; ++k )
    {
        for ( size_t j = 0; j < 2; ++j )
        {
            for ( size_t i = 0; i < 2; ++i )
            {
                auto topFace    = adapter.getFaceCorners( i, j, k, cvf::StructGridInterface::NEG_K );
                auto bottomFace = adapter.getFaceCorners( i, j, k, cvf::StructGridInterface::POS_K );

                size_t originalIdx = 0;
                for ( auto originalCorner : makeVector( topFaceCorners, bottomFaceCorners ) )
                {
                    for ( auto refinedCorner : makeVector( topFace, bottomFace ) )
                    {
                        double distance = refinedCorner.pointDistance( originalCorner );
                        if ( distance < tolerance )
                        {
                            // Verify this original corner wasn't already found
                            EXPECT_FALSE( cornerFound[originalIdx] ) << "Original corner " << originalIdx << " found in multiple refined cells";
                            cornerFound[originalIdx] = true;
                        }
                    }
                    originalIdx++;
                }
            }
        }
    }

    // Verify all 8 original corners were found exactly once in the refined cells
    for ( size_t i = 0; i < 8; ++i )
    {
        EXPECT_TRUE( cornerFound[i] ) << "Original corner " << i << " was not found in any refined cell";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test all face types for consistency
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, AllFaceTypes )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 0, 0, 0 ); // Just first cell
    cvf::Vec3st refinement( 1, 1, 1 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    // Test all face types
    std::vector<cvf::StructGridInterface::FaceType> faceTypes = { cvf::StructGridInterface::NEG_K,
                                                                  cvf::StructGridInterface::POS_K,
                                                                  cvf::StructGridInterface::NEG_I,
                                                                  cvf::StructGridInterface::POS_I,
                                                                  cvf::StructGridInterface::NEG_J,
                                                                  cvf::StructGridInterface::POS_J };

    for ( auto faceType : faceTypes )
    {
        auto faceCorners = adapter.getFaceCorners( 0, 0, 0, faceType );

        // Verify that we get exactly 4 corners and they are reasonable values
        EXPECT_EQ( 4, faceCorners.size() ) << "Face type " << static_cast<int>( faceType ) << " should have 4 corners";

        for ( size_t i = 0; i < 4; ++i )
        {
            EXPECT_FALSE( std::isnan( faceCorners[i].x() ) ) << "Face " << static_cast<int>( faceType ) << " corner " << i << " X is NaN";
            EXPECT_FALSE( std::isnan( faceCorners[i].y() ) ) << "Face " << static_cast<int>( faceType ) << " corner " << i << " Y is NaN";
            EXPECT_FALSE( std::isnan( faceCorners[i].z() ) ) << "Face " << static_cast<int>( faceType ) << " corner " << i << " Z is NaN";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test that getFaceCorners with 2x2x2 refinement preserves original cell corners
/// Each of the 8 original cell corners should appear in exactly one of the 8 refined subcells
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapterTest, RefinedCellsContainOriginalCorners )
{
    auto caseData = loadTestGrid();
    ASSERT_TRUE( caseData.notNull() );

    // Test cell (1,0,0) with 2x2x2 refinement
    cvf::Vec3st min( 1, 0, 0 );
    cvf::Vec3st max = cvf::Vec3st::UNDEFINED; //( 1, 0, 0 );
    cvf::Vec3st refinement( 2, 2, 2 );

    RigGridExportAdapter adapter( caseData.p(), min, max, refinement );

    // Get the original cell corners
    const RigMainGrid* mainGrid          = caseData->mainGrid();
    size_t             originalCellIndex = mainGrid->cellIndexFromIJK( 1, 0, 0 );
    auto               originalCorners   = mainGrid->cellCornerVertices( originalCellIndex );

    // Apply same coordinate transformation as the adapter
    if ( adapter.useMapAxes() )
    {
        cvf::Mat4d transform = adapter.mapAxisTransform();
        for ( cvf::Vec3d& corner : originalCorners )
        {
            corner.transformPoint( transform );
        }
    }

    // Track which original corners we've found in refined cells
    std::vector<bool> cornerFound( 8, false );
    const double      tolerance = 0.001;

    // Check all 8 refined cells (2x2x2)
    for ( size_t k = 0; k < 2; ++k )
    {
        for ( size_t j = 0; j < 2; ++j )
        {
            for ( size_t i = 0; i < 2; ++i )
            {
                // Get all 8 corners of this refined cell
                auto refinedCellCorners = adapter.getCellCorners( i, j, k );

                // Check if any refined corner matches any original corner
                for ( size_t refinedIdx = 0; refinedIdx < 8; ++refinedIdx )
                {
                    const auto& refinedCorner = refinedCellCorners[refinedIdx];

                    for ( size_t originalIdx = 0; originalIdx < 8; ++originalIdx )
                    {
                        const auto& originalCorner = originalCorners[originalIdx];

                        double distance = refinedCorner.pointDistance( originalCorner );
                        if ( distance < tolerance )
                        {
                            // Verify this original corner wasn't already found
                            EXPECT_FALSE( cornerFound[originalIdx] ) << "Original corner " << originalIdx << " found in multiple refined cells";

                            cornerFound[originalIdx] = true;

                            EXPECT_NEAR( refinedCorner.x(), originalCorner.x(), tolerance )
                                << "Refined cell (" << i << "," << j << "," << k << ") corner " << refinedIdx
                                << " should match original corner " << originalIdx << " X coordinate";
                            EXPECT_NEAR( refinedCorner.y(), originalCorner.y(), tolerance )
                                << "Refined cell (" << i << "," << j << "," << k << ") corner " << refinedIdx
                                << " should match original corner " << originalIdx << " Y coordinate";
                            EXPECT_NEAR( refinedCorner.z(), originalCorner.z(), tolerance )
                                << "Refined cell (" << i << "," << j << "," << k << ") corner " << refinedIdx
                                << " should match original corner " << originalIdx << " Z coordinate";
                        }
                    }
                }
            }
        }
    }

    // Verify all 8 original corners were found exactly once in the refined cells
    for ( size_t i = 0; i < 8; ++i )
    {
        EXPECT_TRUE( cornerFound[i] ) << "Original corner " << i << " was not found in any refined cell";
    }
}
