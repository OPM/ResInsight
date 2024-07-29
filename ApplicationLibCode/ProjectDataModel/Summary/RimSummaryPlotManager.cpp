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
#include "RiaTextStringTools.h"

#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafSelectionManager.h"

// Multi plot
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryMultiPlot.h"

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
    m_addressCandidates.uiCapability()->setUiReadOnly( true );

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
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonReplace );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonNewPlot, "PushButtonNewPlot", "New (Alt + Enter)" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonNewPlot );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonAppend, "PushButtonAppend", "Append (Shift + Enter)" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonAppend );

    CAF_PDM_InitFieldNoDefault( &m_labelA, "LabelA", "" );
    m_labelA.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelA.xmlCapability()->disableIO();
    m_labelA.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_labelB, "LabelB", "" );
    m_labelB.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelB.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_individualPlotPerObject, "IndividualPlotPerObject", true, "One plot per Object" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_individualPlotPerObject );

    CAF_PDM_InitField( &m_individualPlotPerVector, "IndividualPlotPerVector", false, "One plot per Vector" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_individualPlotPerVector );

    CAF_PDM_InitField( &m_individualPlotPerDataSource, "IndividualPlotPerDataSource", false, "One plot per Data Source" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_individualPlotPerDataSource );

    CAF_PDM_InitField( &m_createMultiPlot, "CreateMultiPlot", true, "Create Multiple Plots in One Window" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createMultiPlot );
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
void RimSummaryPlotManager::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
QList<caf::PdmOptionItemInfo> RimSummaryPlotManager::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_summaryPlot )
    {
        auto coll = RiaSummaryTools::summaryMultiPlotCollection();
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
    else if ( fieldNeedingOptions == &m_addressCandidates )
    {
        auto addresses = filteredAddresses();

        for ( const auto& adr : addresses )
        {
            QString text = QString::fromStdString( adr.uiText() );
            options.push_back( caf::PdmOptionItemInfo( text, text ) );
        }
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

    auto [summaryCases, ensembles] = RiaSummaryStringTools::dataSourcesMatchingFilters( dataSourceFilters );

    for ( auto ensemble : ensembles )
    {
        auto ensembleName = ensemble->name();

        candidates.push_back( std::make_pair( ensembleName, ensemble ) );
    }

    for ( const auto& summaryCase : summaryCases )
    {
        auto summaryCaseName = summaryCase->displayCaseName();

        QString displayName;
        auto    ensemble = summaryCase->ensemble();
        if ( ensemble )
        {
            displayName = ensemble->name() + ":" + summaryCaseName;
        }
        else
        {
            displayName = summaryCaseName;
        }

        candidates.push_back( std::make_pair( displayName, summaryCase ) );
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
void RimSummaryPlotManager::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
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
    uiOrdering.appendToRow( &m_selectedDataSources );

    uiOrdering.add( &m_individualPlotPerVector );
    uiOrdering.appendToRow( &m_individualPlotPerDataSource );
    uiOrdering.add( &m_individualPlotPerObject );
    uiOrdering.appendToRow( &m_createMultiPlot );

    uiOrdering.add( &m_pushButtonAppend );
    uiOrdering.appendToRow( &m_pushButtonReplace );
    uiOrdering.appendToRow( &m_labelB );
    uiOrdering.appendToRow( &m_pushButtonNewPlot );
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
    if ( m_filterText().trimmed().isEmpty() ) return;

    std::vector<RimSummaryCase*>     summaryCases;
    std::vector<RimSummaryEnsemble*> ensembles;
    findFilteredSummaryCasesAndEnsembles( summaryCases, ensembles );

    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource;

    for ( const auto& adr : filteredAddresses() )
    {
        for ( const auto& textSelection : m_addressCandidates() )
        {
            if ( adr.uiText() == textSelection.toStdString() )
            {
                filteredAddressesFromSource.insert( adr );
                continue;
            }
        }
    }

    RicSummaryPlotBuilder plotBuilder;
    plotBuilder.setAddresses( filteredAddressesFromSource );
    plotBuilder.setDataSources( summaryCases, ensembles );

    RicSummaryPlotBuilder::RicGraphCurveGrouping grouping = RicSummaryPlotBuilder::RicGraphCurveGrouping::NONE;
    if ( m_individualPlotPerVector ) grouping = RicSummaryPlotBuilder::RicGraphCurveGrouping::SINGLE_CURVES;
    if ( m_individualPlotPerObject ) grouping = RicSummaryPlotBuilder::RicGraphCurveGrouping::CURVES_FOR_OBJECT;
    plotBuilder.setGrouping( grouping );

    plotBuilder.setIndividualPlotPerDataSource( m_individualPlotPerDataSource );

    auto summaryPlots = plotBuilder.createPlots();
    if ( m_createMultiPlot )
    {
        RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( summaryPlots );
    }
    else
    {
        for ( auto plot : summaryPlots )
        {
            RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( plot );
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
void RimSummaryPlotManager::updateUiFromSelection()
{
    auto destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimSummaryPlot* summaryPlot = nullptr;
    if ( destinationObject ) summaryPlot = destinationObject->firstAncestorOrThisOfType<RimSummaryPlot>();

    if ( summaryPlot && ( m_summaryPlot() != summaryPlot ) )
    {
        m_summaryPlot = summaryPlot;

        updateSelectionFromUiChange();
        updateCurveCandidates();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotManager::filteredAddresses()
{
    std::vector<RimSummaryCase*>     summaryCases;
    std::vector<RimSummaryEnsemble*> ensembles;
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

    QStringList allCurveAddressFilters = RiaTextStringTools::splitSkipEmptyParts( m_filterText(), QRegExp( "\\s+" ) );

    return RiaSummaryStringTools::computeFilteredAddresses( allCurveAddressFilters, nativeAddresses, m_includeDiffCurves );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurvesToPlot( RimSummaryPlot* destinationPlot )
{
    CAF_ASSERT( destinationPlot );

    std::vector<RimSummaryCase*>     summaryCases;
    std::vector<RimSummaryEnsemble*> ensembles;
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
    RiaSummaryTools::summaryMultiPlotCollection()->updateConnectedEditors();

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
QStringList RimSummaryPlotManager::extractDataSourceFilters() const
{
    auto [addressFilters, dataSourceFilters] = RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( m_filterText() );

    return dataSourceFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::findFilteredSummaryCasesAndEnsembles( std::vector<RimSummaryCase*>&     summaryCases,
                                                                  std::vector<RimSummaryEnsemble*>& ensembles ) const
{
    auto filteredDataSources = findDataSourceCandidates();
    for ( const auto& [dataSourceName, dataSource] : filteredDataSources )
    {
        auto selectedDataSources = m_selectedDataSources();

        if ( std::find( selectedDataSources.begin(), selectedDataSources.end(), dataSourceName ) == std::end( selectedDataSources ) )
            continue;

        auto summaryCase = dynamic_cast<RimSummaryCase*>( dataSource );
        if ( summaryCase )
        {
            summaryCases.push_back( summaryCase );
        }

        auto ensemble = dynamic_cast<RimSummaryEnsemble*>( dataSource );
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
