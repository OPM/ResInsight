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

#include "RimSegmentCollection.h"

#include "RiaEclipseUnitTools.h"
#include "RimCustomSegmentInterval.h"
#include "RimCustomSegmentIntervalCollection.h"
#include "RimDiameterRoughnessInterval.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimSegmentInterval.h"
#include "RimWellPath.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace caf
{
template <>
void RimSegmentCollection::ReferenceMDEnum::setUp()
{
    addItem( RimSegmentCollection::ReferenceMDType::AUTO_REFERENCE_MD, "GridEntryPoint", "Grid Entry Point", { "GridIntersectionRefMD" } );
    addItem( RimSegmentCollection::ReferenceMDType::MANUAL_REFERENCE_MD, "UserDefined", "User Defined", { "ManualRefMD" } );
    setDefault( RimSegmentCollection::ReferenceMDType::AUTO_REFERENCE_MD );
}

template <>
void RimSegmentCollection::PressureDropEnum::setUp()
{
    addItem( RimSegmentCollection::PressureDropType::HYDROSTATIC, "H--", "Hydrostatic" );
    addItem( RimSegmentCollection::PressureDropType::HYDROSTATIC_FRICTION, "HF-", "Hydrostatic + Friction" );
    addItem( RimSegmentCollection::PressureDropType::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration" );
    setDefault( RimSegmentCollection::PressureDropType::HYDROSTATIC_FRICTION );
}

template <>
void RimSegmentCollection::LengthAndDepthEnum::setUp()
{
    addItem( RimSegmentCollection::LengthAndDepthType::INC, "INC", "Incremental" );
    addItem( RimSegmentCollection::LengthAndDepthType::ABS, "ABS", "Absolute" );
    setDefault( RimSegmentCollection::LengthAndDepthType::ABS );
}

template <>
void RimSegmentCollection::DiameterRoughnessModeEnum::setUp()
{
    addItem( RimSegmentCollection::DiameterRoughnessMode::UNIFORM, "Uniform", "Uniform" );
    addItem( RimSegmentCollection::DiameterRoughnessMode::INTERVALS, "Intervals", "Intervals" );
    setDefault( RimSegmentCollection::DiameterRoughnessMode::UNIFORM );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimSegmentCollection, "SegmentCollection" );

RimSegmentCollection::RimSegmentCollection()
{
    CAF_PDM_InitScriptableObject( "Segments", ":/CompletionsSymbol16x16.png", "", "SegmentCollection" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_refMDType, "RefMDType", "ReferenceMdType", "Reference MD Type" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_refMD, "RefMD", "UserDefinedReferenceMd", 0.0, "User Defined Reference MD" );
    CAF_PDM_InitScriptableField( &m_customValuesForLateral, "CustomValuesForLateral", false, "Custom Values for Lateral" );

    const auto unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    CAF_PDM_InitScriptableField( &m_linerDiameter, "LinerDiameter", defaultLinerDiameter( unitSystem ), "Liner Inner Diameter" );
    CAF_PDM_InitScriptableField( &m_roughnessFactor, "RoughnessFactor", defaultRoughnessFactor( unitSystem ), "Roughness Factor" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_diameterRoughnessMode, "DiameterRoughnessMode", "Diameter Roughness Mode" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_segmentIntervals, "Intervals", "Segment Intervals" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_pressureDrop, "PressureDrop", "Pressure Drop" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lengthAndDepth, "LengthAndDepth", "Length and Depth" );
    CAF_PDM_InitScriptableField( &m_enforceMaxSegmentLength, "EnforceMaxSegmentLength", false, "Enforce Max Segment Length" );
    CAF_PDM_InitScriptableField( &m_maxSegmentLength, "MaxSegmentLength", 200.0, "Max Segment Length" );
    m_maxSegmentLength.uiCapability()->setUiHidden( true );
}

RimSegmentCollection::ReferenceMDType RimSegmentCollection::referenceMDType() const
{
    return m_refMDType();
}

double RimSegmentCollection::manualReferenceMD() const
{
    if ( m_refMDType == ReferenceMDType::AUTO_REFERENCE_MD ) return std::numeric_limits<double>::infinity();
    return m_refMD();
}

double RimSegmentCollection::linerDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto*  wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    double diameter = m_linerDiameter();
    if ( !wellPath->isTopLevelWellPath() && !m_customValuesForLateral() )
    {
        diameter = wellPath->topLevelWellPath()->segmentCollection()->m_linerDiameter();
    }

    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        return RiaEclipseUnitTools::feetToMeter( diameter );
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        return RiaEclipseUnitTools::meterToFeet( diameter );
    return diameter;
}

double RimSegmentCollection::linerDiameter() const
{
    return m_linerDiameter();
}

double RimSegmentCollection::roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto*  wellPath  = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    double roughness = m_roughnessFactor();
    if ( !wellPath->isTopLevelWellPath() && !m_customValuesForLateral() )
    {
        roughness = wellPath->topLevelWellPath()->segmentCollection()->m_roughnessFactor();
    }

    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        return RiaEclipseUnitTools::feetToMeter( roughness );
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        return RiaEclipseUnitTools::meterToFeet( roughness );
    return roughness;
}

