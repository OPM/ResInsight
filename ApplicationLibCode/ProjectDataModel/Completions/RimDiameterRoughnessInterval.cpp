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
    m_startMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    m_endMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", defaultDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ), "Diameter" );
    m_diameter.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_roughnessFactor,
                                 "RoughnessFactor",
                                 defaultRoughness( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
                                 "Roughness Factor" );
    m_roughnessFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    setUiName( "Diameter Roughness Interval" );
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
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::inchToMeter( m_diameter );
    }
    return m_diameter; // FIELD units - already in inches
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
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_roughnessFactor );
    }
    return m_roughnessFactor; // FIELD units - already in feet
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

    return !( m_endMD <= other->startMD() || m_startMD >= other->endMD() );
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
    return QString( "%1 in" ).arg( m_diameter, 0, 'f', 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDiameterRoughnessInterval::roughnessLabel() const
{
    return QString( "%1 ft" ).arg( m_roughnessFactor, 0, 'e', 2 );
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
    return QString( "MD %.1f-%.1f: D=%.3f\", R=%1e" ).arg( m_startMD ).arg( m_endMD ).arg( m_diameter ).arg( m_roughnessFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::defaultDiameter( RiaDefines::EclipseUnitSystem unitSystem )
{
    // Default diameter in inches (standard for completion design)
    return 7.0; // 7 inch liner diameter as common default
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessInterval::defaultRoughness( RiaDefines::EclipseUnitSystem unitSystem )
{
    // Default roughness factor in feet
    return 1e-4; // Common roughness value for steel pipe
}
