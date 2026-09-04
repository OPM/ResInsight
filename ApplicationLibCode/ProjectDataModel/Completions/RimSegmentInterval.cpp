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

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSegmentInterval, "SegmentInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSegmentInterval::RimSegmentInterval()
{
    CAF_PDM_InitScriptableObject( "Segment Interval", ":/WellPathComponent16x16.png", "", "SegmentInterval" );
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
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToInch( m_diameter );
    }
    return m_diameter; // METRIC units - already in meters
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
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( m_roughnessFactor );
    }
    return m_roughnessFactor; // METRIC units - already in meters
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

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSegmentInterval::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
    uiOrdering.add( &m_diameter );
    uiOrdering.add( &m_roughnessFactor );
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
