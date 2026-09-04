#include "gtest/gtest.h"

#include "RimCustomSegmentIntervalCollection.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"
#include "RimWellPathCompletions.h"

TEST( RimWellPathCompletions, SegmentIntervalsDefineCustomSegmentation )
{
    RimWellPathCompletions completions;
    auto*                  segments = completions.segmentCollection();

    segments->setDiameterRoughnessMode( RimSegmentCollection::DiameterRoughnessMode::INTERVALS );
    auto* interval = segments->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );

    ASSERT_NE( nullptr, interval );
    EXPECT_TRUE( segments->hasCustomSegmentIntervals() );
    EXPECT_EQ( ( std::vector<std::pair<double, double>>{ { 100.0, 200.0 } } ), segments->getSegmentIntervals() );
    EXPECT_DOUBLE_EQ( 0.12, segments->getDiameterAtMD( 150.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 2.0e-5, segments->getRoughnessAtMD( 150.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
}

TEST( RimWellPathCompletions, LegacySegmentIntervalsAreMergedByBounds )
{
    RimMswCompletionParameters legacyParameters;
    legacyParameters.setLinerDiameter( 0.2 );
    legacyParameters.setRoughnessFactor( 3.0e-5 );
    legacyParameters.diameterRoughnessIntervals()->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );
    legacyParameters.customSegmentIntervals()->createInterval( 100.0 + 0.5e-6, 200.0 - 0.5e-6 );
    legacyParameters.customSegmentIntervals()->createInterval( 300.0, 400.0 );

    RimSegmentCollection segments;
    segments.importLegacyData( &legacyParameters );

    const auto intervals = segments.intervals();
    ASSERT_EQ( 2, intervals.size() );
    EXPECT_DOUBLE_EQ( 0.12, intervals[0]->diameter() );
    EXPECT_DOUBLE_EQ( 2.0e-5, intervals[0]->roughnessFactor() );
    EXPECT_DOUBLE_EQ( 300.0, intervals[1]->startMD() );
    EXPECT_DOUBLE_EQ( 400.0, intervals[1]->endMD() );
    EXPECT_DOUBLE_EQ( 0.2, intervals[1]->diameter() );
    EXPECT_DOUBLE_EQ( 3.0e-5, intervals[1]->roughnessFactor() );
}

/*
#include <QRegExpValidator>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathCompletions, WellNameRegExp )
{
    std::vector<QString> validNames   = { "RASASD", "gf0sdf", "sd-ASD12", "1-AA_b" };
    std::vector<QString> invalidNames = { ".AdSD", "+gf0sdf", "sd ASD12", "ABCDEFGHIJKL" };

    QRegExp rx = RimWellPathCompletionSettings::wellNameForExportRegExp();
    EXPECT_TRUE( rx.isValid() );

    for ( QString validName : validNames )
    {
        EXPECT_TRUE( rx.exactMatch( validName ) );
    }
    for ( QString invalidName : invalidNames )
    {
        EXPECT_FALSE( rx.exactMatch( invalidName ) );
    }
}

TEST( RimWellPathCompletions, WellNameRegExpValidator )
{
    std::vector<QString> validNames   = { "RASASD", "gf0sdf", "sd-ASD12", "1-AA_b" };
    std::vector<QString> invalidNames = { ".AdSD", "+gf0sdf", "sd ASD12", "ABCDEFGHIJKL" };
    QString              emptyString  = "";

    QRegExp          rx = RimWellPathCompletionSettings::wellNameForExportRegExp();
    QRegExpValidator validator( nullptr );
    validator.setRegExp( rx );

    for ( QString validName : validNames )
    {
        int dummyPos;
        EXPECT_EQ( QValidator::Acceptable, validator.validate( validName, dummyPos ) );
    }
    for ( QString invalidName : invalidNames )
    {
        int dummyPos;
        EXPECT_EQ( QValidator::Invalid, validator.validate( invalidName, dummyPos ) );
    }

    int dummyPos;
    EXPECT_EQ( QValidator::Intermediate, validator.validate( emptyString, dummyPos ) );
}
*/
