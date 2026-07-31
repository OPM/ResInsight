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

#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCrossPlotDataExtractor.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#include "cvfVector3.h"

#include <map>
#include <memory>

namespace
{
struct MockCase
{
    std::unique_ptr<RimEclipseResultCase> resultCase;
    cvf::ref<RigEclipseCaseData>          eclipseCase;
};

//--------------------------------------------------------------------------------------------------
/// Build a regular ni x nj x nk box grid in memory (no file, no view)
//--------------------------------------------------------------------------------------------------
MockCase buildBoxGridCase( int ni, int nj, int nk )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( ni, nj, nk ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( ni, nj, -nk ) );

    MockCase mockCase;
    mockCase.resultCase.reset( new RimEclipseResultCase );
    mockCase.eclipseCase = new RigEclipseCaseData( mockCase.resultCase.get() );

    builder.createGridsAndCells( mockCase.eclipseCase.p() );
    mockCase.eclipseCase->mainGrid()->computeCachedData();

    mockCase.resultCase->setReservoirData( mockCase.eclipseCase.p() );

    return mockCase;
}

void fillStaticResult( RigCaseCellResultsData* resultsData, const QString& resultName, size_t valueCount, double value )
{
    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, resultName, false, valueCount );

    const RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, resultName );
    std::vector<double>*          resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    ASSERT_NE( resultVector, nullptr );
    ASSERT_EQ( resultVector->size(), valueCount );

    std::fill( resultVector->begin(), resultVector->end(), value );
}

void setupResultDefinition( RimEclipseResultDefinition* resultDefinition, RimEclipseResultCase* resultCase, const QString& resultName )
{
    resultDefinition->setEclipseCase( resultCase );
    resultDefinition->setPorosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL );
    resultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    resultDefinition->setResultVariable( resultName );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A cell visibility array not matching the reservoir cell count must be ignored.
///
/// RimEclipseView::calculateCurrentTotalCellVisibility() returns without resizing when the view has
/// no main grid, leaving an empty array in the visibility map. Indexing that array crashed.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCrossPlotDataExtractorTest, ExtractIgnoresEmptyCellVisibility )
{
    MockCase mockCase = buildBoxGridCase( 10, 10, 5 );

    RigCaseCellResultsData* resultsData = mockCase.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount = resultsData->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_EQ( activeCellCount, 500u );

    fillStaticResult( resultsData, "XVALUES", activeCellCount, 1.0 );
    fillStaticResult( resultsData, "YVALUES", activeCellCount, 2.0 );

    RimEclipseResultDefinition xAddress;
    RimEclipseResultDefinition yAddress;
    RimEclipseResultDefinition groupAddress;
    setupResultDefinition( &xAddress, mockCase.resultCase.get(), "XVALUES" );
    setupResultDefinition( &yAddress, mockCase.resultCase.get(), "YVALUES" );

    // An empty visibility array is what a view without a main grid leaves behind.
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    timeStepCellVisibilityMap[0] = cvf::UByteArray();

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract( mockCase.eclipseCase.p(),
                                                                                  0,
                                                                                  xAddress,
                                                                                  yAddress,
                                                                                  RigGridCrossPlotCurveGrouping::NO_GROUPING,
                                                                                  groupAddress,
                                                                                  timeStepCellVisibilityMap );

    // The filter must be ignored, so all active cells are included.
    EXPECT_EQ( result.xValues.size(), activeCellCount );
    EXPECT_EQ( result.yValues.size(), activeCellCount );
}

//--------------------------------------------------------------------------------------------------
/// A cell visibility array from a smaller grid must be ignored instead of read out of bounds.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCrossPlotDataExtractorTest, ExtractIgnoresTooShortCellVisibility )
{
    MockCase mockCase = buildBoxGridCase( 10, 10, 5 );

    RigCaseCellResultsData* resultsData = mockCase.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount    = resultsData->activeCellInfo()->reservoirActiveCellCount();
    const size_t reservoirCellCount = resultsData->activeCellInfo()->reservoirCellCount();
    ASSERT_EQ( activeCellCount, 500u );

    fillStaticResult( resultsData, "XVALUES", activeCellCount, 3.0 );
    fillStaticResult( resultsData, "YVALUES", activeCellCount, 4.0 );

    RimEclipseResultDefinition xAddress;
    RimEclipseResultDefinition yAddress;
    RimEclipseResultDefinition groupAddress;
    setupResultDefinition( &xAddress, mockCase.resultCase.get(), "XVALUES" );
    setupResultDefinition( &yAddress, mockCase.resultCase.get(), "YVALUES" );

    // Visibility computed for a smaller grid, as when the cell filter view refers to another case.
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    cvf::UByteArray&               cellVisibility = timeStepCellVisibilityMap[0];
    cellVisibility.resize( reservoirCellCount / 2 );
    cellVisibility.setAll( 1 );

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract( mockCase.eclipseCase.p(),
                                                                                  0,
                                                                                  xAddress,
                                                                                  yAddress,
                                                                                  RigGridCrossPlotCurveGrouping::NO_GROUPING,
                                                                                  groupAddress,
                                                                                  timeStepCellVisibilityMap );

    EXPECT_EQ( result.xValues.size(), activeCellCount );
    EXPECT_EQ( result.yValues.size(), activeCellCount );
}
