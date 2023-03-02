#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifRevealCsvSectionSummaryReader.h"

#include <QFile>
#include <QTextStream>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_04 = QString( "%1/RifRevealCsvSectionSummaryReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRevealCsvSectionSummaryReaderTest, ExpectedText )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_04 + "i1.csv";

    QFile file( fileName );

    EXPECT_TRUE( file.open( QFile::ReadOnly | QFile::Text ) );

    QTextStream in( &file );

    QString fileContents = in.readAll();

    QString                          errorMessage;
    RifRevealCsvSectionSummaryReader reader;

    bool isOk = reader.parse( fileContents, RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL, &errorMessage );
    ASSERT_TRUE( isOk );

    EXPECT_TRUE( errorMessage.isEmpty() );

    ASSERT_EQ( 40u, reader.allResultAddresses().size() );
    EXPECT_EQ( 0u, reader.allErrorAddresses().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRevealCsvSectionSummaryReaderTest, EmptyText )
{
    RifRevealCsvSectionSummaryReader reader;

    QString fileContents = "";
    QString errorMessage;

    bool isOk = reader.parse( fileContents, RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_MISC, &errorMessage );
    ASSERT_FALSE( isOk );
}
