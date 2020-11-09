#include "gtest/gtest.h"

#include "RiaDateStringParser.h"
#include "RiaQDateTimeTools.h"

#include <QDate>
#include <QDateTime>

#include <map>
#include <string>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaDateStringParserTest, ParseYearFirstWithSeparators )
{
    QDateTime may2ndDT = QDateTime( QDate( 2011, 05, 02 ) );
    may2ndDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> may2ndStrings = { "2011 05 02",
                                               "2011 May 02",
                                               "2011_05_02",
                                               "2011_May_02",
                                               "2011_may_02",
                                               "2011-05-02",
                                               "2011-May-02",
                                               "2011-may-02",
                                               "2011.05.02",
                                               "2011.May.02" };
    for ( auto may2ndString : may2ndStrings )
    {
        QDateTime parsedDate = RiaDateStringParser::parseDateString( may2ndString );
        EXPECT_TRUE( may2ndDT == parsedDate );
    }

    QDateTime nov24thDT = QDateTime( QDate( 1992, 11, 24 ) );
    nov24thDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> nov24thStrings = { "1992-11-24", "1992-Nov-24", "1992-nov-24", "1992.11.24" };
    for ( auto nov24thString : nov24thStrings )
    {
        EXPECT_TRUE( nov24thDT == RiaDateStringParser::parseDateString( nov24thString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaDateStringParserTest, ParseDayFirstWithSeparators )
{
    QDateTime may2ndDT = QDateTime( QDate( 2011, 05, 02 ) );
    may2ndDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> may2ndStrings = { "02 05 2011",
                                               "02 May 2011",
                                               "02_05_2011",
                                               "02_May_2011",
                                               "02_may_2011",
                                               "02-05-2011",
                                               "02-May-2011",
                                               "02-may-2011",
                                               "02.05.2011" };
    for ( auto may2ndString : may2ndStrings )
    {
        QDateTime parsedDate = RiaDateStringParser::parseDateString( may2ndString );
        EXPECT_TRUE( may2ndDT == parsedDate );
    }

    QDateTime nov24thDT = QDateTime( QDate( 1992, 11, 24 ) );
    nov24thDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> nov24thStrings = { "24-11-1992", "24-Nov-1992", "24.Nov 1992", "24.11.1992" };
    for ( auto nov24thString : nov24thStrings )
    {
        EXPECT_TRUE( nov24thDT == RiaDateStringParser::parseDateString( nov24thString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaDateStringParserTest, ParseMonthFirstWithSeparators )
{
    QDateTime may2ndDT = QDateTime( QDate( 2011, 05, 02 ) );
    may2ndDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> may2ndStrings = { "May 02 2011", "may 02 2011", "May_02_2011", "May.02.2011", "May 02. 2011" };
    for ( auto may2ndString : may2ndStrings )
    {
        QDateTime parsedDate = RiaDateStringParser::parseDateString( may2ndString );
        EXPECT_TRUE( may2ndDT == parsedDate );
    }

    QDateTime nov24thDT = QDateTime( QDate( 1992, 11, 24 ) );
    nov24thDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> nov24thStrings = { "11-24-1992", "Nov-24-1992", "Nov 24. 1992", "11.24.1992", "11 24 1992" };
    for ( auto nov24thString : nov24thStrings )
    {
        EXPECT_TRUE( nov24thDT == RiaDateStringParser::parseDateString( nov24thString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaDateStringParserTest, ParseWithoutSeparators )
{
    QDateTime may2ndDT = QDateTime( QDate( 2011, 05, 02 ) );
    QDateTime feb5thDT = QDateTime( QDate( 2011, 02, 05 ) );
    may2ndDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );
    feb5thDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> may2ndStrings = { "20110502", "02052011", "Date_20110502" };
    for ( auto may2ndString : may2ndStrings )
    {
        QDateTime parsedDate = RiaDateStringParser::parseDateString( may2ndString );
        EXPECT_TRUE( may2ndDT == parsedDate );
    }
    // This is ambiguous and we prefer day first ahead of month first.
    std::string may2ndMonthFirstString( "05022011" );
    EXPECT_FALSE( may2ndDT == RiaDateStringParser::parseDateString( may2ndMonthFirstString ) );
    EXPECT_TRUE( feb5thDT == RiaDateStringParser::parseDateString( may2ndMonthFirstString ) );

    QDateTime nov24thDT = QDateTime( QDate( 1992, 11, 24 ) );
    nov24thDT.setTimeSpec( RiaQDateTimeTools::currentTimeSpec() );

    std::vector<std::string> nov24thStrings = { "19921124", "24111992", "921124", "241192", "11241992", "112492" };
    for ( auto nov24thString : nov24thStrings )
    {
        EXPECT_TRUE( nov24thDT == RiaDateStringParser::parseDateString( nov24thString ) );
    }
}
