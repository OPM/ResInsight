#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifCaseRealizationParametersReader.h"
#include "RifFileParseTools.h"

#include <QString>
#include <numeric>


static const QString TEST_DATA_DIRECTORY = QString("%1/RifCaseRealizationParametersReader/").arg(TEST_DATA_DIR);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifCaseRealizationParametersReaderTest, LocatorTestSuccess)
{
    QString file = RifCaseRealizationParametersFileLocator::locate(TEST_DATA_DIRECTORY + "4/3/2");
    QString expected = TEST_DATA_DIRECTORY + "parameters.txt";
    EXPECT_EQ(expected.toStdString(), file.toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifCaseRealizationParametersReaderTest, LocatorTestFailure)
{
    QString file = RifCaseRealizationParametersFileLocator::locate(TEST_DATA_DIRECTORY + "4/3/2/1");
    QString expected = "";
    EXPECT_EQ(expected.toStdString(), file.toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifCaseRealizationParametersReaderTest, SuccessfulParsing)
{
    RifCaseRealizationParametersReader reader(TEST_DATA_DIRECTORY + "parameters.txt");

    try
    {
        reader.parse();

        const std::shared_ptr<RigCaseRealizationParameters> parameters(reader.parameters());
        std::map<QString, RigCaseRealizationParameters::Value> params = parameters->parameters();

        EXPECT_EQ(1U, params.count("LETSWOF:L_1OW"));
        EXPECT_EQ(1U, params.count("LETSGOF:KRG1"));
        EXPECT_EQ(1U, params.count("LOG10_MULTFLT:MULTFLT_F1"));
        EXPECT_EQ(1U, params.count("TST:TEXT_PARAM"));

        EXPECT_TRUE(params["LETSWOF:L_1OW"].isNumeric());
        EXPECT_EQ(1.83555,          params["LETSWOF:L_1OW"].numericValue());
        EXPECT_TRUE(params["LETSGOF:KRG1"].isNumeric());
        EXPECT_EQ(0.97,             params["LETSGOF:KRG1"].numericValue());
        EXPECT_TRUE(params["TST:TEXT_PARAM"].isText());
        EXPECT_EQ(std::string("YES"), params["TST:TEXT_PARAM"].textValue().toStdString());
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifCaseRealizationParametersReaderTest, ParseFailed_InvalidFormat)
{
    RifCaseRealizationParametersReader reader(TEST_DATA_DIRECTORY + "parameters_invalid_format.txt");

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
TEST(RifCaseRealizationParametersReaderTest, ParseFailed_InvalidNumberFormat)
{
    RifCaseRealizationParametersReader reader(TEST_DATA_DIRECTORY + "parameters_invalid_number_format.txt");

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
