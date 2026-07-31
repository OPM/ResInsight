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
#include "RiaTestDataDirectory.h"

#include "RifReaderEclipseOutput.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCrossPlotDataExtractor.h"
#include "RigEclipseResultAddress.h"

#include <QDir>
#include <QFile>

#include <map>
#include <memory>

namespace
{
struct LoadedCase
{
    std::unique_ptr<RimEclipseResultCase> resultCase;
    cvf::ref<RigEclipseCaseData>          eclipseCase;
};

LoadedCase loadBruggeCase()
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    EXPECT_TRUE( subFolderExists ) << "Could not find test model directory";

    QString filePath = baseFolder.absoluteFilePath( "BRUGGE_0000.EGRID" );
    EXPECT_TRUE( QFile::exists( filePath ) ) << "BRUGGE test model file does not exist: " << filePath.toStdString();

    LoadedCase loaded;
    loaded.resultCase.reset( new RimEclipseResultCase );
    loaded.eclipseCase = new RigEclipseCaseData( loaded.resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( filePath, loaded.eclipseCase.p() );
    EXPECT_TRUE( success ) << "Could not load BRUGGE test model";

    loaded.resultCase->setReservoirData( loaded.eclipseCase.p() );

    return loaded;
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
    LoadedCase caseData = loadBruggeCase();

    RigCaseCellResultsData* resultsData = caseData.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount = resultsData->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( activeCellCount, 10u );

    fillStaticResult( resultsData, "XVALUES", activeCellCount, 1.0 );
    fillStaticResult( resultsData, "YVALUES", activeCellCount, 2.0 );

    RimEclipseResultDefinition xAddress;
    RimEclipseResultDefinition yAddress;
    RimEclipseResultDefinition groupAddress;
    setupResultDefinition( &xAddress, caseData.resultCase.get(), "XVALUES" );
    setupResultDefinition( &yAddress, caseData.resultCase.get(), "YVALUES" );

    // An empty visibility array is what a view without a main grid leaves behind.
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    timeStepCellVisibilityMap[0] = cvf::UByteArray();

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract( caseData.eclipseCase.p(),
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
    LoadedCase caseData = loadBruggeCase();

    RigCaseCellResultsData* resultsData = caseData.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount    = resultsData->activeCellInfo()->reservoirActiveCellCount();
    const size_t reservoirCellCount = resultsData->activeCellInfo()->reservoirCellCount();
    ASSERT_GT( activeCellCount, 10u );

    fillStaticResult( resultsData, "XVALUES", activeCellCount, 3.0 );
    fillStaticResult( resultsData, "YVALUES", activeCellCount, 4.0 );

    RimEclipseResultDefinition xAddress;
    RimEclipseResultDefinition yAddress;
    RimEclipseResultDefinition groupAddress;
    setupResultDefinition( &xAddress, caseData.resultCase.get(), "XVALUES" );
    setupResultDefinition( &yAddress, caseData.resultCase.get(), "YVALUES" );

    // Visibility computed for a smaller grid, as when the cell filter view refers to another case.
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    cvf::UByteArray&               cellVisibility = timeStepCellVisibilityMap[0];
    cellVisibility.resize( reservoirCellCount / 2 );
    cellVisibility.setAll( 1 );

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract( caseData.eclipseCase.p(),
                                                                                  0,
                                                                                  xAddress,
                                                                                  yAddress,
                                                                                  RigGridCrossPlotCurveGrouping::NO_GROUPING,
                                                                                  groupAddress,
                                                                                  timeStepCellVisibilityMap );

    EXPECT_EQ( result.xValues.size(), activeCellCount );
    EXPECT_EQ( result.yValues.size(), activeCellCount );
}
