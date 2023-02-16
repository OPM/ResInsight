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

#include "RifEclipseKeywordContent.h"
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
    // Remove DISABLED_ from the name of the test to run this performance test
    // Intended to be executed locally

    std::string filename       = "c:/temp/GRID.GRDECL";
    size_t      iterationCount = 10;

    {
        auto aggregatedStart = std::chrono::high_resolution_clock::now();

        for ( size_t i = 0; i < iterationCount; i++ )
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

        for ( size_t i = 0; i < iterationCount; i++ )
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
TEST( RifEclipseTextFileReader, ReadKeywordsWithComment )
{
    QString faceTexts = "SPECGRID                   -- Generated : simulator name\n"
                        "124 134 41 1 F /\n"
                        "COORDSYS                   -- Generated : simulator name\n"
                        " 1 41 INCOMP /\n ";

    auto stdString = faceTexts.toStdString();

    size_t bytesRead = 0;
    auto   objects   = RifEclipseTextFileReader::readKeywordAndValues( stdString, 0, bytesRead );
    {
        std::string toMatch( "SPECGRID" );
        EXPECT_STREQ( objects.first.data(), toMatch.data() );

        auto values = objects.second;
        EXPECT_FLOAT_EQ( values[0], 124.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadKeywordsAndValuesFromFile )
{
    QString     qtFileName = QString( "%1/RifEclipseTextFileParser/GRID.GRDECL" ).arg( TEST_DATA_DIR );
    std::string filename   = qtFileName.toStdString();

    auto objects = RifEclipseTextFileReader::readKeywordAndValues( filename );

    EXPECT_EQ( size_t( 7 ), objects.size() );
    for ( const auto& obj : objects )
    {
        std::cout << obj.keyword << "\n";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadLine_EmptyString )
{
    std::string fileContent;
    size_t      offset    = 0;
    size_t      bytesRead = 0;
    auto        line      = RifEclipseTextFileReader::readLine( fileContent, offset, bytesRead );

    EXPECT_EQ( size_t( 0 ), bytesRead );
    EXPECT_EQ( size_t( 0 ), line.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadLine_TooLargeOffset )
{
    std::string fileContent = "f";
    size_t      offset      = 10;
    size_t      bytesRead   = 0;
    auto        line        = RifEclipseTextFileReader::readLine( fileContent, offset, bytesRead );

    EXPECT_EQ( size_t( 0 ), bytesRead );
    EXPECT_EQ( size_t( 0 ), line.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadLine_SingleLineNoLineBreak )
{
    std::string fileContent = "file content";
    size_t      offset      = 0;
    size_t      bytesRead   = 0;
    auto        line        = RifEclipseTextFileReader::readLine( fileContent, offset, bytesRead );

    EXPECT_EQ( size_t( 12 ), bytesRead );
    EXPECT_EQ( size_t( 12 ), line.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadLine_TwoLinesWithLineBreak )
{
    std::string fileContent = "file content\n next Line";
    size_t      offset      = 0;
    size_t      bytesRead   = 0;
    auto        line        = RifEclipseTextFileReader::readLine( fileContent, offset, bytesRead );

    // bytesRead includes line break
    EXPECT_EQ( size_t( 13 ), bytesRead );
    EXPECT_EQ( size_t( 12 ), line.size() );

    auto secondLine = RifEclipseTextFileReader::readLine( fileContent, offset + bytesRead, bytesRead );
    EXPECT_EQ( size_t( 10 ), bytesRead );
    EXPECT_EQ( size_t( 9 ), secondLine.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ValueMultiplier )
{
    std::string fileContent = "ZCORN\n"
                              "2*2.21 0.5 3*12345.12\n"
                              "/\n";

    auto keywordDataItems = RifEclipseTextFileReader::parseStringData( fileContent );

    EXPECT_EQ( size_t( 1 ), keywordDataItems.size() );

    auto firstKeyword = keywordDataItems.front();

    EXPECT_EQ( size_t( 6 ), firstKeyword.values.size() );
    EXPECT_FLOAT_EQ( 2.21f, firstKeyword.values[0] );
    EXPECT_FLOAT_EQ( 2.21f, firstKeyword.values[1] );
    EXPECT_FLOAT_EQ( 0.5f, firstKeyword.values[2] );
    EXPECT_FLOAT_EQ( 12345.12f, firstKeyword.values[3] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, KeywordsWithoutValue )
{
    std::string fileContent = "NOECHO\n"
                              "SWAT\n"
                              "1.0 2.0 3.0\n"
                              "/\n"
                              "SOIL\n"
                              "6.0 7.0 8.0 9.0\n"
                              "/\n"
                              "G4\n"
                              "1.0 2.0 3.0\n"
                              "0.123/ -- 1213 values in comment 123.123 \n";

    auto keywordDataItems = RifEclipseTextFileReader::parseStringData( fileContent );

    EXPECT_EQ( 4u, keywordDataItems.size() );

    auto noEchoKeyword = keywordDataItems[0];
    EXPECT_EQ( 0u, noEchoKeyword.values.size() );

    auto swatKeyword = keywordDataItems[1];
    EXPECT_EQ( 3u, swatKeyword.values.size() );

    auto soilKeyword = keywordDataItems[2];
    EXPECT_EQ( 4u, soilKeyword.values.size() );

    EXPECT_FLOAT_EQ( 1.0f, swatKeyword.values[0] );
    EXPECT_FLOAT_EQ( 6.0f, soilKeyword.values[0] );

    auto g4Keyword = keywordDataItems[3];
    EXPECT_EQ( 4u, g4Keyword.values.size() );
    EXPECT_FLOAT_EQ( 0.123f, g4Keyword.values[3] );
}
