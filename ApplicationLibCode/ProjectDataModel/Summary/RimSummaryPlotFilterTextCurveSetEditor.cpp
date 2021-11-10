/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Statoil ASA
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

#include "RimSummaryPlotFilterTextCurveSetEditor.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaStringListSerializer.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimGridSummaryCase.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuSummaryVectorSelectionUi.h"

#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiToolBarEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QRegularExpression>

#define FILTER_TEXT_OUTDATED_TEXT "<Outdated>"

CAF_PDM_SOURCE_INIT( RimSummaryPlotFilterTextCurveSetEditor, "SummaryPlotFilterTextCurveSetEditor" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotFilterTextCurveSetEditor::RimSummaryPlotFilterTextCurveSetEditor()
    : m_isFieldRecentlyChangedFromGui( false )
{
    CAF_PDM_InitObject( "Curve Set Filter Text", "", "", "" );

    // clang-format off
    QString filterTextHeading = "Create Summary Curves from Text";
    QString filterTextShortcut = " (Ctrl + F)";

    QString filterTextToolTip =
        "A list of vector addresses separated by spaces using the syntax: <vectorshortname>[:<item>[:<subitem>[:i,j,k]]]\n"
        "Wildcards can also be used. Examples:\n"
        "  \"WOPT:*\" One total oil production curve for each well.\n"
        "  \"FOPT FWPT\" Two curves with oil and water total production.\n"
        "  \"BPR:15,28,*\" (no space) Oil phase pressure for all blocks along k as separate curves.\n";
    // clang-format on

    QString toolTipPropertyEditor = filterTextHeading + "\n\n" + filterTextToolTip;
    QString toolTipToolbar        = filterTextHeading + filterTextShortcut + "\n\n" + filterTextToolTip;

    CAF_PDM_InitFieldNoDefault( &m_curveFilterLabelText, "Summary", "Summary", "", "", "" );
    m_curveFilterLabelText.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_curveFilterLabelText.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_curveFilterText, "CurveFilterText", "Curve Filter Text", "", toolTipPropertyEditor, "" );
    m_curveFilterText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_curveFilterText.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    // Special tool tip for toolbar
    m_curveFilterText.uiCapability()->setUiToolTip( toolTipToolbar, caf::PdmUiToolBarEditor::uiEditorConfigName() );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "SummaryCases", "Sources", "", "", "" );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_includeDiffCurves,
                       "IncludeDiffCurves",
                       true,
                       "Include Difference Curves",
                       "",
                       "Difference between simulated and observed(history) curve",
                       "" );

    CAF_PDM_InitField( &m_includeHistoryCurves, "IncludeHistoryCurves", true, "Include History Curves", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_curveCandidates, "CurveCandidates", "Candidates", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotFilterTextCurveSetEditor::~RimSummaryPlotFilterTextCurveSetEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotFilterTextCurveSetEditor::fieldsToShowInToolbar()
{
    std::vector<caf::PdmFieldHandle*> fields;
    fields.push_back( &m_curveFilterLabelText );
    fields.push_back( &m_curveFilterText );

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotFilterTextCurveSetEditor::curveFilterFieldKeyword()
{
    return "CurveFilterText";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::updateTextFilter()
{
    RimSummaryPlot* parentPlot;
    this->firstAncestorOrThisOfType( parentPlot );
    std::set<SummarySource*>              sourcesFromExistingCurves;
    std::set<RifEclipseSummaryAddress>    addressesInUse;
    std::vector<RigGridCellResultAddress> gridaddressesInUse;

    if ( parentPlot )
    {
        std::vector<RimEnsembleCurveSet*> ensembleCurveSets = parentPlot->ensembleCurveSetCollection()->curveSets();
        for ( auto ensCurvSet : ensembleCurveSets )
        {
            sourcesFromExistingCurves.insert( ensCurvSet->summaryCaseCollection() );
            addressesInUse.insert( ensCurvSet->summaryAddress() );
        }

        std::vector<RimSummaryCurve*> sumCurves = parentPlot->summaryCurveCollection()->curves();
        for ( auto sumCurve : sumCurves )
        {
            sourcesFromExistingCurves.insert( sumCurve->summaryCaseY() );
            addressesInUse.insert( sumCurve->summaryAddressY() );
        }

        std::vector<RimGridTimeHistoryCurve*> gridTimeHistoryCurves = parentPlot->gridTimeHistoryCurves();
        for ( auto grCurve : gridTimeHistoryCurves )
        {
            RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>( grCurve->gridCase() );
            if ( eclCase )
            {
                sourcesFromExistingCurves.insert( eclCase );
                gridaddressesInUse.push_back( grCurve->resultAddress() );
            }
        }
    }

    std::vector<caf::PdmPointer<SummarySource>> usedSources( sourcesFromExistingCurves.begin(),
                                                             sourcesFromExistingCurves.end() );

    if ( !usedSources.empty() )
    {
        m_selectedSources.clear();
        m_selectedSources.setValue( usedSources );
    }

    // Check if existing filter text matches all the curves
    // Todo: possibly check grid time history curves also

    QStringList allCurveAddressFilters =
        curveFilterTextWithoutOutdatedLabel().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    std::vector<bool>                  usedFilters;
    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource;
    RicSummaryPlotFeatureImpl::insertFilteredAddressesInSet( allCurveAddressFilters,
                                                             addressesInUse,
                                                             &filteredAddressesFromSource,
                                                             &usedFilters );

    if ( filteredAddressesFromSource != addressesInUse )
    {
        // m_curveFilterText = FILTER_TEXT_OUTDATED_TEXT + curveFilterTextWithoutOutdatedLabel();
        m_curveFilterText = "";
    }
    else
    {
        m_curveFilterText = curveFilterTextWithoutOutdatedLabel();
    }

    m_curveFilterText.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                               const QVariant&            oldValue,
                                                               const QVariant&            newValue )
{
    updateParentPlot();
    updateCurveCandidates();

    if ( changedField == &m_curveFilterText )
    {
        m_curveFilterText = curveFilterTextWithoutOutdatedLabel();

        {
            RiaStringListSerializer stringListSerializer( curveFilterRecentlyUsedRegistryKey() );

            int maxItemCount = 10;
            stringListSerializer.addString( m_curveFilterText, maxItemCount );
        }

        m_curveFilterText.uiCapability()->updateConnectedEditors();
    }

    m_isFieldRecentlyChangedFromGui = true;

    if ( RiaGuiApplication::isRunning() )
    {
        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( mainPlotWindow )
        {
            mainPlotWindow->updateSummaryPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveFilterText );
    uiOrdering.add( &m_includeHistoryCurves );
    uiOrdering.add( &m_includeDiffCurves );
    uiOrdering.add( &m_curveCandidates );

    uiOrdering.add( &m_selectedSources );
    uiOrdering.skipRemainingFields();

    if ( !m_isFieldRecentlyChangedFromGui )
    {
        updateTextFilter();
    }

    m_isFieldRecentlyChangedFromGui = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::setupBeforeSave()
{
    m_curveFilterText = curveFilterTextWithoutOutdatedLabel();

    // If a source case has been deleted, make sure null pointers are removed
    m_selectedSources.removePtr( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                                    QString                    uiConfigName,
                                                                    caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_curveFilterText )
    {
        auto attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->enableEditableContent = true;
            attr->adjustWidthToContents = true;
            attr->placeholderText       = "Click to edit curves";

            if ( uiConfigName == caf::PdmUiToolBarEditor::uiEditorConfigName() )
            {
                attr->minimumWidth = 140;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotFilterTextCurveSetEditor::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                   bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_selectedSources )
    {
        appendOptionItemsForSources( options );
    }

    if ( fieldNeedingOptions == &m_curveFilterText )
    {
        RiaStringListSerializer stringListSerializer( curveFilterRecentlyUsedRegistryKey() );

        for ( const auto& s : stringListSerializer.textStrings() )
        {
            options.push_back( caf::PdmOptionItemInfo( s, s ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::appendOptionItemsForSources( QList<caf::PdmOptionItemInfo>& options )
{
    RimProject* proj = RimProject::current();

    std::vector<RimOilField*> oilFields;

    proj->allOilFields( oilFields );
    for ( RimOilField* oilField : oilFields )
    {
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if ( sumCaseMainColl )
        {
            // Top level cases
            for ( const auto& sumCase : sumCaseMainColl->topLevelSummaryCases() )
            {
                options.push_back( caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase ) );
            }

            // Ensembles
            bool ensembleHeaderCreated = false;
            for ( const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections() )
            {
                if ( !sumCaseColl->isEnsemble() ) continue;

                if ( !ensembleHeaderCreated )
                {
                    options.push_back( caf::PdmOptionItemInfo::createHeader( "Ensembles", true ) );
                    ensembleHeaderCreated = true;
                }

                auto optionItem = caf::PdmOptionItemInfo( sumCaseColl->name(), sumCaseColl );
                optionItem.setLevel( 1 );
                options.push_back( optionItem );
            }

            // Grouped cases
            for ( const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections() )
            {
                if ( sumCaseColl->isEnsemble() ) continue;

                options.push_back( caf::PdmOptionItemInfo::createHeader( sumCaseColl->name(), true ) );

                for ( const auto& sumCase : sumCaseColl->allSummaryCases() )
                {
                    auto optionItem = caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase );
                    optionItem.setLevel( 1 );
                    options.push_back( optionItem );
                }
            }

            // Observed data
            auto observedDataColl = oilField->observedDataCollection();
            if ( observedDataColl->allObservedSummaryData().size() > 0 )
            {
                options.push_back( caf::PdmOptionItemInfo::createHeader( "Observed Data", true ) );

                for ( const auto& obsData : observedDataColl->allObservedSummaryData() )
                {
                    auto optionItem = caf::PdmOptionItemInfo( obsData->caseName(), obsData );
                    optionItem.setLevel( 1 );
                    options.push_back( optionItem );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SummarySource*> RimSummaryPlotFilterTextCurveSetEditor::selectedSummarySources() const
{
    std::vector<SummarySource*> sources;

    for ( const auto& source : m_selectedSources )
    {
        sources.push_back( source );
    }

    // Always add the summary case for calculated curves as this case is not displayed in UI
    sources.push_back( RimProject::current()->calculationCollection()->calculationSummaryCase() );

    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotFilterTextCurveSetEditor::curveFilterTextWithoutOutdatedLabel() const
{
    QString filterText = m_curveFilterText();

    if ( filterText.startsWith( FILTER_TEXT_OUTDATED_TEXT ) )
    {
        return filterText.right( filterText.length() - QString( FILTER_TEXT_OUTDATED_TEXT ).length() );
    }

    return filterText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::updateParentPlot()
{
    RimSummaryPlot* parentPlot;
    this->firstAncestorOrThisOfType( parentPlot );

    if ( parentPlot )
    {
        // Remove all curves, Create new ones

        parentPlot->ensembleCurveSetCollection()->deleteAllCurveSets();
        parentPlot->deleteAllSummaryCurves();
        parentPlot->deleteAllGridTimeHistoryCurves();

        std::set<RiaSummaryCurveDefinition> curveDefinitions;

        QStringList allCurveAddressFilters =
            curveFilterTextWithoutOutdatedLabel().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
        std::vector<bool> accumulatedUsedFilters( allCurveAddressFilters.size(), false );

        for ( SummarySource* currSource : selectedSummarySources() )
        {
            std::set<RifEclipseSummaryAddress> allAddressesFromSource = addressesForSource( currSource );

            std::vector<bool>                  usedFilters;
            std::set<RifEclipseSummaryAddress> filteredAddressesFromSource;
            insertFilteredAddressesInSet( allCurveAddressFilters,
                                          allAddressesFromSource,
                                          &filteredAddressesFromSource,
                                          &usedFilters );

            for ( size_t fIdx = 0; fIdx < accumulatedUsedFilters.size(); ++fIdx )
            {
                accumulatedUsedFilters[fIdx] = accumulatedUsedFilters[fIdx] || usedFilters[fIdx];
            }

            auto sumCase  = dynamic_cast<RimSummaryCase*>( currSource );
            auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( currSource );

            for ( const auto& filteredAddress : filteredAddressesFromSource )
            {
                if ( sumCase )
                {
                    curveDefinitions.insert( RiaSummaryCurveDefinition( sumCase, filteredAddress, ensemble != nullptr ) );
                }
                else if ( ensemble )
                {
                    curveDefinitions.insert( RiaSummaryCurveDefinition( ensemble, filteredAddress ) );
                }
            }
        }

        // Find potensial grid result addresses

        QRegularExpression gridAddressPattern( "^[A-Z]+:[0-9]+,[0-9]+,[0-9]+$" );
        QStringList        gridResultAddressFilters;

        for ( int filterIdx = 0; filterIdx < allCurveAddressFilters.size(); ++filterIdx )
        {
            if ( !accumulatedUsedFilters[filterIdx] )
            {
                const QString& unusedAddressFilter = allCurveAddressFilters[filterIdx];
                if ( gridAddressPattern.match( unusedAddressFilter ).hasMatch() )
                {
                    gridResultAddressFilters.push_back( unusedAddressFilter );
                }
                else
                {
                    RiaLogging::warning( "No summary or restart vectors matched \"" + unusedAddressFilter + "\"" );
                }
            }
        }

        // Create Summary curves and Ensemble curvesets:

        for ( const RiaSummaryCurveDefinition& curveDef : curveDefinitions )
        {
            if ( curveDef.isEnsembleCurve() )
            {
                RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

                curveSet->setSummaryCaseCollection( curveDef.ensemble() );
                curveSet->setSummaryAddress( curveDef.summaryAddress() );

                parentPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
            }
            else
            {
                RimSummaryCurve* newCurve = new RimSummaryCurve();
                parentPlot->addCurveNoUpdate( newCurve );
                if ( curveDef.summaryCase() )
                {
                    newCurve->setSummaryCaseY( curveDef.summaryCase() );
                }
                newCurve->setSummaryAddressYAndApplyInterpolation( curveDef.summaryAddress() );
            }
        }

        // create Grid time history curves
        {
            std::vector<RimEclipseCase*> gridCasesToPlotFrom;

            for ( SummarySource* currSource : selectedSummarySources() )
            {
                RimGridSummaryCase* gridSumCase = dynamic_cast<RimGridSummaryCase*>( currSource );
                if ( gridSumCase )
                {
                    RimEclipseCase* eclCase = gridSumCase->associatedEclipseCase();

                    if ( eclCase )
                    {
                        gridCasesToPlotFrom.push_back( eclCase );
                    }
                }
            }

            bool isEnsembleMode  = gridCasesToPlotFrom.size() > 1;
            int  curveColorIndex = 0;

            for ( const QString& gridAddressFilter : gridResultAddressFilters )
            {
                std::vector<RigGridCellResultAddress> cellResAddrs =
                    RigGridCellResultAddress::createGridCellAddressesFromFilter( gridAddressFilter );

                for ( RigGridCellResultAddress cellResAddr : cellResAddrs )
                {
                    for ( RimEclipseCase* eclCase : gridCasesToPlotFrom )
                    {
                        RigCaseCellResultsData* gridCellResults =
                            eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
                        if ( !( gridCellResults && gridCellResults->resultInfo( cellResAddr.eclipseResultAddress ) ) )
                        {
                            RiaLogging::warning( "Could not find a restart result property with name: \"" +
                                                 cellResAddr.eclipseResultAddress.resultName() + "\"" );
                            continue;
                        }

                        RimGridTimeHistoryCurve* newCurve = new RimGridTimeHistoryCurve();
                        newCurve->setFromEclipseCellAndResult( eclCase,
                                                               cellResAddr.gridIndex,
                                                               cellResAddr.i,
                                                               cellResAddr.j,
                                                               cellResAddr.k,
                                                               cellResAddr.eclipseResultAddress );
                        newCurve->setLineThickness( 2 );
                        newCurve->setColor( RicWellLogPlotCurveFeatureImpl::curveColorFromTable( curveColorIndex ) );

                        if ( !isEnsembleMode ) ++curveColorIndex;

                        parentPlot->addGridTimeHistoryCurveNoUpdate( newCurve );
                    }
                    if ( isEnsembleMode ) ++curveColorIndex;
                }
            }
        }

        parentPlot->applyDefaultCurveAppearances();
        parentPlot->loadDataAndUpdate();
        parentPlot->zoomAll();

        m_isFieldRecentlyChangedFromGui = true;

        parentPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::updateCurveCandidates()
{
    m_curveCandidates.value().clear();

    if ( selectedSummarySources().empty() ) return;

    QStringList allCurveAddressFilters =
        curveFilterTextWithoutOutdatedLabel().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    auto firstSource = selectedSummarySources().front();

    std::set<RifEclipseSummaryAddress> allAddressesFromSource = addressesForSource( firstSource );

    std::vector<bool>                  usedFilters;
    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource;
    insertFilteredAddressesInSet( allCurveAddressFilters, allAddressesFromSource, &filteredAddressesFromSource, &usedFilters );

    std::vector<QString> curveCandidates;

    for ( const auto& adr : filteredAddressesFromSource )
    {
        curveCandidates.push_back( QString::fromStdString( adr.uiText() ) );
    }

    m_curveCandidates = curveCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotFilterTextCurveSetEditor::addressesForSource( SummarySource* summarySource )
{
    auto* ensemble = dynamic_cast<RimSummaryCaseCollection*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto* sumCase = dynamic_cast<RimSummaryCase*>( summarySource );

    if ( sumCase )
    {
        RifSummaryReaderInterface* reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotFilterTextCurveSetEditor::insertFilteredAddressesInSet(
    const QStringList&                        curveFilters,
    const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
    std::set<RifEclipseSummaryAddress>*       setToInsertFilteredAddressesIn,
    std::vector<bool>*                        usedFilters )
{
    std::set<RifEclipseSummaryAddress> candidateAddresses;
    RicSummaryPlotFeatureImpl::insertFilteredAddressesInSet( curveFilters, allAddressesInCase, &candidateAddresses, usedFilters );

    if ( !m_includeDiffCurves || !m_includeHistoryCurves )
    {
        std::set<RifEclipseSummaryAddress> tmp;

        const auto diffText = RifReaderEclipseSummary::differenceIdentifier();

        for ( const auto& adr : candidateAddresses )
        {
            if ( !m_includeDiffCurves && RiaStdStringTools::endsWith( adr.quantityName(), diffText ) ) continue;
            if ( !m_includeHistoryCurves && adr.isHistoryQuantity() ) continue;

            tmp.insert( adr );
        }

        std::swap( tmp, candidateAddresses );
    }

    setToInsertFilteredAddressesIn->insert( candidateAddresses.begin(), candidateAddresses.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotFilterTextCurveSetEditor::curveFilterRecentlyUsedRegistryKey()
{
    return "curveFilterRecentlyUsedStrings";
}