double RimSegmentCollection::roughnessFactor() const
{
    return m_roughnessFactor();
}

RimSegmentCollection::PressureDropEnum RimSegmentCollection::pressureDrop() const
{
    return m_pressureDrop();
}

RimSegmentCollection::LengthAndDepthEnum RimSegmentCollection::lengthAndDepth() const
{
    return m_lengthAndDepth();
}

double RimSegmentCollection::maxSegmentLength() const
{
    return m_enforceMaxSegmentLength() ? m_maxSegmentLength() : std::numeric_limits<double>::infinity();
}

void RimSegmentCollection::setReferenceMDType( ReferenceMDType refType )
{
    m_refMDType = refType;
}

void RimSegmentCollection::setManualReferenceMD( double manualRefMD )
{
    m_refMD = manualRefMD;
}

void RimSegmentCollection::setLinerDiameter( double diameter )
{
    m_linerDiameter = diameter;
}

void RimSegmentCollection::setRoughnessFactor( double roughnessFactor )
{
    m_roughnessFactor = roughnessFactor;
}

void RimSegmentCollection::setPressureDrop( PressureDropType pressureDropType )
{
    m_pressureDrop = pressureDropType;
}

void RimSegmentCollection::setLengthAndDepth( LengthAndDepthType lengthAndDepthType )
{
    m_lengthAndDepth = lengthAndDepthType;
}

RimSegmentCollection::DiameterRoughnessMode RimSegmentCollection::diameterRoughnessMode() const
{
    return m_diameterRoughnessMode();
}

void RimSegmentCollection::setDiameterRoughnessMode( DiameterRoughnessMode mode )
{
    m_diameterRoughnessMode = mode;
}

bool RimSegmentCollection::isUsingIntervalSpecificValues() const
{
    return m_diameterRoughnessMode() == DiameterRoughnessMode::INTERVALS;
}

RimSegmentInterval* RimSegmentCollection::findIntervalAtMD( double md, const std::optional<QDateTime>& exportDate ) const
{
    for ( auto* interval : intervals() )
    {
        if ( interval && interval->containsMD( md ) && ( !exportDate.has_value() || interval->isActiveOnDate( *exportDate ) ) )
            return interval;
    }
    return nullptr;
}

double RimSegmentCollection::getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    if ( isUsingIntervalSpecificValues() )
    {
        if ( auto* interval = findIntervalAtMD( md, std::nullopt ) ) return interval->diameter( unitSystem );
        return defaultLinerDiameter( unitSystem );
    }
    return linerDiameter( unitSystem );
}

double RimSegmentCollection::getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    if ( isUsingIntervalSpecificValues() )
    {
        if ( auto* interval = findIntervalAtMD( md, std::nullopt ) ) return interval->roughnessFactor( unitSystem );
        return defaultRoughnessFactor( unitSystem );
    }
    return roughnessFactor( unitSystem );
}

double RimSegmentCollection::getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem, const QDateTime& exportDate ) const
{
    if ( isUsingIntervalSpecificValues() )
    {
        if ( auto* interval = findIntervalAtMD( md, exportDate ) ) return interval->diameter( unitSystem );
        return defaultLinerDiameter( unitSystem );
    }
    return linerDiameter( unitSystem );
}

double RimSegmentCollection::getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem, const QDateTime& exportDate ) const
{
    if ( isUsingIntervalSpecificValues() )
    {
        if ( auto* interval = findIntervalAtMD( md, exportDate ) ) return interval->roughnessFactor( unitSystem );
        return defaultRoughnessFactor( unitSystem );
    }
    return roughnessFactor( unitSystem );
}

std::vector<RimSegmentInterval*> RimSegmentCollection::intervals() const
{
    return m_segmentIntervals.childrenByType();
}

caf::PdmChildArrayField<RimSegmentInterval*>& RimSegmentCollection::intervalsField()
{
    return m_segmentIntervals;
}

RimSegmentInterval* RimSegmentCollection::createInterval( double startMD, double endMD, double diameter, double roughness )
{
    auto* interval = new RimSegmentInterval;
    interval->setStartMD( startMD );
    interval->setEndMD( endMD );
    interval->setDiameter( diameter );
    interval->setRoughnessFactor( roughness );
    addInterval( interval );
    return interval;
}

