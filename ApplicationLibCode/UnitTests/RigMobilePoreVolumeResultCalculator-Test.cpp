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
#include "RiaTestDataDirectory.h"

#include "RifReaderEclipseOutput.h"

#include "RimEclipseResultCase.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"

#include <QDir>
#include <QFile>

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

void fillStaticResult( RigCaseCellResultsData* resultsData, const RigEclipseResultAddress& resultAddress, size_t valueCount, double value )
{
    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, resultAddress.resultName(), false, valueCount );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    ASSERT_NE( resultVector, nullptr );
    ASSERT_EQ( resultVector->size(), valueCount );

    std::fill( resultVector->begin(), resultVector->end(), value );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Verify MOBILE_PORE_VOLUME handles SWCR size mismatch safely
//--------------------------------------------------------------------------------------------------
TEST( RigMobilePoreVolumeResultCalculatorTest, MobilePvIgnoresMismatchedSwcrSize )
{
    LoadedCase caseData = loadBruggeCase();

    RigCaseCellResultsData* resultsData = caseData.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount = resultsData->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( activeCellCount, 10u );

    const RigEclipseResultAddress porvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv() );
    fillStaticResult( resultsData, porvAddress, activeCellCount, 2.0 );

    // Deliberately shorter than PORV to reproduce mismatch that previously crashed.
    const RigEclipseResultAddress swcrAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "SWCR" );
    fillStaticResult( resultsData, swcrAddress, activeCellCount / 4, 0.25 );

    const RigEclipseResultAddress mobilePvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName() );
    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName(), false, 0 );

    ASSERT_TRUE( resultsData->ensureKnownResultLoaded( mobilePvAddress ) );

    const auto& mobilePvValues = resultsData->cellScalarResults( mobilePvAddress, 0 );
    ASSERT_EQ( mobilePvValues.size(), activeCellCount );

    // SWCR must be ignored on size mismatch, so fallback should copy PORV values.
    for ( double value : mobilePvValues )
    {
        EXPECT_DOUBLE_EQ( value, 2.0 );
    }
}

//--------------------------------------------------------------------------------------------------
/// Verify MOBILE_PORE_VOLUME handles MULTPV size mismatch safely
//--------------------------------------------------------------------------------------------------
TEST( RigMobilePoreVolumeResultCalculatorTest, MobilePvIgnoresMismatchedMultpvSize )
{
    LoadedCase caseData = loadBruggeCase();

    RigCaseCellResultsData* resultsData = caseData.eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );
    ASSERT_NE( resultsData->activeCellInfo(), nullptr );

    const size_t activeCellCount = resultsData->activeCellInfo()->reservoirActiveCellCount();
    ASSERT_GT( activeCellCount, 10u );

    const RigEclipseResultAddress porvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv() );
    fillStaticResult( resultsData, porvAddress, activeCellCount, 3.0 );

    // Deliberately shorter than PORV to reproduce mismatch that previously crashed.
    const RigEclipseResultAddress multpvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTPV" );
    fillStaticResult( resultsData, multpvAddress, activeCellCount / 5, 0.5 );

    const RigEclipseResultAddress mobilePvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName() );
    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName(), false, 0 );

    ASSERT_TRUE( resultsData->ensureKnownResultLoaded( mobilePvAddress ) );

    const auto& mobilePvValues = resultsData->cellScalarResults( mobilePvAddress, 0 );
    ASSERT_EQ( mobilePvValues.size(), activeCellCount );

    // MULTPV must be ignored on size mismatch, so fallback should copy PORV values.
    for ( double value : mobilePvValues )
    {
        EXPECT_DOUBLE_EQ( value, 3.0 );
    }
}
