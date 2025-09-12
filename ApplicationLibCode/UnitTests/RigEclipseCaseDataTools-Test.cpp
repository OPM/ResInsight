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

#include "RiaTestDataDirectory.h"

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseOutput.h"

#include "RimEclipseResultCase.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
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

    ASSERT_NE( minIjk, cvf::Vec3st::UNDEFINED ) << "Min IJK should be valid";
    ASSERT_NE( maxIjk, cvf::Vec3st::UNDEFINED ) << "Max IJK should be valid";

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

    ASSERT_NE( minIjk, cvf::Vec3st::UNDEFINED ) << "Min IJK should be valid";
    ASSERT_NE( maxIjk, cvf::Vec3st::UNDEFINED ) << "Max IJK should be valid";

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
    cvf::Vec3st originalMin( 20, 20, 3 );
    cvf::Vec3st originalMax( 22, 22, 4 );
    size_t      padding = 3;

    auto [expandedMin1, expandedMax1] = RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), originalMin, originalMax, padding );

    ASSERT_NE( expandedMin1, cvf::Vec3st::UNDEFINED ) << "Expanded min should be valid";
    ASSERT_NE( expandedMax1, cvf::Vec3st::UNDEFINED ) << "Expanded max should be valid";

    // Check expansion worked correctly (these values should not hit grid boundaries)
    ASSERT_EQ( expandedMin1.x(), originalMin.x() - padding ) << "Min I should be reduced by padding";
    ASSERT_EQ( expandedMin1.y(), originalMin.y() - padding ) << "Min J should be reduced by padding";
    ASSERT_EQ( expandedMin1.z(), originalMin.z() - padding ) << "Min K should be reduced by padding";

    ASSERT_EQ( expandedMax1.x(), originalMax.x() + padding ) << "Max I should be increased by padding";
    ASSERT_EQ( expandedMax1.y(), originalMax.y() + padding ) << "Max J should be increased by padding";
    ASSERT_EQ( expandedMax1.z(), originalMax.z() + padding ) << "Max K should be increased by padding";

    // Test case 2: Expansion at grid boundary - ensure min never goes below 0
    cvf::Vec3st boundaryMin( 1, 1, 1 );
    cvf::Vec3st boundaryMax( 3, 3, 3 );
    size_t      largePadding = 5;

    auto [expandedMin2, expandedMax2] =
        RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), boundaryMin, boundaryMax, largePadding );

    ASSERT_NE( expandedMin2, cvf::Vec3st::UNDEFINED ) << "Boundary expanded min should be valid";
    ASSERT_NE( expandedMax2, cvf::Vec3st::UNDEFINED ) << "Boundary expanded max should be valid";

    // Ensure min coordinates never go negative (should be clamped to 0)
    ASSERT_EQ( expandedMin2.x(), 0 ) << "Min I should be clamped to 0";
    ASSERT_EQ( expandedMin2.y(), 0 ) << "Min J should be clamped to 0";
    ASSERT_EQ( expandedMin2.z(), 0 ) << "Min K should be clamped to 0";

    // Test case 3: Expansion at grid upper boundary - ensure max never exceeds grid dimensions
    cvf::Vec3st upperBoundaryMin( gridCellCountI - 3, gridCellCountJ - 3, gridCellCountK - 3 );
    cvf::Vec3st upperBoundaryMax( gridCellCountI - 1, gridCellCountJ - 1, gridCellCountK - 1 );

    auto [expandedMin3, expandedMax3] =
        RigEclipseCaseDataTools::expandBoundingBoxIjk( eclipseCase.p(), upperBoundaryMin, upperBoundaryMax, largePadding );

    ASSERT_NE( expandedMin3, cvf::Vec3st::UNDEFINED ) << "Upper boundary expanded min should be valid";
    ASSERT_NE( expandedMax3, cvf::Vec3st::UNDEFINED ) << "Upper boundary expanded max should be valid";

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
