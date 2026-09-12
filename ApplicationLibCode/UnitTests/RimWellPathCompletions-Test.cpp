#include "gtest/gtest.h"

#include "RimCustomSegmentIntervalCollection.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <memory>

TEST( RimWellPathCompletions, SegmentIntervalsDefineCustomSegmentation )
{
    RimWellPath wellPath;
    auto*       segments = wellPath.segmentCollection();

    segments->setLinerDiameter( 0.18 );
    segments->setRoughnessFactor( 3.0e-5 );
    auto* interval = segments->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );

    ASSERT_NE( nullptr, interval );
    EXPECT_TRUE( segments->hasCustomSegmentIntervals() );
    EXPECT_EQ( ( std::vector<std::pair<double, double>>{ { 100.0, 200.0 } } ), segments->getSegmentIntervals() );
    EXPECT_DOUBLE_EQ( 0.12, segments->getDiameterAtMD( 150.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 2.0e-5, segments->getRoughnessAtMD( 150.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 0.18, segments->getDiameterAtMD( 250.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 3.0e-5, segments->getRoughnessAtMD( 250.0, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
}

TEST( RimWellPathCompletions, SegmentIntervalUiUsesMeasuredDepthNameAndMetricUnits )
{
    RimWellPath wellPath;
    auto*       interval = wellPath.segmentCollection()->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );

    std::unique_ptr<caf::PdmUiTreeOrdering> treeOrdering( interval->uiCapability()->uiTreeOrdering() );
    ASSERT_NE( nullptr, treeOrdering );
    EXPECT_EQ( "100 - 200", interval->uiCapability()->uiName() );

    caf::PdmUiOrdering uiOrdering;
    interval->uiCapability()->uiOrdering( "", uiOrdering );

    ASSERT_NE( nullptr, interval->findField( "StartMd" ) );
    ASSERT_NE( nullptr, interval->findField( "EndMd" ) );
    ASSERT_NE( nullptr, interval->findField( "Diameter" ) );
    ASSERT_NE( nullptr, interval->findField( "RoughnessFactor" ) );
    EXPECT_EQ( "Start MD [m]", interval->findField( "StartMd" )->uiCapability()->uiName() );
    EXPECT_EQ( "End MD [m]", interval->findField( "EndMd" )->uiCapability()->uiName() );
    EXPECT_EQ( "Diameter [m]", interval->findField( "Diameter" )->uiCapability()->uiName() );
    EXPECT_EQ( "Roughness Factor [m]", interval->findField( "RoughnessFactor" )->uiCapability()->uiName() );
}

TEST( RimWellPathCompletions, SegmentIntervalUiUsesFieldUnits )
{
    RimWellPath wellPath;
    wellPath.setUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    auto* interval = wellPath.segmentCollection()->createInterval( 100.0, 200.0, 0.5, 3.28e-5 );

    caf::PdmUiOrdering uiOrdering;
    interval->uiCapability()->uiOrdering( "", uiOrdering );

    EXPECT_EQ( "Start MD [ft]", interval->findField( "StartMd" )->uiCapability()->uiName() );
    EXPECT_EQ( "End MD [ft]", interval->findField( "EndMd" )->uiCapability()->uiName() );
    EXPECT_EQ( "Diameter [ft]", interval->findField( "Diameter" )->uiCapability()->uiName() );
    EXPECT_EQ( "Roughness Factor [ft]", interval->findField( "RoughnessFactor" )->uiCapability()->uiName() );
    EXPECT_DOUBLE_EQ( 0.5, interval->diameter( RiaDefines::EclipseUnitSystem::UNITS_FIELD ) );
    EXPECT_DOUBLE_EQ( 0.1524, interval->diameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
}

TEST( RimWellPathCompletions, LegacySegmentIntervalsAreMergedByBounds )
{
    RimMswCompletionParameters legacyParameters;
    legacyParameters.setLinerDiameter( 0.2 );
    legacyParameters.setRoughnessFactor( 3.0e-5 );
    legacyParameters.setDiameterRoughnessMode( RimMswCompletionParameters::DiameterRoughnessMode::INTERVALS );
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

TEST( RimWellPathCompletions, LegacyUniformDiameterIntervalsRemainInactive )
{
    RimMswCompletionParameters legacyParameters;
    legacyParameters.diameterRoughnessIntervals()->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );

    RimSegmentCollection segments;
    segments.importLegacyData( &legacyParameters );

    EXPECT_TRUE( segments.intervals().empty() );
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
