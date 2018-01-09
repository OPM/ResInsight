#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifElementPropertyTableReader.h"

#include <QString>
#include <numeric>


static const QString TEST_DATA_DIRECTORY = QString("%1/ElementPropertyTable/").arg(TEST_DATA_DIR);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicElementPropertyTableReaderTest, BasicUsage)
{
    RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(TEST_DATA_DIRECTORY + "ELASTIC_TABLE.inp");

    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData(&metadata, &table);

    EXPECT_TRUE(table.hasData);

    EXPECT_EQ(2, metadata.dataColumns.size());
    EXPECT_STREQ("MODULUS", metadata.dataColumns[0].toStdString().c_str());
    EXPECT_STREQ("RATIO", metadata.dataColumns[1].toStdString().c_str());

    EXPECT_EQ(2, table.data.size());
    EXPECT_EQ(4320, table.elementIds.size());
    EXPECT_EQ(4320, table.data[0].size());
    EXPECT_EQ(4320, table.data[1].size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicElementPropertyTableReaderTest, ParseFailed)
{
    try
    {
        RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(TEST_DATA_DIRECTORY + "ELASTIC_TABLE_error.inp");

        RifElementPropertyTable table;
        RifElementPropertyTableReader::readData(&metadata, &table);

        EXPECT_TRUE(false);
    }
    catch (FileParseException e)
    {
        EXPECT_TRUE(e.message.startsWith("Number of columns mismatch"));
    }
}
