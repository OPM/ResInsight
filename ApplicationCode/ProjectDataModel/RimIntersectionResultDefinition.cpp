/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimIntersectionResultDefinition.h"

#include "Rim2dIntersectionView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGridView.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimTools.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimIntersectionResultDefinition, "IntersectionResultDefinition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultDefinition::RimIntersectionResultDefinition()
{
    CAF_PDM_InitObject( "Intersection Result Definition", "", "", "" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case", "", "", "" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_autoName, "IntersectionResultDefinitionDescription", "Description", "", "", "" );
    m_autoName.registerGetMethod( this, &RimIntersectionResultDefinition::autoName );
    m_autoName.uiCapability()->setUiHidden( true );
    m_autoName.uiCapability()->setUiReadOnly( true );
    m_autoName.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_eclipseResultDefinition, "EclipseResultDef", "EclipseResultDef", "", "", "" );
    m_eclipseResultDefinition.uiCapability()->setUiHidden( true );
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_eclipseResultDefinition = new RimEclipseResultDefinition;

    CAF_PDM_InitFieldNoDefault( &m_geomResultDefinition, "GeoMechResultDef", "GeoMechResultDef", "", "", "" );
    m_geomResultDefinition.uiCapability()->setUiHidden( true );
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "Legend", "", "", "" );
    m_legendConfig.uiCapability()->setUiHidden( true );
    m_legendConfig.uiCapability()->setUiTreeChildrenHidden( false );
    m_legendConfig = new RimRegularLegendConfig;

    CAF_PDM_InitFieldNoDefault( &m_ternaryLegendConfig, "TernaryLegendConfig", "Legend", "", "", "" );
    m_ternaryLegendConfig.uiCapability()->setUiHidden( true );
    m_ternaryLegendConfig.uiCapability()->setUiTreeChildrenHidden( false );
    m_ternaryLegendConfig = new RimTernaryLegendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultDefinition::~RimIntersectionResultDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionResultDefinition::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimIntersectionResultDefinition::autoName() const
{
    QString timestepName;
    QString caseName = "Default undefined source";

    if ( !m_case )
    {
        RimCase* ownerCase = nullptr;
        this->firstAncestorOrThisOfType( ownerCase );
        const_cast<RimIntersectionResultDefinition*>( this )->setActiveCase( ownerCase );
    }

    if ( m_case )
    {
        QStringList timestepNames = m_case->timeStepStrings();
        if ( timestepNames.size() > m_timeStep() )
        {
            timestepName = timestepNames[m_timeStep()];
        }
        caseName = m_case->caseUserDescription();
    }

    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QString resultVarUiName;
    if ( eclipseCase )
    {
        resultVarUiName = m_eclipseResultDefinition->resultVariableUiName();
    }
    else if ( geomCase )
    {
        resultVarUiName = m_geomResultDefinition->resultFieldUiName() + ":" +
                          m_geomResultDefinition->resultComponentUiName();
    }

    return resultVarUiName + " " + timestepName + " " + caseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimIntersectionResultDefinition::activeCase() const
{
    return m_case();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::setActiveCase( RimCase* activeCase )
{
    m_case = activeCase;

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    m_geomResultDefinition->setGeoMechCase( geomCase );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    m_eclipseResultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimIntersectionResultDefinition::timeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionResultDefinition::hasResult()
{
    if ( isEclipseResultDefinition() )
    {
        return m_eclipseResultDefinition->hasResult() || m_eclipseResultDefinition->isTernarySaturationSelected();
    }
    else
    {
        return m_geomResultDefinition->hasResult();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimIntersectionResultDefinition::regularLegendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTernaryLegendConfig* RimIntersectionResultDefinition::ternaryLegendConfig() const
{
    return m_ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionResultDefinition::isEclipseResultDefinition()
{
    if ( dynamic_cast<RimEclipseCase*>( m_case() ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition* RimIntersectionResultDefinition::eclipseResultDefinition() const
{
    return m_eclipseResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition* RimIntersectionResultDefinition::geoMechResultDefinition() const
{
    return m_geomResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::updateLegendRangesTextAndVisibility( const QString& title,
                                                                           RiuViewer*     nativeOrOverrideViewer,
                                                                           bool           isUsingOverrideViewer )
{
    if ( !this->isInAction() ) return;

    if ( ( this->isEclipseResultDefinition() && m_eclipseResultDefinition()->hasCategoryResult() ) ||
         ( !this->isEclipseResultDefinition() && m_geomResultDefinition()->hasCategoryResult() ) )
    {
        regularLegendConfig()->setMappingMode( RimRegularLegendConfig::CATEGORY_INTEGER );
        regularLegendConfig()->setColorRange( RimRegularLegendConfig::CATEGORY );
    }
    else
    {
        if ( regularLegendConfig()->mappingMode() == RimRegularLegendConfig::CATEGORY_INTEGER )
        {
            regularLegendConfig()->setMappingMode( RimRegularLegendConfig::LINEAR_CONTINUOUS );
        }

        if ( regularLegendConfig()->colorRange() == RimRegularLegendConfig::CATEGORY )
        {
            regularLegendConfig()->setColorRange( RimRegularLegendConfig::NORMAL );
        }
    }

    if ( this->isEclipseResultDefinition() )
    {
        RimEclipseResultDefinition* eclResultDef = this->eclipseResultDefinition();
        eclResultDef->updateRangesForExplicitLegends( this->regularLegendConfig(),
                                                      this->ternaryLegendConfig(),
                                                      this->timeStep() );

        eclResultDef->updateLegendTitle( this->regularLegendConfig(), title );

        if ( this->regularLegendConfig()->showLegend() && eclResultDef->hasResult() )
        {
            nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( this->regularLegendConfig()->titledOverlayFrame(),
                                                                      isUsingOverrideViewer );
        }
        else if ( eclResultDef->isTernarySaturationSelected() &&
                  eclResultDef->currentGridCellResults()->maxTimeStepCount() > 1 &&
                  this->ternaryLegendConfig()->showLegend() && this->ternaryLegendConfig()->titledOverlayFrame() )
        {
            this->ternaryLegendConfig()->setTitle( title );
            nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( this->ternaryLegendConfig()->titledOverlayFrame(),
                                                                      isUsingOverrideViewer );
        }
    }
    else
    {
        this->geoMechResultDefinition()->updateLegendTextAndRanges( this->regularLegendConfig(), title, this->timeStep() );

        if ( this->geoMechResultDefinition()->hasResult() && this->regularLegendConfig()->showLegend() )
        {
            nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( this->regularLegendConfig()->titledOverlayFrame(),
                                                                      isUsingOverrideViewer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionResultDefinition::userDescriptionField()
{
    return &m_autoName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionResultDefinition::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                        const QVariant&            oldValue,
                                                        const QVariant&            newValue )
{
    if ( changedField == &m_case )
    {
        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );
        m_geomResultDefinition->setGeoMechCase( geomCase );
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
        m_eclipseResultDefinition->setEclipseCase( eclipseCase );
    }

    this->updateConnectedEditors();

    RimIntersectionResultsDefinitionCollection* interResDefColl = nullptr;
    this->firstAncestorOrThisOfType( interResDefColl );
    bool isInAction = isActive() && interResDefColl && interResDefColl->isActive();

    if ( changedField == &m_isActive || ( changedField == &m_timeStep && isInAction ) )
    {
        RimGridView* gridView = nullptr;
        this->firstAncestorOrThisOfType( gridView );
        if ( gridView ) gridView->scheduleCreateDisplayModelAndRedraw();

        update2dIntersectionViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::update2dIntersectionViews()
{
    // Update 2D Intersection views

    std::vector<RimExtrudedCurveIntersection*> intersections;
    this->objectsWithReferringPtrFieldsOfType( intersections );

    for ( auto intersection : intersections )
    {
        if ( intersection && intersection->correspondingIntersectionView() )
        {
            intersection->correspondingIntersectionView()->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimIntersectionResultDefinition::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                            bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_case )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        QStringList timeStepNames;

        if ( m_case )
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for ( int i = 0; i < timeStepNames.size(); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStepNames[i], i ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );

    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    if ( eclipseCase )
    {
        m_eclipseResultDefinition->uiOrdering( uiConfigName, uiOrdering );
    }
    else if ( geomCase )
    {
        m_geomResultDefinition->uiOrdering( uiConfigName, uiOrdering );
    }

    if ( ( eclipseCase && m_eclipseResultDefinition->hasDynamicResult() ||
           m_eclipseResultDefinition->isTernarySaturationSelected() ) ||
         geomCase )
    {
        uiOrdering.add( &m_timeStep );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                            QString                 uiConfigName /*= ""*/ )
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );

    if ( !geomCase && m_eclipseResultDefinition->resultVariable() == RiaDefines::ternarySaturationResultName() )
    {
        uiTreeOrdering.add( m_ternaryLegendConfig() );
    }
    else
    {
        uiTreeOrdering.add( m_legendConfig() );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultDefinition::initAfterRead()
{
    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    if ( eclipseCase )
    {
        m_eclipseResultDefinition->setEclipseCase( eclipseCase );
    }
    else if ( geomCase )
    {
        m_geomResultDefinition->setGeoMechCase( geomCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionResultDefinition::isInAction() const
{
    RimIntersectionResultsDefinitionCollection* interResDefColl = nullptr;
    this->firstAncestorOrThisOfType( interResDefColl );

    return isActive() && interResDefColl && interResDefColl->isActive();
}
