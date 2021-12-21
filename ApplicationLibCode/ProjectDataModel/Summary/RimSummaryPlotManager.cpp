////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSummaryPlotManager.h"

#include "RiaStdStringTools.h"
#include "RiaStringListSerializer.h"
#include "RiaSummaryStringTools.h"
#include "RiaSummaryTools.h"

#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafSelectionManager.h"

// Multi plot
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultiSummaryPlot.h"
#include "RimProject.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimSummaryPlotManager, "RimSummaryPlotManager" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotManager::RimSummaryPlotManager()
{
    CAF_PDM_InitObject( "Summary Plot Manager" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot" );
    CAF_PDM_InitFieldNoDefault( &m_filterText, "FilterText", "Filter Text" );
    m_filterText.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_addressCandidates, "AddressCandidates", "Vectors" );
    m_addressCandidates.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    CAF_PDM_InitFieldNoDefault( &m_selectedDataSources, "SelectedDataSources", "Data Sources" );
    m_selectedDataSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_selectedDataSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_includeDiffCurves,
                       "IncludeDiffCurves",
                       false,
                       "Include Difference Vectors",
                       "",
                       "Difference between simulated and observed(history) curve",
                       "" );
    m_includeDiffCurves.uiCapability()->setUiEditorTypeName( caf::PdmUiNativeCheckBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonReplace, "PushButtonReplace", "Replace (CTRL + Enter)" );
    m_pushButtonReplace.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonReplace.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonNewPlot, "PushButtonNewPlot", "New (Alt + Enter)" );
    m_pushButtonNewPlot.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonNewPlot.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonAppend, "PushButtonAppend", "Append (Shift + Enter)" );
    m_pushButtonAppend.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonAppend.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_labelA, "LabelA", "" );
    m_labelA.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelA.xmlCapability()->disableIO();
    m_labelA.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_labelB, "LabelB", "" );
    m_labelB.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelB.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_individualPlotPerVector, "IndividualPlotPerVector", false, "One plot per Vector" );
    CAF_PDM_InitField( &m_individualPlotPerDataSource, "IndividualPlotPerDataSource", false, "One plot per Data Source" );
    CAF_PDM_InitField( &m_createMultiPlot, "CreateMultiPlot", false, "Create Multiple Plots in One Window" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::setFocusToFilterText()
{
    setFocusToEditorWidget( m_filterText.uiCapability() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::resetDataSourceSelection()
{
    m_previousDataSourceText.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    updateUiFromSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    if ( changedField == &m_summaryPlot || changedField == &m_filterText || changedField == &m_includeDiffCurves ||
         changedField == &m_selectedDataSources )
    {
        updateSelectionFromUiChange();
        updateCurveCandidates();
    }
    else if ( changedField == &m_pushButtonReplace )
    {
        replaceCurves();
        m_pushButtonReplace = false;
        updateFilterTextHistory();
    }
    else if ( changedField == &m_pushButtonNewPlot )
    {
        createNewPlot();
        m_pushButtonNewPlot = false;
        updateFilterTextHistory();
    }
    else if ( changedField == &m_pushButtonAppend )
    {
        appendCurves();
        m_pushButtonAppend = false;
        updateFilterTextHistory();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotManager::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_summaryPlot )
    {
        auto coll = RiaSummaryTools::summaryPlotCollection();
        coll->summaryPlotItemInfos( &options );
    }
    else if ( fieldNeedingOptions == &m_filterText )
    {
        RiaStringListSerializer stringListSerializer( curveFilterRecentlyUsedRegistryKey() );

        for ( const auto& s : stringListSerializer.textStrings() )
        {
            options.push_back( caf::PdmOptionItemInfo( s, s ) );
        }
    }
    else if ( fieldNeedingOptions == &m_selectedDataSources )
    {
        for ( const auto& displayName : dataSourceDisplayNames() )
        {
            options.push_back( caf::PdmOptionItemInfo( displayName, displayName ) );
        }

        updateSelectionFromUiChange();
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateCurveCandidates()
{
    std::vector<QString> curveCandidates;

    auto addresses = filteredAddresses();

    for ( const auto& adr : addresses )
    {
        curveCandidates.push_back( QString::fromStdString( adr.uiText() ) );
    }

    m_addressCandidates = curveCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, caf::PdmObject*>> RimSummaryPlotManager::findDataSourceCandidates() const
{
    std::vector<std::pair<QString, PdmObject*>> candidates;

    QStringList dataSourceFilters = extractDataSourceFilters();

    auto [summaryCases, ensembles] = allDataSourcesInProject();

    for ( const auto& dsFilter : dataSourceFilters )
    {
        QString searchString = dsFilter.left( dsFilter.indexOf( ':' ) );
        QRegExp searcher( searchString, Qt::CaseInsensitive, QRegExp::WildcardUnix );

        for ( const auto& ensemble : ensembles )
        {
            auto ensembleName = ensemble->name();
            if ( searcher.exactMatch( ensembleName ) )
            {
                if ( searchString == dsFilter )
                {
                    // Match on ensemble name without realization filter

                    candidates.push_back( std::make_pair( ensembleName, ensemble ) );
                }
                else
                {
                    // Match on subset of realisations in ensemble

                    QString realizationSearchString = dsFilter.right( dsFilter.size() - dsFilter.indexOf( ':' ) - 1 );
                    QRegExp realizationSearcher( realizationSearchString, Qt::CaseInsensitive, QRegExp::WildcardUnix );

                    for ( const auto& summaryCase : ensemble->allSummaryCases() )
                    {
                        auto realizationName = summaryCase->displayCaseName();
                        if ( realizationSearcher.exactMatch( realizationName ) )
                        {
                            QString displayName = ensembleName + ":" + realizationName;
                            candidates.push_back( std::make_pair( displayName, summaryCase ) );
                        }
                    }
                }
            }
        }

        for ( const auto& summaryCase : summaryCases )
        {
            auto summaryCaseName = summaryCase->displayCaseName();
            if ( searcher.exactMatch( summaryCaseName ) )
            {
                candidates.push_back( std::make_pair( summaryCase->displayCaseName(), summaryCase ) );
            }
        }
    }

    return candidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSummaryPlotManager::dataSourceDisplayNames() const
{
    std::vector<QString>                        displayNames;
    std::vector<std::pair<QString, PdmObject*>> dataSources = findDataSourceCandidates();
    for ( const auto& dataSource : dataSources )
    {
        auto displayName = dataSource.first;
        displayNames.push_back( displayName );
    }

    return displayNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RimSummaryPlotManager::computeFilteredAddresses( const QStringList&                        textFilters,
                                                     const std::set<RifEclipseSummaryAddress>& sourceAddresses )
{
    std::set<RifEclipseSummaryAddress> addresses;

    std::vector<bool> usedFilters;
    RicSummaryPlotFeatureImpl::insertFilteredAddressesInSet( textFilters, sourceAddresses, &addresses, &usedFilters );

    if ( m_includeDiffCurves ) return addresses;

    const auto diffText = RifReaderEclipseSummary::differenceIdentifier();

    std::set<RifEclipseSummaryAddress> addressesWithoutDiffVectors;
    for ( const auto& adr : addresses )
    {
        if ( RiaStdStringTools::endsWith( adr.quantityName(), diffText ) ) continue;

        addressesWithoutDiffVectors.insert( adr );
    }

    return addressesWithoutDiffVectors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
{
    {
        auto attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attr )
        {
            if ( field == &m_pushButtonReplace )
            {
                attr->m_buttonText = "Replace Curves \n(Ctrl + Enter)";
            }
            if ( field == &m_pushButtonNewPlot )
            {
                attr->m_buttonText = "Create New Plot \n(Enter)";
            }
            if ( field == &m_pushButtonAppend )
            {
                attr->m_buttonText = "Append Curves \n(Shift + Enter)";
            }
        }
    }

    if ( field == &m_filterText )
    {
        auto attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->enableEditableContent  = true;
            attr->enableAutoComplete     = false;
            attr->adjustWidthToContents  = true;
            attr->notifyWhenTextIsEdited = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_summaryPlot );

    uiOrdering.add( &m_includeDiffCurves );

    uiOrdering.add( &m_filterText );
    uiOrdering.add( &m_addressCandidates );
    uiOrdering.add( &m_selectedDataSources, false );

    uiOrdering.add( &m_individualPlotPerVector );
    uiOrdering.add( &m_individualPlotPerDataSource );
    uiOrdering.add( &m_createMultiPlot );

    uiOrdering.add( &m_pushButtonAppend );
    uiOrdering.add( &m_pushButtonReplace, { false } );
    uiOrdering.add( &m_labelB, { false } );
    uiOrdering.add( &m_pushButtonNewPlot, { false } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurves()
{
    if ( !m_summaryPlot ) return;

    appendCurvesToPlot( m_summaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::replaceCurves()
{
    if ( !m_summaryPlot ) return;

    RimSummaryPlot* destinationPlot = m_summaryPlot;
    destinationPlot->deleteAllSummaryCurves();
    destinationPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    RiuPlotMainWindowTools::selectAsCurrentItem( destinationPlot );

    appendCurvesToPlot( destinationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::createNewPlot()
{
    std::vector<RimSummaryCase*>           summaryCases;
    std::vector<RimSummaryCaseCollection*> ensembles;
    findFilteredSummaryCasesAndEnsembles( summaryCases, ensembles );

    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource = filteredAddresses();

    RicSummaryPlotBuilder plotBuilder;
    plotBuilder.setAddresses( filteredAddressesFromSource );
    plotBuilder.setDataSources( summaryCases, ensembles );
    plotBuilder.setIndividualPlotPerAddress( m_individualPlotPerVector );
    plotBuilder.setIndividualPlotPerDataSource( m_individualPlotPerDataSource );

    auto plots = plotBuilder.createPlots();
    if ( m_createMultiPlot )
    {
        std::vector<RimPlot*> plotsForMultiPlot;
        for ( auto p : plots )
        {
            p->loadDataAndUpdate();
            plotsForMultiPlot.push_back( dynamic_cast<RimPlot*>( p ) );
        }

        RicSummaryPlotBuilder::createAndAppendMultiPlot( plotsForMultiPlot );

        {
            auto                  myCopyOfPlots = plotBuilder.createPlots();
            std::vector<RimPlot*> myRimPlots;
            for ( auto p : myCopyOfPlots )
            {
                p->loadDataAndUpdate();
                myRimPlots.push_back( dynamic_cast<RimPlot*>( p ) );
            }
            RimMultiSummaryPlot::createAndAppendMultiPlot( myRimPlots );
        }
    }
    else
    {
        auto plotCollection = RiaSummaryTools::summaryPlotCollection();
        for ( auto plot : plots )
        {
            plot->setAsPlotMdiWindow();

            plotCollection->addPlot( plot );

            plot->loadDataAndUpdate();
        }
    }

    updateProjectTreeAndRefresUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotManager::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

        if ( keyEvent && ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return ) )
        {
            auto mods = keyEvent->modifiers();

            if ( mods & Qt::ShiftModifier )
                appendCurves();
            else if ( mods & Qt::ControlModifier )
                replaceCurves();
            else if ( mods == Qt::NoModifier )
                createNewPlot();
        }
    }

    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>>
    RimSummaryPlotManager::allDataSourcesInProject() const
{
    auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection();

    auto summaryCases = sumCaseMainColl->topLevelSummaryCases();
    auto ensembles    = sumCaseMainColl->summaryCaseCollections();

    return { summaryCases, ensembles };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateUiFromSelection()
{
    auto destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimSummaryPlot* summaryPlot = nullptr;
    if ( destinationObject ) destinationObject->firstAncestorOrThisOfType( summaryPlot );

    if ( summaryPlot && ( m_summaryPlot() != summaryPlot ) )
    {
        m_summaryPlot = summaryPlot;

        updateSelectionFromUiChange();
        updateCurveCandidates();
    }

    if ( !summaryPlot )
    {
        m_summaryPlot = nullptr;

        std::vector<QString> tmp;
        m_addressCandidates = tmp;
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotManager::filteredAddresses()
{
    std::vector<RimSummaryCase*>           summaryCases;
    std::vector<RimSummaryCaseCollection*> ensembles;
    findFilteredSummaryCasesAndEnsembles( summaryCases, ensembles );

    std::set<RifEclipseSummaryAddress> nativeAddresses;
    if ( !summaryCases.empty() )
    {
        nativeAddresses = RicSummaryPlotBuilder::addressesForSource( summaryCases.front() );
    }
    else if ( !ensembles.empty() )
    {
        nativeAddresses = RicSummaryPlotBuilder::addressesForSource( ensembles.front() );
    }

    if ( nativeAddresses.empty() ) return {};

    QStringList allCurveAddressFilters = m_filterText().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    return computeFilteredAddresses( allCurveAddressFilters, nativeAddresses );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurvesToPlot( RimSummaryPlot* destinationPlot )
{
    CAF_ASSERT( destinationPlot );

    std::vector<RimSummaryCase*>           summaryCases;
    std::vector<RimSummaryCaseCollection*> ensembles;
    findFilteredSummaryCasesAndEnsembles( summaryCases, ensembles );

    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource = filteredAddresses();
    RicSummaryPlotBuilder::appendCurvesToPlot( destinationPlot, filteredAddressesFromSource, summaryCases, ensembles );

    destinationPlot->applyDefaultCurveAppearances();
    destinationPlot->loadDataAndUpdate();

    updateProjectTreeAndRefresUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateFilterTextHistory()
{
    RiaStringListSerializer stringListSerializer( curveFilterRecentlyUsedRegistryKey() );

    int maxItemCount = 10;
    stringListSerializer.addString( m_filterText, maxItemCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateProjectTreeAndRefresUi()
{
    RiaSummaryTools::summaryPlotCollection()->updateConnectedEditors();

    updateFilterTextHistory();
    m_filterText.uiCapability()->updateConnectedEditors();

    setFocusToFilterText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateSelectionFromUiChange()
{
    QStringList dataSourceFilters = extractDataSourceFilters();
    if ( m_previousDataSourceText != dataSourceFilters )
    {
        m_selectedDataSources = dataSourceDisplayNames();

        m_previousDataSourceText = dataSourceFilters;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::setFocusToEditorWidget( caf::PdmUiFieldHandle* uiFieldHandle )
{
    CAF_ASSERT( uiFieldHandle );

    auto editors = uiFieldHandle->connectedEditors();
    if ( !editors.empty() )
    {
        auto fieldEditorHandle = dynamic_cast<caf::PdmUiFieldEditorHandle*>( editors.front() );
        if ( fieldEditorHandle && fieldEditorHandle->editorWidget() )
        {
            auto widget = fieldEditorHandle->editorWidget();

            // If the dock widget is floating, activateWindow() must be called to make sure the top level widget has
            // focus before the editor widget is given focus
            widget->activateWindow();
            widget->setFocus();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::splitIntoAddressAndDataSourceFilters( QStringList& addressFilters,
                                                                  QStringList& dataSourceFilters ) const
{
    QStringList filterItems = m_filterText().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    auto [summaryCases, ensembles] = allDataSourcesInProject();

    QStringList dataSourceNames;
    for ( const auto& summaryCase : summaryCases )
    {
        dataSourceNames.push_back( summaryCase->displayCaseName() );
    }

    for ( const auto& ensemble : ensembles )
    {
        dataSourceNames.push_back( ensemble->name() );
    }

    RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( filterItems,
                                                                 dataSourceNames,
                                                                 addressFilters,
                                                                 dataSourceFilters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimSummaryPlotManager::extractDataSourceFilters() const
{
    QStringList addressFilters;
    QStringList dataSourceFilters;
    splitIntoAddressAndDataSourceFilters( addressFilters, dataSourceFilters );

    // If no filter on data source is specified, use wildcard to match all
    if ( dataSourceFilters.empty() ) dataSourceFilters.push_back( "*" );

    return dataSourceFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::findFilteredSummaryCasesAndEnsembles( std::vector<RimSummaryCase*>&           summaryCases,
                                                                  std::vector<RimSummaryCaseCollection*>& ensembles ) const
{
    auto filteredDataSources = findDataSourceCandidates();
    for ( const auto& [dataSourceName, dataSource] : filteredDataSources )
    {
        auto selectedDataSources = m_selectedDataSources();

        if ( std::find( selectedDataSources.begin(), selectedDataSources.end(), dataSourceName ) ==
             std::end( selectedDataSources ) )
            continue;

        auto summaryCase = dynamic_cast<RimSummaryCase*>( dataSource );
        if ( summaryCase )
        {
            summaryCases.push_back( summaryCase );
        }

        auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( dataSource );
        if ( ensemble )
        {
            ensembles.push_back( ensemble );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotManager::curveFilterRecentlyUsedRegistryKey()
{
    return "SummaryPlotManagerCurveFilterStrings";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::onSummaryDataSourceHasChanged( const caf::SignalEmitter* emitter )
{
    resetDataSourceSelection();
    updateSelectionFromUiChange();
}
