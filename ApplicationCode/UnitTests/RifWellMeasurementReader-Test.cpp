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

#include "RifFileParseTools.h"
#include "RifWellMeasurementReader.h"

#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadCorrectInputFile )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "NO 1234/5-6, 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n"
            << "NO 4321/7-8, 4321.79, 2024-12-24, DP, 88.8, 1, Poor test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );

    std::vector<RifWellMeasurement> wellMeasurements;
    RifWellMeasurementReader::readWellMeasurements( wellMeasurements, filePaths );

    ASSERT_EQ( 2u, wellMeasurements.size() );

    ASSERT_EQ( "NO 1234/5-6", wellMeasurements[0].wellName.toStdString() );
    ASSERT_EQ( "NO 4321/7-8", wellMeasurements[1].wellName.toStdString() );

    ASSERT_DOUBLE_EQ( 1234.56, wellMeasurements[0].MD );
    ASSERT_DOUBLE_EQ( 4321.79, wellMeasurements[1].MD );

    QDate date0( 2019, 11, 13 );
    ASSERT_EQ( date0.year(), wellMeasurements[0].date.year() );
    ASSERT_EQ( date0.month(), wellMeasurements[0].date.month() );
    ASSERT_EQ( date0.day(), wellMeasurements[0].date.day() );

    QDate date1( 2024, 12, 24 );
    ASSERT_EQ( date1.year(), wellMeasurements[1].date.year() );
    ASSERT_EQ( date1.month(), wellMeasurements[1].date.month() );
    ASSERT_EQ( date1.day(), wellMeasurements[1].date.day() );

    ASSERT_DOUBLE_EQ( 99.9, wellMeasurements[0].value );
    ASSERT_DOUBLE_EQ( 88.8, wellMeasurements[1].value );

    ASSERT_EQ( "XLOT", wellMeasurements[0].kind.toStdString() );
    ASSERT_EQ( "DP", wellMeasurements[1].kind.toStdString() );

    ASSERT_EQ( 3, wellMeasurements[0].quality );
    ASSERT_EQ( 1, wellMeasurements[1].quality );

    ASSERT_EQ( "Good test", wellMeasurements[0].remark.toStdString() );
    ASSERT_EQ( "Poor test", wellMeasurements[1].remark.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, KindsAreUppercased )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "NO 1234/1-2, 1234.56, 2019-11-13, LOT, 99.9, 3, Good test\n"
            << "NO 1234/3-4, 2345.67, 2024-12-24, lot, 88.8, 1, Poor test\n"
            << "NO 1234/5-6, 3456.78, 2026-12-24, lOt, 77.7, 2, Poor test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );

    std::vector<RifWellMeasurement> wellMeasurements;
    RifWellMeasurementReader::readWellMeasurements( wellMeasurements, filePaths );

    ASSERT_EQ( 3u, wellMeasurements.size() );

    for ( unsigned int i = 0; i < wellMeasurements.size(); i++ )
    {
        ASSERT_EQ( "LOT", wellMeasurements[i].kind.toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper to check exception messages when reading invalid files
//--------------------------------------------------------------------------------------------------
::testing::AssertionResult readingThrowsException( const QStringList& filePaths, const QString& expectedMessage )
{
    std::vector<RifWellMeasurement> wellMeasurements;
    try
    {
        RifWellMeasurementReader::readWellMeasurements( wellMeasurements, filePaths );
        // No exception thrown: fail!
        return ::testing::AssertionFailure() << "readWellMeasurements did not throw exception";
    }
    catch ( FileParseException& error )
    {
        // Should always have cleaned up the well measurements on failure
        EXPECT_EQ( 0u, wellMeasurements.size() );
        // Check that we get the expected message
        EXPECT_EQ( expectedMessage.toStdString(), error.message.toStdString() );
        return ::testing::AssertionSuccess();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadMissingFileThrows )
{
    QStringList filePaths;
    QString     nonExistingFile( "this/is/a/file/which/does/not/exist.csv" );
    filePaths.append( nonExistingFile );
    ASSERT_TRUE( readingThrowsException( filePaths, QString( "Unable to open file: %1" ).arg( nonExistingFile ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadShortLinesFileThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "NO 1234/5-6, 1234.56\n"
            << "NO 4321/7-8, 4321.79, 2024-12-24\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingThrowsException( filePaths, QString( "Incomplete data on line 1: %1" ).arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadEmptyWellNameThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "NO 1234/5-6, 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n";
        out << ", 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingThrowsException( filePaths,
                                         QString( "Unexpected empty 'Well Name' on line 2: %1" ).arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadInvalidMeasureDepthThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "well 1, 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n";
        out << "well 2, this is should be a number, 2019-11-13, XLOT, 99.9, 3, Good test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE(
        readingThrowsException( filePaths,
                                QString( "Invalid number for 'Measured Depth' on line 2: %1" ).arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadInvalidQualityThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "well 1, 1234.56, 2019-11-13, XLOT, 99.9, this is not an int, Good test\n";
        out << "well 2, 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingThrowsException( filePaths,
                                         QString( "Invalid number for 'Quality' on line 1: %1" ).arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, ReadInvalidDateFormateThrows )
{
    QTemporaryFile file;
    EXPECT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << "well 1, 1234.56, 17th May 1814, XLOT, 99.9, 1, Good test\n";
        out << "well 2, 1234.56, 2019-11-13, XLOT, 99.9, 3, Good test\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    ASSERT_TRUE( readingThrowsException( filePaths,
                                         QString( "Invalid date format (must be ISO 8601) for 'Date' on line 1: %1" )
                                             .arg( file.fileName() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifWellMeasurementReaderTest, CommentsAndEmptyLinesAreIgnored )
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
        out << "well 1, 1234.56, 2019-11-11, XLOT, 199.9, 1, Good test\n";
        // Comment in-between data should be ignored
        out << "# One more comment in-between the data\n";
        out << "well 2, 2234.56, 2019-11-12, XLOT, 299.9, 2, Good test\n";
        // Empty line in-between data should be ignored
        out << "\n";
        // Data with comment sign inside it is not ignored
        out << "well #3, 3234.56, 2019-11-13, XLOT, 399.9, 3, Good test\n";
        // Trailing empty lines should be ignored
        out << "\n\n\n";
    }

    QStringList filePaths;
    filePaths.append( file.fileName() );
    std::vector<RifWellMeasurement> wellMeasurements;
    RifWellMeasurementReader::readWellMeasurements( wellMeasurements, filePaths );

    ASSERT_EQ( 3u, wellMeasurements.size() );

    ASSERT_EQ( "well 1", wellMeasurements[0].wellName.toStdString() );
    ASSERT_EQ( "well 2", wellMeasurements[1].wellName.toStdString() );
    ASSERT_EQ( "well #3", wellMeasurements[2].wellName.toStdString() );
}
