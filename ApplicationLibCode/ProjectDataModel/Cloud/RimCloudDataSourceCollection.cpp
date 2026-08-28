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

#include "RiaLogging.h"

#include "Cloud/RiaCloudApiService.h"
#include "RiaApplication.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "Cloud/RimSummaryEnsembleSumo.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSumoDataSource.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDialogButtonBox>

#include <set>

CAF_PDM_SOURCE_INIT( RimCloudDataSourceCollection, "RimCloudDataSourceCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCloudDataSourceCollection::RimCloudDataSourceCollection()
{
    CAF_PDM_InitObject( "Cloud Data" + RiaDefines::betaFeaturePostfix(), ":/Cloud.svg" );

    CAF_PDM_InitFieldNoDefault( &m_authenticate, "Authenticate", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_authenticate );

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

    CAF_PDM_InitFieldNoDefault( &m_startServer, "StartCloudApiServer", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_startServer );

    CAF_PDM_InitFieldNoDefault( &m_stopServer, "StopCloudApiServer", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_stopServer );

    CAF_PDM_InitFieldNoDefault( &m_restartServer, "RestartCloudApiServer", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_restartServer );

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
std::vector<RimSumoDataSource*> RimCloudDataSourceCollection::sumoDataSources() const
{
    return m_sumoDataSources.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::initAfterRead()
{
    refreshDataSourcesFromSumo();
}

//--------------------------------------------------------------------------------------------------
/// The realization ids, vector names and grid names of a data source are not written to the project file,
/// only the identity of the ensemble and the realization filter are. A data source read back from a project
/// therefore has none of them, which leaves the realization filter without the set it filters. Ask Sumo for
/// them again here, so the data source is as complete as one just added.
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::refreshDataSourcesFromSumo()
{
    if ( !m_sumoConnector ) return;

    for ( auto dataSource : sumoDataSources() )
    {
        if ( !dataSource ) continue;

        const auto caseId       = dataSource->caseId();
        const auto ensembleName = dataSource->ensembleName();
        if ( caseId.get().isEmpty() || ensembleName.isEmpty() ) continue;

        // Only what is missing is asked for, and an empty answer is not stored: a failed request must not
        // replace values that are already there. Note that Sumo answering with nothing is indistinguishable
        // from the request failing, so an ensemble genuinely without realizations is asked about again on the
        // next load. That costs a request and is preferable to recording "no realizations" after a failure.
        if ( !dataSource->hasFetchedRealizations() )
        {
            if ( const auto realizationIds = m_sumoConnector->explore().realizationIds( caseId, ensembleName ); !realizationIds.empty() )
            {
                dataSource->setAvailableRealizationIds( realizationIds );
            }
            else
            {
                // Without the realizations the realization filter has nothing to filter, and editing it
                // cannot be told from deselecting everything. Say so once, here, rather than leaving the
                // data source looking empty for no stated reason.
                RiaLogging::warning( QString( "Unable to load the realizations of Sumo ensemble '%1'. Editing the "
                                              "realization filter will have no effect until they are available." )
                                         .arg( ensembleName )
                                         .toStdString() );
                continue;
            }
        }

        if ( dataSource->vectorNames().empty() )
        {
            if ( const auto vectorNames = m_sumoConnector->summary().vectorNames( caseId, ensembleName ); !vectorNames.empty() )
            {
                dataSource->setVectorNames( vectorNames );
            }
        }

        if ( dataSource->gridNames().empty() )
        {
            if ( const auto gridNames = m_sumoConnector->grid().gridNames( caseId, ensembleName ); !gridNames.empty() )
            {
                dataSource->setGridNames( gridNames );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::createEnsemblesFromSelectedDataSources( const std::vector<RimSumoDataSource*>& dataSources )
{
    auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection();
    if ( !sumCaseMainColl ) return;

    for ( auto dataSource : dataSources )
    {
        RimSummaryEnsembleSumo* ensemble = new RimSummaryEnsembleSumo();
        ensemble->setUsePathKey1( true );
        ensemble->setSumoDataSource( dataSource );
        sumCaseMainColl->addEnsemble( ensemble );
        ensemble->loadDataAndUpdate();

        RiaSummaryPlotTools::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
    }

    RiaSummaryTools::updateSummaryEnsembleNames();
    sumCaseMainColl->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // The Cloud API server controls are handled before the Sumo connector guard below, so the server can
    // be started, stopped and restarted also when no connector is present or authentication failed.
    if ( changedField == &m_startServer )
    {
        // Authenticate up front, as the server is of little use without a token. The server is started
        // regardless of the outcome, so it can still be force-started if authentication is cancelled.
        if ( m_sumoConnector && !m_sumoConnector->isGranted() )
        {
            m_sumoConnector->requestTokenWithCancelButton();
        }

        RiaApplication::instance()->cloudApiService()->start();

        m_startServer = false;
        updateConnectedEditors();
        return;
    }
    else if ( changedField == &m_stopServer )
    {
        RiaApplication::instance()->cloudApiService()->stop();

        m_stopServer = false;
        updateConnectedEditors();
        return;
    }
    else if ( changedField == &m_restartServer )
    {
        RiaApplication::instance()->cloudApiService()->restart();

        m_restartServer = false;
        updateConnectedEditors();
        return;
    }

    if ( !m_sumoConnector ) return;

    if ( changedField == &m_authenticate )
    {
        // Authentication only, the server is started from its own button.
        m_sumoConnector->requestTokenWithCancelButton();

        m_authenticate = false;
    }

    if ( changedField == &m_sumoFieldName )
    {
        // What was picked below belonged to the asset that was just left, both the selection and the options
        // it was chosen from. Forget the cached answers as well, or the case list would keep offering the
        // cases of the previous asset. The editors are refreshed by the caller, which updates them as soon
        // as this returns, so asking for that here would only fetch everything a second time.
        m_sumoCaseId = "";
        m_sumoEnsembleNames.setValue( {} );

        clearCachedCases();
        clearCachedEnsembleNames();
    }
    else if ( changedField == &m_sumoCaseId )
    {
        m_sumoEnsembleNames.setValue( {} );

        clearCachedEnsembleNames();
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
/// True once the local service is serving requests. Being launched is not enough, as uvicorn may still
/// be booting.
//--------------------------------------------------------------------------------------------------
bool RimCloudDataSourceCollection::isCloudApiServerAvailable()
{
    auto* cloudApiService = RiaApplication::instance()->cloudApiService();
    return cloudApiService && cloudApiService->isResponding();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCloudDataSourceCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( !m_sumoConnector || !m_sumoConnector->isGranted() ) return {};

    // The Sumo data is read through the local service, so requesting anything before it is up produces
    // failing requests against an empty server address.
    if ( !isCloudApiServerAvailable() ) return {};

    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_sumoFieldName )
    {
        for ( const auto& asset : cachedAssets() )
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
        for ( const auto& sumoCase : cachedCases( m_sumoFieldName ) )
        {
            options.push_back( { sumoCase.name, sumoCase.caseId.get() } );
        }
    }
    else if ( fieldNeedingOptions == &m_sumoEnsembleNames && !m_sumoCaseId().isEmpty() )
    {
        for ( const auto& name : cachedEnsembleNames( SumoCaseId( m_sumoCaseId ) ) )
        {
            options.push_back( { name, name } );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<SumoAsset>& RimCloudDataSourceCollection::cachedAssets()
{
    if ( m_assets.empty() && m_sumoConnector )
    {
        m_assets = m_sumoConnector->explore().assets();
    }

    return m_assets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<SumoCase>& RimCloudDataSourceCollection::cachedCases( const QString& assetName )
{
    if ( m_casesAssetName != assetName && m_sumoConnector )
    {
        m_cases          = m_sumoConnector->explore().cases( assetName );
        m_casesAssetName = assetName;
    }

    return m_cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RimCloudDataSourceCollection::cachedEnsembleNames( const SumoCaseId& caseId )
{
    if ( m_ensembleNamesCaseId != caseId.get() && m_sumoConnector )
    {
        m_ensembleNames       = m_sumoConnector->explore().ensembleNames( caseId );
        m_ensembleNamesCaseId = caseId.get();
    }

    return m_ensembleNames;
}

//--------------------------------------------------------------------------------------------------
/// Forget the cases Sumo answered with, so the next request for them asks again. Called when the asset they
/// belong to is left behind.
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::clearCachedCases()
{
    m_casesAssetName.clear();
    m_cases.clear();
}

//--------------------------------------------------------------------------------------------------
/// The same for the ensemble names, which belong to a case.
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::clearCachedEnsembleNames()
{
    m_ensembleNamesCaseId.clear();
    m_ensembleNames.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Authentication group
    auto authGroup = uiOrdering.addNewGroup( "Authentication" );
    authGroup->add( &m_authenticate );

    bool    isGranted = m_sumoConnector && m_sumoConnector->isGranted();
    QString text      = " Authentication Status: ";
    text += isGranted ? "<font color='#228B22'>✔ Granted</font>" : "<font color='#FFA500'>❌ Not Granted</font>";

    m_authenticate.uiCapability()->setUiName( text );
    m_authenticate.uiCapability()->setUiReadOnly( isGranted );

    // Cloud server group
    auto serverGroup = uiOrdering.addNewGroup( QString( "Cloud API Server %1" ).arg( RiaDefines::betaFeaturePostfix() ) );

    auto* cloudApiService = RiaApplication::instance()->cloudApiService();
    bool  isServerRunning = cloudApiService && cloudApiService->isRunning();

    // A launched process is not yet a working server: uvicorn may still be booting, or may be about
    // to exit on a missing dependency. Only report "Running" once a health check has succeeded.
    bool    isServerResponding = cloudApiService && cloudApiService->isResponding();
    QString serverStatus       = "Server Status: ";
    if ( isServerResponding )
    {
        serverStatus += QString( "<font color='#228B22'>✔ Running (port %1)</font>" ).arg( cloudApiService->port() );
    }
    else if ( isServerRunning )
    {
        serverStatus += QString( "<font color='#FFA500'>… Starting (port %1)</font>" ).arg( cloudApiService->port() );
    }
    else
    {
        serverStatus += "<font color='#FFA500'>❌ Stopped</font>";
    }

    // The status text is shown as the left label of the Start button, mirroring the Authentication status above.
    m_startServer.uiCapability()->setUiName( serverStatus );
    m_startServer.uiCapability()->setUiReadOnly( isServerRunning );
    m_stopServer.uiCapability()->setUiReadOnly( !isServerRunning );
    m_restartServer.uiCapability()->setUiReadOnly( !isServerRunning );

    serverGroup->add( &m_startServer );
    serverGroup->add( &m_stopServer );
    serverGroup->add( &m_restartServer );

    // Cloud selector. Both a token and a running service are required to browse the Sumo content.
    if ( isGranted && isCloudApiServerAvailable() )
    {
        caf::PdmUiOrdering::LayoutOptions layout = { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 };
        uiOrdering.add( &m_sumoFieldName, layout );
        uiOrdering.add( &m_sumoCaseId, layout );
        uiOrdering.add( &m_sumoEnsembleNames, layout );

        uiOrdering.add( &m_addDataSources, layout );
        uiOrdering.add( &m_addEnsembles, layout );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudDataSourceCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_authenticate )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Authenticate";
        }
    }
    else if ( field == &m_addDataSources )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Add Data Sources(s)";
        }
    }
    else if ( field == &m_addEnsembles )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Add Ensemble(s)";
        }
    }
    else if ( field == &m_startServer )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Start";
        }
    }
    else if ( field == &m_stopServer )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Stop";
        }
    }
    else if ( field == &m_restartServer )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Restart";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSumoDataSource*> RimCloudDataSourceCollection::addDataSources()
{
    if ( !m_sumoConnector ) return {};

    std::vector<RimSumoDataSource*> dataSources;

    RimSumoDataSource* objectToSelect = nullptr;
    auto               sumoCaseId     = SumoCaseId( m_sumoCaseId );

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
        for ( const auto& sumoCase : cachedCases( m_sumoFieldName ) )
        {
            if ( sumoCase.caseId == sumoCaseId )
            {
                caseName = sumoCase.name;
                break;
            }
        }

        const auto availableRealizationIds = m_sumoConnector->explore().realizationIds( sumoCaseId, ensembleName );
        const auto gridNames               = m_sumoConnector->grid().gridNames( sumoCaseId, ensembleName );
        const auto vectorNames             = m_sumoConnector->summary().vectorNames( sumoCaseId, ensembleName );

        auto dataSource = new RimSumoDataSource();
        dataSource->setCaseId( sumoCaseId );
        dataSource->setAssetName( m_sumoFieldName );
        dataSource->setCaseName( caseName );
        dataSource->setEnsembleName( ensembleName );
        dataSource->setAvailableRealizationIds( availableRealizationIds );
        dataSource->setVectorNames( vectorNames );
        dataSource->setGridNames( gridNames );
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
