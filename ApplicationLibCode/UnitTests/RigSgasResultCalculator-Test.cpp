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

void addSwatResult( RigCaseCellResultsData* cellResults, const std::vector<std::vector<double>>& valuesForEachTimeStep )
{
    RigEclipseResultAddress swatAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() );
    cellResults->createResultEntry( swatAddress, false );

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;
    for ( int i = 0; i < static_cast<int>( valuesForEachTimeStep.size() ); i++ )
    {
        timeStepInfos.push_back( RigEclipseTimeStepInfo( QDateTime(), i, i ) );
    }
    cellResults->setTimeStepInfos( swatAddress, timeStepInfos );

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( swatAddress );
    timeStepValues->resize( valuesForEachTimeStep.size() );
    for ( size_t i = 0; i < valuesForEachTimeStep.size(); i++ )
    {
        ( *timeStepValues )[i] = valuesForEachTimeStep[i];
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// SGAS is not reported by the simulator for two-phase gas/water models, compute SGAS = 1 - SWAT
//--------------------------------------------------------------------------------------------------
TEST( RigSgasResultCalculatorTest, ComputeSgasForTwoPhaseGasWater )
{
    MockCase mockCase = createMockCase( { RiaDefines::PhaseType::GAS_PHASE, RiaDefines::PhaseType::WATER_PHASE } );

    RigCaseCellResultsData* cellResults = mockCase.caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( cellResults, nullptr );

    const size_t cellCount = cellResults->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( cellCount, 0u );

    const std::vector<std::vector<double>> swatValues = { std::vector<double>( cellCount, 0.2 ), std::vector<double>( cellCount, 0.7 ) };
    addSwatResult( cellResults, swatValues );

    cellResults->createPlaceholderResultEntries();

    const RigEclipseResultAddress sgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
    ASSERT_TRUE( cellResults->hasResultEntry( sgasAddress ) );
    ASSERT_TRUE( cellResults->ensureKnownResultLoaded( sgasAddress ) );

    for ( size_t timeStepIndex = 0; timeStepIndex < swatValues.size(); timeStepIndex++ )
    {
        const auto& sgasValues = cellResults->cellScalarResults( sgasAddress, timeStepIndex );
        ASSERT_EQ( sgasValues.size(), cellCount );

        for ( size_t i = 0; i < cellCount; i++ )
        {
            EXPECT_DOUBLE_EQ( sgasValues[i], 1.0 - swatValues[timeStepIndex][i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// SGAS must not be derived from SWAT when the oil phase is present
//--------------------------------------------------------------------------------------------------
TEST( RigSgasResultCalculatorTest, NoComputedSgasWhenOilPhaseIsPresent )
{
    MockCase mockCase =
        createMockCase( { RiaDefines::PhaseType::OIL_PHASE, RiaDefines::PhaseType::GAS_PHASE, RiaDefines::PhaseType::WATER_PHASE } );

    RigCaseCellResultsData* cellResults = mockCase.caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( cellResults, nullptr );

    const size_t cellCount = cellResults->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( cellCount, 0u );

    addSwatResult( cellResults, { std::vector<double>( cellCount, 0.2 ) } );

    cellResults->createPlaceholderResultEntries();

    const RigEclipseResultAddress sgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
    EXPECT_FALSE( cellResults->hasResultEntry( sgasAddress ) );
}
