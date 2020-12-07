/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCellColors.h"

#include "RicfCommandObject.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimEclipseCellColors, "ResultSlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::RimEclipseCellColors()
    : legendConfigChanged( this )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Cell Result",
                                                    ":/CellResult.png",
                                                    "",
                                                    "",
                                                    "CellColors",
                                                    "Eclipse Cell Colors class" );

    CAF_PDM_InitFieldNoDefault( &obsoleteField_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    this->obsoleteField_legendConfig.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Color Legend", "", "", "" );
    this->m_ternaryLegendConfig = new RimTernaryLegendConfig();

    CAF_PDM_InitFieldNoDefault( &m_legendConfigPtrField, "LegendDefinitionPtrField", "Color Legend PtrField", "", "", "" );

    // Make sure we have a created legend for the default/undefined result variable
    changeLegendConfig( this->resultVariable() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::~RimEclipseCellColors()
{
    CVF_ASSERT( obsoleteField_legendConfig() == nullptr );

    m_legendConfigData.deleteAllChildObjects();

    delete m_ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    RimEclipseResultDefinition::fieldChangedByUi( changedField, oldValue, newValue );

    // Update of legend config must happen after RimEclipseResultDefinition::fieldChangedByUi(), as this function
    // modifies this->resultVariable()
    if ( changedField == &m_resultVariableUiField )
    {
        if ( oldValue != newValue )
        {
            changeLegendConfig( this->resultVariable() );
        }

        if ( newValue != RiaDefines::undefinedResultName() )
        {
            if ( m_reservoirView ) m_reservoirView->hasUserRequestedAnimation = true;
        }

        RimEclipseFaultColors* faultColors = dynamic_cast<RimEclipseFaultColors*>( this->parentField()->ownerObject() );
        if ( faultColors )
        {
            faultColors->updateConnectedEditors();
        }

        RimCellEdgeColors* cellEdgeColors = dynamic_cast<RimCellEdgeColors*>( this->parentField()->ownerObject() );
        if ( cellEdgeColors )
        {
            cellEdgeColors->updateConnectedEditors();
        }
    }

    if ( m_reservoirView ) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::changeLegendConfig( QString resultVarNameOfNewLegend )
{
    if ( resultVarNameOfNewLegend != RiaDefines::ternarySaturationResultName() )
    {
        QString legendResultVariable;

        if ( this->m_legendConfigPtrField() )
        {
            legendResultVariable = this->m_legendConfigPtrField()->resultVariableName();
        }

        if ( !this->m_legendConfigPtrField() || legendResultVariable != resultVarNameOfNewLegend )
        {
            bool found = false;
            for ( size_t i = 0; i < m_legendConfigData.size(); i++ )
            {
                if ( m_legendConfigData[i]->resultVariableName() == resultVarNameOfNewLegend )
                {
                    this->m_legendConfigPtrField = m_legendConfigData[i];
                    found                        = true;
                    break;
                }
            }

            if ( !found )
            {
                RimRegularLegendConfig* newLegend = new RimRegularLegendConfig;
                newLegend->resultVariableName     = resultVarNameOfNewLegend;

                bool useLog = false;
                {
                    QStringList subStringsToMatch{ "TRAN", "MULT", "PERM" };

                    for ( const auto& s : subStringsToMatch )
                    {
                        if ( resultVarNameOfNewLegend.contains( s, Qt::CaseInsensitive ) )
                        {
                            useLog = true;
                        }
                    }
                }

                if ( useLog )
                {
                    newLegend->setMappingMode( RimRegularLegendConfig::MappingType::LOG10_DISCRETE );
                    newLegend->setTickNumberFormat( RimRegularLegendConfig::NumberFormatType::AUTO );
                    newLegend->setRangeMode( RimLegendConfig::RangeModeType::USER_DEFINED );
                    newLegend->resetUserDefinedValues();
                }

                if ( this->hasCategoryResult() )
                {
                    newLegend->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
                    newLegend->setColorLegend(
                        RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::CATEGORY ) );
                }
                m_legendConfigData.push_back( newLegend );

                this->m_legendConfigPtrField = newLegend;
            }
        }
    }

    for ( auto legendConfig : m_legendConfigData )
    {
        legendConfig->changed.connect( this, &RimEclipseCellColors::onLegendConfigChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType )
{
    legendConfigChanged.send( changeType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::initAfterRead()
{
    RimEclipseResultDefinition::initAfterRead();

    if ( this->m_legendConfigPtrField() && this->m_legendConfigPtrField()->resultVariableName == "" )
    {
        this->m_legendConfigPtrField()->resultVariableName = this->resultVariable();
    }

    if ( obsoleteField_legendConfig )
    {
        // The current legend config is NOT stored in <ResultVarLegendDefinitionList> in ResInsight up to v 1.3.7-dev
        RimRegularLegendConfig* obsoleteLegend = obsoleteField_legendConfig();

        // set to nullptr before pushing into container
        obsoleteField_legendConfig = nullptr;

        m_legendConfigData.push_back( obsoleteLegend );
        m_legendConfigPtrField = obsoleteLegend;
    }

    changeLegendConfig( this->resultVariable() );

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimEclipseResultDefinition::defineUiOrdering( uiConfigName, uiOrdering );

    if ( uiConfigName == "AddLegendLevels" )
    {
        legendConfig()->uiOrdering( "NumIntervalsOnly", uiOrdering );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( this->resultVariable() == RiaDefines::ternarySaturationResultName() )
    {
        uiTreeOrdering.add( m_ternaryLegendConfig() );
    }
    else
    {
        uiTreeOrdering.add( m_legendConfigPtrField() );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendCategorySettings()
{
    changeLegendConfig( this->resultVariable() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::uiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    defineUiTreeOrdering( uiTreeOrdering, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setReservoirView( RimEclipseView* ownerReservoirView )
{
    m_reservoirView = ownerReservoirView;
    if ( ownerReservoirView )
    {
        this->setEclipseCase( ownerReservoirView->eclipseCase() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCellColors::reservoirView()
{
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateRangesForEmbeddedLegends( int currentTimeStep )
{
    this->updateRangesForExplicitLegends( legendConfig(), m_ternaryLegendConfig(), currentTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setResultVariable( const QString& val )
{
    RimEclipseResultDefinition::setResultVariable( val );

    this->changeLegendConfig( val );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEclipseCellColors::legendConfig()
{
    return m_legendConfigPtrField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTernaryLegendConfig* RimEclipseCellColors::ternaryLegendConfig()
{
    return m_ternaryLegendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateIconState()
{
    if ( m_reservoirView )
    {
        RimViewController* viewController = m_reservoirView->viewController();
        if ( viewController && viewController->isResultColorControlled() )
        {
            updateUiIconFromState( false );
        }
        else
        {
            updateUiIconFromState( true );
        }
    }
    uiCapability()->updateConnectedEditors();
}
