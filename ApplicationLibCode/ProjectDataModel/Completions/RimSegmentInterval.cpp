/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimSegmentInterval.h"

#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "RimSegmentCollection.h"
#include "RimWellPath.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSegmentInterval, "SegmentInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSegmentInterval::RimSegmentInterval()
{
    CAF_PDM_InitScriptableObject( "Segment Interval", ":/Segment.svg", "", "SegmentInterval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    CAF_PDM_InitScriptableField( &m_diameter,
                                 "Diameter",
                                 RimSegmentCollection::defaultLinerDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                                 "Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor,
                                 "RoughnessFactor",
                                 RimSegmentCollection::defaultRoughnessFactor( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                                 "Roughness Factor" );

    CAF_PDM_InitField( &m_fixedSegmentLength, "FixedSegmentLength", std::make_pair( false, 50.0 ), "Fixed Segment Length" );
    m_fixedSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_minSegmentLength, "MinSegmentLength", std::make_pair( false, 10.0 ), "Min Segment Length" );
    m_minSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_maxSegmentLength, "MaxSegmentLength", std::make_pair( false, 100.0 ), "Max Segment Length" );
    m_maxSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_useCustomStartDate, "UseCustomStartDate", false, "Custom Start Date" );
    CAF_PDM_InitField( &m_startDate, "StartDate", QDateTime::currentDateTime(), "Start Date" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSegmentInterval::~RimSegmentInterval()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::diameter() const
{
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::diameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto* wellPath         = firstAncestorOrThisOfType<RimWellPath>();
    auto  sourceUnitSystem = wellPath ? wellPath->unitSystem() : RiaDefines::EclipseUnitSystem::UNITS_METRIC;

    if ( sourceUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        return RiaEclipseUnitTools::feetToMeter( m_diameter );
    if ( sourceUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        return RiaEclipseUnitTools::meterToFeet( m_diameter );
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::roughnessFactor() const
{
    return m_roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto* wellPath         = firstAncestorOrThisOfType<RimWellPath>();
    auto  sourceUnitSystem = wellPath ? wellPath->unitSystem() : RiaDefines::EclipseUnitSystem::UNITS_METRIC;

    if ( sourceUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        return RiaEclipseUnitTools::feetToMeter( m_roughnessFactor );
    if ( sourceUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        return RiaEclipseUnitTools::meterToFeet( m_roughnessFactor );
    return m_roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setStartMD( double startMD )
{
    m_startMD = startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setEndMD( double endMD )
{
    m_endMD = endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setDiameter( double diameter )
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setRoughnessFactor( double roughness )
{
    m_roughnessFactor = roughness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimSegmentInterval::fixedSegmentLength() const
{
    if ( m_fixedSegmentLength().first && m_fixedSegmentLength().second > 0.0 ) return m_fixedSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimSegmentInterval::minSegmentLength() const
{
    if ( m_minSegmentLength().first && m_minSegmentLength().second > 0.0 ) return m_minSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimSegmentInterval::maxSegmentLength() const
{
    if ( m_maxSegmentLength().first && m_maxSegmentLength().second > 0.0 ) return m_maxSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setFixedSegmentLength( std::optional<double> length )
{
    m_fixedSegmentLength = std::make_pair( length.has_value(), length.value_or( m_fixedSegmentLength().second ) );
    if ( length ) enforceSegmentationRules( &m_fixedSegmentLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setMinSegmentLength( std::optional<double> length )
{
    m_minSegmentLength = std::make_pair( length.has_value(), length.value_or( m_minSegmentLength().second ) );
    if ( length ) enforceSegmentationRules( &m_minSegmentLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setMaxSegmentLength( std::optional<double> length )
{
    m_maxSegmentLength = std::make_pair( length.has_value(), length.value_or( m_maxSegmentLength().second ) );
    if ( length ) enforceSegmentationRules( &m_maxSegmentLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::enableCustomStartDate( bool enable )
{
    m_useCustomStartDate = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setCustomStartDate( const QDate& date )
{
    if ( date.isValid() )
    {
        m_startDate = RiaQDateTimeTools::createDateTime( date );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::setCustomStartDate( const QDateTime& date )
{
    m_startDate = date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::isActiveOnDate( const QDateTime& date ) const
{
    return !( m_useCustomStartDate() && date < m_startDate() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::isValidInterval() const
{
    return m_endMD > m_startMD && m_diameter > 0.0 && m_roughnessFactor >= 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::overlaps( const RimSegmentInterval* other ) const
{
    if ( !other ) return false;

    return m_endMD > other->startMD() && m_startMD < other->endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::containsMD( double md ) const
{
    return md >= m_startMD && md <= m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSegmentInterval::diameterLabel() const
{
    return QString( "%1 m" ).arg( m_diameter(), 0, 'f', 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSegmentInterval::roughnessLabel() const
{
    return QString( "%1 m" ).arg( m_roughnessFactor(), 0, 'e', 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::operator<( const RimSegmentInterval& rhs ) const
{
    return m_startMD < rhs.m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSegmentInterval::isEnabled() const
{
    return true; // Always enabled for now
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimSegmentInterval::componentType() const
{
    return RiaDefines::WellPathComponentType::CASING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSegmentInterval::componentLabel() const
{
    return generateDisplayLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSegmentInterval::componentTypeLabel() const
{
    return "Segment";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSegmentInterval::defaultComponentColor() const
{
    return cvf::Color3f( 0.6f, 0.4f, 0.2f ); // Brown color for segment intervals
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::applyOffset( double offsetMD )
{
    m_startMD = m_startMD + offsetMD;
    m_endMD   = m_endMD + offsetMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicDeleteSegmentIntervalFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_startMD || changedField == &m_endMD )
    {
        updateUiName();

        // Validate interval
        if ( m_startMD >= m_endMD )
        {
            RiaLogging::warning( "Invalid interval: Start MD must be less than End MD" );
        }

        // Update overlap visual feedback in parent collection
        auto* collection = firstAncestorOrThisOfType<RimSegmentCollection>();
        if ( collection )
        {
            collection->updateOverlapVisualFeedback();
        }
    }

    const bool segmentationEnabled = ( changedField == &m_fixedSegmentLength && m_fixedSegmentLength().first ) ||
                                     ( changedField == &m_minSegmentLength && m_minSegmentLength().first ) ||
                                     ( changedField == &m_maxSegmentLength && m_maxSegmentLength().first );
    if ( segmentationEnabled ) enforceSegmentationRules( changedField );

    if ( changedField == &m_fixedSegmentLength || changedField == &m_minSegmentLength || changedField == &m_maxSegmentLength )
    {
        updateUiName();
        uiCapability()->updateConnectedEditors();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto* wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        const bool isMetric = wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        m_startMD.uiCapability()->setUiName( isMetric ? "Start MD [m]" : "Start MD [ft]" );
        m_endMD.uiCapability()->setUiName( isMetric ? "End MD [m]" : "End MD [ft]" );
        m_diameter.uiCapability()->setUiName( isMetric ? "Diameter [m]" : "Diameter [ft]" );
        m_roughnessFactor.uiCapability()->setUiName( isMetric ? "Roughness Factor [m]" : "Roughness Factor [ft]" );
        m_fixedSegmentLength.uiCapability()->setUiName( isMetric ? "Fixed Segment Length [m]" : "Fixed Segment Length [ft]" );
        m_minSegmentLength.uiCapability()->setUiName( isMetric ? "Min Segment Length [m]" : "Min Segment Length [ft]" );
        m_maxSegmentLength.uiCapability()->setUiName( isMetric ? "Max Segment Length [m]" : "Max Segment Length [ft]" );
    }

    auto* intervalGroup = uiOrdering.addNewGroup( "Interval" );
    intervalGroup->add( &m_startMD );
    intervalGroup->add( &m_endMD );

    auto* diameterGroup = uiOrdering.addNewGroup( "Diameter and Roughness" );
    diameterGroup->add( &m_diameter );
    diameterGroup->add( &m_roughnessFactor );

    auto* segmentationGroup = uiOrdering.addNewGroup( "Segmentation" );
    segmentationGroup->add( &m_fixedSegmentLength );
    segmentationGroup->add( &m_minSegmentLength );
    segmentationGroup->add( &m_maxSegmentLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    updateUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::updateUiName()
{
    QString name = QString( "%1 - %2" ).arg( m_startMD() ).arg( m_endMD() );
    if ( auto fixedLength = fixedSegmentLength() ) name += QString( " (fixed %1)" ).arg( *fixedLength );
    if ( auto minLength = minSegmentLength() ) name += QString( " (min %1)" ).arg( *minLength );
    if ( auto maxLength = maxSegmentLength() ) name += QString( " (max %1)" ).arg( *maxLength );
    uiCapability()->setUiName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::updateConnectedEditors()
{
    // Update any connected UI editors
    m_startMD.uiCapability()->updateConnectedEditors();
    m_endMD.uiCapability()->updateConnectedEditors();
    m_diameter.uiCapability()->updateConnectedEditors();
    m_roughnessFactor.uiCapability()->updateConnectedEditors();
    m_fixedSegmentLength.uiCapability()->updateConnectedEditors();
    m_minSegmentLength.uiCapability()->updateConnectedEditors();
    m_maxSegmentLength.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Fixed length cannot be combined with min/max length. Min and max length can be combined.
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::enforceSegmentationRules( const caf::PdmFieldHandle* activeField )
{
    if ( activeField == &m_fixedSegmentLength )
    {
        m_minSegmentLength = std::make_pair( false, m_minSegmentLength().second );
        m_maxSegmentLength = std::make_pair( false, m_maxSegmentLength().second );
    }
    else
    {
        m_fixedSegmentLength = std::make_pair( false, m_fixedSegmentLength().second );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSegmentInterval::generateDisplayLabel() const
{
    return QString( "MD %.1f-%.1f: D=%.3fm, R=%1em" ).arg( m_startMD() ).arg( m_endMD() ).arg( m_diameter() ).arg( m_roughnessFactor() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::defaultDiameter( RiaDefines::EclipseUnitSystem unitSystem )
{
    return RimSegmentCollection::defaultLinerDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSegmentInterval::defaultRoughness( RiaDefines::EclipseUnitSystem unitSystem )
{
    return RimSegmentCollection::defaultRoughnessFactor( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::updateOverlapVisualFeedback( bool hasOverlap )
{
    if ( hasOverlap )
    {
        // Set red color for overlapping fields
        m_startMD.uiCapability()->setUiContentTextColor( Qt::red );
        m_endMD.uiCapability()->setUiContentTextColor( Qt::red );

        // Create tooltip with overlap information
        QString tooltip = "This interval overlaps with another interval!";

        m_startMD.uiCapability()->setUiToolTip( tooltip );
        m_endMD.uiCapability()->setUiToolTip( tooltip );
    }
    else
    {
        // Reset color and tooltip
        m_startMD.uiCapability()->setUiContentTextColor( QColor() );
        m_endMD.uiCapability()->setUiContentTextColor( QColor() );
        m_startMD.uiCapability()->setUiToolTip( "" );
        m_endMD.uiCapability()->setUiToolTip( "" );
    }
}
