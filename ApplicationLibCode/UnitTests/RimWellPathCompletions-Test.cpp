#include "gtest/gtest.h"

#include "RimCustomSegmentIntervalCollection.h"
#include "RimDiameterRoughnessInterval.h"
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

TEST( RimWellPathCompletions, LegacyMigrationPreservesDisabledSettings )
{
    RimMswCompletionParameters legacyParameters;
    legacyParameters.setManualReferenceMD( 123.0 );
    auto* maxLength    = dynamic_cast<caf::PdmField<double>*>( legacyParameters.findField( "MaxSegmentLength" ) );
    auto* customValues = dynamic_cast<caf::PdmField<bool>*>( legacyParameters.findField( "CustomValuesForLateral" ) );
    ASSERT_NE( nullptr, maxLength );
    ASSERT_NE( nullptr, customValues );
    maxLength->setValue( 345.0 );
    customValues->setValue( true );
    legacyParameters.setPressureDrop( RimMswCompletionParameters::PressureDropType::HYDROSTATIC_FRICTION_ACCELERATION );
    legacyParameters.setLengthAndDepth( RimMswCompletionParameters::LengthAndDepthType::INC );

    RimSegmentCollection segments;
    segments.importLegacyData( &legacyParameters );

    EXPECT_EQ( RimSegmentCollection::ReferenceMDType::AUTO_REFERENCE_MD, segments.referenceMDType() );
    auto* migratedMaxLength    = dynamic_cast<caf::PdmField<double>*>( segments.findField( "MaxSegmentLength" ) );
    auto* migratedCustomValues = dynamic_cast<caf::PdmField<bool>*>( segments.findField( "CustomValuesForLateral" ) );
    auto* enforceMaxLength     = dynamic_cast<caf::PdmField<bool>*>( segments.findField( "EnforceMaxSegmentLength" ) );
    ASSERT_NE( nullptr, migratedMaxLength );
    ASSERT_NE( nullptr, migratedCustomValues );
    ASSERT_NE( nullptr, enforceMaxLength );
    EXPECT_DOUBLE_EQ( 345.0, migratedMaxLength->value() );
    EXPECT_TRUE( migratedCustomValues->value() );
    EXPECT_FALSE( enforceMaxLength->value() );
    EXPECT_EQ( RimSegmentCollection::PressureDropType::HYDROSTATIC_FRICTION_ACCELERATION, segments.pressureDrop() );
    EXPECT_EQ( RimSegmentCollection::LengthAndDepthType::INC, segments.lengthAndDepth() );

    segments.setReferenceMDType( RimSegmentCollection::ReferenceMDType::MANUAL_REFERENCE_MD );
    EXPECT_DOUBLE_EQ( 123.0, segments.manualReferenceMD() );
    enforceMaxLength->setValue( true );
    EXPECT_DOUBLE_EQ( 345.0, segments.maxSegmentLength() );
}

TEST( RimWellPathCompletions, DefaultLegacySettingsDoNotOverwriteSegments )
{
    RimMswCompletionParameters legacyParameters;
    RimSegmentCollection       segments;
    segments.setLinerDiameter( 0.3 );
    segments.createInterval( 100.0, 200.0, 0.12, 2.0e-5 );

    segments.importLegacyData( &legacyParameters );

    EXPECT_DOUBLE_EQ( 0.3, segments.linerDiameter() );
    EXPECT_EQ( 1u, segments.intervals().size() );
}

TEST( RimWellPathCompletions, LegacyMigrationPreservesIntervalDateTime )
{
    const auto startDate = QDateTime::fromString( "2026-09-14T13:45:12.123+02:00", Qt::ISODateWithMs );
    ASSERT_TRUE( startDate.isValid() );
    for ( bool useCustomDate : { false, true } )
    {
        RimMswCompletionParameters legacyParameters;
        legacyParameters.setDiameterRoughnessMode( RimMswCompletionParameters::DiameterRoughnessMode::INTERVALS );
        auto* oldInterval = legacyParameters.diameterRoughnessIntervals()->createInterval( 100.0, 200.0, 0.12, 2.0e-5 );
        oldInterval->enableCustomStartDate( useCustomDate );
        auto* dateField = dynamic_cast<caf::PdmField<QDateTime>*>( oldInterval->findField( "StartDate" ) );
        ASSERT_NE( nullptr, dateField );
        dateField->setValue( startDate );

        RimSegmentCollection segments;
        segments.importLegacyData( &legacyParameters );

        ASSERT_EQ( 1u, segments.intervals().size() );
        auto* interval     = segments.intervals().front();
        auto* migratedDate = dynamic_cast<caf::PdmField<QDateTime>*>( interval->findField( "StartDate" ) );
        ASSERT_NE( nullptr, migratedDate );
        EXPECT_EQ( startDate, migratedDate->value() );
        EXPECT_EQ( startDate.offsetFromUtc(), migratedDate->value().offsetFromUtc() );
        EXPECT_EQ( !useCustomDate, interval->isActiveOnDate( startDate.addMSecs( -1 ) ) );
        EXPECT_TRUE( interval->isActiveOnDate( startDate ) );
    }
}

TEST( RimWellPathCompletions, LateralSegmentsInheritTopLevelSettings )
{
    auto        topLevelWell = std::make_unique<RimWellPath>();
    RimWellPath lateral;
    auto*       localSegments = lateral.segmentCollection();
    localSegments->setLinerDiameter( 0.1 );
    localSegments->setRoughnessFactor( 1.0e-5 );
    topLevelWell->segmentCollection()->setReferenceMDType( RimSegmentCollection::ReferenceMDType::MANUAL_REFERENCE_MD );
    topLevelWell->segmentCollection()->setManualReferenceMD( 123.0 );
    topLevelWell->segmentCollection()->setLinerDiameter( 0.2 );
    topLevelWell->segmentCollection()->setRoughnessFactor( 3.0e-5 );
    lateral.connectWellPaths( topLevelWell.get(), 100.0 );

    EXPECT_EQ( localSegments, lateral.segmentCollection() );
    EXPECT_DOUBLE_EQ( 123.0, localSegments->manualReferenceMD() );
    EXPECT_DOUBLE_EQ( 0.2, localSegments->linerDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 3.0e-5, localSegments->roughnessFactor( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );

    topLevelWell->segmentCollection()->setManualReferenceMD( 456.0 );
    const RimWellPath& constLateral = lateral;
    EXPECT_DOUBLE_EQ( 456.0, constLateral.segmentCollection()->manualReferenceMD() );

    topLevelWell.reset();
    EXPECT_EQ( localSegments, lateral.segmentCollection() );
    EXPECT_EQ( localSegments, constLateral.segmentCollection() );
    EXPECT_DOUBLE_EQ( 0.1, localSegments->linerDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
    EXPECT_DOUBLE_EQ( 1.0e-5, localSegments->roughnessFactor( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) );
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
