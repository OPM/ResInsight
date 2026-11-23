/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigEclipseCaseData.h"

#include "ert/ecl/ecl_kw_magic.h"
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>
#include <ert/ecl_well/well_info.hpp>

#include "RiaStringEncodingTools.h"
#include "RiaTestDataDirectory.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderOpmCommon.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"
#include "RimEclipseResultCase.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <limits>
#include <memory>
#include <set>
#include <vector>

using namespace RiaDefines;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, BasicTest10k )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    {
        RigCaseCellResultsData* cellData = reservoir->results( PorosityModelType::MATRIX_MODEL );

        QStringList staticResults = cellData->resultNames( ResultCatType::STATIC_NATIVE );
        EXPECT_EQ( 0, staticResults.size() );
        // qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = cellData->resultNames( ResultCatType::DYNAMIC_NATIVE );
        EXPECT_EQ( 0, dynamicResults.size() );
        // qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>( cellData->maxTimeStepCount() );
        EXPECT_EQ( 0, numTimeSteps );
    }

    {
        cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
        bool                             result             = readerInterfaceEcl->open( filePath, reservoir.p() );
        EXPECT_TRUE( result );
        int numTimeSteps = static_cast<int>( readerInterfaceEcl->allTimeSteps().size() );
        EXPECT_EQ( 9, numTimeSteps );
    }

    {
        RigCaseCellResultsData* cellData = reservoir->results( PorosityModelType::MATRIX_MODEL );

        QStringList staticResults = cellData->resultNames( ResultCatType::STATIC_NATIVE );
        EXPECT_EQ( 44, staticResults.size() );
        // qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = cellData->resultNames( ResultCatType::DYNAMIC_NATIVE );
        EXPECT_EQ( 23, dynamicResults.size() );
        // qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>( cellData->maxTimeStepCount() );
        EXPECT_EQ( 9, numTimeSteps );
    }
}

