/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimMultiPlot.h"

#include "RiaSummaryStringTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultipleSummaryPlotNameHelper.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryPlotSourceStepping.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "RiuSummaryVectorSelectionUi.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlot, "MultiSummaryPlot" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot::RimSummaryMultiPlot()
{
    CAF_PDM_InitObject( "Multi Summary Plot" );
    this->setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_filterText, "FilterText", "Filter Text" );
    m_filterText.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_individualPlotPerVector, "IndividualPlotPerVector", false, "One plot per Vector" );
    CAF_PDM_InitField( &m_individualPlotPerObject, "IndividualPlotPerObject", false, "One plot per Object" );
    CAF_PDM_InitField( &m_individualPlotPerDataSource, "IndividualPlotPerDataSource", false, "One plot per Data Source" );
    CAF_PDM_InitField( &m_autoPlotTitles, "AutoPlotTitles", false, "Auto Plot Titles" );
    CAF_PDM_InitField( &m_autoPlotTitlesOnSubPlots, "AutoPlotTitlesSubPlots", false, "Auto Plot Titles Sub Plots" );

    CAF_PDM_InitField( &m_showMultiPlotInProjectTree, "ShowMultiPlotInProjectTree", true, "Show Multi Plot In Project Tree" );

    CAF_PDM_InitFieldNoDefault( &m_multiPlot, "MultiPlot", "Multi Plot" );
    m_multiPlot.uiCapability()->setUiTreeHidden( true );
    m_multiPlot = new RimMultiPlot;

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );
    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    m_nameHelper = std::make_unique<RimSummaryPlotNameHelper>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot::~RimSummaryMultiPlot()
{
    removeMdiWindowFromMdiArea();
    m_multiPlot->cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryMultiPlot::viewWidget()
{
    return m_multiPlot->viewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimSummaryMultiPlot::snapshotWindowContent()
{
    return m_multiPlot->snapshotWindowContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::zoomAll()
{
    m_multiPlot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryMultiPlot::description() const
{
    return "RimSummaryMultiPlot Placeholder Text";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::addPlot( RimSummaryPlot* plot )
{
    m_multiPlot->addPlot( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RimSummaryMultiPlot::createAndAppendMultiPlot( const std::vector<RimSummaryPlot*>& plots )
{
    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->multiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot;
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiSummaryPlot( plotWindow );

    insertGraphsIntoPlot( plotWindow, plots );

    plotCollection->updateAllRequiredEditors();

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryDataSourceStepping::Axis> RimSummaryMultiPlot::availableAxes() const
{
    return { RimSummaryDataSourceStepping::Axis::X_AXIS };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->curvesForStepping( axis ) )
        {
            curves.push_back( curve );
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimSummaryMultiPlot::curveSets() const
{
    std::vector<RimEnsembleCurveSet*> curveSets;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curveSet : summaryPlot->curveSets() )
        {
            curveSets.push_back( curveSet );
        }
    }

    return curveSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::allCurves( RimSummaryDataSourceStepping::Axis axis ) const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->allCurves( axis ) )
        {
            curves.push_back( curve );
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::populateNameHelper( RimSummaryPlotNameHelper* nameHelper )
{
    nameHelper->clear();

    std::vector<RifEclipseSummaryAddress>  addresses;
    std::vector<RimSummaryCase*>           sumCases;
    std::vector<RimSummaryCaseCollection*> ensembleCases;

    for ( RimSummaryCurve* curve : allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
    {
        addresses.push_back( curve->summaryAddressY() );
        sumCases.push_back( curve->summaryCaseY() );
    }

    for ( auto curveSet : curveSets() )
    {
        addresses.push_back( curveSet->summaryAddress() );
        ensembleCases.push_back( curveSet->summaryCaseCollection() );
    }

    nameHelper->appendAddresses( addresses );
    nameHelper->setSummaryCases( sumCases );
    nameHelper->setEnsembleCases( ensembleCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryMultiPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr*/ )
{
    return m_multiPlot->createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::deleteViewWidget()
{
    m_multiPlot->deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_autoPlotTitles )
    {
        updatePlotTitles();
    }

    m_multiPlot->onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    m_multiPlot->doRenderWindowContent( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryMultiPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                          bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_autoPlotTitles );
    uiOrdering.add( &m_autoPlotTitlesOnSubPlots );

    {
        auto group = uiOrdering.addNewGroup( "Data Source" );
        m_sourceStepping()->uiOrdering( uiConfigName, *group );
    }

    auto group = uiOrdering.addNewGroup( "Multi Plot Options" );
    m_multiPlot->uiOrderingForSummaryMultiPlot( *group );

    {
        auto group = uiOrdering.addNewGroup( "Graph Building" );
        group->setCollapsedByDefault( true );

        group->add( &m_filterText );
        group->add( &m_individualPlotPerVector );
        group->add( &m_individualPlotPerDataSource );
        group->add( &m_individualPlotPerObject );
    }
    uiOrdering.add( &m_showMultiPlotInProjectTree );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow && m_showWindow() )
    {
        // Plots contained in a RimMultiPlot will automatically be set invisible
        // Restore plot visibility

        for ( auto p : m_multiPlot->plots() )
        {
            p->setShowWindow( true );
        }
    }
    else if ( changedField == &m_filterText || changedField == &m_individualPlotPerDataSource ||
              changedField == &m_individualPlotPerVector || changedField == &m_individualPlotPerObject )
    {
        updatePlots();
    }
    else if ( changedField == &m_autoPlotTitles || changedField == &m_autoPlotTitlesOnSubPlots )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
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
void RimSummaryMultiPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( !m_showMultiPlotInProjectTree );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlots()
{
    auto [addressFilters, dataSourceFilters] =
        RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( m_filterText() );

    auto [matchingSummaryCases, matchingEnsembles] = RiaSummaryStringTools::dataSourcesMatchingFilters( dataSourceFilters );

    std::set<RifEclipseSummaryAddress> allAddresses;
    if ( !matchingSummaryCases.empty() )
    {
        allAddresses = RicSummaryPlotBuilder::addressesForSource( matchingSummaryCases.front() );
    }
    else if ( !matchingEnsembles.empty() )
    {
        allAddresses = RicSummaryPlotBuilder::addressesForSource( matchingEnsembles.front() );
    }

    bool includeDiffCurves = false;
    auto filteredAddresses =
        RiaSummaryStringTools::computeFilteredAddresses( addressFilters, allAddresses, includeDiffCurves );

    {
        m_multiPlot->deleteAllPlots();

        RicSummaryPlotBuilder plotBuilder;
        plotBuilder.setAddresses( filteredAddresses );
        plotBuilder.setDataSources( matchingSummaryCases, matchingEnsembles );
        plotBuilder.setIndividualPlotPerDataSource( m_individualPlotPerDataSource );

        RicSummaryPlotBuilder::RicGraphCurveGrouping groping = RicSummaryPlotBuilder::RicGraphCurveGrouping::NONE;
        if ( m_individualPlotPerVector ) groping = RicSummaryPlotBuilder::RicGraphCurveGrouping::SINGLE_CURVES;
        if ( m_individualPlotPerObject ) groping = RicSummaryPlotBuilder::RicGraphCurveGrouping::CURVES_FOR_OBJECT;
        plotBuilder.setGrouping( groping );

        auto plots = plotBuilder.createPlots();

        insertGraphsIntoPlot( this, plots );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotTitles()
{
    populateNameHelper( m_nameHelper.get() );

    auto title = m_nameHelper->plotTitle();
    m_multiPlot->setMultiPlotTitle( title );

    if ( m_autoPlotTitlesOnSubPlots )
    {
        for ( auto plot : summaryPlots() )
        {
            auto subPlotNameHelper = plot->plotTitleHelper();

            // Disable auto plot, as this is required to be able to include the information in the multi plot title
            plot->enableAutoPlotTitle( false );

            auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper.get() );
            plot->setDescription( plotName );
            plot->updatePlotTitle();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryNameHelper* RimSummaryMultiPlot::nameHelper() const
{
    return m_nameHelper.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoTitlePlot( bool enable )
{
    m_autoPlotTitles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoTitleGraphs( bool enable )
{
    m_autoPlotTitlesOnSubPlots = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryMultiPlot::summaryPlots() const
{
    std::vector<RimSummaryPlot*> typedPlots;

    for ( auto plot : m_multiPlot->plots() )
    {
        auto summaryPlot = dynamic_cast<RimSummaryPlot*>( plot );
        if ( summaryPlot ) typedPlots.push_back( summaryPlot );
    }

    return typedPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::insertGraphsIntoPlot( RimSummaryMultiPlot* plot, const std::vector<RimSummaryPlot*>& graphs )
{
    RiuMultiPlotPage::ColumnCount columnCount = RiuMultiPlotPage::ColumnCount::COLUMNS_2;
    RimMultiPlot::RowCount        rowCount    = RimMultiPlot::RowCount::ROWS_2;

    bool showTitleSubGraph = true;
    if ( graphs.size() == 1 ) showTitleSubGraph = false;

    if ( 4 < graphs.size() && graphs.size() <= 6 )
    {
        columnCount = RiuMultiPlotPage::ColumnCount::COLUMNS_3;
        rowCount    = RimMultiPlot::RowCount::ROWS_2;
    }
    else if ( 6 < graphs.size() && graphs.size() <= 12 )
    {
        columnCount = RiuMultiPlotPage::ColumnCount::COLUMNS_4;
        rowCount    = RimMultiPlot::RowCount::ROWS_3;
    }
    else
    {
        columnCount = RiuMultiPlotPage::ColumnCount::COLUMNS_4;
        rowCount    = RimMultiPlot::RowCount::ROWS_4;
    }

    plot->setAutoTitlePlot( true );
    plot->setAutoTitleGraphs( showTitleSubGraph );

    plot->m_multiPlot->setColumnCount( columnCount );
    plot->m_multiPlot->setRowCount( rowCount );
    plot->m_multiPlot->setShowPlotTitles( showTitleSubGraph );

    for ( auto graph : graphs )
    {
        plot->addPlot( graph );

        graph->resolveReferencesRecursively();
        graph->revokeMdiWindowStatus();
        graph->setShowWindow( true );

        graph->loadDataAndUpdate();
    }

    plot->loadDataAndUpdate();
}
