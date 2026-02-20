/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "Cloud/RiaConnectorTools.h"

#include <QTemporaryFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_nonExistentFile )
{
    auto result = RiaConnectorTools::readKeyValuePairs( "/this/path/does/not/exist.json" );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_emptyFile )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_stringValue )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"connection_string": "InstrumentationKey=abc123"})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( QString( "InstrumentationKey=abc123" ), result["connection_string"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_numericValue )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"batch_timeout_ms": 5000, "sampling_rate": 0.5})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 2u, result.size() );
    EXPECT_EQ( QString( "5000" ), result["batch_timeout_ms"] );
    EXPECT_EQ( QString( "0.5" ), result["sampling_rate"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_boolValue )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"enabled": true, "verbose": false})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 2u, result.size() );
    EXPECT_EQ( QString( "true" ), result["enabled"] );
    EXPECT_EQ( QString( "false" ), result["verbose"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_arrayValue )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"event_allowlist": ["app.started", "crash.*", "grpc.*"]})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( QString( "app.started,crash.*,grpc.*" ), result["event_allowlist"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_emptyArray )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"event_denylist": []})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( QString( "" ), result["event_denylist"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_arrayIgnoresNonStringElements )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({"mixed_array": ["health.status", 42, true, "test.*"]})";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( QString( "health.status,test.*" ), result["mixed_array"] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaConnectorToolsTest, readKeyValuePairs_mixedTypes )
{
    QTemporaryFile file;
    ASSERT_TRUE( file.open() );

    {
        QTextStream out( &file );
        out << R"({
            "connection_string": "InstrumentationKey=key1",
            "max_batch_size": 512,
            "enabled": true,
            "event_denylist": ["health.status", "test.*"]
        })";
    }

    auto result = RiaConnectorTools::readKeyValuePairs( file.fileName() );
    ASSERT_EQ( 4u, result.size() );
    EXPECT_EQ( QString( "InstrumentationKey=key1" ), result["connection_string"] );
    EXPECT_EQ( QString( "512" ), result["max_batch_size"] );
    EXPECT_EQ( QString( "true" ), result["enabled"] );
    EXPECT_EQ( QString( "health.status,test.*" ), result["event_denylist"] );
}