//--------------------------------------------------------------------------------------------------
/// Regression guard for issue #14227. The depth-related geometry results
/// (DEPTH/DX/DY/DZ/TOPS/BOTTOM) are computed independently for the matrix and fracture porosity
/// models. They are derived purely from the shared grid geometry, so for a dual-porosity model a
/// cell that is active in both models must receive identical values in each model.
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, DualPorosityDepthResults )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "dualporo-testcase" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "DUALPORO-WELL-COMPLETION-DEFAULT.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             result             = readerInterfaceEcl->open( filePath, reservoir.p() );
    EXPECT_TRUE( result );

    RigCaseCellResultsData* matrixResults   = reservoir->results( PorosityModelType::MATRIX_MODEL );
    RigCaseCellResultsData* fractureResults = reservoir->results( PorosityModelType::FRACTURE_MODEL );
    ASSERT_TRUE( matrixResults != nullptr );
    ASSERT_TRUE( fractureResults != nullptr );

    RigActiveCellInfo* matrixActiveInfo   = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveInfo = reservoir->activeCellInfo( PorosityModelType::FRACTURE_MODEL );
    ASSERT_TRUE( matrixActiveInfo != nullptr );
    ASSERT_TRUE( fractureActiveInfo != nullptr );

    // This model must actually exercise both porosity models, otherwise the test is meaningless.
    EXPECT_GT( matrixActiveInfo->reservoirActiveCellCount(), size_t( 0 ) );
    EXPECT_GT( fractureActiveInfo->reservoirActiveCellCount(), size_t( 0 ) );

    // Compute depth-related geometry results for both porosity models.
    reservoir->computeDepthRelatedResults();

    const QStringList propertyNames = { "DEPTH", "DX", "DY", "DZ", "TOPS", "BOTTOM" };

    const double undefinedValue = std::numeric_limits<double>::max();
    const size_t totalCellCount = reservoir->mainGrid()->totalCellCount();

    for ( const QString& propertyName : propertyNames )
    {
        RigEclipseResultAddress resAddr( ResultCatType::STATIC_NATIVE, propertyName );

        EXPECT_TRUE( matrixResults->hasResultEntry( resAddr ) );
        EXPECT_TRUE( fractureResults->hasResultEntry( resAddr ) );

        const std::vector<double>& matrixValues   = matrixResults->cellScalarResults( resAddr, 0 );
        const std::vector<double>& fractureValues = fractureResults->cellScalarResults( resAddr, 0 );

        EXPECT_EQ( matrixValues.size(), matrixActiveInfo->reservoirActiveCellCount() );
        EXPECT_EQ( fractureValues.size(), fractureActiveInfo->reservoirActiveCellCount() );

        // For every reservoir cell active in both models, the geometry-derived value must match.
        size_t comparedCellCount = 0;
        for ( size_t cellIdx = 0; cellIdx < totalCellCount; cellIdx++ )
        {
            size_t matrixResultIdx   = matrixActiveInfo->cellResultIndex( ReservoirCellIndex( cellIdx ) ).value();
            size_t fractureResultIdx = fractureActiveInfo->cellResultIndex( ReservoirCellIndex( cellIdx ) ).value();
            if ( matrixResultIdx == cvf::UNDEFINED_SIZE_T || fractureResultIdx == cvf::UNDEFINED_SIZE_T ) continue;
            if ( matrixResultIdx >= matrixValues.size() || fractureResultIdx >= fractureValues.size() ) continue;

            const double matrixValue   = matrixValues[matrixResultIdx];
            const double fractureValue = fractureValues[fractureResultIdx];

            EXPECT_NE( matrixValue, undefinedValue );
            EXPECT_NE( fractureValue, undefinedValue );
            EXPECT_DOUBLE_EQ( matrixValue, fractureValue );
            comparedCellCount++;
        }

        EXPECT_GT( comparedCellCount, size_t( 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Intersect writes ACTNUM with all cells active and no NNCs, and defines the active cells by PORV in the INIT file.
/// Matrix and fracture then have different active cells, and the OPM reader must map them like resdata.
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, DualPorosityDifferentMatrixAndFractureActiveCells )
{
    QDir baseFolder( TEST_MODEL_DIR );
    baseFolder.cd( "dualporo-testcase" );
    const QString sourceBaseName = baseFolder.absoluteFilePath( "DUALPORO-WELL-COMPLETION-DEFAULT" );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    const QString baseName = tempDir.filePath( "DUALPORO" );

    // Copy the EGRID without NNCs, as OPM EGrid::load_nnc_data() requires equal matrix and fracture active cell counts
    {
        ecl_file_type* gridFile =
            ecl_file_open( RiaStringEncodingTools::toNativeEncoded( sourceBaseName + ".EGRID" ).data(), ECL_FILE_CLOSE_STREAM );
        ASSERT_TRUE( gridFile );

        fortio_type* fortio =
            fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( baseName + ".EGRID" ).data(), false, ECL_ENDIAN_FLIP );
        ASSERT_TRUE( fortio );

        for ( int kwIdx = 0; kwIdx < ecl_file_get_size( gridFile ); kwIdx++ )
        {
            ecl_kw_type*      kw     = ecl_file_iget_kw( gridFile, kwIdx );
            const std::string header = ecl_kw_get_header( kw );
            if ( header == NNCHEAD_KW || header == NNC1_KW || header == NNC2_KW ) continue;

            ecl_kw_fwrite( kw, fortio );
        }

        fortio_fclose( fortio );
        ecl_file_close( gridFile );
    }

    // Fracture is active in the bottom layer only, and one bottom layer cell is fracture only
    const int cellCount            = 125;
    const int firstFractureCell    = 100;
    const int fractureOnlyCell     = 110;
    auto      isMatrixActiveCell   = [&]( int cellIdx ) { return cellIdx != fractureOnlyCell; };
    auto      isFractureActiveCell = [&]( int cellIdx ) { return cellIdx >= firstFractureCell; };

    std::vector<int> keptValueIndices;
    for ( int i = 0; i < 2 * cellCount; i++ )
    {
        const int cellIdx = i % cellCount;
        if ( i < cellCount ? isMatrixActiveCell( cellIdx ) : isFractureActiveCell( cellIdx ) ) keptValueIndices.push_back( i );
    }

    // Rewrite the INIT and UNRST files with PORV zero for inactive cells, and remove these cells from all active cell arrays
    auto rewriteFile = [&]( const QString& extension ) -> bool
    {
        ecl_file_type* initFile =
            ecl_file_open( RiaStringEncodingTools::toNativeEncoded( sourceBaseName + extension ).data(), ECL_FILE_CLOSE_STREAM );
        if ( !initFile ) return false;

        fortio_type* fortio =
            fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( baseName + extension ).data(), false, ECL_ENDIAN_FLIP );
        if ( !fortio ) return false;

        for ( int kwIdx = 0; kwIdx < ecl_file_get_size( initFile ); kwIdx++ )
        {
            ecl_kw_type* kw     = ecl_file_iget_kw( initFile, kwIdx );
            const char*  header = ecl_kw_get_header( kw );

            if ( std::string( header ) == INTEHEAD_KW )
            {
                ecl_kw_type* copy             = ecl_kw_alloc_copy( kw );
                const int    activeValueCount = static_cast<int>( keptValueIndices.size() );
                ecl_kw_iset( copy, INTEHEAD_NACTIVE_INDEX, &activeValueCount );
                ecl_kw_fwrite( copy, fortio );
                ecl_kw_free( copy );
            }
            else if ( std::string( header ) == PORV_KW )
            {
                ecl_kw_type* copy = ecl_kw_alloc_copy( kw );
                const float  zero = 0.0f;
                size_t       kept = 0;
                for ( int i = 0; i < ecl_kw_get_size( copy ); i++ )
                {
                    if ( kept < keptValueIndices.size() && keptValueIndices[kept] == i )
                        kept++;
                    else
                        ecl_kw_iset( copy, i, &zero );
                }
                ecl_kw_fwrite( copy, fortio );
                ecl_kw_free( copy );
            }
            else if ( ecl_kw_get_size( kw ) == 2 * cellCount )
            {
                ecl_kw_type* filtered = ecl_kw_alloc( header, static_cast<int>( keptValueIndices.size() ), ecl_kw_get_data_type( kw ) );
                for ( size_t i = 0; i < keptValueIndices.size(); i++ )
                {
                    ecl_kw_iset( filtered, static_cast<int>( i ), ecl_kw_iget_ptr( kw, keptValueIndices[i] ) );
                }
                ecl_kw_fwrite( filtered, fortio );
                ecl_kw_free( filtered );
            }
            else
            {
                ecl_kw_fwrite( kw, fortio );
            }
        }

        fortio_fclose( fortio );
        ecl_file_close( initFile );
        return true;
    };

    ASSERT_TRUE( rewriteFile( ".INIT" ) );
    ASSERT_TRUE( rewriteFile( ".UNRST" ) );

    cvf::ref<RifReaderInterface>          readers[2] = { new RifReaderEclipseOutput, new RifReaderOpmCommon };
    std::unique_ptr<RimEclipseResultCase> resultCases[2];
    cvf::ref<RigEclipseCaseData>          caseData[2];
    for ( int i = 0; i < 2; i++ )
    {
        resultCases[i] = std::make_unique<RimEclipseResultCase>();
        caseData[i]    = new RigEclipseCaseData( resultCases[i].get() );
        ASSERT_TRUE( readers[i]->open( baseName + ".EGRID", caseData[i].p() ) );
        ASSERT_TRUE( caseData[i]->mainGrid()->isDualPorosity() );
    }

    for ( auto porosityModel : { PorosityModelType::MATRIX_MODEL, PorosityModelType::FRACTURE_MODEL } )
    {
        const size_t expectedActiveCount = porosityModel == PorosityModelType::MATRIX_MODEL ? cellCount - 1 : cellCount - firstFractureCell;

        const RigActiveCellInfo* resdataActiveInfo = caseData[0]->activeCellInfo( porosityModel );
        const RigActiveCellInfo* opmActiveInfo     = caseData[1]->activeCellInfo( porosityModel );
        EXPECT_EQ( expectedActiveCount, resdataActiveInfo->reservoirActiveCellCount() );
        EXPECT_EQ( expectedActiveCount, opmActiveInfo->reservoirActiveCellCount() );

        for ( int cellIdx = 0; cellIdx < cellCount; cellIdx++ )
        {
            const ReservoirCellIndex reservoirCellIndex( static_cast<size_t>( cellIdx ) );
            EXPECT_EQ( resdataActiveInfo->cellResultIndex( reservoirCellIndex ), opmActiveInfo->cellResultIndex( reservoirCellIndex ) )
                << "cell " << cellIdx;
        }

        for ( const QString& resultName : { "PORV", "PERMX", "PORO" } )
        {
            std::vector<double> resdataValues;
            std::vector<double> opmValues;
            EXPECT_TRUE( readers[0]->staticResult( resultName, porosityModel, &resdataValues ) );
            EXPECT_TRUE( readers[1]->staticResult( resultName, porosityModel, &opmValues ) );
            EXPECT_EQ( expectedActiveCount, resdataValues.size() );
            EXPECT_EQ( resdataValues, opmValues ) << resultName.toStdString();
        }

        // Dynamic results are read through the fracture result index, which differs from the matrix index
        for ( const QString& resultName : { "PRESSURE", "SWAT" } )
        {
            std::vector<double> resdataValues;
            std::vector<double> opmValues;
            EXPECT_TRUE( readers[0]->dynamicResult( resultName, porosityModel, 0, &resdataValues ) );
            EXPECT_TRUE( readers[1]->dynamicResult( resultName, porosityModel, 0, &opmValues ) );
            EXPECT_EQ( expectedActiveCount, resdataValues.size() );
            EXPECT_EQ( resdataValues, opmValues ) << resultName.toStdString();
        }
    }
}

