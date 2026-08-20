#include "gtest/gtest.h"

#include "RimSummaryTimeAxisProperties.h"

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
/// MONTHS and YEARS use calendar arithmetic, the shorter units are a plain scaling of the number of
/// seconds since the simulation start.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTimeAxisPropertiesTest, TimeFromSimulationStart )
{
    const QDateTime start = utcTime( 2000, 1, 1 );

    // 2000 is a leap year, a fixed seconds per year scaling would report 1.002074 years
    EXPECT_NEAR( 1.0,
                 RimSummaryTimeAxisProperties::timeFromSimulationStart( start, utcTime( 2001, 1, 1 ), RimSummaryTimeAxisProperties::YEARS ),
                 1e-9 );

    EXPECT_NEAR( 12.0,
                 RimSummaryTimeAxisProperties::timeFromSimulationStart( start, utcTime( 2001, 1, 1 ), RimSummaryTimeAxisProperties::MONTHS ),
                 1e-9 );

    EXPECT_NEAR( 366.0,
                 RimSummaryTimeAxisProperties::timeFromSimulationStart( start, utcTime( 2001, 1, 1 ), RimSummaryTimeAxisProperties::DAYS ),
                 1e-9 );

    EXPECT_NEAR( 24.0,
                 RimSummaryTimeAxisProperties::timeFromSimulationStart( start, utcTime( 2000, 1, 2 ), RimSummaryTimeAxisProperties::HOURS ),
                 1e-9 );

    EXPECT_NEAR( 86400.0,
                 RimSummaryTimeAxisProperties::timeFromSimulationStart( start, utcTime( 2000, 1, 2 ), RimSummaryTimeAxisProperties::SECONDS ),
                 1e-9 );
}
