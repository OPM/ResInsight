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

#include "RimMultipleSummaryPlotNameHelper.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlot, "MultiSummaryPlot" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot::RimSummaryMultiPlot()
{
    CAF_PDM_InitObject( "Multi Summary Plot", "", "", "" );
    this->setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_filterText, "FilterText", "Filter Text" );
    m_filterText.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_individualPlotPerVector, "IndividualPlotPerVector", false, "One plot per Vector" );
    CAF_PDM_InitField( &m_individualPlotPerDataSource, "IndividualPlotPerDataSource", false, "One plot per Data Source" );
    CAF_PDM_InitField( &m_autoPlotTitles, "AutoPlotTitles", false, "Auto Plot Titles" );

    CAF_PDM_InitField( &m_showMultiPlotInProjectTree, "ShowMultiPlotInProjectTree", true, "Show Multi Plot In Project Tree" );

    CAF_PDM_InitFieldNoDefault( &m_multiPlot, "MultiPlot", "Multi Plot" );
    m_multiPlot.uiCapability()->setUiTreeHidden( true );
    m_multiPlot = new RimMultiPlot;

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );
    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingType( RimSummarySourceSteppingInterface::Axis::Y_AXIS );
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();
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
void RimSummaryMultiPlot::addPlot( RimPlot* plot )
{
    m_multiPlot->addPlot( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RimSummaryMultiPlot::createAndAppendMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->multiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot;
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiSummaryPlot( plotWindow );

    for ( auto plot : plots )
    {
        plotWindow->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->revokeMdiWindowStatus();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummarySourceSteppingInterface::Axis> RimSummaryMultiPlot::availableAxes() const
{
    return { RimSummarySourceSteppingInterface::Axis::X_AXIS };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::curvesForStepping( RimSummarySourceSteppingInterface::Axis axis ) const
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
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::allCurves( RimSummarySourceSteppingInterface::Axis axis ) const
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
    uiOrdering.add( &m_filterText );
    uiOrdering.add( &m_individualPlotPerVector );
    uiOrdering.add( &m_individualPlotPerDataSource );
    uiOrdering.add( &m_autoPlotTitles );

    auto group = uiOrdering.addNewGroup( "Multi Plot Options" );
    m_multiPlot->uiOrderingForSummaryMultiPlot( *group );

    {
        auto group = uiOrdering.addNewGroup( "Data Source" );
        m_sourceStepping()->uiOrdering( uiConfigName, *group );
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
              changedField == &m_individualPlotPerVector )
    {
        updatePlots();
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
        // Remove existing plots

        m_multiPlot->deleteAllPlots();

        // Add new plots
        RicSummaryPlotBuilder plotBuilder;
        plotBuilder.setAddresses( filteredAddresses );
        plotBuilder.setDataSources( matchingSummaryCases, matchingEnsembles );
        plotBuilder.setIndividualPlotPerAddress( m_individualPlotPerVector );
        plotBuilder.setIndividualPlotPerDataSource( m_individualPlotPerDataSource );

        auto plots = plotBuilder.createPlots();

        std::vector<RimPlot*> plotsForMultiPlot;
        for ( auto p : plots )
        {
            plotsForMultiPlot.push_back( dynamic_cast<RimPlot*>( p ) );
        }

        for ( auto plot : plotsForMultiPlot )
        {
            this->addPlot( plot );

            plot->resolveReferencesRecursively();
            plot->revokeMdiWindowStatus();
            plot->setShowWindow( true );
        }

        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotTitles()
{
    RimSummaryPlotNameHelper multiPlotNameHelper;

    {
        std::vector<RifEclipseSummaryAddress>  addresses;
        std::vector<RimSummaryCase*>           sumCases;
        std::vector<RimSummaryCaseCollection*> ensembleCases;

        {
            for ( RimSummaryCurve* curve : allCurves( RimSummarySourceSteppingInterface::Axis::Y_AXIS ) )
            {
                {
                    addresses.push_back( curve->summaryAddressY() );
                    sumCases.push_back( curve->summaryCaseY() );
                }
            }
        }

        for ( auto curveSet : curveSets() )
        {
            addresses.push_back( curveSet->summaryAddress() );
            ensembleCases.push_back( curveSet->summaryCaseCollection() );
        }

        multiPlotNameHelper.appendAddresses( addresses );
        multiPlotNameHelper.setSummaryCases( sumCases );
        multiPlotNameHelper.setEnsembleCases( ensembleCases );

        auto title = multiPlotNameHelper.plotTitle();
        m_multiPlot->setMultiPlotTitle( title );
    }

    for ( auto plot : summaryPlots() )
    {
        auto subPlotNameHelper = plot->plotTitleHelper();

        plot->enableAutoPlotTitle( false );
        auto plotName = subPlotNameHelper->aggregatedPlotTitle( multiPlotNameHelper );
        plot->setDescription( plotName );
        plot->updatePlotTitle();
    }
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
