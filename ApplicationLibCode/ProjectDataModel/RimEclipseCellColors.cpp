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

#include "RiaResultNames.h"

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
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Cell Result", ":/CellResult.png", "", "", "CellColors", "Eclipse Cell Colors class" );

    CAF_PDM_InitFieldNoDefault( &obsoleteField_legendConfig, "LegendDefinition", "Color Legend" );
    obsoleteField_legendConfig.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_legendConfigData, "ResultVarLegendDefinitionList", "" );

    CAF_PDM_InitFieldNoDefault( &m_ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Color Legend" );
    m_ternaryLegendConfig = new RimTernaryLegendConfig();

    CAF_PDM_InitFieldNoDefault( &m_legendConfigPtrField, "LegendDefinitionPtrField", "Color Legend PtrField" );

    // Make sure we have a created legend for the default/undefined result variable
    changeLegendConfig( resultVariable() );

    m_useDiscreteLogLevels = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::~RimEclipseCellColors()
{
    CVF_ASSERT( obsoleteField_legendConfig() == nullptr );

    m_legendConfigData.deleteChildren();

    delete m_ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimEclipseResultDefinition::fieldChangedByUi( changedField, oldValue, newValue );

    // Update of legend config must happen after RimEclipseResultDefinition::fieldChangedByUi(), as this function
    // modifies resultVariable()
    if ( changedField == &m_resultVariableUiField )
    {
        if ( oldValue != newValue )
        {
            changeLegendConfig( resultVariableUiName() );
        }

        RimEclipseFaultColors* faultColors = dynamic_cast<RimEclipseFaultColors*>( parentField()->ownerObject() );
        if ( faultColors )
        {
            faultColors->updateConnectedEditors();
        }

        RimCellEdgeColors* cellEdgeColors = dynamic_cast<RimCellEdgeColors*>( parentField()->ownerObject() );
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
    if ( resultVarNameOfNewLegend != RiaResultNames::ternarySaturationResultName() )
    {
        QString legendResultVariable;

        if ( m_legendConfigPtrField() )
        {
            legendResultVariable = m_legendConfigPtrField()->resultVariableName();
        }

        if ( !m_legendConfigPtrField() || legendResultVariable != resultVarNameOfNewLegend )
        {
            bool found = false;
            for ( size_t i = 0; i < m_legendConfigData.size(); i++ )
            {
                if ( m_legendConfigData[i]->resultVariableName() == resultVarNameOfNewLegend )
                {
                    m_legendConfigPtrField = m_legendConfigData[i];
                    found                  = true;
                    break;
                }
            }

            if ( !found )
            {
                int caseId = 0;
                if ( eclipseCase() ) caseId = eclipseCase()->caseId();

                bool useCategoryLegend = hasCategoryResult();
                if ( m_resultType() == RiaDefines::ResultCatType::FORMATION_NAMES ) useCategoryLegend = true;

                auto newLegend = createLegendForResult( caseId, resultVarNameOfNewLegend, m_useDiscreteLogLevels, useCategoryLegend );

                newLegend->changed.connect( this, &RimEclipseCellColors::onLegendConfigChanged );

                m_legendConfigData.push_back( newLegend );

                m_legendConfigPtrField = newLegend;
            }
        }
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
RimRegularLegendConfig*
    RimEclipseCellColors::createLegendForResult( int caseId, const QString& resultName, bool useDiscreteLogLevels, bool isCategoryResult )
{
    auto* newLegend               = new RimRegularLegendConfig;
    newLegend->resultVariableName = resultName;

    newLegend->setDefaultConfigForResultName( caseId, resultName, useDiscreteLogLevels, isCategoryResult );

    return newLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::initAfterRead()
{
    RimEclipseResultDefinition::initAfterRead();

    if ( m_legendConfigPtrField() && m_legendConfigPtrField()->resultVariableName() == "" )
    {
        m_legendConfigPtrField()->resultVariableName = resultVariable();
    }

    if ( obsoleteField_legendConfig )
    {
        // The current legend config is NOT stored in <ResultVarLegendDefinitionList> in ResInsight up to v 1.3.7-dev
        RimRegularLegendConfig* obsoleteLegend = obsoleteField_legendConfig();

        // set to nullptr before pushing into container
        obsoleteField_legendConfig = nullptr;

        obsoleteLegend->changed.connect( this, &RimEclipseCellColors::onLegendConfigChanged );
        m_legendConfigData.push_back( obsoleteLegend );

        m_legendConfigPtrField = obsoleteLegend;
    }

    changeLegendConfig( resultVariable() );

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
    if ( resultVariable() == RiaResultNames::ternarySaturationResultName() )
    {
        uiTreeOrdering.add( m_ternaryLegendConfig() );
    }
    else
    {
        uiTreeOrdering.add( m_legendConfigPtrField() );
    }

    for ( const auto& obj : m_additionalUiTreeObjects )
    {
        uiTreeOrdering.add( obj );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendCategorySettings()
{
    changeLegendConfig( resultVariableUiName() );
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
void RimEclipseCellColors::useDiscreteLogLevels( bool enable )
{
    m_useDiscreteLogLevels = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setAdditionalUiTreeObjects( const std::vector<caf::PdmObject*>& objects )
{
    m_additionalUiTreeObjects = objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setReservoirView( RimEclipseView* ownerReservoirView )
{
    m_reservoirView = ownerReservoirView;
    if ( ownerReservoirView )
    {
        setEclipseCase( ownerReservoirView->eclipseCase() );
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
void RimEclipseCellColors::setResultVariable( const QString& val )
{
    RimEclipseResultDefinition::setResultVariable( val );

    changeLegendConfig( val );
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
