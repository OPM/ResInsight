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

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiObjectEditorHandle.h"

#include <limits>

namespace caf
{
template <>
void RimMswCompletionParameters::ReferenceMDEnum::setUp()
{
    addItem( RimMswCompletionParameters::ReferenceMDType::AUTO_REFERENCE_MD, "GridEntryPoint", "Grid Entry Point", { "GridIntersectionRefMD" } );
    addItem( RimMswCompletionParameters::ReferenceMDType::MANUAL_REFERENCE_MD, "UserDefined", "User Defined", { "ManualRefMD" } );
    setDefault( RimMswCompletionParameters::ReferenceMDType::AUTO_REFERENCE_MD );
}

template <>
void RimMswCompletionParameters::PressureDropEnum::setUp()
{
    addItem( RimMswCompletionParameters::PressureDropType::HYDROSTATIC, "H--", "Hydrostatic" );
    addItem( RimMswCompletionParameters::PressureDropType::HYDROSTATIC_FRICTION, "HF-", "Hydrostatic + Friction" );
    addItem( RimMswCompletionParameters::PressureDropType::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration" );
    setDefault( RimMswCompletionParameters::PressureDropType::HYDROSTATIC_FRICTION );
}

template <>
void RimMswCompletionParameters::LengthAndDepthEnum::setUp()
{
    addItem( RimMswCompletionParameters::LengthAndDepthType::INC, "INC", "Incremental" );
    addItem( RimMswCompletionParameters::LengthAndDepthType::ABS, "ABS", "Absolute" );
    setDefault( RimMswCompletionParameters::LengthAndDepthType::ABS );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimMswCompletionParameters, "RimMswCompletionParameters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::RimMswCompletionParameters()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "MSW Completion Parameters",
                                                    ":/CompletionsSymbol16x16.png",
                                                    "",
                                                    "",
                                                    "MswSettings",
                                                    "Multi Segment Well Completion Settings" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_refMDType, "RefMDType", "ReferenceMdType", "" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_refMD, "RefMD", "UserDefinedReferenceMd", 0.0, "User Defined Reference MD" );

    CAF_PDM_InitScriptableField( &m_customValuesForLateral, "CustomValuesForLateral", false, "Custom Values for Lateral" );

    const auto unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    CAF_PDM_InitScriptableField( &m_linerDiameter,
                                 "LinerDiameter",
                                 RimMswCompletionParameters::defaultLinerDiameter( unitSystem ),
                                 "Liner Inner Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor,
                                 "RoughnessFactor",
                                 RimMswCompletionParameters::defaultRoughnessFactor( unitSystem ),
                                 "Roughness Factor" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_pressureDrop, "PressureDrop", "Pressure Drop" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lengthAndDepth, "LengthAndDepth", "Length and Depth" );

    CAF_PDM_InitScriptableField( &m_enforceMaxSegmentLength, "EnforceMaxSegmentLength", false, "Enforce Max Segment Length" );
    CAF_PDM_InitScriptableField( &m_maxSegmentLength, "MaxSegmentLength", 200.0, "Max Segment Length" );
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
    if ( m_refMDType == ReferenceMDType::AUTO_REFERENCE_MD )
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
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    double diameter = m_linerDiameter();
    if ( !wellPath->isTopLevelWellPath() && !m_customValuesForLateral )
    {
        diameter = wellPath->topLevelWellPath()->mswCompletionParameters()->m_linerDiameter();
    }

    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( diameter );
    }
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( diameter );
    }
    return diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::linerDiameter() const
{
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

    return 0.5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    double rFactor = m_roughnessFactor();
    if ( !wellPath->isTopLevelWellPath() && !m_customValuesForLateral )
    {
        rFactor = wellPath->topLevelWellPath()->mswCompletionParameters()->m_roughnessFactor();
    }

    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( rFactor );
    }
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( rFactor );
    }

    return rFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::roughnessFactor() const
{
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

    return 3.28e-5;
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
void RimMswCompletionParameters::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_refMDType )
    {
        m_refMD.uiCapability()->setUiHidden( m_refMDType == ReferenceMDType::AUTO_REFERENCE_MD );
        updateAllRequiredEditors();
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
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
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

        if ( wellPath->isTopLevelWellPath() )
        {
            uiOrdering.add( &m_refMDType );
            uiOrdering.add( &m_refMD );
            m_refMD.uiCapability()->setUiHidden( m_refMDType == ReferenceMDType::AUTO_REFERENCE_MD );

            uiOrdering.add( &m_linerDiameter );
            uiOrdering.add( &m_roughnessFactor );
            uiOrdering.add( &m_pressureDrop );
            uiOrdering.add( &m_lengthAndDepth );
            uiOrdering.add( &m_enforceMaxSegmentLength );
            uiOrdering.add( &m_maxSegmentLength );
        }
        else
        {
            uiOrdering.add( &m_customValuesForLateral );

            uiOrdering.add( &m_linerDiameter );
            uiOrdering.add( &m_roughnessFactor );

            m_linerDiameter.uiCapability()->setUiReadOnly( !m_customValuesForLateral() );
            m_linerDiameter.uiCapability()->setUiReadOnly( !m_customValuesForLateral() );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::initAfterRead()
{
    if ( m_linerDiameter() == std::numeric_limits<double>::infinity() && m_roughnessFactor() == std::numeric_limits<double>::infinity() )
    {
        setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setUnitSystemSpecificDefaults()
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( wellPath )
    {
        m_linerDiameter   = defaultLinerDiameter( wellPath->unitSystem() );
        m_roughnessFactor = defaultRoughnessFactor( wellPath->unitSystem() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::updateFromTopLevelWell( const RimMswCompletionParameters* topLevelWellParameters )
{
    m_refMDType               = topLevelWellParameters->m_refMDType();
    m_refMD                   = topLevelWellParameters->m_refMD();
    m_pressureDrop            = topLevelWellParameters->m_pressureDrop();
    m_enforceMaxSegmentLength = topLevelWellParameters->m_enforceMaxSegmentLength();
    m_maxSegmentLength        = topLevelWellParameters->m_maxSegmentLength();

    // The following parameters can be defined per lateral
    /*
        m_linerDiameter           = rhs.m_linerDiameter();
        m_roughnessFactor         = rhs.m_roughnessFactor();
    */
}