void RimSegmentCollection::addInterval( RimSegmentInterval* interval )
{
    m_segmentIntervals.push_back( interval );
}

void RimSegmentCollection::removeInterval( RimSegmentInterval* interval )
{
    m_segmentIntervals.removeChild( interval );
    delete interval;
}

void RimSegmentCollection::removeAllIntervals()
{
    m_segmentIntervals.deleteChildren();
}

bool RimSegmentCollection::hasIntervals() const
{
    return !m_segmentIntervals.empty();
}

void RimSegmentCollection::updateOverlapVisualFeedback()
{
    auto segmentIntervals = intervals();
    for ( auto* interval : segmentIntervals )
    {
        bool hasOverlap = std::ranges::any_of( segmentIntervals,
                                               [interval]( const RimSegmentInterval* other )
                                               { return interval != other && interval->overlaps( other ); } );
        interval->updateOverlapVisualFeedback( hasOverlap );
    }
}

std::vector<std::pair<double, double>> RimSegmentCollection::getSegmentIntervals() const
{
    std::vector<std::pair<double, double>> result;
    for ( auto* interval : intervals() )
    {
        if ( interval && interval->isValidInterval() ) result.emplace_back( interval->startMD(), interval->endMD() );
    }
    return result;
}

bool RimSegmentCollection::hasCustomSegmentIntervals() const
{
    return hasIntervals();
}

void RimSegmentCollection::setUnitSystemSpecificDefaults()
{
    auto* wellPath    = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    m_linerDiameter   = defaultLinerDiameter( wellPath->unitSystem() );
    m_roughnessFactor = defaultRoughnessFactor( wellPath->unitSystem() );
}

void RimSegmentCollection::updateFromTopLevelWell( const RimSegmentCollection* topLevelWellParameters )
{
    m_refMDType               = topLevelWellParameters->m_refMDType();
    m_refMD                   = topLevelWellParameters->m_refMD();
    m_pressureDrop            = topLevelWellParameters->m_pressureDrop();
    m_lengthAndDepth          = topLevelWellParameters->m_lengthAndDepth();
    m_enforceMaxSegmentLength = topLevelWellParameters->m_enforceMaxSegmentLength();
    m_maxSegmentLength        = topLevelWellParameters->m_maxSegmentLength();
}

void RimSegmentCollection::importLegacyData( const RimMswCompletionParameters* legacyParameters )
{
    if ( !legacyParameters ) return;

    const bool hasLegacyData = legacyParameters->m_refMDType() != RimMswCompletionParameters::ReferenceMDType::AUTO_REFERENCE_MD ||
                               legacyParameters->m_refMD() != 0.0 || legacyParameters->m_customValuesForLateral() ||
                               legacyParameters->m_linerDiameter() != defaultLinerDiameter( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) ||
                               legacyParameters->m_roughnessFactor() != defaultRoughnessFactor( RiaDefines::EclipseUnitSystem::UNITS_METRIC ) ||
                               legacyParameters->m_diameterRoughnessMode() != RimMswCompletionParameters::DiameterRoughnessMode::UNIFORM ||
                               legacyParameters->m_pressureDrop() != RimMswCompletionParameters::PressureDropType::HYDROSTATIC_FRICTION ||
                               legacyParameters->m_lengthAndDepth() != RimMswCompletionParameters::LengthAndDepthType::ABS ||
                               legacyParameters->m_enforceMaxSegmentLength() || legacyParameters->m_maxSegmentLength() != 200.0 ||
                               !legacyParameters->m_diameterRoughnessIntervals->intervals().empty() ||
                               !legacyParameters->m_customSegmentIntervals->intervals().empty();
    if ( !hasLegacyData ) return;

    m_refMDType = static_cast<ReferenceMDType>(
        static_cast<int>( static_cast<RimMswCompletionParameters::ReferenceMDType>( legacyParameters->m_refMDType() ) ) );
    m_refMD                  = legacyParameters->m_refMD();
    m_customValuesForLateral = legacyParameters->m_customValuesForLateral();
    m_linerDiameter          = legacyParameters->m_linerDiameter();
    m_roughnessFactor        = legacyParameters->m_roughnessFactor();
    m_diameterRoughnessMode  = static_cast<DiameterRoughnessMode>(
        static_cast<int>( static_cast<RimMswCompletionParameters::DiameterRoughnessMode>( legacyParameters->m_diameterRoughnessMode() ) ) );
    m_pressureDrop = static_cast<PressureDropType>(
        static_cast<int>( static_cast<RimMswCompletionParameters::PressureDropType>( legacyParameters->m_pressureDrop() ) ) );
    m_lengthAndDepth = static_cast<LengthAndDepthType>(
        static_cast<int>( static_cast<RimMswCompletionParameters::LengthAndDepthType>( legacyParameters->m_lengthAndDepth() ) ) );
    m_enforceMaxSegmentLength = legacyParameters->m_enforceMaxSegmentLength();
    m_maxSegmentLength        = legacyParameters->m_maxSegmentLength();

    removeAllIntervals();
    for ( auto* oldInterval : legacyParameters->m_diameterRoughnessIntervals->intervals() )
    {
        auto* interval =
            createInterval( oldInterval->startMD(), oldInterval->endMD(), oldInterval->diameter(), oldInterval->roughnessFactor() );
        interval->m_useCustomStartDate = oldInterval->m_useCustomStartDate();
        interval->m_startDate          = oldInterval->m_startDate();
    }

    constexpr double boundTolerance = 1.0e-6;
    for ( auto* oldInterval : legacyParameters->m_customSegmentIntervals->intervals() )
    {
        const auto currentIntervals = intervals();
        const auto matchingInterval =
            std::ranges::find_if( currentIntervals,
                                  [oldInterval]( const RimSegmentInterval* interval )
                                  {
                                      return std::abs( interval->startMD() - oldInterval->startMD() ) <= boundTolerance &&
                                             std::abs( interval->endMD() - oldInterval->endMD() ) <= boundTolerance;
                                  } );
        if ( matchingInterval == currentIntervals.end() )
        {
            createInterval( oldInterval->startMD(),
                            oldInterval->endMD(),
                            legacyParameters->m_linerDiameter(),
                            legacyParameters->m_roughnessFactor() );
        }
    }
}

