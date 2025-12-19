/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimCustomSegmentInterval.h"

#include "RiaLogging.h"

#include "RimCustomSegmentIntervalCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimCustomSegmentInterval, "CustomSegmentInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentInterval::RimCustomSegmentInterval()
{
    CAF_PDM_InitScriptableObject( "Custom Segment Interval", ":/WellPathComponent16x16.png", "", "CustomSegmentInterval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 100.0, "End MD" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentInterval::~RimCustomSegmentInterval()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimCustomSegmentInterval::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimCustomSegmentInterval::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::setStartMD( double startMD )
{
    m_startMD = startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::setEndMD( double endMD )
{
    m_endMD = endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimCustomSegmentInterval::validate( const QString& configName ) const
{
    // First validate all fields using base implementation
    auto errors = PdmObject::validate( configName );

    // Validate that end MD is greater than start MD
    if ( m_endMD <= m_startMD )
    {
        errors["EndMd"] = "End MD must be greater than Start MD";
    }

    return errors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomSegmentInterval::isValidInterval() const
{
    return m_endMD > m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomSegmentInterval::overlaps( const RimCustomSegmentInterval* other ) const
{
    if ( !other ) return false;

    return m_endMD > other->startMD() && m_startMD < other->endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomSegmentInterval::containsMD( double md ) const
{
    return md >= m_startMD && md <= m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomSegmentInterval::operator<( const RimCustomSegmentInterval& rhs ) const
{
    return m_startMD < rhs.m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomSegmentInterval::isEnabled() const
{
    return true; // Always enabled
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimCustomSegmentInterval::componentType() const
{
    return RiaDefines::WellPathComponentType::PERFORATION_INTERVAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomSegmentInterval::componentLabel() const
{
    return generateDisplayLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomSegmentInterval::componentTypeLabel() const
{
    return "Custom Segment";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimCustomSegmentInterval::defaultComponentColor() const
{
    return cvf::Color3f( 0.2f, 0.6f, 0.8f ); // Blue color for custom segments
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::applyOffset( double offsetMD )
{
    m_startMD = m_startMD + offsetMD;
    m_endMD   = m_endMD + offsetMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_startMD || changedField == &m_endMD )
    {
        // Validate interval
        if ( m_startMD >= m_endMD )
        {
            RiaLogging::warning( "Invalid interval: Start MD must be less than End MD" );
        }

        // Update overlap visual feedback in parent collection
        auto* collection = firstAncestorOrThisOfType<RimCustomSegmentIntervalCollection>();
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
void RimCustomSegmentInterval::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::updateConnectedEditors()
{
    // Update any connected UI editors
    m_startMD.uiCapability()->updateConnectedEditors();
    m_endMD.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomSegmentInterval::generateDisplayLabel() const
{
    return QString( "MD %.1f-%.1f" ).arg( m_startMD() ).arg( m_endMD() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentInterval::updateOverlapVisualFeedback( bool hasOverlap )
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
