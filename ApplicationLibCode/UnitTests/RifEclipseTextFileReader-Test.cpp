/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RiaTestDataDirectory.h"
#include "RifEclipseTextFileReader.h"

#include <QString>
#include <chrono>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, DISABLED_ReadKeywordsAndValuesPerformanceTest )
{
    // std::string filename       = "d:/scratchMsj/2021-10-13-norne-1M-single-grdecl/GRID.GRDECL";
    // std::string filename = "e:/project/2021-10-13-norne-1M-single-grdecl/GRID.GRDECL";
    std::string filename       = "c:/temp/GRID.GRDECL";
    size_t      iterationCount = 10;

    {
        auto aggregatedStart = std::chrono::high_resolution_clock::now();

        for ( auto i = 0; i < iterationCount; i++ )
        {
            auto start = std::chrono::high_resolution_clock::now();

            auto objects = RifEclipseTextFileReader::readKeywordAndValuesMemoryMappedFile( filename );

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "RifEclipseTextFileReader MM : " << std::setw( 9 ) << diff.count() << " s\n";
        }

        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - aggregatedStart;
        std::cout << "RifEclipseTextFileReader MM [" << iterationCount << " runs] : " << std::setw( 9 ) << diff.count()
                  << " s\n";
    }

    {
        auto aggregatedStart = std::chrono::high_resolution_clock::now();

        for ( auto i = 0; i < iterationCount; i++ )
        {
            auto start = std::chrono::high_resolution_clock::now();

            auto objects = RifEclipseTextFileReader::readKeywordAndValuesFile( filename );

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "RifEclipseTextFileReader File : " << std::setw( 9 ) << diff.count() << " s\n";
        }

        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - aggregatedStart;
        std::cout << "RifEclipseTextFileReader File [" << iterationCount << " runs] : " << std::setw( 9 )
                  << diff.count() << " s\n";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadKeywordsAndValuesFromFile )
{
    QString qtFileName = QString( "%1/RifEclipseTextFileParser/GRID.GRDECL" ).arg( TEST_DATA_DIR );
    //    "e:/gitroot-ceesol/ResInsight-regression-test/ModelData/TestCase_Ascii_no_map_axis/geocell.grdecl";
    // filename = "d:/scratch/R5_H25_C1_aug_grid.grdecl";

    std::string filename = qtFileName.toStdString();

    RifEclipseTextFileReader reader;
    auto                     objects = reader.readKeywordAndValues( filename );

    EXPECT_EQ( size_t( 7 ), objects.size() );
    for ( const auto& obj : objects )
    {
        std::cout << obj.keyword << "\n";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadLine )
{
    {
        // Empty string

        std::string              fileContent;
        RifEclipseTextFileReader reader;
        size_t                   offset    = 0;
        size_t                   bytesRead = 0;
        auto                     line      = reader.readLine( fileContent, offset, bytesRead );

        EXPECT_EQ( size_t( 0 ), bytesRead );
        EXPECT_EQ( size_t( 0 ), line.size() );
    }

    {
        // Offset too large
        std::string              fileContent = "f";
        RifEclipseTextFileReader reader;
        size_t                   offset    = 10;
        size_t                   bytesRead = 0;
        auto                     line      = reader.readLine( fileContent, offset, bytesRead );

        EXPECT_EQ( size_t( 0 ), bytesRead );
        EXPECT_EQ( size_t( 0 ), line.size() );
    }

    {
        // One line, no line break
        std::string              fileContent = "file content";
        RifEclipseTextFileReader reader;
        size_t                   offset    = 0;
        size_t                   bytesRead = 0;
        auto                     line      = reader.readLine( fileContent, offset, bytesRead );

        EXPECT_EQ( size_t( 12 ), bytesRead );
        EXPECT_EQ( size_t( 12 ), line.size() );
    }

    {
        // two lines with line break
        std::string              fileContent = "file content\n next Line";
        RifEclipseTextFileReader reader;
        size_t                   offset    = 0;
        size_t                   bytesRead = 0;
        auto                     line      = reader.readLine( fileContent, offset, bytesRead );

        // bytesRead includes line break
        EXPECT_EQ( size_t( 13 ), bytesRead );
        EXPECT_EQ( size_t( 12 ), line.size() );

        auto secondLine = reader.readLine( fileContent, offset + bytesRead, bytesRead );
        EXPECT_EQ( size_t( 10 ), bytesRead );
        EXPECT_EQ( size_t( 9 ), secondLine.size() );
    }
}