double RimSegmentCollection::defaultLinerDiameter( RiaDefines::EclipseUnitSystem unitSystem )
{
    return unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC ? 0.152 : 0.5;
}

double RimSegmentCollection::defaultRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem )
{
    return unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC ? 1.0e-5 : 3.28e-5;
}

void RimSegmentCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_refMDType || changedField == &m_diameterRoughnessMode ) updateAllRequiredEditors();
    if ( changedField == &m_enforceMaxSegmentLength )
    {
        m_maxSegmentLength.uiCapability()->setUiHidden( !m_enforceMaxSegmentLength() );
        caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
    }
}

void RimSegmentCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewSegmentIntervalFeature";
    if ( hasIntervals() ) menuBuilder << "RicDeleteSegmentIntervalFeature";
}

void RimSegmentCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto*      wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    const bool isMetric = wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    m_linerDiameter.uiCapability()->setUiName( isMetric ? "Liner Inner Diameter [m]" : "Liner Inner Diameter [ft]" );
    m_roughnessFactor.uiCapability()->setUiName( isMetric ? "Roughness Factor [m]" : "Roughness Factor [ft]" );

    if ( wellPath->isTopLevelWellPath() )
    {
        uiOrdering.add( &m_refMDType );
        uiOrdering.add( &m_refMD );
        m_refMD.uiCapability()->setUiHidden( m_refMDType == ReferenceMDType::AUTO_REFERENCE_MD );
    }
    else
    {
        uiOrdering.add( &m_customValuesForLateral );
    }

    auto* diameterGroup = uiOrdering.addNewGroup( "Diameter and Roughness" );
    diameterGroup->add( &m_diameterRoughnessMode );
    if ( m_diameterRoughnessMode() == DiameterRoughnessMode::UNIFORM )
    {
        diameterGroup->add( &m_linerDiameter );
        diameterGroup->add( &m_roughnessFactor );
    }

    if ( wellPath->isTopLevelWellPath() )
    {
        uiOrdering.add( &m_pressureDrop );
        uiOrdering.add( &m_lengthAndDepth );
        uiOrdering.add( &m_enforceMaxSegmentLength );
        uiOrdering.add( &m_maxSegmentLength );
    }

    const bool readOnly = !wellPath->isTopLevelWellPath() && !m_customValuesForLateral();
    m_linerDiameter.uiCapability()->setUiReadOnly( readOnly );
    m_roughnessFactor.uiCapability()->setUiReadOnly( readOnly );
    m_diameterRoughnessMode.uiCapability()->setUiReadOnly( readOnly );
    m_segmentIntervals.uiCapability()->setUiReadOnly( readOnly );
    uiOrdering.skipRemainingFields( true );
}

void RimSegmentCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( auto* interval : intervals() )
        uiTreeOrdering.add( interval );
    uiTreeOrdering.skipRemainingChildren( true );
}

void RimSegmentCollection::initAfterRead()
{
    if ( m_linerDiameter() == std::numeric_limits<double>::infinity() && m_roughnessFactor() == std::numeric_limits<double>::infinity() )
        setUnitSystemSpecificDefaults();
}