TEST( RigReservoirTest, OpenGridFileWithoutDimensKeyword )
{
    // Importing a GRID file without the DIMENS keyword threw an uncaught std::out_of_range from the
    // resdata keyword lookup and terminated the application. The import must fail gracefully instead.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString gridFileName = QDir( tempDir.path() ).filePath( "TEST.GRID" );
    {
        std::vector<int> data( 10, 0 );
        ecl_kw_fwrite_param( RiaStringEncodingTools::toNativeEncoded( gridFileName ).data(),
                             false,
                             "INTEHEAD",
                             ECL_INT,
                             (int)data.size(),
                             data.data() );
    }
    ASSERT_TRUE( QFile::exists( gridFileName ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    EXPECT_FALSE( readerInterfaceEcl->open( gridFileName, reservoir.p() ) );
}

TEST( RigReservoirTest, OpenEgridWithInvalidGridHeader )
{
    // ecl_grid_alloc_EGRID__() returns NULL for invalid grid dimensions. The caller dereferenced the NULL main
    // grid and crashed instead of returning NULL.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString gridFileName = QDir( tempDir.path() ).filePath( "TEST.EGRID" );
    {
        std::vector<int>   intData( 100, 0 );
        std::vector<float> floatData( 1, 0.0f );

        fortio_type* fortio = fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( gridFileName ).data(), false, ECL_ENDIAN_FLIP );
        ASSERT_TRUE( fortio );

        auto writeKeyword = [fortio]( const char* name, int size, ecl_data_type type, const void* data )
        {
            ecl_kw_type* kw = ecl_kw_alloc_new( name, size, type, data );
            ecl_kw_fwrite( kw, fortio );
            ecl_kw_free( kw );
        };

        // GRIDHEAD with zero grid dimensions is rejected
        writeKeyword( "FILEHEAD", (int)intData.size(), ECL_INT, intData.data() );
        writeKeyword( "GRIDHEAD", (int)intData.size(), ECL_INT, intData.data() );
        writeKeyword( "COORD", (int)floatData.size(), ECL_FLOAT, floatData.data() );
        writeKeyword( "ZCORN", (int)floatData.size(), ECL_FLOAT, floatData.data() );

        fortio_fclose( fortio );
    }

    EXPECT_EQ( nullptr, ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( gridFileName ).data() ) );
}

