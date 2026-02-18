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

#include "CompletionExportCommands/RicTransmissibilityCalculator.h"

#include "RifReaderMockModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimWellPath.h"

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
        fractureActiveCellInfo->setCellResultIndex( i, i );
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
