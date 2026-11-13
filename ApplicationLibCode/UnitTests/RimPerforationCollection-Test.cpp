#include "gtest/gtest.h"

#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"

#include <QDate>
#include <QDateTime>

namespace
{
RimPerforationInterval* addInterval( RimPerforationCollection* collection, double startMD, double endMD, bool isChecked = true )
{
    auto* interval = new RimPerforationInterval;
    interval->setStartAndEndMD( startMD, endMD );
    interval->setCheckState( isChecked );
    collection->appendPerforation( interval );
    return interval;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimPerforationCollection, NoActivePerforationsGivesNoMdRange )
{
    RimWellPath wellPath;
    auto*       collection = wellPath.perforationIntervalCollection();
    ASSERT_NE( nullptr, collection );

    const QDateTime date( QDate( 2020, 1, 1 ), QTime( 0, 0 ) );
    EXPECT_FALSE( collection->activePerforationMdRange( date ).has_value() );

    addInterval( collection, 100.0, 200.0, false );
    EXPECT_FALSE( collection->activePerforationMdRange( date ).has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimPerforationCollection, MdRangeSpansCheckedIntervalsIncludingGaps )
{
    RimWellPath wellPath;
    auto*       collection = wellPath.perforationIntervalCollection();
    ASSERT_NE( nullptr, collection );

    addInterval( collection, 300.0, 400.0 );
    addInterval( collection, 100.0, 200.0 );
    addInterval( collection, 700.0, 800.0, false ); // unchecked, must not extend the range

    const auto range = collection->activePerforationMdRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0 ) ) );
    ASSERT_TRUE( range.has_value() );
    EXPECT_DOUBLE_EQ( 100.0, range->start );
    EXPECT_DOUBLE_EQ( 400.0, range->end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimPerforationCollection, IntervalsNotActiveOnDateAreExcluded )
{
    RimWellPath wellPath;
    auto*       collection = wellPath.perforationIntervalCollection();
    ASSERT_NE( nullptr, collection );

    addInterval( collection, 100.0, 200.0 );

    auto* later = addInterval( collection, 300.0, 400.0 );
    later->enableCustomStartDate( true );
    later->setCustomStartDate( QDate( 2021, 1, 1 ) );

    const auto before = collection->activePerforationMdRange( QDateTime( QDate( 2020, 6, 1 ), QTime( 0, 0 ) ) );
    ASSERT_TRUE( before.has_value() );
    EXPECT_DOUBLE_EQ( 100.0, before->start );
    EXPECT_DOUBLE_EQ( 200.0, before->end );

    const auto after = collection->activePerforationMdRange( QDateTime( QDate( 2021, 6, 1 ), QTime( 0, 0 ) ) );
    ASSERT_TRUE( after.has_value() );
    EXPECT_DOUBLE_EQ( 100.0, after->start );
    EXPECT_DOUBLE_EQ( 400.0, after->end );
}