TEST( RigReservoirTest, ReadWellsFromRestartFileWithoutIntehead )
{
    // ecl_rsthead_alloc() returns NULL when INTEHEAD is missing or unreadable (truncated file). The well reader
    // dereferenced the NULL header and crashed.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString restartFileName = QDir( tempDir.path() ).filePath( "TEST.UNRST" );
    {
        std::vector<int> data( 1, 1 );
        ecl_kw_fwrite_param( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(),
                             false,
                             "SEQNUM",
                             ECL_INT,
                             (int)data.size(),
                             data.data() );
    }

    ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), 0 );
    ASSERT_TRUE( ecl_file );

    well_info_type* wellInfo = well_info_alloc( { "GLOBAL" } );
    well_info_add_UNRST_wells( wellInfo, ecl_file, false );
    EXPECT_EQ( 0, well_info_get_num_wells( wellInfo ) );

    well_info_free( wellInfo );
    ecl_file_close( ecl_file );
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Write a restart file with one well, optionally corrupt the data record of a keyword, and return
/// the number of wells read. Returns -1 if the file could not be created.
//--------------------------------------------------------------------------------------------------
int wellCountFromRestartFile( const QString& restartFileName, const char* corruptKeyword )
{
    {
        fortio_type* fortio = fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), false, ECL_ENDIAN_FLIP );
        if ( !fortio ) return -1;

        auto writeKeyword = [fortio]( const char* name, int size, ecl_data_type type, const void* data )
        {
            ecl_kw_type* kw = ecl_kw_alloc_new( name, size, type, data );
            ecl_kw_fwrite( kw, fortio );
            ecl_kw_free( kw );
        };

        std::vector<int> seqnum( 1, 1 );
        writeKeyword( SEQNUM_KW, (int)seqnum.size(), ECL_INT, seqnum.data() );

        std::vector<int> intehead( 411, 0 );
        intehead[INTEHEAD_DAY_INDEX]    = 1;
        intehead[INTEHEAD_MONTH_INDEX]  = 1;
        intehead[INTEHEAD_YEAR_INDEX]   = 2000;
        intehead[INTEHEAD_NWELLS_INDEX] = 1;
        intehead[INTEHEAD_NIWELZ_INDEX] = 155;
        intehead[INTEHEAD_NXWELZ_INDEX] = 130;
        intehead[INTEHEAD_NZWELZ_INDEX] = 3;
        writeKeyword( INTEHEAD_KW, (int)intehead.size(), ECL_INT, intehead.data() );

        std::vector<int> iwel( 155, 0 );
        writeKeyword( IWEL_KW, (int)iwel.size(), ECL_INT, iwel.data() );

        std::vector<double> xwel( 130, 0.0 );
        writeKeyword( XWEL_KW, (int)xwel.size(), ECL_DOUBLE, xwel.data() );

        ecl_kw_type* zwel = ecl_kw_alloc( ZWEL_KW, 3, ECL_CHAR );
        ecl_kw_iset_string8( zwel, 0, "WELL" );
        ecl_kw_fwrite( zwel, fortio );
        ecl_kw_free( zwel );

        std::vector<int> iseg( 22, 0 );
        writeKeyword( ISEG_KW, (int)iseg.size(), ECL_INT, iseg.data() );

        fortio_fclose( fortio );
    }

    if ( corruptKeyword )
    {
        // Corrupt the record marker in front of the keyword data
        QFile file( restartFileName );
        if ( !file.open( QIODevice::ReadWrite ) ) return -1;

        const QByteArray content   = file.readAll();
        const qsizetype  headerPos = content.indexOf( QByteArray( corruptKeyword ).leftJustified( 8, ' ' ) );
        if ( headerPos < 0 || !file.seek( headerPos + 8 + 4 + 4 + 4 ) ) return -1;

        const char invalidMarker[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
        if ( file.write( invalidMarker, 4 ) != 4 ) return -1;
    }

    ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), 0 );
    if ( !ecl_file ) return -1;

    well_info_type* wellInfo = well_info_alloc( { "GLOBAL" } );
    well_info_add_UNRST_wells( wellInfo, ecl_file, false );
    const int wellCount = well_info_get_num_wells( wellInfo );

    well_info_free( wellInfo );
    ecl_file_close( ecl_file );

    return wellCount;
}
} // namespace

