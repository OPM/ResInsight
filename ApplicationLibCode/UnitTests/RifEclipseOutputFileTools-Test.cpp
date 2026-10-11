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

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseReportKeywords.h"

#include "RiaDefines.h"
#include "RiaStringEncodingTools.h"

#include "RigActiveCellInfo.h"

#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/fortio.h>

#include <QDir>
#include <QTemporaryDir>

//--------------------------------------------------------------------------------------------------
/// A grid with no active matrix cells must not cause division by zero
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseOutputFileTools, ValidKeywordsNoActiveMatrixCells )
{
    std::vector<RifEclipseKeywordValueCount> keywordItemCounts;
    keywordItemCounts.emplace_back( "PRESSURE", 50, RifEclipseKeywordValueCount::KeywordDataType::FLOAT );

    RigActiveCellInfo matrixActiveCellInfo;
    matrixActiveCellInfo.setReservoirCellCount( 100 );

    RigActiveCellInfo fractureActiveCellInfo;
    fractureActiveCellInfo.setReservoirCellCount( 100 );
    fractureActiveCellInfo.setGridCount( 1 );
    fractureActiveCellInfo.setGridActiveCellCounts( 0, 50 );
    fractureActiveCellInfo.computeDerivedData();

    EXPECT_EQ( size_t( 0 ), matrixActiveCellInfo.reservoirActiveCellCount() );

    auto matrixKeywords = RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                                    &matrixActiveCellInfo,
                                                                                    &fractureActiveCellInfo,
                                                                                    RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    1 );
    EXPECT_TRUE( matrixKeywords.empty() );

    auto fractureKeywords = RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                                      &matrixActiveCellInfo,
                                                                                      &fractureActiveCellInfo,
                                                                                      RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                      1 );
    EXPECT_EQ( size_t( 1 ), fractureKeywords.size() );
}

//--------------------------------------------------------------------------------------------------
/// Empty active cell info for both porosity models must not cause division by zero
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseOutputFileTools, ValidKeywordsNoActiveCells )
{
    std::vector<RifEclipseKeywordValueCount> keywordItemCounts;
    keywordItemCounts.emplace_back( "PRESSURE", 100, RifEclipseKeywordValueCount::KeywordDataType::FLOAT );

    RigActiveCellInfo matrixActiveCellInfo;
    RigActiveCellInfo fractureActiveCellInfo;

    EXPECT_TRUE( RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                           &matrixActiveCellInfo,
                                                                           &fractureActiveCellInfo,
                                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                           1 )
                     .empty() );

    EXPECT_TRUE( RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                           &matrixActiveCellInfo,
                                                                           &fractureActiveCellInfo,
                                                                           RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                           1 )
                     .empty() );
}

//--------------------------------------------------------------------------------------------------
/// A restart file without INTEHEAD keywords produces no time steps, and must not cause division by zero
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseOutputFileTools, TimeStepsFileWithoutIntehead )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString fileName = QDir( tempDir.path() ).filePath( "CASE.UNRST" );
    {
        fortio_type* fortio = fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( fileName ).data(), false, ECL_ENDIAN_FLIP );
        ASSERT_TRUE( fortio != nullptr );

        ecl_kw_type* kw = ecl_kw_alloc( SEQNUM_KW, 1, ECL_INT );
        ecl_kw_iset_int( kw, 0, 0 );
        ecl_kw_fwrite( kw, fortio );
        ecl_kw_free( kw );

        fortio_fclose( fortio );
    }

    ecl_file_type* eclFile = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( fileName ).data(), ECL_FILE_CLOSE_STREAM );
    ASSERT_TRUE( eclFile != nullptr );

    std::vector<QDateTime> timeSteps;
    std::vector<double>    daysSinceSimulationStart;
    size_t                 perTimeStepHeaderKeywordCount = 1;
    RifEclipseOutputFileTools::timeSteps( eclFile, &timeSteps, &daysSinceSimulationStart, &perTimeStepHeaderKeywordCount );

    EXPECT_TRUE( timeSteps.empty() );
    EXPECT_TRUE( daysSinceSimulationStart.empty() );
    EXPECT_EQ( size_t( 1 ), perTimeStepHeaderKeywordCount );

    ecl_file_close( eclFile );
}
