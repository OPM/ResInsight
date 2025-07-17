#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifCaseRealizationParametersReader.h"
#include "RifFileParseTools.h"

#include <QString>
#include <numeric>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_01 = QString( "%1/RifCaseRealizationParametersReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifCaseRealizationParametersReaderTest, MixOfTabAndWhiteSpace )
{
    RifCaseRealizationParametersReader reader( CASE_REAL_TEST_DATA_DIRECTORY_01 + "parameters-mix-tab-whitespace.txt" );

    try
    {
        reader.parse();

        const std::shared_ptr<RigCaseRealizationParameters>    parameters( reader.parameters() );
        std::map<QString, RigCaseRealizationParameters::Value> params = parameters->parameters();

        std::vector<std::pair<QString, double>> expectedValues = { { "LETSWOF:L_1OW", 1.83555 },
                                                                   { "LETSWOF:E_1OW", 5.84645 },
                                                                   { "LETSWOF:T_1OW", 1.46894 },
                                                                   { "LETSWOF:L_1WO", 4.9974 },
                                                                   { "LETSWOF:E_1WO", 3.38433e-05 } };

        for ( const auto& expected : expectedValues )
        {
            EXPECT_EQ( expected.second, params[expected.first].numericValue() )
                << "Parameter value mismatch for: " << expected.first.toStdString();
        }
    }
    catch ( ... )
    {
        EXPECT_TRUE( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifCaseRealizationParametersReaderTest, LocatorTestSuccess )
{
    QString file     = RifCaseRealizationParametersFileLocator::locate( CASE_REAL_TEST_DATA_DIRECTORY_01 + "4/3/2" );
    QString expected = CASE_REAL_TEST_DATA_DIRECTORY_01 + "parameters.txt";
    EXPECT_EQ( expected.toStdString(), file.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifCaseRealizationParametersReaderTest, LocatorTestFailure )
{
    QString file     = RifCaseRealizationParametersFileLocator::locate( CASE_REAL_TEST_DATA_DIRECTORY_01 + "6/5/4/3/2/1" );
    QString expected = "";
    EXPECT_EQ( expected.toStdString(), file.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifCaseRealizationParametersReaderTest, SuccessfulParsing )
{
    RifCaseRealizationParametersReader reader( CASE_REAL_TEST_DATA_DIRECTORY_01 + "parameters.txt" );

    try
    {
        reader.parse();

        const std::shared_ptr<RigCaseRealizationParameters>    parameters( reader.parameters() );
        std::map<QString, RigCaseRealizationParameters::Value> params = parameters->parameters();

        EXPECT_EQ( 1U, params.count( "LETSWOF:L_1OW" ) );
        EXPECT_EQ( 1U, params.count( "LETSGOF:KRG1" ) );
        EXPECT_EQ( 1U, params.count( "LOG10_MULTFLT:MULTFLT_F1" ) );
        EXPECT_EQ( 1U, params.count( "TST:TEXT_PARAM" ) );

        EXPECT_TRUE( params["LETSWOF:L_1OW"].isNumeric() );
        EXPECT_EQ( 1.83555, params["LETSWOF:L_1OW"].numericValue() );
        EXPECT_TRUE( params["LETSGOF:KRG1"].isNumeric() );
        EXPECT_EQ( 0.97, params["LETSGOF:KRG1"].numericValue() );
        EXPECT_TRUE( params["TST:TEXT_PARAM"].isText() );
        EXPECT_EQ( std::string( "YES" ), params["TST:TEXT_PARAM"].textValue().toStdString() );
    }
    catch ( ... )
    {
        EXPECT_TRUE( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifCaseRealizationParametersReaderTest, FindRealizationNumber )
{
    QString filePath = "d:/gitroot-ceesol/ResInsight-regression-test/ModelData/ensemble_reek_with_params/realization-"
                       "7/iter-0/eclipse/model/3_R001_REEK-7.SMSPEC";

    int realisationNumber = RifCaseRealizationParametersFileLocator::realizationNumberFromFullPath( filePath );

    EXPECT_EQ( 7, realisationNumber );
}
