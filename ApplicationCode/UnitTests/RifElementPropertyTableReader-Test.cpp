#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifElementPropertyTableReader.h"
#include "RifFileParseTools.h"

#include <QString>
#include <numeric>


static const QString ELEM_PROP_TEST_DATA_DIRECTORY = QString("%1/RifElementPropertyTableReader/").arg(TEST_DATA_DIR);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicElementPropertyTableReaderTest, BasicUsage)
{
    RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE.inp");

    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData(&metadata, &table);

    EXPECT_TRUE(table.hasData);

    EXPECT_EQ(2u, metadata.dataColumns.size());
    EXPECT_STREQ("MODULUS", metadata.dataColumns[0].toStdString().c_str());
    EXPECT_STREQ("RATIO", metadata.dataColumns[1].toStdString().c_str());

    EXPECT_EQ(2u, table.data.size());
    EXPECT_EQ(4320u, table.elementIds.size());
    EXPECT_EQ(4320u, table.data[0].size());
    EXPECT_EQ(4320u, table.data[1].size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicElementPropertyTableReaderTest, ParseFailed)
{
    try
    {
        RifElementPropertyMetadata metadata = RifElementPropertyTableReader::readMetadata(ELEM_PROP_TEST_DATA_DIRECTORY + "ELASTIC_TABLE_error.inp");

        RifElementPropertyTable table;
        RifElementPropertyTableReader::readData(&metadata, &table);

        EXPECT_TRUE(false);
    }
    catch (FileParseException e)
    {
        EXPECT_TRUE(e.message.startsWith("Number of columns mismatch"));
    }
}
