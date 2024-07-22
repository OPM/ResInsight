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

#include "RimOilField.h"
#include "RimProject.h"
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

    CAF_PDM_InitFieldNoDefault( &m_addEnsembles, "AddEnsembles", "" );
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
        addEnsemble();

        m_addEnsembles = false;
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
    uiOrdering.add( &m_sumoFieldName );
    uiOrdering.add( &m_sumoCaseId );
    uiOrdering.add( &m_sumoEnsembleNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
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
void RimCloudDataSourceCollection::addEnsemble()
{
    if ( !m_sumoConnector ) return;

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
    }

    uiCapability()->updateAllRequiredEditors();

    if ( objectToSelect )
    {
        RiuPlotMainWindowTools::setExpanded( objectToSelect );
        RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
    }
}
