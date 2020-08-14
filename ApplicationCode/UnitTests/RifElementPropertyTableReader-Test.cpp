#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifElementPropertyTableReader.h"
#include "RifFileParseTools.h"

#include <QString>
#include <numeric>

static const QString ELEM_PROP_TEST_DATA_DIRECTORY = QString( "%1/RifElementPropertyTableReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicElementPropertyTableReaderTest, BasicUsage )
{
    RifElementPropertyMetadata metadata =
        RifElementPropertyTableReader::readMetadata( ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE.inp" );

    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData( &metadata, &table );

    EXPECT_TRUE( table.hasData );

    EXPECT_EQ( 2u, metadata.dataColumns.size() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[0].toStdString().c_str() );
    EXPECT_STREQ( "RATIO", metadata.dataColumns[1].toStdString().c_str() );

    EXPECT_EQ( 2u, table.data.size() );
    EXPECT_EQ( 4320u, table.elementIds.size() );
    EXPECT_EQ( 4320u, table.data[0].size() );
    EXPECT_EQ( 4320u, table.data[1].size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicElementPropertyTableReaderTest, ParseFailedForTooManyColumns )
{
    try
    {
        RifElementPropertyMetadata metadata =
            RifElementPropertyTableReader::readMetadata( ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE_error_too_many_columns.inp" );

        RifElementPropertyTable table;
        RifElementPropertyTableReader::readData( &metadata, &table );

        EXPECT_TRUE( false );
    }
    catch ( FileParseException e )
    {
        EXPECT_TRUE( e.message.startsWith( "Number of columns mismatch" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicElementPropertyTableReaderTest, ParseFailedForTooFewColumns )
{
    try
    {
        RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(
            ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE_error_too_few_columns.inp" );

        RifElementPropertyTable table;
        RifElementPropertyTableReader::readData( &metadata, &table );

        EXPECT_TRUE( false );
    }
    catch ( FileParseException e )
    {
        EXPECT_TRUE( e.message.startsWith( "Number of columns mismatch" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicElementPropertyTableReaderTest, MoreThanEightColumns )
{
    RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(
        ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE_morethan8columns.inp" );

    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData( &metadata, &table );

    EXPECT_TRUE( table.hasData );

    EXPECT_EQ( 9u, metadata.dataColumns.size() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[0].toStdString().c_str() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[1].toStdString().c_str() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[2].toStdString().c_str() );
    EXPECT_STREQ( "RATIO", metadata.dataColumns[3].toStdString().c_str() );
    EXPECT_STREQ( "RATIO", metadata.dataColumns[4].toStdString().c_str() );
    EXPECT_STREQ( "RATIO", metadata.dataColumns[5].toStdString().c_str() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[6].toStdString().c_str() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[7].toStdString().c_str() );
    EXPECT_STREQ( "MODULUS", metadata.dataColumns[8].toStdString().c_str() );


    EXPECT_EQ( 9u, table.data.size() );
    EXPECT_EQ( 8u, table.elementIds.size() );
    EXPECT_EQ( 8u, table.data[0].size() );
    EXPECT_EQ( 8u, table.data[1].size() );
    EXPECT_EQ( 8u, table.data[2].size() );
    EXPECT_EQ( 8u, table.data[3].size() );
    EXPECT_EQ( 8u, table.data[4].size() );
    EXPECT_EQ( 8u, table.data[5].size() );
    EXPECT_EQ( 8u, table.data[6].size() );
    EXPECT_EQ( 8u, table.data[7].size() );
    EXPECT_EQ( 8u, table.data[8].size() );
}