TEST( RigReservoirTest, ReadWellsFromRestartFileWithUnreadableKeywords )
{
    // Keywords are indexed when the file is opened, but their data is not read until requested. If the data block is
    // corrupt, the keyword lookup returns NULL. The well reader dereferenced a NULL IWEL keyword and crashed. A well
    // with unreadable keywords is skipped.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    EXPECT_EQ( 1, wellCountFromRestartFile( QDir( tempDir.path() ).filePath( "VALID.UNRST" ), nullptr ) );
    EXPECT_EQ( 0, wellCountFromRestartFile( QDir( tempDir.path() ).filePath( "IWEL.UNRST" ), IWEL_KW ) );
    EXPECT_EQ( 0, wellCountFromRestartFile( QDir( tempDir.path() ).filePath( "ISEG.UNRST" ), ISEG_KW ) );
    EXPECT_EQ( 0, wellCountFromRestartFile( QDir( tempDir.path() ).filePath( "XWEL.UNRST" ), XWEL_KW ) );
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Write a restart file with one well per nzwelz words of ZWEL, and return the well names read.
//--------------------------------------------------------------------------------------------------
std::vector<std::string> wellNamesFromRestartFile( const QString& restartFileName, int nzwelz, const std::vector<std::string>& zwelWords )
{
    const int nwells = (int)zwelWords.size() / nzwelz;
    {
        fortio_type* fortio = fortio_open_writer( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), false, ECL_ENDIAN_FLIP );
        if ( !fortio ) return {};

        auto writeKeyword = [fortio]( const char* name, int size, ecl_data_type type, const void* data )
        {
            ecl_kw_type* kw = ecl_kw_alloc_new( name, size, type, data );
            ecl_kw_fwrite( kw, fortio );
            ecl_kw_free( kw );
        };

        std::vector<int> seqnum( 1, 1 );
        writeKeyword( SEQNUM_KW, (int)seqnum.size(), ECL_INT, seqnum.data() );

        const int        niwelz = 155;
        const int        nxwelz = 130;
        std::vector<int> intehead( 411, 0 );
        intehead[INTEHEAD_DAY_INDEX]    = 1;
        intehead[INTEHEAD_MONTH_INDEX]  = 1;
        intehead[INTEHEAD_YEAR_INDEX]   = 2000;
        intehead[INTEHEAD_NWELLS_INDEX] = nwells;
        intehead[INTEHEAD_NIWELZ_INDEX] = niwelz;
        intehead[INTEHEAD_NXWELZ_INDEX] = nxwelz;
        intehead[INTEHEAD_NZWELZ_INDEX] = nzwelz;
        writeKeyword( INTEHEAD_KW, (int)intehead.size(), ECL_INT, intehead.data() );

        std::vector<int> iwel( niwelz * nwells, 0 );
        writeKeyword( IWEL_KW, (int)iwel.size(), ECL_INT, iwel.data() );

        std::vector<double> xwel( nxwelz * nwells, 0.0 );
        writeKeyword( XWEL_KW, (int)xwel.size(), ECL_DOUBLE, xwel.data() );

        ecl_kw_type* zwel = ecl_kw_alloc( ZWEL_KW, (int)zwelWords.size(), ECL_CHAR );
        for ( size_t i = 0; i < zwelWords.size(); i++ )
            ecl_kw_iset_string8( zwel, (int)i, zwelWords[i].c_str() );
        ecl_kw_fwrite( zwel, fortio );
        ecl_kw_free( zwel );

        fortio_fclose( fortio );
    }

    ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), 0 );
    if ( !ecl_file ) return {};

    well_info_type* wellInfo = well_info_alloc( { "GLOBAL" } );
    well_info_add_UNRST_wells( wellInfo, ecl_file, false );

    std::vector<std::string> wellNames;
    for ( int i = 0; i < well_info_get_num_wells( wellInfo ); i++ )
        wellNames.push_back( well_info_iget_well_name( wellInfo, i ) );

    well_info_free( wellInfo );
    ecl_file_close( ecl_file );

    return wellNames;
}
} // namespace

TEST( RigReservoirTest, ReadWellNamesLongerThanEightCharacters )
{
    // IX stores well names longer than 8 characters across consecutive ZWEL words. Only the first word was read, and
    // wells sharing the first 8 characters were merged into one well.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    const std::vector<std::string> ixWords = { "PRODUCER", "A01", "PRODUCER", "A02", "SHORT", "" };
    const std::vector<std::string> ixNames = { "PRODUCERA01", "PRODUCERA02", "SHORT" };
    EXPECT_EQ( ixNames, wellNamesFromRestartFile( QDir( tempDir.path() ).filePath( "IX.UNRST" ), 2, ixWords ) );

    // Eclipse 100 and OPM Flow leave the second word blank and store the ACTIONX name in the third word
    const std::vector<std::string> eclipseWords = { "WELL0001", "", "ACTION1", "PROD", "", "ACTION2" };
    const std::vector<std::string> eclipseNames = { "WELL0001", "PROD" };
    EXPECT_EQ( eclipseNames, wellNamesFromRestartFile( QDir( tempDir.path() ).filePath( "E100.UNRST" ), 3, eclipseWords ) );

    // Eclipse 300 uses one word per well
    const std::vector<std::string> e300Words = { "WELL0001", "WELL0002" };
    const std::vector<std::string> e300Names = { "WELL0001", "WELL0002" };
    EXPECT_EQ( e300Names, wellNamesFromRestartFile( QDir( tempDir.path() ).filePath( "E300.UNRST" ), 1, e300Words ) );
}

TEST( RigReservoirTest, WellNamesFromRestartFilesMatchFirstZwelWord )
{
    // Joining ZWEL words for long well names must not change the well names read from Eclipse restart files
    const QStringList restartFiles = { "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.UNRST",
                                       "dualporo-testcase/DUALPORO-WELL-COMPLETION-DEFAULT.UNRST",
                                       "msw-export/opm-simulation-reference/BASE_MODEL_1.UNRST" };

    for ( const QString& restartFile : restartFiles )
    {
        const QString fileName = QDir( TEST_MODEL_DIR ).filePath( restartFile );
        if ( !QFile::exists( fileName ) ) continue;

        ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( fileName ).data(), 0 );
        ASSERT_TRUE( ecl_file );

        std::set<std::string> firstWordNames;
        int                   nzwelz = 1;
        int                   nwells = 0;
        for ( int i = 0; i < ecl_file_get_size( ecl_file ); i++ )
        {
            const std::string header = ecl_file_iget_header( ecl_file, i );
            if ( header == INTEHEAD_KW )
            {
                const ecl_kw_type* intehead = ecl_file_iget_kw( ecl_file, i );
                nzwelz                      = ecl_kw_iget_int( intehead, INTEHEAD_NZWELZ_INDEX );
                nwells                      = ecl_kw_iget_int( intehead, INTEHEAD_NWELLS_INDEX );
            }
            else if ( header == ZWEL_KW )
            {
                const ecl_kw_type* zwel = ecl_file_iget_kw( ecl_file, i );
                for ( int w = 0; w < nwells; w++ )
                    firstWordNames.insert( QString( ecl_kw_iget_char_ptr( zwel, w * nzwelz ) ).trimmed().toStdString() );
            }
        }

        well_info_type* wellInfo = well_info_alloc( { "GLOBAL" } );
        well_info_add_UNRST_wells( wellInfo, ecl_file, false );

        std::set<std::string> wellNames;
        for ( int i = 0; i < well_info_get_num_wells( wellInfo ); i++ )
            wellNames.insert( well_info_iget_well_name( wellInfo, i ) );

        well_info_free( wellInfo );
        ecl_file_close( ecl_file );

        EXPECT_FALSE( wellNames.empty() ) << restartFile.toStdString();
        EXPECT_EQ( firstWordNames, wellNames ) << restartFile.toStdString();
    }
}

