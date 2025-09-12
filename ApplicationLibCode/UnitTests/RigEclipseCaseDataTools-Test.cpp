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
