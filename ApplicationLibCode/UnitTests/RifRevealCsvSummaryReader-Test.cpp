#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifRevealCsvSummaryReader.h"

#include <QFile>
#include <QTextStream>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_05 = QString( "%1/RifRevealCsvSectionSummaryReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRevealCsvSummaryReaderTest, ExpectedText )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_05 + "welldata.csv";

    QString                   errorMessage;
    RifRevealCsvSummaryReader reader;
    auto [isOk, caseName] = reader.parse( fileName, &errorMessage );
    ASSERT_TRUE( isOk );
    ASSERT_EQ( std::string( "Clean water 60degC 3000Sm3" ), caseName.toStdString() );

    EXPECT_TRUE( errorMessage.isEmpty() );

    ASSERT_EQ( 115u, reader.allResultAddresses().size() );
    EXPECT_EQ( 0u, reader.allErrorAddresses().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRevealCsvSummaryReaderTest, EmptyText )
{
    RifRevealCsvSummaryReader reader;

    QString fileName = "/tmp/this/file/does/not/exist.12345";
    QString errorMessage;

    auto [isOk, caseName] = reader.parse( fileName, &errorMessage );
    ASSERT_FALSE( isOk );
    ASSERT_TRUE( caseName.isEmpty() );
}