TEST( RigReservoirTest, TimeStepsFromFileWithoutIntehead )
{
    // A restart file without INTEHEAD gives no time steps, and computing the header keyword count per time step
    // divided by zero.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString restartFileName = QDir( tempDir.path() ).filePath( "TEST.UNRST" );
    {
        std::vector<int> data( 1, 1 );
        ecl_kw_fwrite_param( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(),
                             false,
                             "SEQNUM",
                             ECL_INT,
                             (int)data.size(),
                             data.data() );
    }

    ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( restartFileName ).data(), 0 );
    ASSERT_TRUE( ecl_file );

    std::vector<QDateTime> timeSteps;
    std::vector<double>    daysSinceSimulationStart;
    size_t                 perTimeStepHeaderKeywordCount = 0;
    RifEclipseOutputFileTools::timeSteps( ecl_file, &timeSteps, &daysSinceSimulationStart, &perTimeStepHeaderKeywordCount );

    EXPECT_TRUE( timeSteps.empty() );
    EXPECT_EQ( size_t( 0 ), perTimeStepHeaderKeywordCount );

    ecl_file_close( ecl_file );
}

TEST( RigReservoirTest, BasicTest10kRestart )
{
    RifEclipseUnifiedRestartFileAccess unrstAccess;

    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.UNRST" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QStringList filenames;
    filenames << filePath;
    unrstAccess.setRestartFiles( filenames );

    auto keywordValueCounts = unrstAccess.keywordValueCounts();
    EXPECT_EQ( (size_t)83, keywordValueCounts.size() );

    auto reportNums = unrstAccess.reportNumbers();
    EXPECT_EQ( (size_t)9, reportNums.size() );
}

TEST( RigReservoirTest, BasicTest10k_NativeECL )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    ecl_grid_type* grid = ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( filePath ).data() );
    EXPECT_TRUE( grid );

    QString subDir( "RifReaderEclipseOutput" );
    QDir    dataDir( TEST_DATA_DIR );
    dataDir.mkdir( subDir );
    dataDir.cd( subDir );
    QString outFilePath = dataDir.absoluteFilePath( "TEST10K_FLT_LGR_NNC_OUT.GRDECL" );

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
    FILE* filePtr = fopen( RiaStringEncodingTools::toNativeEncoded( outFilePath ).data(), "w" );
    EXPECT_TRUE( filePtr != nullptr );
    ecl_grid_fprintf_grdecl( grid, filePtr );
    fclose( filePtr );
    EXPECT_TRUE( QFile::exists( outFilePath ) );

#ifdef _WIN32
    QString expectedMd5( QByteArray::fromHex( "e993b85140568f13f4c3849604700a0f" ) );
#else
    QString expectedMd5( QByteArray::fromHex( "274e44fe51299c1f9ca6645c384e237d" ) );
#endif
    QByteArray generatedMd5 = RifEclipseOutputFileTools::md5sum( outFilePath );

    // Enable to produce text string so expectedMd5 can be updated
    // Qt4 doesn't take a parameter for toHex()
    // qDebug() << expectedMd5.toLatin1().toHex(0) << "     " << generatedMd5.toHex(0);
    EXPECT_TRUE( generatedMd5 == expectedMd5 );

#ifdef _MSC_VER
#pragma warning( pop )
#endif
    ecl_grid_free( grid );
}

TEST( RigReservoirTest, Test10k_ReadThenWriteToECL )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    resultCase->setGridFileName( filePath );
    resultCase->importGridAndResultMetaData( false );

    QString subDir( "RifReaderEclipseOutput" );
    QDir    dataDir( TEST_DATA_DIR );
    dataDir.mkdir( subDir );
    dataDir.cd( subDir );
    QString outFilePath = dataDir.absoluteFilePath( "TEST10K_FLT_LGR_NNC_OUT_FROM_RES.GRDECL" );

    /* bool worked = RifReaderEclipseOutput::saveEclipseGrid(outFilePath, resultCase->eclipseCaseData());
    EXPECT_TRUE(worked);
    EXPECT_TRUE(QFile::exists(outFilePath));

    QString dataFilePath = dataDir.absoluteFilePath("TEST10K_FLT_LGR_NNC_OUT_FROM_RES.VARS");

    QStringList allStaticResults =
    resultCase->eclipseCaseData()->results(MATRIX_MODEL)->resultNames(RiaDefines::STATIC_NATIVE);

    std::vector<QString> keywords;
    for (QString keyword : allStaticResults)
    {
        keywords.push_back(keyword);
    }

    worked = RifReaderEclipseOutput::saveEclipseResults(dataFilePath, resultCase->eclipseCaseData(), keywords);
    EXPECT_TRUE(worked);
    EXPECT_TRUE(QFile::exists(dataFilePath)); */
}
#if 0



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, DISABLED_BasicTest)
{

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigCaseData> reservoir = new RigCaseData;

    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.EGRID");

    RifReaderSettings readerSettings;
    readerInterfaceEcl->setReaderSetting(&readerSettings);

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    {
//         QStringList staticResults = readerInterfaceEcl->staticResults();
//         EXPECT_EQ(42, staticResults.size());
//         qDebug() << "Static results\n" << staticResults;
// 
//         QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
//         EXPECT_EQ(23, dynamicResults.size());
//         qDebug() << "Dynamic results\n" << dynamicResults;
// 
//         int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
//         EXPECT_EQ(9, numTimeSteps);
// 
//         QStringList timeStepText = readerInterfaceEcl->timeStepText();
//         EXPECT_EQ(numTimeSteps, timeStepText.size());
//         qDebug() << "Time step texts\n" << timeStepText;
    }


    readerInterfaceEcl->close();

    {
//         QStringList staticResults = readerInterfaceEcl->staticResults();
//         EXPECT_EQ(0, staticResults.size());
// 
//         QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
//         EXPECT_EQ(0, dynamicResults.size());
// 
//         int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
//         EXPECT_EQ(0, numTimeSteps);
// 
//         QStringList timeStepText = readerInterfaceEcl->timeStepText();
//         EXPECT_EQ(numTimeSteps, timeStepText.size());
    }

}



