#include "gtest/gtest.h"

#include "RiaQDateTimeTools.h"

#include <QDate>

namespace
{
QDateTime utcTime( int year, int month, int day )
{
    return RiaQDateTimeTools::createUtcDateTime( QDate( year, month, day ) );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Time steps landing on a calendar year boundary must report a whole number of years, also when a
/// leap year is part of the interval.
//--------------------------------------------------------------------------------------------------
TEST( RiaQDateTimeToolsTest, CalendarYearsBetween )
{
    const QDateTime start = utcTime( 2000, 1, 1 );

    for ( int i = 0; i <= 10; i++ )
    {
        EXPECT_NEAR( static_cast<double>( i ), RiaQDateTimeTools::calendarYearsBetween( start, utcTime( 2000 + i, 1, 1 ) ), 1e-9 );
    }

    // Half way into a non-leap year
    EXPECT_NEAR( 1.5, RiaQDateTimeTools::calendarYearsBetween( start, utcTime( 2001, 7, 2 ) ), 1e-2 );

    // Start time that is not at a year boundary
    EXPECT_NEAR( 3.0, RiaQDateTimeTools::calendarYearsBetween( utcTime( 1997, 11, 6 ), utcTime( 2000, 11, 6 ) ), 1e-9 );

    // Time steps before the start time are reported as negative
    EXPECT_NEAR( -2.0, RiaQDateTimeTools::calendarYearsBetween( start, utcTime( 1998, 1, 1 ) ), 1e-9 );

    EXPECT_NEAR( 0.0, RiaQDateTimeTools::calendarYearsBetween( start, start ), 1e-9 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaQDateTimeToolsTest, CalendarMonthsBetween )
{
    const QDateTime start = utcTime( 2000, 1, 1 );

    for ( int i = 0; i <= 36; i++ )
    {
        int year  = 2000 + i / 12;
        int month = 1 + i % 12;

        EXPECT_NEAR( static_cast<double>( i ), RiaQDateTimeTools::calendarMonthsBetween( start, utcTime( year, month, 1 ) ), 1e-9 );
    }

    // 16 days into a 31 day month
    EXPECT_NEAR( 16.0 / 31.0, RiaQDateTimeTools::calendarMonthsBetween( start, utcTime( 2000, 1, 17 ) ), 1e-9 );

    EXPECT_NEAR( -1.0, RiaQDateTimeTools::calendarMonthsBetween( start, utcTime( 1999, 12, 1 ) ), 1e-9 );
}
