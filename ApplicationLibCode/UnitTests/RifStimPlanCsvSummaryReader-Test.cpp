#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifStimPlanCsvSummaryReader.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_05 = QString( "%1/RifStimPlanCsvSummaryReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifStimPlanCsvSummaryReaderTest, ExpectedText )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_05 + "data_vs_time.csv";

    QString                     errorMessage;
    QDateTime                   startDateTime = QDateTime::currentDateTime();
    RifStimPlanCsvSummaryReader reader;
    auto [isOk, caseName] = reader.parse( fileName, startDateTime, &errorMessage );
    EXPECT_TRUE( isOk );
    EXPECT_EQ( std::string( "[Frac 1]" ), caseName.toStdString() );
    EXPECT_TRUE( errorMessage.isEmpty() );

    ASSERT_EQ( 14u, reader.allResultAddresses().size() );
    EXPECT_EQ( 0u, reader.allErrorAddresses().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifStimPlanCsvSummaryReaderTest, EmptyText )
{
    QString   fileName = "/tmp/this/file/does/not/exist.12345";
    QString   errorMessage;
    QDateTime startDateTime = QDateTime::currentDateTime();

    RifStimPlanCsvSummaryReader reader;
    auto [isOk, caseName] = reader.parse( fileName, startDateTime, &errorMessage );
    ASSERT_FALSE( isOk );
    ASSERT_TRUE( caseName.isEmpty() );
}