TEST(RigReservoirTest, DISABLED_FileOutputToolsTest)
{
    cvf::DebugTimer timer("test");


//    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
//    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
//    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.UNRST");
    QString filename("c:/tmp/troll_MSW/T07-4A-W2012-16-F3.UNRST");
    

    ecl_file_type* ertFile = ecl_file_open(filename.toAscii().data(), ECL_FILE_CLOSE_STREAM);
    EXPECT_TRUE(ertFile);


    QStringList keywords;
    std::vector<size_t> keywordDataItemCounts;
    RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(ertFile, &keywords, &keywordDataItemCounts);

    EXPECT_TRUE(keywords.size() == keywordDataItemCounts.size());

    qDebug() << "Keyword - Number of data items";
    for (int i = 0; i < keywords.size(); i++)
    {
        QString paddedKeyword = QString("%1").arg(keywords[i], 8);
        qDebug() << paddedKeyword << " - " << keywordDataItemCounts[i];
    }

    ecl_file_close(ertFile);
    ertFile = nullptr;

    timer.reportTime();
    //qDebug() << timer.lapt;
}


TEST(RigReservoirTest, UnifiedTestFile)
{
    //QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.UNRST");

    {
        cvf::ref<RifEclipseUnifiedRestartFileAccess> restartFile = new RifEclipseUnifiedRestartFileAccess();

        QStringList fileNameList;
        fileNameList << filename;
        restartFile->setRestartFiles(fileNameList);
        restartFile->open();

        QStringList resultNames;
        std::vector<size_t> resultDataItemCounts;
        restartFile->resultNames(&resultNames, &resultDataItemCounts);

        qDebug() << "Result names\n";
        for (int i = 0; i < resultNames.size(); i++)
        {
            qDebug() << resultNames[i] << "\t" << resultDataItemCounts[i];
        }

        std::vector<QDateTime> tsteps = restartFile->timeSteps();

        qDebug() << "Time step texts\n";
        for (int i = 0; i < tsteps.size(); i++)
        {
            qDebug() << tsteps[i].toString();
        }

        /*
        std::vector<double> resultValues;
        size_t timeStep = 0;
        restartFile->results(resultNames[0], timeStep, &resultValues);

        size_t i;
        for (i = 0; i < 500; i++)
        {
            qDebug() <<  resultValues[i];
        }
        */
    }

}



void buildResultInfoString(RigReservoir* reservoir, RiaDefines::PorosityModelType porosityModel, RiaDefines::ResultCatType resultType)
{
    RigCaseCellResultsData* matrixResults = reservoir->results(porosityModel);
    {
        QStringList resultNames = matrixResults->resultNames(resultType);

        for (size_t i = 0 ; i < resultNames.size(); i++)
        {
            std::vector<double> values;
            size_t scalarResultIndex = matrixResults->findOrLoadScalarResult(resultType, resultNames[i]);
            EXPECT_TRUE(scalarResultIndex != cvf::UNDEFINED_SIZE_T);

            QString resultText = QString("%1").arg(resultNames[i], 8);
            std::vector< std::vector<double> > & resultValues = matrixResults->cellScalarResults(scalarResultIndex);
            for (size_t timeStepIdx = 0; timeStepIdx < matrixResults->timeStepCount(scalarResultIndex); timeStepIdx++)
            {
                size_t resultValueCount = resultValues[timeStepIdx].size();
                resultText += QString(" %1").arg(resultValueCount);
            }

            qDebug() << resultText;
        }
        qDebug() << "Number of items : " << resultNames.size();
    }
}

TEST(RigReservoirTest, DualPorosityTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    QString filename("d:/Models/Statoil/DualProperty/DUALPORO.EGRID");
    //QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");


    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);


    qDebug() << "\n\n" << 
        "Matrix porosities, DYNAMIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::MATRIX_MODEL, RiaDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Matrix porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::MATRIX_MODEL, RiaDefines::STATIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, DYNAMIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::FRACTURE_MODEL, RiaDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::FRACTURE_MODEL, RiaDefines::STATIC_NATIVE);
}














//#include "RifEclipseUnifiedRestartFileAccess.h"

