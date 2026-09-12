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

#include "RiaDefines.h"
#include "RiaResultNames.h"

#include "RifReaderMockModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCaseEvaluator.h"

#include <QDateTime>

#include <memory>
#include <set>

namespace
{
struct MockCase
{
    std::unique_ptr<RimEclipseResultCase> resultCase;
    cvf::ref<RigEclipseCaseData>          caseData;
};

MockCase createMockCase( const std::set<RiaDefines::PhaseType>& phases )
{
    MockCase mockCase;
    mockCase.resultCase.reset( new RimEclipseResultCase );
    mockCase.caseData = new RigEclipseCaseData( mockCase.resultCase.get() );

    cvf::ref<RifReaderMockModel> mockReader = new RifReaderMockModel;
    mockReader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    mockReader->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
    mockReader->setResultInfo( 0, 0 );
    mockReader->enableWellData( false );
    mockReader->open( "", mockCase.caseData.p() );
    mockCase.caseData->mainGrid()->computeCachedData();

    mockCase.caseData->setAvailablePhases( phases );
    mockCase.resultCase->setReservoirData( mockCase.caseData.p() );

    return mockCase;
}

void addStaticResult( RigCaseCellResultsData* cellResults, const QString& resultName, const std::vector<double>& values )
{
    RigEclipseResultAddress addr( RiaDefines::ResultCatType::STATIC_NATIVE, resultName );
    cellResults->createResultEntry( addr, false );
    cellResults->setTimeStepInfos( addr, { RigEclipseTimeStepInfo( QDateTime(), 0, 0 ) } );

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( addr );
    timeStepValues->resize( 1 );
    ( *timeStepValues )[0] = values;
}

void addDynamicResult( RigCaseCellResultsData* cellResults, const QString& resultName, const std::vector<std::vector<double>>& valuesForEachTimeStep )
{
    RigEclipseResultAddress addr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultName );
    cellResults->createResultEntry( addr, false );

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;
    for ( int i = 0; i < static_cast<int>( valuesForEachTimeStep.size() ); i++ )
    {
        timeStepInfos.push_back( RigEclipseTimeStepInfo( QDateTime(), i, i ) );
    }
    cellResults->setTimeStepInfos( addr, timeStepInfos );

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( addr );
    timeStepValues->resize( valuesForEachTimeStep.size() );
    for ( size_t i = 0; i < valuesForEachTimeStep.size(); i++ )
    {
        ( *timeStepValues )[i] = valuesForEachTimeStep[i];
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Reproduces the per-time-step loading pattern used by RimEclipseStatisticsCaseEvaluator when computing
/// ensemble grid statistics (P10/P50/Mean/P90). Each time step is requested individually via
/// findOrLoadKnownScalarResultForTimeStep(), instead of loading all time steps at once.
//--------------------------------------------------------------------------------------------------
TEST( RigPorvSoilSgasResultCalculatorTest, ComputeRiPorvSoilPerTimeStep )
{
    MockCase mockCase =
        createMockCase( { RiaDefines::PhaseType::OIL_PHASE, RiaDefines::PhaseType::GAS_PHASE, RiaDefines::PhaseType::WATER_PHASE } );

    RigCaseCellResultsData* cellResults = mockCase.caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( cellResults, nullptr );

    const size_t cellCount = cellResults->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( cellCount, 0u );

    addStaticResult( cellResults, RiaResultNames::porv(), std::vector<double>( cellCount, 10.0 ) );

    const std::vector<std::vector<double>> soilValues = { std::vector<double>( cellCount, 0.2 ), std::vector<double>( cellCount, 0.4 ) };
    addDynamicResult( cellResults, RiaResultNames::soil(), soilValues );

    cellResults->createPlaceholderResultEntries();

    const RigEclipseResultAddress riPorvSoilAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil() );
    ASSERT_TRUE( cellResults->hasResultEntry( riPorvSoilAddr ) );

    // Mimic RimEclipseStatisticsCaseEvaluator::evaluateForResults(), which requests one time step at a time.
    for ( size_t timeStepIndex = 0; timeStepIndex < soilValues.size(); timeStepIndex++ )
    {
        size_t scalarResultIndex = cellResults->findOrLoadKnownScalarResultForTimeStep( riPorvSoilAddr, timeStepIndex );
        EXPECT_NE( scalarResultIndex, cvf::UNDEFINED_SIZE_T );

        const auto& riPorvSoilValues = cellResults->cellScalarResults( riPorvSoilAddr, timeStepIndex );
        ASSERT_EQ( riPorvSoilValues.size(), cellCount ) << "No values computed for time step " << timeStepIndex;

        for ( size_t i = 0; i < cellCount; i++ )
        {
            EXPECT_DOUBLE_EQ( riPorvSoilValues[i], 10.0 * soilValues[timeStepIndex][i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Reproduces issue #14703: computing ensemble grid statistics (P10/P50/Mean/P90 per cell across
/// realizations) for riPORV*SOIL produces no results, even though PORV and SOIL separately work.
//--------------------------------------------------------------------------------------------------
TEST( RigPorvSoilSgasResultCalculatorTest, EnsembleStatisticsForRiPorvSoil )
{
    std::vector<MockCase> sourceMockCases;
    for ( int caseIdx = 0; caseIdx < 2; caseIdx++ )
    {
        MockCase mockCase =
            createMockCase( { RiaDefines::PhaseType::OIL_PHASE, RiaDefines::PhaseType::GAS_PHASE, RiaDefines::PhaseType::WATER_PHASE } );

        RigCaseCellResultsData* cellResults = mockCase.caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        const size_t            cellCount   = cellResults->activeCellInfo()->reservoirActiveCellCount();
        ASSERT_GT( cellCount, 0u );

        double porvValue = 10.0 + caseIdx;
        addStaticResult( cellResults, RiaResultNames::porv(), std::vector<double>( cellCount, porvValue ) );

        double soilValueT0 = 0.2 + 0.1 * caseIdx;
        double soilValueT1 = 0.4 + 0.1 * caseIdx;
        addDynamicResult( cellResults,
                          RiaResultNames::soil(),
                          { std::vector<double>( cellCount, soilValueT0 ), std::vector<double>( cellCount, soilValueT1 ) } );

        cellResults->createPlaceholderResultEntries();

        sourceMockCases.push_back( std::move( mockCase ) );
    }

    std::vector<RimEclipseCase*> sourceCases;
    for ( auto& mockCase : sourceMockCases )
    {
        sourceCases.push_back( mockCase.resultCase.get() );
    }

    MockCase destinationMockCase = createMockCase( { RiaDefines::PhaseType::OIL_PHASE } );

    RimStatisticsConfig statisticsConfig;

    RigActiveCellInfo* unionOfMatrixActiveCells = destinationMockCase.caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* unionOfFractureActiveCells =
        destinationMockCase.caseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    RimEclipseStatisticsCaseEvaluator evaluator( sourceCases,
                                                 { 0, 1 },
                                                 statisticsConfig,
                                                 destinationMockCase.caseData.p(),
                                                 unionOfMatrixActiveCells,
                                                 unionOfFractureActiveCells,
                                                 false );

    QList<RimEclipseStatisticsCaseEvaluator::ResSpec> resultSpecification;
    resultSpecification.push_back( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                               RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                               RiaResultNames::riPorvSoil() ) );

    evaluator.evaluateForResults( resultSpecification, nullptr );

    RigCaseCellResultsData* destinationCellResults = destinationMockCase.caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const RigEclipseResultAddress meanAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil() + "_MEAN" );
    ASSERT_TRUE( destinationCellResults->hasResultEntry( meanAddr ) );

    const size_t cellCount = destinationCellResults->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( cellCount, 0u );

    for ( size_t timeStepIndex = 0; timeStepIndex < 2; timeStepIndex++ )
    {
        const auto& meanValues = destinationCellResults->cellScalarResults( meanAddr, timeStepIndex );
        ASSERT_EQ( meanValues.size(), cellCount ) << "No statistics computed for time step " << timeStepIndex;

        for ( size_t i = 0; i < cellCount; i++ )
        {
            EXPECT_NE( meanValues[i], HUGE_VAL ) << "Undefined riPORV*SOIL statistics value at cell " << i << ", time step " << timeStepIndex;
        }
    }
}
