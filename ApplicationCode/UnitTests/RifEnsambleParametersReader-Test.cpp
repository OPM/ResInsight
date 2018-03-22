#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifEnsambleParametersReader.h"
#include "RifFileParseTools.h"

#include <QString>
#include <numeric>


static const QString TEST_DATA_DIRECTORY = QString("%1/RifEnsambleParametersReader/").arg(TEST_DATA_DIR);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEnsambleParametersReaderTest, SuccessfulParsing)
{
    RifEnsambleParametersReader reader(TEST_DATA_DIRECTORY + "parameters.txt");

    try
    {
        reader.parse();

        const RifEnsambleParameters& parameters = reader.parameters();
        std::map<QString, double> params = parameters.parameters();

        EXPECT_TRUE(params.count("LETSWOF:L_1OW"));
        EXPECT_TRUE(params.count("LETSGOF:KRG1"));
        EXPECT_TRUE(params.count("LOG10_MULTFLT:MULTFLT_F1"));

        EXPECT_EQ(params["LETSWOF:L_1OW"], 1.83555);
        EXPECT_EQ(params["LETSGOF:KRG1"], 0.97);
        EXPECT_EQ(params["LOG10_MULTFLT:MULTFLT_F1"], -0.168356);
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEnsambleParametersReaderTest, ParseFailed_InvalidFormat)
{
    RifEnsambleParametersReader reader(TEST_DATA_DIRECTORY + "parameters_invalid_format.txt");

    try
    {
        reader.parse();

        EXPECT_TRUE(false);
    }
    catch (FileParseException e)
    {
        EXPECT_TRUE(e.message.contains("Invalid file format in line 10"));
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEnsambleParametersReaderTest, ParseFailed_InvalidNumberFormat)
{
    RifEnsambleParametersReader reader(TEST_DATA_DIRECTORY + "parameters_invalid_number_format.txt");

    try
    {
        reader.parse();

        EXPECT_TRUE(false);
    }
    catch (FileParseException e)
    {
        EXPECT_TRUE(e.message.contains("Invalid number format in line 10"));
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
}