/*
TEST(RigReservoirTest, UnifiedTestFile)
{

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
#endif

    size_t numActiveCells = 11125;
    size_t numGrids = 2;

    {
        cvf::ref<RifEclipseUnifiedRestartFileAccess> restartFile = new RifEclipseUnifiedRestartFileAccess(numGrids, numActiveCells);

        QStringList fileNameList;
        fileNameList << filename;
        restartFile->open(fileNameList);

        QStringList resultNames = restartFile->resultNames();
        qDebug() << "Result names\n" << resultNames;

        QStringList timeStepText = restartFile->timeStepsText();
        qDebug() << "Time step texts\n" << timeStepText;

        std::vector<double> resultValues;
        size_t timeStep = 0;
        restartFile->results(resultNames[0], timeStep, &resultValues);

        size_t i;
        for (i = 0; i < 500; i++)
        {
            qDebug() <<  resultValues[i];
        }

    }


}
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, BasicTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("TEST10K_FLT_LGR_NNC.EGRID");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
#endif

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    {
        QStringList staticResults = readerInterfaceEcl->staticResults();
        EXPECT_EQ(42, staticResults.size());
        qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
        EXPECT_EQ(23, dynamicResults.size());
        qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
        EXPECT_EQ(9, numTimeSteps);

        QStringList timeStepText = readerInterfaceEcl->timeStepText();
        EXPECT_EQ(numTimeSteps, timeStepText.size());
        qDebug() << "Time step texts\n" << timeStepText;
    }


    readerInterfaceEcl->close();

    {
        QStringList staticResults = readerInterfaceEcl->staticResults();
        EXPECT_EQ(0, staticResults.size());

        QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
        EXPECT_EQ(0, dynamicResults.size());

        int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
        EXPECT_EQ(0, numTimeSteps);

        QStringList timeStepText = readerInterfaceEcl->timeStepText();
        EXPECT_EQ(numTimeSteps, timeStepText.size());
    }

}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, WellTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("TEST10K_FLT_LGR_NNC.EGRID");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
#endif

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    cvf::UByteArray* mainGridWellCells = reservoir->wellCellsInGrid(0);
    EXPECT_TRUE(mainGridWellCells->size() == reservoir->mainGrid()->cellCount());
}

#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RigReservoirTest, WellTest )
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigEclipseCaseData>     reservoir          = new RigEclipseCaseData( nullptr );

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename( "d:/Models/Statoil/soursim/PKMUNK_NOV_TEST_SS.GRID" );
    QString sourSim( "d:/Models/Statoil/soursim/result.sourres.00001" );
#else
    QString filename( "/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID" );
    QString sourSim( "d:/Models/Statoil/soursim/result.sourres" );
#endif

    bool result = readerInterfaceEcl->open( filename, reservoir.p() );
    EXPECT_TRUE( result );

    readerInterfaceEcl->setHdf5FileName( sourSim );
}

//--------------------------------------------------------------------------------------------------
/// Removing duplicate nodes must not change computed faults or NNC geometry
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, RemoveDuplicateNodesPreservesFaultsAndNncs )
{
    QDir baseFolder( TEST_MODEL_DIR );
    baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    QString filePath = baseFolder.absoluteFilePath( "TEST10K_FLT_LGR_NNC.EGRID" );

    auto loadCase = [&]( bool removeDuplicates, RimEclipseResultCase* rimCase )
    {
        cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( rimCase );
        cvf::ref<RifReaderOpmCommon> reader   = new RifReaderOpmCommon;
        EXPECT_TRUE( reader->open( filePath, caseData.p() ) );

        auto mainGrid = caseData->mainGrid();
        if ( removeDuplicates ) EXPECT_GT( mainGrid->removeDuplicateNodes(), 0u );
        mainGrid->computeCachedData();
        mainGrid->calculateFaults( caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ) );
        mainGrid->nncData()->ensureAllConnectionDataIsProcessed();
        return caseData;
    };

    RimEclipseResultCase rimCaseA;
    RimEclipseResultCase rimCaseB;
    auto                 original     = loadCase( false, &rimCaseA );
    auto                 deduplicated = loadCase( true, &rimCaseB );

    const auto& faultsA = original->mainGrid()->faults();
    const auto& faultsB = deduplicated->mainGrid()->faults();
    ASSERT_EQ( faultsA.size(), faultsB.size() );
    for ( size_t i = 0; i < faultsA.size(); i++ )
    {
        EXPECT_EQ( faultsA[i]->faultFaces().size(), faultsB[i]->faultFaces().size() );
    }

    const auto& nncsA = original->mainGrid()->nncData()->allConnections();
    const auto& nncsB = deduplicated->mainGrid()->nncData()->allConnections();
    ASSERT_EQ( nncsA.size(), nncsB.size() );

    // Consecutive coinciding vertices can be collapsed when nodes are shared, so compare without them
    auto withoutRepeats = []( std::vector<cvf::Vec3f> polygon )
    {
        polygon.erase( std::unique( polygon.begin(), polygon.end() ), polygon.end() );
        return polygon;
    };

    for ( size_t i = 0; i < nncsA.size(); i++ )
    {
        EXPECT_EQ( nncsA[i].c1GlobIdx(), nncsB[i].c1GlobIdx() );
        EXPECT_EQ( nncsA[i].c2GlobIdx(), nncsB[i].c2GlobIdx() );
        EXPECT_EQ( nncsA[i].face(), nncsB[i].face() );
        EXPECT_TRUE( withoutRepeats( nncsA[i].polygon() ) == withoutRepeats( nncsB[i].polygon() ) );
    }
}
