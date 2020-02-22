////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaStringEncodingTools.h"
#include "RiaTestDataDirectory.h"
#include "RifActiveCellsReader.h"

#include "ert/ecl/ecl_file.hpp"
#include "ert/ecl/ecl_grid.hpp"

#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifActiveCellsReaderTest, BasicTest10k )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );

    ecl_grid_type* mainEclGrid = nullptr;

    {
        QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
        QString filePath = baseFolder.absoluteFilePath( filename );

        mainEclGrid = ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( filePath ).data() );
    }

    std::vector<std::vector<int>> activeCellsFromActnum;
    std::vector<std::vector<int>> activeCellsFromPorv;
    {
        QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
        QString filePath = baseFolder.absoluteFilePath( filename );

        ecl_file_type* gridFile =
            ecl_file_open( RiaStringEncodingTools::toNativeEncoded( filePath ).data(), ECL_FILE_CLOSE_STREAM );
        activeCellsFromActnum = RifActiveCellsReader::activeCellsFromActnumKeyword( gridFile );
        EXPECT_EQ( (size_t)2, activeCellsFromActnum.size() );
        ecl_file_close( gridFile );
    }

    {
        QString filename( "TEST10K_FLT_LGR_NNC.INIT" );
        QString filePath = baseFolder.absoluteFilePath( filename );

        ecl_file_type* initFile =
            ecl_file_open( RiaStringEncodingTools::toNativeEncoded( filePath ).data(), ECL_FILE_CLOSE_STREAM );

        activeCellsFromPorv = RifActiveCellsReader::activeCellsFromPorvKeyword( initFile, false );
        EXPECT_EQ( 2, (int)activeCellsFromPorv.size() );

        ecl_file_close( initFile );
    }

    for ( size_t gridIndex = 0; gridIndex < activeCellsFromActnum.size(); gridIndex++ )
    {
        for ( size_t valueIndex = 0; valueIndex < activeCellsFromActnum[gridIndex].size(); valueIndex++ )
        {
            auto actnumValue = activeCellsFromActnum[gridIndex][valueIndex];
            auto porvValue   = activeCellsFromPorv[gridIndex][valueIndex];
            if ( actnumValue > 0 )
            {
                EXPECT_TRUE( porvValue > 0 );
            }
            else
            {
                EXPECT_EQ( 0, porvValue );
            }
        }
    }

    std::vector<int> expectedActiveCellCountPerGrid;
    expectedActiveCellCountPerGrid.push_back( 8517 );
    expectedActiveCellCountPerGrid.push_back( 2608 );

    for ( int gridIndex = 0; gridIndex < static_cast<int>( activeCellsFromActnum.size() ); gridIndex++ )
    {
        ecl_grid_type* currentGrid = nullptr;
        if ( gridIndex == 0 )
        {
            currentGrid = mainEclGrid;
        }
        else
        {
            currentGrid = ecl_grid_iget_lgr( mainEclGrid, gridIndex - 1 );
        }

        auto activeCellsForGrid = activeCellsFromActnum[gridIndex];
        if ( ecl_grid_get_global_size( currentGrid ) == static_cast<int>( activeCellsForGrid.size() ) )
        {
            int expectedCellCount = expectedActiveCellCountPerGrid[gridIndex];
            EXPECT_EQ( expectedCellCount, ecl_grid_get_nactive( currentGrid ) );

            int* actnum_values = activeCellsForGrid.data();

            ecl_grid_reset_actnum( currentGrid, actnum_values );
            EXPECT_EQ( expectedCellCount, ecl_grid_get_nactive( currentGrid ) );
        }
    }

    ecl_grid_free( mainEclGrid );
}
