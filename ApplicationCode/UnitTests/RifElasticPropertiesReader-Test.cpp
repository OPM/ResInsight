/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-  Equinor ASA
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

#include "RifElasticPropertiesReader.h"
#include "RifFileParseTools.h"

#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadCorrectInputFile )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.4\n"
            << "Norne,Not,Sand,0.10,19,0.27,2099,0.3,0.4,0.5,0.2,0.5,0.55\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );

    std::vector<RifElasticProperties> elasticProperties;
    RifElasticPropertiesReader::readElasticProperties( elasticProperties, filePaths );

    ASSERT_EQ( 2u, elasticProperties.size() );

    ASSERT_EQ( "Norne", elasticProperties[0].fieldName.toStdString() );
    ASSERT_EQ( "Norne", elasticProperties[1].fieldName.toStdString() );

    ASSERT_EQ( "Not", elasticProperties[0].formationName.toStdString() );
    ASSERT_EQ( "Not", elasticProperties[1].formationName.toStdString() );

    ASSERT_EQ( "Sand", elasticProperties[0].faciesName.toStdString() );
    ASSERT_EQ( "Sand", elasticProperties[1].faciesName.toStdString() );

    ASSERT_DOUBLE_EQ( 0.0, elasticProperties[0].porosity );
    ASSERT_DOUBLE_EQ( 0.1, elasticProperties[1].porosity );

    ASSERT_DOUBLE_EQ( 25.0, elasticProperties[0].youngsModulus );
    ASSERT_DOUBLE_EQ( 19.0, elasticProperties[1].youngsModulus );

    ASSERT_DOUBLE_EQ( 0.25, elasticProperties[0].poissonsRatio );
    ASSERT_DOUBLE_EQ( 0.27, elasticProperties[1].poissonsRatio );

    ASSERT_DOUBLE_EQ( 2000.0, elasticProperties[0].K_Ic );
    ASSERT_DOUBLE_EQ( 2099.0, elasticProperties[1].K_Ic );

    ASSERT_DOUBLE_EQ( 0.2, elasticProperties[0].proppantEmbedment );
    ASSERT_DOUBLE_EQ( 0.3, elasticProperties[1].proppantEmbedment );

    ASSERT_DOUBLE_EQ( 0.4, elasticProperties[0].immobileFluidSaturation );
    ASSERT_DOUBLE_EQ( 0.55, elasticProperties[1].immobileFluidSaturation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadCorrectInputFileWithTrailingSeparator )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.4,\n"
            << "Norne,Not,Sand,0.10,19,0.27,2099,0.3,0.4,0.5,0.2,0.5,0.55,\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );

    std::vector<RifElasticProperties> elasticProperties;
    RifElasticPropertiesReader::readElasticProperties( elasticProperties, filePaths );

    ASSERT_EQ( 2u, elasticProperties.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadCorrectInputFileWithCustomSeparator )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne;Not;Sand;0.00;25;0.25;2000;0.2;0.3;0.4;0.5;0.6;0.4;\n"
            << "Norne;Not;Sand;0.10;19;0.27;2099;0.3;0.4;0.5;0.2;0.5;0.55;\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );

    std::vector<RifElasticProperties> elasticProperties;
    RifElasticPropertiesReader::readElasticProperties( elasticProperties, filePaths, ";" );

    ASSERT_EQ( 2u, elasticProperties.size() );
}

//--------------------------------------------------------------------------------------------------
/// Helper to check exception messages when reading invalid files
//--------------------------------------------------------------------------------------------------
::testing::AssertionResult readingElasticPropertiesThrowsException( const QStringList& filePaths,
                                                                    const QString&     expectedMessage )
{
    std::vector<RifElasticProperties> elasticProperties;
    try
    {
        RifElasticPropertiesReader::readElasticProperties( elasticProperties, filePaths );
        // No exception thrown: fail!
        return ::testing::AssertionFailure() << "readElasticProperties did not throw exception";
    }
    catch ( FileParseException& error )
    {
        // Should always have cleaned up on failure
        EXPECT_EQ( 0u, elasticProperties.size() );
        // Check that we get the expected message
        EXPECT_EQ( expectedMessage.toStdString(), error.message.toStdString() );
        return ::testing::AssertionSuccess();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadMissingFileThrows )
{
    QStringList filePaths;
    QString     nonExistingFile( "this/is/a/file/which/does/not/exist.csv" );
    filePaths.append( nonExistingFile );
    ASSERT_TRUE( readingElasticPropertiesThrowsException( filePaths,
                                                          QString( "Unable to open file: %1" ).arg( nonExistingFile ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadShortLinesFileThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.7\n"
            << "Norne,Not,Sand,0.10,19,0.27\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE(
        readingElasticPropertiesThrowsException( filePaths,
                                                 QString( "Incomplete data on line 2: %1" ).arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadEmptyFieldNameThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.89\n"
            << ",Not,Sand,0.10,19,0.27,2099,0.3,0.3,0.4,0.5,0.6,0.89\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingElasticPropertiesThrowsException( filePaths,
                                                          QString( "Unexpected empty 'Field Name' on line 2: %1" )
                                                              .arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, ReadInvalidMeasureDepthThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.9\n"
            << "Norne,Not,Sand, not a number,23.4,0.27,2099,0.3,0.3,0.4,0.5,0.6,0.77\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingElasticPropertiesThrowsException( filePaths,
                                                          QString( "Invalid number for 'Porosity' on line 2: %1" )
                                                              .arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifElasticPropertiesReaderTest, CommentsAndEmptyLinesAreIgnored )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        // Comment should be ignored
        out << "# This is a comment.\n";
        out << "#This is also a comment.\n";
        out << " # This is also a comment which does not start on first character.\n";
        // Should skip empty lines
        out << "\n";
        out << "\t\n";
        out << "        \n";
        // Then some data
        out << "Norne,Not,Sand,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.7\n";
        // Comment in-between data should be ignored
        out << "# One more comment in-between the data\n";
        out << "Norne,Not,Silt,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.7\n";
        // Empty line in-between data should be ignored
        out << "\n";
        // Data with comment sign inside it is not ignored
        out << "Norne,Not,Shale,0.00,25,0.25,2000,0.2,0.3,0.4,0.5,0.6,0.8\n";
        // Trailing empty lines should be ignored
        out << "\n\n\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    std::vector<RifElasticProperties> elasticProperties;
    RifElasticPropertiesReader::readElasticProperties( elasticProperties, filePaths );

    ASSERT_EQ( 3u, elasticProperties.size() );

    ASSERT_EQ( "Sand", elasticProperties[0].faciesName.toStdString() );
    ASSERT_EQ( "Silt", elasticProperties[1].faciesName.toStdString() );
    ASSERT_EQ( "Shale", elasticProperties[2].faciesName.toStdString() );
}
