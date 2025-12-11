/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RiaPorosityModel.h"
#include "RiaResultNames.h"
#include "RiaTestDataDirectory.h"

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseOutput.h"

#include "RimEclipseResultCase.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultTools.h"
#include "RigMainGrid.h"
#include "Well/RigSimWellData.h"

#include "cvfBoundingBox.h"

#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, WellBoundingBoxWithBruggeModel )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    eclipseCase->mainGrid()->computeCachedData();

    // Find the P19-02 well
    auto                  wells    = eclipseCase->wellResults();
    const RigSimWellData* testWell = nullptr;

    for ( const auto& well : wells )
    {
        if ( well->m_wellName == "P19-02" )
        {
            testWell = well.p();
            break;
        }
    }

    // List available wells for debugging if P19-02 is not found
    QString availableWells;
    for ( const auto& well : wells )
    {
        availableWells += well->m_wellName + ", ";
    }

    ASSERT_TRUE( testWell != nullptr ) << "Could not find P19-02 well in BRUGGE model. Available wells: " << availableWells.toStdString();

    // Test domain coordinates bounding box
    int  timeStep                 = 0;
    bool isAutoDetectingBranches  = true;
    bool isUsingCellCenterForPipe = true;

    cvf::BoundingBox domainBB = RigEclipseCaseDataTools::wellBoundingBoxInDomainCoords( eclipseCase.p(),
                                                                                        testWell,
                                                                                        timeStep,
                                                                                        isAutoDetectingBranches,
                                                                                        isUsingCellCenterForPipe );

    ASSERT_TRUE( domainBB.isValid() ) << "Domain bounding box should be valid";

    cvf::Vec3d minPt = domainBB.min();
    cvf::Vec3d maxPt = domainBB.max();

    ASSERT_LE( minPt.x(), maxPt.x() );
    ASSERT_LE( minPt.y(), maxPt.y() );
    ASSERT_LE( minPt.z(), maxPt.z() );

    // Test IJK coordinates bounding box
    auto [minIjk, maxIjk] =
        RigEclipseCaseDataTools::wellBoundingBoxIjk( eclipseCase.p(), testWell, timeStep, isAutoDetectingBranches, isUsingCellCenterForPipe );

    ASSERT_NE( minIjk, caf::VecIjk0::UNDEFINED ) << "Min IJK should be valid";
    ASSERT_NE( maxIjk, caf::VecIjk0::UNDEFINED ) << "Max IJK should be valid";

    // Sanity checks
    ASSERT_LE( minIjk.x(), maxIjk.x() ) << "Min I should be <= Max I";
    ASSERT_LE( minIjk.y(), maxIjk.y() ) << "Min J should be <= Max J";
    ASSERT_LE( minIjk.z(), maxIjk.z() ) << "Min K should be <= Max K";

    // Expected values from visual inspection
    ASSERT_EQ( minIjk.x(), 51 );
    ASSERT_EQ( minIjk.y(), 42 );
    ASSERT_EQ( minIjk.z(), 2 );

    ASSERT_EQ( maxIjk.x(), 51 );
    ASSERT_EQ( maxIjk.y(), 42 );
    ASSERT_EQ( maxIjk.z(), 4 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, MultipleWellsBoundingBoxWithBruggeModel )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    eclipseCase->mainGrid()->computeCachedData();

    // Find wells P19-03 and P20-03
    auto                               wells = eclipseCase->wellResults();
    std::vector<const RigSimWellData*> testWells;

    for ( const auto& well : wells )
    {
        if ( well->m_wellName == "P19-03" || well->m_wellName == "P20-03" )
        {
            testWells.push_back( well.p() );
        }
    }

    // List available wells for debugging
    QString availableWells;
    for ( const auto& well : wells )
    {
        availableWells += well->m_wellName + ", ";
    }

    ASSERT_EQ( testWells.size(), 2 ) << "Could not find both P19-03 and P20-03 wells in BRUGGE model. Available wells: "
                                     << availableWells.toStdString();

    // Test multiple wells IJK coordinates bounding box
    int  timeStep                 = 0;
    bool isAutoDetectingBranches  = true;
    bool isUsingCellCenterForPipe = true;

    auto [minIjk, maxIjk] =
        RigEclipseCaseDataTools::wellsBoundingBoxIjk( eclipseCase.p(), testWells, timeStep, isAutoDetectingBranches, isUsingCellCenterForPipe );

    ASSERT_NE( minIjk, caf::VecIjk0::UNDEFINED ) << "Min IJK should be valid";
    ASSERT_NE( maxIjk, caf::VecIjk0::UNDEFINED ) << "Max IJK should be valid";

    // Expected values from visual inspection
    ASSERT_EQ( minIjk.x(), 44 );
    ASSERT_EQ( minIjk.y(), 41 );
    ASSERT_EQ( minIjk.z(), 5 );

    ASSERT_EQ( maxIjk.x(), 51 );
    ASSERT_EQ( maxIjk.y(), 42 );
    ASSERT_EQ( maxIjk.z(), 7 );

    // Calculate total cells in bounding box
    size_t totalCells = ( maxIjk.x() - minIjk.x() + 1 ) * ( maxIjk.y() - minIjk.y() + 1 ) * ( maxIjk.z() - minIjk.z() + 1 );

    // Verify that we have a reasonable number of cells (48 based on the actual grid structure)
    ASSERT_EQ( totalCells, 48 ) << "Expected 48 cells in bounding box for P19-03 and P20-03";

    // Sanity checks
    ASSERT_LE( minIjk.x(), maxIjk.x() ) << "Min I should be <= Max I";
    ASSERT_LE( minIjk.y(), maxIjk.y() ) << "Min J should be <= Max J";
    ASSERT_LE( minIjk.z(), maxIjk.z() ) << "Min K should be <= Max K";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, ExpandBoundingBoxIjkWithPadding )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    eclipseCase->mainGrid()->computeCachedData();

    // Get grid dimensions for boundary testing
    auto   mainGrid       = eclipseCase->mainGrid();
    size_t gridCellCountI = mainGrid->cellCountI();
    size_t gridCellCountJ = mainGrid->cellCountJ();
    size_t gridCellCountK = mainGrid->cellCountK();

    // Test case 1: Normal expansion with padding (using safe values that won't hit grid boundaries)
    caf::VecIjk0 originalMin( 20, 20, 3 );
    caf::VecIjk0 originalMax( 22, 22, 4 );
    size_t       padding = 3;

    auto [expandedMin1, expandedMax1] = RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), originalMin, originalMax, padding );

    ASSERT_NE( expandedMin1, caf::VecIjk0::UNDEFINED ) << "Expanded min should be valid";
    ASSERT_NE( expandedMax1, caf::VecIjk0::UNDEFINED ) << "Expanded max should be valid";

    // Check expansion worked correctly (these values should not hit grid boundaries)
    ASSERT_EQ( expandedMin1.x(), originalMin.x() - padding ) << "Min I should be reduced by padding";
    ASSERT_EQ( expandedMin1.y(), originalMin.y() - padding ) << "Min J should be reduced by padding";
    ASSERT_EQ( expandedMin1.z(), originalMin.z() - padding ) << "Min K should be reduced by padding";

    ASSERT_EQ( expandedMax1.x(), originalMax.x() + padding ) << "Max I should be increased by padding";
    ASSERT_EQ( expandedMax1.y(), originalMax.y() + padding ) << "Max J should be increased by padding";
    ASSERT_EQ( expandedMax1.z(), originalMax.z() + padding ) << "Max K should be increased by padding";

    // Test case 2: Expansion at grid boundary - ensure min never goes below 0
    caf::VecIjk0 boundaryMin( 1, 1, 1 );
    caf::VecIjk0 boundaryMax( 3, 3, 3 );
    size_t       largePadding = 5;

    auto [expandedMin2, expandedMax2] =
        RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), boundaryMin, boundaryMax, largePadding );

    ASSERT_NE( expandedMin2, caf::VecIjk0::UNDEFINED ) << "Boundary expanded min should be valid";
    ASSERT_NE( expandedMax2, caf::VecIjk0::UNDEFINED ) << "Boundary expanded max should be valid";

    // Ensure min coordinates never go negative (should be clamped to 0)
    ASSERT_EQ( expandedMin2.x(), 0 ) << "Min I should be clamped to 0";
    ASSERT_EQ( expandedMin2.y(), 0 ) << "Min J should be clamped to 0";
    ASSERT_EQ( expandedMin2.z(), 0 ) << "Min K should be clamped to 0";

    // Test case 3: Expansion at grid upper boundary - ensure max never exceeds grid dimensions
    caf::VecIjk0 upperBoundaryMin( gridCellCountI - 3, gridCellCountJ - 3, gridCellCountK - 3 );
    caf::VecIjk0 upperBoundaryMax( gridCellCountI - 1, gridCellCountJ - 1, gridCellCountK - 1 );

    auto [expandedMin3, expandedMax3] =
        RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), upperBoundaryMin, upperBoundaryMax, largePadding );

    ASSERT_NE( expandedMin3, caf::VecIjk0::UNDEFINED ) << "Upper boundary expanded min should be valid";
    ASSERT_NE( expandedMax3, caf::VecIjk0::UNDEFINED ) << "Upper boundary expanded max should be valid";

    // Ensure max coordinates never exceed grid dimensions (should be clamped to grid size - 1)
    ASSERT_EQ( expandedMax3.x(), gridCellCountI - 1 ) << "Max I should be clamped to grid size - 1";
    ASSERT_EQ( expandedMax3.y(), gridCellCountJ - 1 ) << "Max J should be clamped to grid size - 1";
    ASSERT_EQ( expandedMax3.z(), gridCellCountK - 1 ) << "Max K should be clamped to grid size - 1";

    // General sanity checks for all test cases
    ASSERT_LE( expandedMin1.x(), expandedMax1.x() ) << "Expanded Min I should be <= Max I";
    ASSERT_LE( expandedMin1.y(), expandedMax1.y() ) << "Expanded Min J should be <= Max J";
    ASSERT_LE( expandedMin1.z(), expandedMax1.z() ) << "Expanded Min K should be <= Max K";

    ASSERT_LE( expandedMin2.x(), expandedMax2.x() ) << "Boundary expanded Min I should be <= Max I";
    ASSERT_LE( expandedMin2.y(), expandedMax2.y() ) << "Boundary expanded Min J should be <= Max J";
    ASSERT_LE( expandedMin2.z(), expandedMax2.z() ) << "Boundary expanded Min K should be <= Max K";

    ASSERT_LE( expandedMin3.x(), expandedMax3.x() ) << "Upper boundary expanded Min I should be <= Max I";
    ASSERT_LE( expandedMin3.y(), expandedMax3.y() ) << "Upper boundary expanded Min J should be <= Max J";
    ASSERT_LE( expandedMin3.z(), expandedMax3.z() ) << "Upper boundary expanded Min K should be <= Max K";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, CreateVisibilityFromIjkBounds )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    ASSERT_NE( eclipseCase->mainGrid(), nullptr ) << "Main grid should not be null";
    ASSERT_GT( eclipseCase->mainGrid()->cellCount(), 0 ) << "Grid should contain cells";

    // Get grid dimensions for test validation
    size_t gridCellCountI = eclipseCase->mainGrid()->cellCountI();
    size_t gridCellCountJ = eclipseCase->mainGrid()->cellCountJ();
    size_t gridCellCountK = eclipseCase->mainGrid()->cellCountK();
    size_t totalCellCount = eclipseCase->mainGrid()->cellCount();

    // Test case 1: Basic visibility generation with small bounds
    caf::VecIjk0 minIjk( 5, 5, 5 );
    caf::VecIjk0 maxIjk( 10, 10, 8 );

    auto visibility = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), minIjk, maxIjk );

    ASSERT_FALSE( visibility.isNull() ) << "Visibility array should not be null";
    ASSERT_EQ( visibility->size(), totalCellCount ) << "Visibility array should match total cell count";

    // Count visible cells and verify they match expected IJK range
    size_t expectedVisibleCells = ( maxIjk.x() - minIjk.x() + 1 ) * ( maxIjk.y() - minIjk.y() + 1 ) * ( maxIjk.z() - minIjk.z() + 1 );
    size_t actualVisibleCells   = 0;

    for ( size_t i = 0; i < totalCellCount; ++i )
    {
        if ( visibility->val( i ) )
        {
            actualVisibleCells++;
        }
    }

    ASSERT_EQ( actualVisibleCells, expectedVisibleCells ) << "Number of visible cells should match IJK range volume";

    // Test case 2: Verify specific cells within bounds are visible
    for ( size_t i = minIjk.x(); i <= maxIjk.x(); ++i )
    {
        for ( size_t j = minIjk.y(); j <= maxIjk.y(); ++j )
        {
            for ( size_t k = minIjk.z(); k <= maxIjk.z(); ++k )
            {
                size_t cellIndex = eclipseCase->mainGrid()->cellIndexFromIJK( i, j, k );
                ASSERT_TRUE( visibility->val( cellIndex ) ) << "Cell at IJK (" << i << "," << j << "," << k << ") should be visible";
            }
        }
    }

    // Test case 3: Verify cells outside bounds are invisible
    for ( size_t i = 0; i < std::min( minIjk.x(), size_t( 3 ) ); ++i )
    {
        for ( size_t j = 0; j < std::min( minIjk.y(), size_t( 3 ) ); ++j )
        {
            for ( size_t k = 0; k < std::min( minIjk.z(), size_t( 3 ) ); ++k )
            {
                size_t cellIndex = eclipseCase->mainGrid()->cellIndexFromIJK( i, j, k );
                ASSERT_FALSE( visibility->val( cellIndex ) ) << "Cell at IJK (" << i << "," << j << "," << k << ") should be invisible";
            }
        }
    }

    // Test case 4: Edge case - single cell bounds
    caf::VecIjk0 singleCellMin( 2, 2, 2 );
    caf::VecIjk0 singleCellMax( 2, 2, 2 );

    auto singleCellVisibility = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), singleCellMin, singleCellMax );

    ASSERT_FALSE( singleCellVisibility.isNull() ) << "Single cell visibility should not be null";

    size_t singleCellVisibleCount = 0;
    for ( size_t i = 0; i < totalCellCount; ++i )
    {
        if ( singleCellVisibility->val( i ) )
        {
            singleCellVisibleCount++;
        }
    }

    ASSERT_EQ( singleCellVisibleCount, 1 ) << "Exactly one cell should be visible for single cell bounds";

    // Verify the correct single cell is visible
    size_t expectedSingleCellIndex = eclipseCase->mainGrid()->cellIndexFromIJK( 2, 2, 2 );
    ASSERT_TRUE( singleCellVisibility->val( expectedSingleCellIndex ) ) << "The specified single cell should be visible";

    // Test case 5: Error handling - invalid inputs
    auto invalidVisibility1 = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( nullptr, minIjk, maxIjk );
    ASSERT_TRUE( invalidVisibility1.isNull() ) << "Null case data should return null visibility";

    auto invalidVisibility2 = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), caf::VecIjk0::UNDEFINED, maxIjk );
    ASSERT_TRUE( invalidVisibility2.isNull() ) << "Undefined minIjk should return null visibility";

    auto invalidVisibility3 = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), minIjk, caf::VecIjk0::UNDEFINED );
    ASSERT_TRUE( invalidVisibility3.isNull() ) << "Undefined maxIjk should return null visibility";

    // Test case 6: Boundary conditions - bounds at grid limits
    caf::VecIjk0 gridMinBounds( 0, 0, 0 );
    caf::VecIjk0 gridMaxBounds( gridCellCountI - 1, gridCellCountJ - 1, gridCellCountK - 1 );

    auto fullGridVisibility = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), gridMinBounds, gridMaxBounds );

    ASSERT_FALSE( fullGridVisibility.isNull() ) << "Full grid visibility should not be null";

    size_t fullGridVisibleCount = 0;
    for ( size_t i = 0; i < totalCellCount; ++i )
    {
        if ( fullGridVisibility->val( i ) )
        {
            fullGridVisibleCount++;
        }
    }

    ASSERT_EQ( fullGridVisibleCount, totalCellCount ) << "All cells should be visible when using full grid bounds";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, GenerateBorderResultFromIjkBounds )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    resultCase->setReservoirData( eclipseCase.p() );

    // Define IJK bounds for testing
    caf::VecIjk0 minIjk( 20, 20, 5 );
    caf::VecIjk0 maxIjk( 30, 30, 8 );

    // Create visibility from IJK bounds
    auto visibility = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), minIjk, maxIjk );
    ASSERT_FALSE( visibility.isNull() ) << "Visibility should be created successfully";

    // Generate border result using the custom visibility (returns vector)
    auto borderResult = RigEclipseResultTools::generateBorderResult( resultCase.get(), visibility );

    // Verify that the result was generated
    ASSERT_GT( borderResult.size(), 0 ) << "Border result should not be empty";

    // Count cells with different values
    int invisibleCells = 0;
    int borderCells    = 0;
    int interiorCells  = 0;

    for ( int value : borderResult )
    {
        if ( value == RigEclipseResultTools::BorderType::INVISIBLE_CELL )
            invisibleCells++;
        else if ( value == RigEclipseResultTools::BorderType::BORDER_CELL )
            borderCells++;
        else if ( value == RigEclipseResultTools::BorderType::INTERIOR_CELL )
            interiorCells++;
    }

    // Verify that we have all three types of cells
    ASSERT_GT( invisibleCells, 0 ) << "Should have invisible cells (value 0)";
    ASSERT_GT( borderCells, 0 ) << "Should have border cells (value 1)";
    ASSERT_GE( interiorCells, 0 ) << "May have interior cells (value 2)";

    // The total should match the result vector size
    int totalCells = invisibleCells + borderCells + interiorCells;
    ASSERT_EQ( totalCells, static_cast<int>( borderResult.size() ) ) << "Total should match result vector size";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataToolsTest, GenerateOperNumResultFromBorderResult )
{
    // Load the BRUGGE test model using RifReaderEclipseOutput
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filename( "BRUGGE_0000.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    ASSERT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          eclipseCase = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, eclipseCase.p() );
    ASSERT_TRUE( success ) << "Could not load BRUGGE test model";

    resultCase->setReservoirData( eclipseCase.p() );

    // Define IJK bounds for testing
    caf::VecIjk0 minIjk( 20, 20, 5 );
    caf::VecIjk0 maxIjk( 30, 30, 8 );

    // Create visibility from IJK bounds
    auto visibility = RigEclipseCaseDataTools::createVisibilityFromIjkBounds( eclipseCase.p(), minIjk, maxIjk );
    ASSERT_FALSE( visibility.isNull() ) << "Visibility should be created successfully";

    // Generate border result first (required for OPERNUM generation)
    auto borderResult = RigEclipseResultTools::generateBorderResult( resultCase.get(), visibility );

    // Verify border result was generated
    ASSERT_GT( borderResult.size(), 0 ) << "Border result should not be empty";

    // Test case 1: Generate OPERNUM with automatic border cell value (no existing OPERNUM, should get 2)
    auto opernumResult = RigEclipseResultTools::generateOperNumResult( resultCase.get(), borderResult );

    // Verify OPERNUM was generated
    ASSERT_EQ( borderResult.size(), opernumResult.size() ) << "BORDNUM and OPERNUM should have same size";
    ASSERT_GT( opernumResult.size(), 0 ) << "OPERNUM vector should not be empty";

    // Count cells with different values and verify the logic
    int borderCellsWithOperValue = 0; // BORDNUM=1 cells that should get OPERNUM=2 (max 0 + 1, then default 2)
    int defaultOperCells         = 0; // BORDNUM!=1 cells that should get OPERNUM=1
    int borderCellsTotal         = 0; // Total BORDNUM=1 cells

    for ( size_t i = 0; i < borderResult.size(); ++i )
    {
        int bordnumValue = borderResult[i];
        int opernumValue = opernumResult[i];

        if ( bordnumValue == RigEclipseResultTools::BorderType::BORDER_CELL )
        {
            borderCellsTotal++;
            ASSERT_EQ( opernumValue, 2 ) << "Border cells (BORDNUM=1) should have OPERNUM=2 at index " << i;
            borderCellsWithOperValue++;
        }
        else
        {
            ASSERT_EQ( opernumValue, 1 ) << "Non-border cells should have OPERNUM=1 at index " << i;
            defaultOperCells++;
        }
    }

    // Verify we have the expected distribution
    ASSERT_GT( borderCellsTotal, 0 ) << "Should have border cells (BORDNUM=1)";
    ASSERT_EQ( borderCellsWithOperValue, borderCellsTotal ) << "All border cells should get OPERNUM=2";
    ASSERT_GT( defaultOperCells, 0 ) << "Should have non-border cells with OPERNUM=1";

    // Test case 2: Test automatic max value detection with existing OPERNUM
    // First create a custom OPERNUM result with some predefined values
    auto activeReservoirCellIdxs = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();
    int  numActiveCells          = static_cast<int>( activeReservoirCellIdxs.size() );

    std::vector<int> customOperValues;
    customOperValues.resize( eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirCellCount(),
                             5 ); // Set all to 5 initially
    // Add some higher values to test max detection
    if ( numActiveCells > 10 )
    {
        customOperValues[5] = 15; // Maximum will be 15
        customOperValues[8] = 12;
    }

    // Create the OPERNUM result directly
    RigEclipseResultTools::createResultVector( *resultCase, RiaResultNames::opernum(), customOperValues );

    // Test case 3: Test the utility function directly (before regeneration)
    int maxOperValue = RigEclipseResultTools::findMaxOperNumValue( resultCase.get() );
    ASSERT_EQ( maxOperValue, 15 ) << "Max OPERNUM value should be 15";

    // Now generate OPERNUM again - should use max value + 1 = 16 for border cells
    auto updatedOpernumResult = RigEclipseResultTools::generateOperNumResult( resultCase.get(), borderResult );

    // Verify the behavior
    int preservedValues = 0;
    int modifiedValues  = 0;

    for ( size_t i = 0; i < borderResult.size(); ++i )
    {
        int bordnumValue = borderResult[i];
        int opernumValue = updatedOpernumResult[i];

        if ( bordnumValue == RigEclipseResultTools::BorderType::BORDER_CELL )
        {
            ASSERT_EQ( opernumValue, 16 ) << "Border cells should be updated to OPERNUM=16 (max 15 + 1) at index " << i;
            modifiedValues++;
        }
        else
        {
            // Non-border cells should preserve their original values
            if ( numActiveCells > 10 && i == 5 )
                ASSERT_EQ( opernumValue, 15 ) << "Non-border cells should preserve original max value at index " << i;
            else if ( numActiveCells > 10 && i == 8 )
                ASSERT_EQ( opernumValue, 12 ) << "Non-border cells should preserve original value at index " << i;
            else
                ASSERT_EQ( opernumValue, 5 ) << "Non-border cells should preserve original value at index " << i;
            preservedValues++;
        }
    }

    ASSERT_GT( modifiedValues, 0 ) << "Should have modified some border cells";
    ASSERT_GT( preservedValues, 0 ) << "Should have preserved some non-border cells";

    // Total should match vector size
    int totalChecked = modifiedValues + preservedValues;
    ASSERT_EQ( totalChecked, static_cast<int>( updatedOpernumResult.size() ) ) << "Should check all cells";
}
