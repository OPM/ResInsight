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

#include "CompletionExportCommands/RicFishbonesTransmissibilityCalculationFeatureImp.h"
#include "CompletionExportCommands/RicTransmissibilityCalculator.h"

#include "RifReaderMockModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

//--------------------------------------------------------------------------------------------------
/// Helper: Register a STATIC_NATIVE result with a uniform value for all cells
//--------------------------------------------------------------------------------------------------
static void addStaticResult( RigCaseCellResultsData* cellResults, const QString& name, size_t cellCount, double value )
{
    RigEclipseResultAddress address( RiaDefines::ResultCatType::STATIC_NATIVE, name );
    cellResults->createResultEntry( address, false );

    auto timeStepInfo = RigEclipseTimeStepInfo( QDateTime(), 0, 0.0 );
    cellResults->setTimeStepInfos( address, { timeStepInfo } );

    auto* timesteps = cellResults->modifiableCellScalarResultTimesteps( address );
    timesteps->resize( 1 );
    ( *timesteps )[0].assign( cellCount, value );
}

//--------------------------------------------------------------------------------------------------
/// Helper: Create a RimEclipseCase with mock grid and populated result data
//--------------------------------------------------------------------------------------------------
static RimEclipseCase*
    createMockEclipseCase( double dxVal, double dyVal, double dzVal, double permxVal, double permyVal, double permzVal, double ntgVal )
{
    auto* eclipseCase = new RimEclipseResultCase;

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( eclipseCase );

    {
        cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
        mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
        mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
        mockReader->enableWellData( false );
        mockReader->open( "", caseData.p() );
        caseData->mainGrid()->computeCachedData();
    }

    eclipseCase->setReservoirData( caseData.p() );

    size_t                  cellCount   = caseData->mainGrid()->totalCellCount();
    RigCaseCellResultsData* cellResults = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    addStaticResult( cellResults, "DX", cellCount, dxVal );
    addStaticResult( cellResults, "DY", cellCount, dyVal );
    addStaticResult( cellResults, "DZ", cellCount, dzVal );
    addStaticResult( cellResults, "PERMX", cellCount, permxVal );
    addStaticResult( cellResults, "PERMY", cellCount, permyVal );
    addStaticResult( cellResults, "PERMZ", cellCount, permzVal );
    addStaticResult( cellResults, "NTG", cellCount, ntgVal );

    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicTransmissibilityCalculator, CalculateCellMainDirection_DirectionI )
{
    RimEclipseCase* eclipseCase = createMockEclipseCase( 10.0, 20.0, 30.0, 100.0, 100.0, 10.0, 1.0 );

    // lengthsInCell = (8, 3, 2) → fractions: x=0.8, y=0.15, z=0.067 → DIR_I
    cvf::Vec3d lengthsInCell( 8.0, 3.0, 2.0 );

    auto direction = RicTransmissibilityCalculator::calculateCellMainDirection( eclipseCase, 0, lengthsInCell );

    EXPECT_EQ( RigCompletionData::CellDirection::DIR_I, direction );

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicTransmissibilityCalculator, CalculateCellMainDirection_DirectionK )
{
    RimEclipseCase* eclipseCase = createMockEclipseCase( 10.0, 20.0, 30.0, 100.0, 100.0, 10.0, 1.0 );

    // lengthsInCell = (1, 1, 25) → fractions: x=0.1, y=0.05, z=0.833 → DIR_K
    cvf::Vec3d lengthsInCell( 1.0, 1.0, 25.0 );

    auto direction = RicTransmissibilityCalculator::calculateCellMainDirection( eclipseCase, 0, lengthsInCell );

    EXPECT_EQ( RigCompletionData::CellDirection::DIR_K, direction );

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicTransmissibilityCalculator, CalculateTransmissibilityData_Valid )
{
    RimEclipseCase* eclipseCase = createMockEclipseCase( 100.0, 100.0, 10.0, 100.0, 100.0, 10.0, 1.0 );

    RimWellPath wellPath;
    wellPath.setUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    cvf::Vec3d internalCellLengths( 50.0, 50.0, 5.0 );
    double     skinFactor = 0.0;
    double     wellRadius = 0.1;

    auto result =
        RicTransmissibilityCalculator::calculateTransmissibilityData( eclipseCase, &wellPath, internalCellLengths, skinFactor, wellRadius, 0, false );

    EXPECT_TRUE( result.isValid() );
    EXPECT_GT( result.connectionFactor(), 0.0 );
    EXPECT_GT( result.kh(), 0.0 );
    EXPECT_GT( result.effectiveH(), 0.0 );
    EXPECT_GT( result.effectiveK(), 0.0 );

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicTransmissibilityCalculator, CalculateTransmissibilityAsEclipseDoes_DirectionK )
{
    RimEclipseCase* eclipseCase = createMockEclipseCase( 100.0, 100.0, 10.0, 100.0, 100.0, 10.0, 1.0 );

    double skinFactor = 0.0;
    double wellRadius = 0.1;

    double trans = RicTransmissibilityCalculator::calculateTransmissibilityAsEclipseDoes( eclipseCase,
                                                                                          skinFactor,
                                                                                          wellRadius,
                                                                                          0,
                                                                                          RigCompletionData::CellDirection::DIR_K );

    EXPECT_GT( trans, 0.0 );

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// Helper: Set up fracture active cell info and enable dual porosity
//--------------------------------------------------------------------------------------------------
static void enableDualPorosity( RigEclipseCaseData* caseData )
{
    size_t cellCount = caseData->mainGrid()->totalCellCount();

    RigActiveCellInfo* fractureActiveCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    fractureActiveCellInfo->setReservoirCellCount( cellCount );
    for ( size_t i = 0; i < cellCount; i++ )
    {
        fractureActiveCellInfo->setCellResultIndex( ReservoirCellIndex( i ), ActiveCellIndex( i ) );
    }
    fractureActiveCellInfo->setGridCount( 1 );
    fractureActiveCellInfo->setGridActiveCellCounts( 0, cellCount );
    fractureActiveCellInfo->computeDerivedData();

    caseData->mainGrid()->setDualPorosity( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicTransmissibilityCalculator, CalculateCellMainDirection_DualPorosity_UsesFractureProperties )
{
    // Matrix: DX=10, DY=20, DZ=30. Fracture: DX=100, DY=20, DZ=30.
    // lengthsInCell = (8, 3, 2)
    // Matrix fracs:   x=8/10=0.8, y=3/20=0.15, z=2/30=0.067 → DIR_I
    // Fracture fracs: x=8/100=0.08, y=3/20=0.15, z=2/30=0.067 → DIR_J
    RimEclipseCase* eclipseCase = createMockEclipseCase( 10.0, 20.0, 30.0, 100.0, 100.0, 10.0, 1.0 );

    RigEclipseCaseData* caseData  = eclipseCase->eclipseCaseData();
    size_t              cellCount = caseData->mainGrid()->totalCellCount();

    enableDualPorosity( caseData );

    // Populate fracture results with different DX
    RigCaseCellResultsData* fractureResults = caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    addStaticResult( fractureResults, "DX", cellCount, 100.0 );
    addStaticResult( fractureResults, "DY", cellCount, 20.0 );
    addStaticResult( fractureResults, "DZ", cellCount, 30.0 );
    addStaticResult( fractureResults, "PERMX", cellCount, 100.0 );
    addStaticResult( fractureResults, "PERMY", cellCount, 100.0 );
    addStaticResult( fractureResults, "PERMZ", cellCount, 10.0 );

    cvf::Vec3d lengthsInCell( 8.0, 3.0, 2.0 );

    auto direction = RicTransmissibilityCalculator::calculateCellMainDirection( eclipseCase, 0, lengthsInCell );

    // With matrix properties this would be DIR_I, but with fracture properties it should be DIR_J
    EXPECT_EQ( RigCompletionData::CellDirection::DIR_J, direction );

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicFishbonesTransmissibilityCalculation, NullWellPath_ReturnsEmptyMap )
{
    auto result = RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( nullptr, nullptr );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicFishbonesTransmissibilityCalculation, WellPathWithNoGeometry_ReturnsEmptyMap )
{
    RimWellPath wellPath;

    auto result = RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( &wellPath, nullptr );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicFishbonesTransmissibilityCalculation, WellPathWithSingleMeasuredDepth_ReturnsEmptyMap )
{
    RimWellPath wellPath;

    cvf::ref<RigWellPath> geometry = new RigWellPath( { cvf::Vec3d( 0, 0, -1000 ) }, { 1000.0 } );
    wellPath.setWellPathGeometry( geometry.p() );

    auto result = RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( &wellPath, nullptr );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicFishbonesTransmissibilityCalculation, MainBoreWithMockGrid_ReturnsIntersectedCells )
{
    // 2x2x2 mock grid, world coordinates (0,0,-100) to (100,100,0) — each cell is 50x50x50, z=0 is the top
    auto* eclipseCase = new RimEclipseResultCase;
    {
        cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( eclipseCase );

        cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
        mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, -100 ), cvf::Vec3d( 100, 100, 0 ) );
        mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
        mockReader->enableWellData( false );
        mockReader->open( "", caseData.p() );
        caseData->mainGrid()->computeCachedData();

        eclipseCase->setReservoirData( caseData.p() );

        size_t                  cellCount   = caseData->mainGrid()->totalCellCount();
        RigCaseCellResultsData* cellResults = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        addStaticResult( cellResults, "DX", cellCount, 50.0 );
        addStaticResult( cellResults, "DY", cellCount, 50.0 );
        addStaticResult( cellResults, "DZ", cellCount, 50.0 );
        addStaticResult( cellResults, "PERMX", cellCount, 100.0 );
        addStaticResult( cellResults, "PERMY", cellCount, 100.0 );
        addStaticResult( cellResults, "PERMZ", cellCount, 10.0 );
        addStaticResult( cellResults, "NTG", cellCount, 1.0 );
    }

    auto* project  = new RimProject;
    auto* wellPath = new RimWellPath;
    wellPath->setName( "TestWell" );
    wellPath->setUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Vertical well at (25, 25) passing through z=0 (top of grid) at MD=10, then into the grid
    cvf::ref<RigWellPath> geometry = new RigWellPath( { cvf::Vec3d( 25, 25, 10 ), cvf::Vec3d( 25, 25, -110 ) }, { 0.0, 120.0 } );
    wellPath->setWellPathGeometry( geometry.p() );

    project->oilFields[0]->wellPathCollection->addWellPath( wellPath );

    // Add a fishbone sub at MD=10, where the well passes through z=0 — fishbonesCollection is checked by default
    auto* sub = new RimFishbones;
    wellPath->fishbonesCollection()->appendFishbonesSubs( sub );
    sub->setMeasuredDepthAndCount( 10.0, 10.0, 1 );
    sub->setSubsOrientationMode( RimFishbonesDefines::LateralsOrientationType::FIXED );

    auto result = RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( wellPath, eclipseCase );

    EXPECT_FALSE( result.empty() );

    bool hasMainBoreEntry = false;
    for ( const auto& [cellIndex, parts] : result )
    {
        for ( const auto& part : parts )
        {
            if ( part.isMainBore ) hasMainBoreEntry = true;
        }
    }
    EXPECT_TRUE( hasMainBoreEntry );

    delete eclipseCase;
    project->close();
    delete project;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicFishbonesTransmissibilityCalculation, MainBoreWithMockGrid_ValidatesResultValues )
{
    // 2x2x2 mock grid, world coordinates (0,0,-100) to (100,100,0) — each cell is 50x50x50, z=0 is the top.
    // Cell layout (global index): k=0 is z=[-100,-50], k=1 is z=[-50,0].
    // Cells in k=1 (top layer): index 4=(i0,j0), 5=(i1,j0), 6=(i0,j1), 7=(i1,j1).
    auto* eclipseCase = new RimEclipseResultCase;
    {
        cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( eclipseCase );

        cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
        mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, -100 ), cvf::Vec3d( 100, 100, 0 ) );
        mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
        mockReader->enableWellData( false );
        mockReader->open( "", caseData.p() );
        caseData->mainGrid()->computeCachedData();

        eclipseCase->setReservoirData( caseData.p() );

        size_t                  cellCount   = caseData->mainGrid()->totalCellCount();
        RigCaseCellResultsData* cellResults = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        addStaticResult( cellResults, "DX", cellCount, 50.0 );
        addStaticResult( cellResults, "DY", cellCount, 50.0 );
        addStaticResult( cellResults, "DZ", cellCount, 50.0 );
        addStaticResult( cellResults, "PERMX", cellCount, 100.0 );
        addStaticResult( cellResults, "PERMY", cellCount, 100.0 );
        addStaticResult( cellResults, "PERMZ", cellCount, 10.0 );
        addStaticResult( cellResults, "NTG", cellCount, 1.0 );
    }

    auto* project  = new RimProject;
    auto* wellPath = new RimWellPath;
    wellPath->setName( "TestWell" );
    wellPath->setUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Vertical well at (25,25): starts above grid at z=10 (MD=0), crosses z=0 at MD=10, ends at z=-110 (MD=120).
    cvf::ref<RigWellPath> geometry = new RigWellPath( { cvf::Vec3d( 25, 25, 10 ), cvf::Vec3d( 25, 25, -110 ) }, { 0.0, 120.0 } );
    wellPath->setWellPathGeometry( geometry.p() );

    project->oilFields[0]->wellPathCollection->addWellPath( wellPath );

    // Fishbone sub at MD=10 (z=0, top of grid). Default metric mainBoreDiameter=0.216 m → radius=0.108 m.
    auto* sub = new RimFishbones;
    wellPath->fishbonesCollection()->appendFishbonesSubs( sub );
    sub->setMeasuredDepthAndCount( 10.0, 10.0, 1 );
    sub->setSubsOrientationMode( RimFishbonesDefines::LateralsOrientationType::FIXED );

    auto result = RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( wellPath, eclipseCase );

    // Expect exactly one cell intersected: the top-layer cell at (i=0, j=0, k=1), global index 4.
    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( 4u, result.begin()->first );

    const auto& parts = result.begin()->second;

    // Find the single main bore part (laterals also land in this cell but have isMainBore=false).
    const WellBorePartForTransCalc* mainBorePart  = nullptr;
    int                             mainBoreCount = 0;
    for ( const auto& part : parts )
    {
        if ( part.isMainBore )
        {
            mainBorePart = &part;
            mainBoreCount++;
        }
    }
    ASSERT_EQ( 1, mainBoreCount );
    ASSERT_NE( nullptr, mainBorePart );

    EXPECT_DOUBLE_EQ( 0.0, mainBorePart->skinFactor );
    EXPECT_NEAR( 0.108, mainBorePart->wellRadius, 1e-6 );

    // The well is vertical — only z-component of the intersection length should be non-zero.
    EXPECT_NEAR( 0.0, mainBorePart->lengthsInCell.x(), 1e-6 );
    EXPECT_NEAR( 0.0, mainBorePart->lengthsInCell.y(), 1e-6 );
    EXPECT_GT( mainBorePart->lengthsInCell.z(), 0.0 );

    // Intersection begins where the well crosses z=0 (top of grid), which is MD=10.
    EXPECT_NEAR( 10.0, mainBorePart->intersectionWithWellMeasuredDepth, 1.0 );

    EXPECT_TRUE( mainBorePart->metaData.contains( "main bore" ) );

    delete eclipseCase;
    project->close();
    delete project;
}
