/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimWellPathValve.h"

#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"

#include "Riu3DMainWindowTools.h"

#include "RigWellPath.h"

#include "RimMultipleValveLocations.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimWellPath.h"

#include "CompletionCommands/RicNewValveTemplateFeature.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiToolButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimWellPathValve, "WellPathValve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::RimWellPathValve()
{
    CAF_PDM_InitObject( "WellPathValve", ":/ICDValve16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_valveTemplate, "ValveTemplate", "Valve Template", "", "", "" );
    CAF_PDM_InitField( &m_measuredDepth, "StartMeasuredDepth", 0.0, "Start MD", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_multipleValveLocations, "ValveLocations", "Valve Locations", "", "", "" );
    CAF_PDM_InitField( &m_editValveTemplate, "EditTemplate", false, "Edit", "", "", "" );
    CAF_PDM_InitField( &m_createValveTemplate, "CreateTemplate", false, "Create", "", "", "" );

    m_measuredDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_multipleValveLocations = new RimMultipleValveLocations;
    m_multipleValveLocations.uiCapability()->setUiTreeHidden( true );
    m_multipleValveLocations.uiCapability()->setUiTreeChildrenHidden( true );
    m_editValveTemplate.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_editValveTemplate.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_createValveTemplate.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_createValveTemplate.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );

    nameField()->uiCapability()->setUiReadOnly( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::~RimWellPathValve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::perforationIntervalUpdated()
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        const RimPerforationInterval* perfInterval = nullptr;
        this->firstAncestorOrThisOfType( perfInterval );
        double startMD  = perfInterval->startMD();
        double endMD    = perfInterval->endMD();
        m_measuredDepth = std::clamp( m_measuredDepth(), std::min( startMD, endMD ), std::max( startMD, endMD ) );
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::ICD ||
              componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        m_multipleValveLocations->perforationIntervalUpdated();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::setMeasuredDepthAndCount( double startMD, double spacing, int valveCount )
{
    m_measuredDepth = startMD;
    double endMD    = startMD + spacing * valveCount;
    m_multipleValveLocations->initFields( RimMultipleValveLocations::VALVE_COUNT, startMD, endMD, spacing, valveCount, {} );
    m_multipleValveLocations->computeRangesAndLocations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::multipleValveGeometryUpdated()
{
    if ( m_multipleValveLocations->valveLocations().empty() ) return;

    m_measuredDepth = m_multipleValveLocations->valveLocations().front();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellPathValve::valveLocations() const
{
    std::vector<double> valveDepths;
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        valveDepths.push_back( m_measuredDepth );
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::ICD ||
              componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        valveDepths = m_multipleValveLocations->valveLocations();
    }
    return valveDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::orificeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    if ( m_valveTemplate() )
    {
        double templateDiameter = m_valveTemplate()->orificeDiameter();
        return convertOrificeDiameter( templateDiameter, m_valveTemplate()->templateUnits(), unitSystem );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::flowCoefficient() const
{
    if ( m_valveTemplate() )
    {
        double templateCoefficient = m_valveTemplate()->flowCoefficient();
        return templateCoefficient;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate* RimWellPathValve::valveTemplate() const
{
    return m_valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::setValveTemplate( RimValveTemplate* valveTemplate )
{
    m_valveTemplate = valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::applyValveLabelAndIcon()
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        this->setUiIconFromResourceString( ":/ICVValve16x16.png" );
        QString fullName = QString( "%1: %2" ).arg( componentLabel() ).arg( m_measuredDepth() );
        this->setName( fullName );
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        this->setUiIconFromResourceString( ":/ICDValve16x16.png" );
        QString fullName = QString( "%1 %2: %3 - %4" )
                               .arg( m_multipleValveLocations->valveLocations().size() )
                               .arg( componentLabel() )
                               .arg( m_multipleValveLocations->rangeStart() )
                               .arg( m_multipleValveLocations->rangeEnd() );
        this->setName( fullName );
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        this->setUiIconFromResourceString( ":/AICDValve16x16.png" );
        QString fullName = QString( "%1 %2: %3 - %4" )
                               .arg( m_multipleValveLocations->valveLocations().size() )
                               .arg( componentLabel() )
                               .arg( m_multipleValveLocations->rangeStart() )
                               .arg( m_multipleValveLocations->rangeEnd() );
        this->setName( fullName );
    }
    else
    {
        this->setName( "Unspecified Valve" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathAicdParameters* RimWellPathValve::aicdParameters() const
{
    if ( m_valveTemplate() )
    {
        return m_valveTemplate()->aicdParameters();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::convertOrificeDiameter( double                        orificeDiameterWellPathUnits,
                                                 RiaDefines::EclipseUnitSystem wellPathUnits,
                                                 RiaDefines::EclipseUnitSystem unitSystem )
{
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        if ( wellPathUnits == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            return RiaEclipseUnitTools::inchToMeter( orificeDiameterWellPathUnits );
        }
        else
        {
            return RiaEclipseUnitTools::mmToMeter( orificeDiameterWellPathUnits );
        }
    }
    else if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        if ( wellPathUnits == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            return RiaEclipseUnitTools::meterToFeet( RiaEclipseUnitTools::mmToMeter( orificeDiameterWellPathUnits ) );
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet( orificeDiameterWellPathUnits );
        }
    }
    CVF_ASSERT( false );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellPathValve::valveSegments() const
{
    RimPerforationInterval* perforationInterval = nullptr;
    this->firstAncestorOrThisOfType( perforationInterval );

    std::vector<std::pair<double, double>> segments;
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        // Flow for ICV is defined as the complete perforation interval

        segments.push_back( std::make_pair( perforationInterval->startMD(), perforationInterval->endMD() ) );
    }
    else
    {
        // ICD/AICD : Use the valve start/end, can be a subset of perforation interval

        double startMD = this->startMD();
        double endMD   = this->endMD();

        std::vector<double> valveMDs = valveLocations();
        segments.reserve( valveMDs.size() );

        for ( size_t i = 0; i < valveMDs.size(); ++i )
        {
            double segmentStart = startMD;
            double segmentEnd   = endMD;
            if ( i > 0 )
            {
                segmentStart = 0.5 * ( valveMDs[i - 1] + valveMDs[i] );
            }
            if ( i < valveMDs.size() - 1u )
            {
                segmentEnd = 0.5 * ( valveMDs[i] + valveMDs[i + 1] );
            }
            segments.push_back( std::make_pair( segmentStart, segmentEnd ) );
        }
    }

    return segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathValve::isEnabled() const
{
    RimPerforationInterval* perforationInterval = nullptr;
    this->firstAncestorOrThisOfType( perforationInterval );
    return perforationInterval->isEnabled() && isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimWellPathValve::componentType() const
{
    if ( m_valveTemplate() )
    {
        return m_valveTemplate()->type();
    }
    return RiaDefines::WellPathComponentType::UNDEFINED_COMPONENT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentLabel() const
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        if ( m_multipleValveLocations->valveLocations().size() > 1 )
        {
            return "ICDs";
        }
        else
        {
            return "ICD";
        }
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        if ( m_multipleValveLocations->valveLocations().size() > 1 )
        {
            return "AICDs";
        }
        else
        {
            return "AICD";
        }
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        return "ICV";
    }
    return "Valve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentTypeLabel() const
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        return "ICD";
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        return "AICD";
    }
    else if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        return "ICV";
    }
    return "Valve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPathValve::defaultComponentColor() const
{
    return RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::startMD() const
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        return m_measuredDepth;
    }
    else if ( m_multipleValveLocations()->valveLocations().size() < 2 )
    {
        return m_multipleValveLocations->rangeStart();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::endMD() const
{
    if ( componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        return m_measuredDepth + 0.5;
    }
    else if ( m_multipleValveLocations()->valveLocations().size() < 2 )
    {
        return m_multipleValveLocations->rangeEnd();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::templateUpdated()
{
    applyValveLabelAndIcon();

    RimPerforationInterval* perfInterval;
    this->firstAncestorOrThisOfTypeAsserted( perfInterval );
    perfInterval->updateAllReferringTracks();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathValve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                       bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* project = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( project );
    std::vector<RimValveTemplate*> allTemplates = project->allValveTemplates();
    for ( RimValveTemplate* valveTemplate : allTemplates )
    {
        options.push_back( caf::PdmOptionItemInfo( valveTemplate->name(), valveTemplate ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_valveTemplate )
    {
        applyValveLabelAndIcon();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_createValveTemplate )
    {
        m_createValveTemplate = false;
        RicNewValveTemplateFeature::createNewValveTemplateForValveAndUpdate( this );
    }
    else if ( changedField == &m_editValveTemplate )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( m_valveTemplate() );
    }

    RimPerforationInterval* perfInterval;
    this->firstAncestorOrThisOfTypeAsserted( perfInterval );
    perfInterval->updateAllReferringTracks();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );

    uiOrdering.add( &m_valveTemplate, { true, 2, 1 } );

    {
        if ( m_valveTemplate() != nullptr )
        {
            uiOrdering.add( &m_editValveTemplate, false );
        }
        uiOrdering.add( &m_createValveTemplate, false );
    }

    if ( componentType() == RiaDefines::WellPathComponentType::ICV ||
         componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        if ( componentType() == RiaDefines::WellPathComponentType::ICV )
        {
            RimWellPath* wellPath;
            firstAncestorOrThisOfType( wellPath );
            if ( wellPath )
            {
                if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
                {
                    m_measuredDepth.uiCapability()->setUiName( "Measured Depth [m]" );
                }
                else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
                {
                    m_measuredDepth.uiCapability()->setUiName( "Measured Depth [ft]" );
                }
            }
            uiOrdering.add( &m_measuredDepth, { true, 3, 1 } );
        }
    }

    if ( componentType() == RiaDefines::WellPathComponentType::ICD ||
         componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Multiple Valve Locations" );
        m_multipleValveLocations->uiOrdering( uiConfigName, *group );
    }

    if ( m_valveTemplate() != nullptr )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Parameters from Template" );
        m_valveTemplate->uiOrdering( "InsideValve", *group );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_measuredDepth )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr )
        {
            double minimumValue = 0.0, maximumValue = 0.0;

            RimPerforationInterval* perforationInterval = nullptr;
            this->firstAncestorOrThisOfType( perforationInterval );

            if ( perforationInterval )
            {
                minimumValue = perforationInterval->startMD();
                maximumValue = perforationInterval->endMD();
            }
            else
            {
                RimWellPath* wellPath = nullptr;
                this->firstAncestorOrThisOfTypeAsserted( wellPath );

                minimumValue = wellPath->startMD();
                maximumValue = wellPath->endMD();
            }
            myAttr->m_minimum = minimumValue;
            myAttr->m_maximum = maximumValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    applyValveLabelAndIcon();
}
