/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimCloudDataSourceCollection.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "Sumo/RimSummaryEnsembleSumo.h"
#include "Sumo/RimSummarySumoDataSource.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimCloudDataSourceCollection, "RimCloudDataSourceCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCloudDataSourceCollection::RimCloudDataSourceCollection()
{
    CAF_PDM_InitObject( "Cloud Data", ":/Cloud.svg" );

    CAF_PDM_InitFieldNoDefault( &m_sumoFieldName, "SumoFieldId", "Field Id" );
    CAF_PDM_InitFieldNoDefault( &m_sumoCaseId, "SumoCaseId", "Case Id" );
    m_sumoCaseId.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_sumoEnsembleNames, "SumoEnsembleNames", "Ensembles" );
    m_sumoEnsembleNames.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_addDataSources, "AddDataSources", "", "", "Add Data Sources without Ensembles" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_addDataSources );

    CAF_PDM_InitFieldNoDefault( &m_addEnsembles, "AddEnsembles", "", "", "Add Data Sources and Create Summary Ensemble Plots" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_addEnsembles );

    CAF_PDM_InitFieldNoDefault( &m_sumoDataSources, "SumoDataSources", "Sumo Data Sources" );

    m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCloudDataSourceCollection* RimCloudDataSourceCollection::instance()
{
    return RimProject::current()->activeOilField()->cloudDataCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummarySumoDataSource*> RimCloudDataSourceCollection::sumoDataSources() const
{
    return m_sumoDataSources.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::createEnsemblesFromSelectedDataSources( const std::vector<RimSummarySumoDataSource*>& dataSources )
{
    for ( auto dataSource : dataSources )
    {
        RimSummaryEnsembleSumo* ensemble = new RimSummaryEnsembleSumo();
        ensemble->setSumoDataSource( dataSource );
        ensemble->updateName();
        RiaSummaryTools::summaryCaseMainCollection()->addEnsemble( ensemble );
        ensemble->loadDataAndUpdate();

        RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
    }

    RiaSummaryTools::summaryCaseMainCollection()->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( !m_sumoConnector ) return;

    if ( changedField == &m_sumoFieldName )
    {
        m_sumoCaseId = "";
        m_sumoEnsembleNames.v().clear();

        m_sumoConnector->requestCasesForFieldBlocking( m_sumoFieldName );
    }
    else if ( changedField == &m_sumoCaseId )
    {
        m_sumoEnsembleNames.v().clear();
    }
    if ( changedField == &m_addEnsembles )
    {
        addEnsembles();

        m_addEnsembles = false;
    }
    if ( changedField == &m_addDataSources )
    {
        addDataSources();

        m_addDataSources = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCloudDataSourceCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( !m_sumoConnector ) return {};

    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_sumoFieldName )
    {
        if ( m_sumoConnector->assets().empty() )
        {
            m_sumoConnector->requestAssetsBlocking();
        }

        for ( const auto& asset : m_sumoConnector->assets() )
        {
            if ( m_sumoFieldName().isEmpty() )
            {
                m_sumoFieldName = asset.name;
            }

            options.push_back( { asset.name, asset.name } );
        }
    }
    else if ( fieldNeedingOptions == &m_sumoCaseId && !m_sumoFieldName().isEmpty() )
    {
        if ( m_sumoConnector->cases().empty() )
        {
            m_sumoConnector->requestCasesForFieldBlocking( m_sumoFieldName );
        }

        for ( const auto& sumoCase : m_sumoConnector->cases() )
        {
            options.push_back( { sumoCase.name, sumoCase.caseId.get() } );
        }
    }
    else if ( fieldNeedingOptions == &m_sumoEnsembleNames && !m_sumoCaseId().isEmpty() )
    {
        if ( m_sumoConnector->ensembleNamesForCase( SumoCaseId( m_sumoCaseId ) ).empty() )
        {
            m_sumoConnector->requestEnsembleByCasesIdBlocking( SumoCaseId( m_sumoCaseId ) );
        }

        for ( const auto& name : m_sumoConnector->ensembleNamesForCase( SumoCaseId( m_sumoCaseId ) ) )
        {
            options.push_back( { name, name } );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiOrdering::LayoutOptions layout = { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 };
    uiOrdering.add( &m_sumoFieldName, layout );
    uiOrdering.add( &m_sumoCaseId, layout );
    uiOrdering.add( &m_sumoEnsembleNames, layout );

    uiOrdering.add( &m_addDataSources, { .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    uiOrdering.add( &m_addEnsembles, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_addDataSources )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Add Data Sources(s)";
        }
    }
    if ( field == &m_addEnsembles )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Add Ensemble(s)";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummarySumoDataSource*> RimCloudDataSourceCollection::addDataSources()
{
    if ( !m_sumoConnector ) return {};

    std::vector<RimSummarySumoDataSource*> dataSources;

    RimSummarySumoDataSource* objectToSelect = nullptr;
    auto                      sumoCaseId     = SumoCaseId( m_sumoCaseId );

    for ( const auto& ensembleName : m_sumoEnsembleNames() )
    {
        bool createNewDataSource = true;
        for ( const auto dataSource : sumoDataSources() )
        {
            if ( dataSource->caseId() == sumoCaseId && dataSource->ensembleName() == ensembleName )
            {
                createNewDataSource = false;
                break;
            }
        }

        if ( !createNewDataSource )
        {
            continue;
        }

        QString caseName;
        for ( const auto& sumoCase : m_sumoConnector->cases() )
        {
            if ( sumoCase.caseId == sumoCaseId )
            {
                caseName = sumoCase.name;
                break;
            }
        }

        m_sumoConnector->requestRealizationIdsForEnsembleBlocking( sumoCaseId, ensembleName );
        m_sumoConnector->requestVectorNamesForEnsembleBlocking( sumoCaseId, ensembleName );

        auto realizationIds = m_sumoConnector->realizationIds();
        auto vectorNames    = m_sumoConnector->vectorNames();

        auto dataSource = new RimSummarySumoDataSource();
        dataSource->setCaseId( sumoCaseId );
        dataSource->setCaseName( caseName );
        dataSource->setEnsembleName( ensembleName );
        dataSource->setRealizationIds( realizationIds );
        dataSource->setVectorNames( vectorNames );
        dataSource->updateName();

        objectToSelect = dataSource;

        m_sumoDataSources.push_back( dataSource );
        dataSources.push_back( dataSource );
    }

    uiCapability()->updateAllRequiredEditors();

    if ( objectToSelect )
    {
        RiuPlotMainWindowTools::setExpanded( objectToSelect );
        RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
    }

    return dataSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::addEnsembles()
{
    auto dataSources = addDataSources();
    createEnsemblesFromSelectedDataSources( dataSources );
}
