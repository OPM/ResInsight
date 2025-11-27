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

#include "RimDiameterRoughnessInterval.h"

#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimDiameterRoughnessInterval, "DiameterRoughnessInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessInterval::RimDiameterRoughnessInterval()
{
    CAF_PDM_InitScriptableObject( "Diameter Roughness Interval", ":/WellPathComponent16x16.png", "", "DiameterRoughnessInterval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    CAF_PDM_InitScriptableField( &m_diameter,
                                 "Diameter",
                                 RimMswCompletionParameters::defaultLinerDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                                 "Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor,
                                 "RoughnessFactor",
                                 RimMswCompletionParameters::defaultRoughnessFactor( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                                 "Roughness Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessInterval::~RimDiameterRoughnessInterval()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::diameter() const
{
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::diameter( RiaDefines::EclipseUnitSystem unitSystem ) const
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
double RimDiameterRoughnessInterval::roughnessFactor() const
{
    return m_roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
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
void RimDiameterRoughnessInterval::setStartMD( double startMD )
{
    m_startMD = startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::setEndMD( double endMD )
{
    m_endMD = endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::setDiameter( double diameter )
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::setRoughnessFactor( double roughness )
{
    m_roughnessFactor = roughness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessInterval::isValidInterval() const
{
    return m_endMD > m_startMD && m_diameter > 0.0 && m_roughnessFactor >= 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessInterval::overlaps( const RimDiameterRoughnessInterval* other ) const
{
    if ( !other ) return false;

    return m_endMD > other->startMD() && m_startMD < other->endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessInterval::containsMD( double md ) const
{
    return md >= m_startMD && md <= m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDiameterRoughnessInterval::diameterLabel() const
{
    return QString( "%1 m" ).arg( m_diameter(), 0, 'f', 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDiameterRoughnessInterval::roughnessLabel() const
{
    return QString( "%1 m" ).arg( m_roughnessFactor(), 0, 'e', 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessInterval::operator<( const RimDiameterRoughnessInterval& rhs ) const
{
    return m_startMD < rhs.m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessInterval::isEnabled() const
{
    return true; // Always enabled for now
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimDiameterRoughnessInterval::componentType() const
{
    return RiaDefines::WellPathComponentType::CASING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDiameterRoughnessInterval::componentLabel() const
{
    return generateDisplayLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDiameterRoughnessInterval::componentTypeLabel() const
{
    return "Diameter/Roughness";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimDiameterRoughnessInterval::defaultComponentColor() const
{
    return cvf::Color3f( 0.6f, 0.4f, 0.2f ); // Brown color for diameter/roughness intervals
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::applyOffset( double offsetMD )
{
    m_startMD = m_startMD + offsetMD;
    m_endMD   = m_endMD + offsetMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_startMD || changedField == &m_endMD )
    {
        // Validate interval
        if ( m_startMD >= m_endMD )
        {
            RiaLogging::warning( "Invalid interval: Start MD must be less than End MD" );
        }

        // Update overlap visual feedback in parent collection
        auto* collection = firstAncestorOrThisOfType<RimDiameterRoughnessIntervalCollection>();
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
void RimDiameterRoughnessInterval::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
    uiOrdering.add( &m_diameter );
    uiOrdering.add( &m_roughnessFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::updateConnectedEditors()
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
QString RimDiameterRoughnessInterval::generateDisplayLabel() const
{
    return QString( "MD %.1f-%.1f: D=%.3fm, R=%1em" ).arg( m_startMD() ).arg( m_endMD() ).arg( m_diameter() ).arg( m_roughnessFactor() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::defaultDiameter( RiaDefines::EclipseUnitSystem unitSystem )
{
    return RimMswCompletionParameters::defaultLinerDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::defaultRoughness( RiaDefines::EclipseUnitSystem unitSystem )
{
    return RimMswCompletionParameters::defaultRoughnessFactor( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessInterval::updateOverlapVisualFeedback( bool hasOverlap )
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
