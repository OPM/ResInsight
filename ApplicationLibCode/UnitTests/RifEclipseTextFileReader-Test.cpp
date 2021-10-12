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

#include "mio/single_include/mio/mio.hpp"

#include <QString>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadKeywordsAndValues )
{
    QString     qtFileName = QString( "%1/RifEclipseTextFileParser/GRID.GRDECL" ).arg( TEST_DATA_DIR );
    std::string fileName   = qtFileName.toStdString();

    //    "e:/gitroot-ceesol/ResInsight-regression-test/ModelData/TestCase_Ascii_no_map_axis/geocell.grdecl";
    // filename = "d:/scratch/R5_H25_C1_aug_grid.grdecl";

    std::error_code error;

    mio::mmap_sink   rw_mmap    = mio::make_mmap_sink( fileName, 0, mio::map_entire_file, error );
    std::string_view stringData = rw_mmap.data();

    size_t offset    = 0;
    size_t bytesRead = 0;

    while ( offset < stringData.size() )
    {
        RifEclipseTextFileReader reader;
        auto [keyword, values] = reader.readKeywordAndValues( stringData, offset, bytesRead );
        offset += bytesRead;

        std::cout << keyword << " : " << values.size() << "\n";

        if ( !values.empty() )
        {
            std::cout << values.front() << " " << values.back();
            std::cout << "\n\n";
        }
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
