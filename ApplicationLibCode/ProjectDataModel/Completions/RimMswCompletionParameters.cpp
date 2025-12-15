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
#include "RimCustomSegmentInterval.h"
#include "RimCustomSegmentIntervalCollection.h"
#include "RimDiameterRoughnessIntervalCollection.h"

#include "RimWellPath.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiTableViewEditor.h"

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

template <>
void RimMswCompletionParameters::DiameterRoughnessModeEnum::setUp()
{
    addItem( RimMswCompletionParameters::DiameterRoughnessMode::UNIFORM, "Uniform", "Uniform" );
    addItem( RimMswCompletionParameters::DiameterRoughnessMode::INTERVALS, "Intervals", "Intervals" );
    setDefault( RimMswCompletionParameters::DiameterRoughnessMode::UNIFORM );
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

    // New interval-based fields
    CAF_PDM_InitScriptableFieldNoDefault( &m_diameterRoughnessMode, "DiameterRoughnessMode", "Diameter Roughness Mode" );
    CAF_PDM_InitFieldNoDefault( &m_diameterRoughnessIntervals, "DiameterRoughnessIntervals", "Diameter Roughness Intervals" );
    m_diameterRoughnessIntervals = new RimDiameterRoughnessIntervalCollection();
    m_diameterRoughnessIntervals->intervalsField().uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_diameterRoughnessIntervals->intervalsField().uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_diameterRoughnessIntervals->intervalsField().uiCapability()->setCustomContextMenuEnabled( true );

    // Enable custom context menu on this object as well
    setCustomContextMenuEnabled( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_pressureDrop, "PressureDrop", "Pressure Drop" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lengthAndDepth, "LengthAndDepth", "Length and Depth" );

    CAF_PDM_InitScriptableField( &m_enforceMaxSegmentLength, "EnforceMaxSegmentLength", false, "Enforce Max Segment Length" );
    CAF_PDM_InitScriptableField( &m_maxSegmentLength, "MaxSegmentLength", 200.0, "Max Segment Length" );
    m_maxSegmentLength.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_customSegmentIntervals, "CustomSegmentIntervals", "Custom Segment Intervals" );
    m_customSegmentIntervals = new RimCustomSegmentIntervalCollection();
    m_customSegmentIntervals->intervalsField().uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_customSegmentIntervals->intervalsField().uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_customSegmentIntervals->intervalsField().uiCapability()->setCustomContextMenuEnabled( true );
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
RimMswCompletionParameters::DiameterRoughnessMode RimMswCompletionParameters::diameterRoughnessMode() const
{
    return m_diameterRoughnessMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setDiameterRoughnessMode( DiameterRoughnessMode mode )
{
    m_diameterRoughnessMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswCompletionParameters::isUsingIntervalSpecificValues() const
{
    return m_diameterRoughnessMode() == DiameterRoughnessMode::INTERVALS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    if ( isUsingIntervalSpecificValues() && m_diameterRoughnessIntervals() )
    {
        return m_diameterRoughnessIntervals()->getDiameterAtMD( md, unitSystem );
    }

    // Fall back to single value
    return linerDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    if ( isUsingIntervalSpecificValues() && m_diameterRoughnessIntervals() )
    {
        return m_diameterRoughnessIntervals()->getRoughnessAtMD( md, unitSystem );
    }

    // Fall back to single value
    return roughnessFactor( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessIntervalCollection* RimMswCompletionParameters::diameterRoughnessIntervals() const
{
    return m_diameterRoughnessIntervals();
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
RimCustomSegmentIntervalCollection* RimMswCompletionParameters::customSegmentIntervals() const
{
    return m_customSegmentIntervals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimMswCompletionParameters::getSegmentIntervals() const
{
    std::vector<std::pair<double, double>> result;

    if ( m_customSegmentIntervals() && !m_customSegmentIntervals()->isEmpty() )
    {
        auto intervals = m_customSegmentIntervals()->intervals();
        for ( auto* interval : intervals )
        {
            if ( interval && interval->isValidInterval() )
            {
                result.push_back( { interval->startMD(), interval->endMD() } );
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswCompletionParameters::hasCustomSegmentIntervals() const
{
    return m_customSegmentIntervals() && !m_customSegmentIntervals()->isEmpty();
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

    if ( changedField == &m_diameterRoughnessMode )
    {
        updateAllRequiredEditors();
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

            // Diameter and Roughness section
            auto* diameterRoughnessGroup = uiOrdering.addNewGroup( "Diameter and Roughness" );
            diameterRoughnessGroup->add( &m_diameterRoughnessMode );

            bool usingIntervals = m_diameterRoughnessMode() == DiameterRoughnessMode::INTERVALS;

            if ( usingIntervals )
            {
                diameterRoughnessGroup->add( &m_diameterRoughnessIntervals->intervalsField() );
                m_linerDiameter.uiCapability()->setUiHidden( true );
                m_roughnessFactor.uiCapability()->setUiHidden( true );
            }
            else
            {
                diameterRoughnessGroup->add( &m_linerDiameter );
                diameterRoughnessGroup->add( &m_roughnessFactor );
                m_linerDiameter.uiCapability()->setUiHidden( false );
                m_roughnessFactor.uiCapability()->setUiHidden( false );
            }

            uiOrdering.add( &m_pressureDrop );
            uiOrdering.add( &m_lengthAndDepth );

            // Custom Segment Configuration section
            auto* segmentConfigGroup = uiOrdering.addNewGroup( "Custom Segment Configuration" );
            segmentConfigGroup->add( &m_enforceMaxSegmentLength );
            segmentConfigGroup->add( &m_maxSegmentLength );
            segmentConfigGroup->add( &m_customSegmentIntervals->intervalsField() );
        }
        else
        {
            uiOrdering.add( &m_customValuesForLateral );

            // Diameter and Roughness section for laterals
            auto* diameterRoughnessGroup = uiOrdering.addNewGroup( "Diameter and Roughness" );
            diameterRoughnessGroup->add( &m_diameterRoughnessMode );

            bool usingIntervals = m_diameterRoughnessMode() == DiameterRoughnessMode::INTERVALS;

            if ( usingIntervals )
            {
                diameterRoughnessGroup->add( &m_diameterRoughnessIntervals );
                m_linerDiameter.uiCapability()->setUiHidden( true );
                m_roughnessFactor.uiCapability()->setUiHidden( true );
            }
            else
            {
                diameterRoughnessGroup->add( &m_linerDiameter );
                diameterRoughnessGroup->add( &m_roughnessFactor );
                m_linerDiameter.uiCapability()->setUiHidden( false );
                m_roughnessFactor.uiCapability()->setUiHidden( false );
            }

            bool isReadOnly = !m_customValuesForLateral();
            m_linerDiameter.uiCapability()->setUiReadOnly( isReadOnly );
            m_roughnessFactor.uiCapability()->setUiReadOnly( isReadOnly );
            m_diameterRoughnessMode.uiCapability()->setUiReadOnly( isReadOnly );
            m_diameterRoughnessIntervals.uiCapability()->setUiReadOnly( isReadOnly );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget )
{
    if ( fieldNeedingMenu == &m_diameterRoughnessIntervals )
    {
        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "RicNewDiameterRoughnessIntervalFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteDiameterRoughnessIntervalFeature";

        menuBuilder.appendToMenu( menu );
    }

    if ( fieldNeedingMenu == &m_customSegmentIntervals )
    {
        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "RicNewCustomSegmentIntervalFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteCustomSegmentIntervalFeature";

        menuBuilder.appendToMenu( menu );
    }
}
