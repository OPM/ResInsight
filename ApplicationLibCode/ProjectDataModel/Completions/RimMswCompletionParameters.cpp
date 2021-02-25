/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Equinor ASA
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
#include "RimMswCompletionParameters.h"

#include "RiaEclipseUnitTools.h"

#include "RimWellPath.h"

#include "cafPdmUiObjectEditorHandle.h"

#include <limits>

namespace caf
{
template <>
void RimMswCompletionParameters::ReferenceMDEnum::setUp()
{
    addItem( RimMswCompletionParameters::AUTO_REFERENCE_MD, "GridIntersectionRefMD", "Grid Entry Point" );
    addItem( RimMswCompletionParameters::MANUAL_REFERENCE_MD, "ManualRefMD", "User Defined" );
    setDefault( RimMswCompletionParameters::AUTO_REFERENCE_MD );
}

template <>
void RimMswCompletionParameters::PressureDropEnum::setUp()
{
    addItem( RimMswCompletionParameters::HYDROSTATIC, "H--", "Hydrostatic" );
    addItem( RimMswCompletionParameters::HYDROSTATIC_FRICTION, "HF-", "Hydrostatic + Friction" );
    addItem( RimMswCompletionParameters::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration" );
    setDefault( RimMswCompletionParameters::HYDROSTATIC_FRICTION );
}

template <>
void RimMswCompletionParameters::LengthAndDepthEnum::setUp()
{
    addItem( RimMswCompletionParameters::INC, "INC", "Incremental" );
    addItem( RimMswCompletionParameters::ABS, "ABS", "Absolute" );
    setDefault( RimMswCompletionParameters::INC );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimMswCompletionParameters, "RimMswCompletionParameters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::RimMswCompletionParameters( bool enableReferenceDepth /* = true */ )
    : m_enableReferenceDepth( enableReferenceDepth )
{
    CAF_PDM_InitObject( "MSW Completion Parameters", ":/CompletionsSymbol16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_refMDType, "RefMDType", "Reference MD", "", "", "" );
    CAF_PDM_InitField( &m_refMD, "RefMD", 0.0, "", "", "", "" );

    if ( !m_enableReferenceDepth )
    {
        m_refMDType.xmlCapability()->disableIO();
        m_refMD.xmlCapability()->disableIO();
    }

    CAF_PDM_InitField( &m_linerDiameter,
                       "LinerDiameter",
                       std::numeric_limits<double>::infinity(),
                       "Liner Inner Diameter",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_roughnessFactor,
                       "RoughnessFactor",
                       std::numeric_limits<double>::infinity(),
                       "Roughness Factor",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_pressureDrop, "PressureDrop", "Pressure Drop", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "" );

    CAF_PDM_InitField( &m_enforceMaxSegmentLength, "EnforceMaxSegmentLength", false, "Enforce Max Segment Length", "", "", "" );
    CAF_PDM_InitField( &m_maxSegmentLength, "MaxSegmentLength", 200.0, "Max Segment Length", "", "", "" );
    m_maxSegmentLength.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::~RimMswCompletionParameters()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswCompletionParameters::isDefault() const
{
    return m_refMDType() == ReferenceMDEnum() && m_refMD() == m_refMD.defaultValue() &&
           m_linerDiameter() == m_linerDiameter.defaultValue() &&
           m_roughnessFactor() == m_roughnessFactor.defaultValue() && m_pressureDrop == PressureDropEnum() &&
           m_enforceMaxSegmentLength() == m_enforceMaxSegmentLength.defaultValue() &&
           m_maxSegmentLength() == m_maxSegmentLength.defaultValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters& RimMswCompletionParameters::operator=( const RimMswCompletionParameters& rhs )
{
    m_refMDType               = rhs.m_refMDType();
    m_refMD                   = rhs.m_refMD();
    m_linerDiameter           = rhs.m_linerDiameter();
    m_roughnessFactor         = rhs.m_roughnessFactor();
    m_pressureDrop            = rhs.m_pressureDrop();
    m_enforceMaxSegmentLength = rhs.m_enforceMaxSegmentLength();
    m_maxSegmentLength        = rhs.m_maxSegmentLength();
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::ReferenceMDType RimMswCompletionParameters::referenceMDType() const
{
    return m_refMDType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::manualReferenceMD() const
{
    if ( m_refMDType == AUTO_REFERENCE_MD )
    {
        return std::numeric_limits<double>::infinity();
    }
    return m_refMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::linerDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted( wellPath );
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD &&
         unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_linerDiameter() );
    }
    else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC &&
              unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( m_linerDiameter() );
    }
    return m_linerDiameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::defaultLinerDiameter( RiaDefines::EclipseUnitSystem unitSystem )
{
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return 0.152;
    }
    else
    {
        return 0.5;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted( wellPath );
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD &&
         unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_roughnessFactor() );
    }
    else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC &&
              unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( m_roughnessFactor() );
    }
    return m_roughnessFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::defaultRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem )
{
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return 1.0e-5;
    }
    else
    {
        return 3.28e-5;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::PressureDropEnum RimMswCompletionParameters::pressureDrop() const
{
    return m_pressureDrop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::LengthAndDepthEnum RimMswCompletionParameters::lengthAndDepth() const
{
    return m_lengthAndDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::maxSegmentLength() const
{
    return m_enforceMaxSegmentLength ? m_maxSegmentLength : std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setReferenceMDType( ReferenceMDType refType )
{
    m_refMDType = refType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setManualReferenceMD( double manualRefMD )
{
    m_refMD = manualRefMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setLinerDiameter( double diameter )
{
    m_linerDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setRoughnessFactor( double roughnessFactor )
{
    m_roughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setPressureDrop( PressureDropType pressureDropType )
{
    m_pressureDrop = pressureDropType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setLengthAndDepth( LengthAndDepthType lengthAndDepthType )
{
    m_lengthAndDepth = lengthAndDepthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    if ( changedField == &m_refMDType )
    {
        m_refMD.uiCapability()->setUiHidden( m_refMDType == AUTO_REFERENCE_MD );
        this->updateAllRequiredEditors();
    }

    if ( changedField == &m_enforceMaxSegmentLength )
    {
        m_maxSegmentLength.uiCapability()->setUiHidden( !m_enforceMaxSegmentLength() );
        caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType( wellPath );
        if ( wellPath )
        {
            if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
            {
                m_linerDiameter.uiCapability()->setUiName( "Liner Inner Diameter [m]" );
                m_roughnessFactor.uiCapability()->setUiName( "Roughness Factor [m]" );
            }
            else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
            {
                m_linerDiameter.uiCapability()->setUiName( "Liner Inner Diameter [ft]" );
                m_roughnessFactor.uiCapability()->setUiName( "Roughness Factor [ft]" );
            }
        }
    }

    if ( m_enableReferenceDepth )
    {
        uiOrdering.add( &m_refMDType );
        uiOrdering.add( &m_refMD );
        m_refMD.uiCapability()->setUiHidden( m_refMDType == AUTO_REFERENCE_MD );
    }

    uiOrdering.add( &m_linerDiameter );
    uiOrdering.add( &m_roughnessFactor );
    uiOrdering.add( &m_pressureDrop );
    uiOrdering.add( &m_lengthAndDepth );
    uiOrdering.add( &m_enforceMaxSegmentLength );
    uiOrdering.add( &m_maxSegmentLength );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::initAfterRead()
{
    if ( m_linerDiameter() == std::numeric_limits<double>::infinity() &&
         m_roughnessFactor() == std::numeric_limits<double>::infinity() )
    {
        setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType( wellPath );
    if ( wellPath )
    {
        m_linerDiameter   = defaultLinerDiameter( wellPath->unitSystem() );
        m_roughnessFactor = defaultRoughnessFactor( wellPath->unitSystem() );
    }
}
