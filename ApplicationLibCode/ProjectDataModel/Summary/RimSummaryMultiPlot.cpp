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

#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryStringTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultipleSummaryPlotNameHelper.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlotControls.h"

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

    CAF_PDM_InitField( &m_autoPlotTitles, "AutoPlotTitles", true, "Auto Plot Titles" );
    CAF_PDM_InitField( &m_autoPlotTitlesOnSubPlots, "AutoPlotTitlesSubPlots", true, "Auto Plot Titles Sub Plots" );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::addPlot( RimPlot* plot )
{
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot ) RimMultiPlot::addPlot( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot ) RimMultiPlot::insertPlot( plot, index );
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
void RimSummaryMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* titlesGroup = uiOrdering.addNewGroup( "Titles" );
    titlesGroup->add( &m_autoPlotTitles );
    titlesGroup->add( &m_autoPlotTitlesOnSubPlots );

    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_showIndividualPlotTitles );
    titlesGroup->add( &m_titleFontSize );
    titlesGroup->add( &m_subTitleFontSize );

    caf::PdmUiGroup* legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendFontSize );

    caf::PdmUiGroup* layoutGroup = uiOrdering.addNewGroup( "Layout" );
    layoutGroup->add( &m_columnCount );
    layoutGroup->add( &m_rowsPerPage );
    layoutGroup->add( &m_majorTickmarkCount );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    if ( changedField == &m_autoPlotTitles || changedField == &m_autoPlotTitlesOnSubPlots )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else
    {
        RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotWindowTitle()
{
    if ( m_autoPlotTitles )
    {
        populateNameHelper( m_nameHelper.get() );

        auto title = m_nameHelper->plotTitle();
        setMultiPlotTitle( title );
    }

    if ( m_autoPlotTitlesOnSubPlots )
    {
        for ( auto plot : summaryPlots() )
        {
            auto subPlotNameHelper = plot->plotTitleHelper();

            // Disable auto plot, as this is required to be able to include the information in the multi plot title
            plot->enableAutoPlotTitle( false );

            auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper );
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

    for ( auto plot : plots() )
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
    auto columnCount   = RiuMultiPlotPage::ColumnCount::COLUMNS_2;
    auto rowCount      = RimMultiPlot::RowCount::ROWS_2;
    auto tickmarkCount = RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT;

    bool showTitleSubGraph = true;
    if ( graphs.size() == 1 )
    {
        showTitleSubGraph = false;
        tickmarkCount     = RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_MANY;
    }
    else if ( 4 < graphs.size() && graphs.size() <= 6 )
    {
        columnCount   = RiuMultiPlotPage::ColumnCount::COLUMNS_3;
        rowCount      = RimMultiPlot::RowCount::ROWS_2;
        tickmarkCount = RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW;
    }
    else if ( 6 < graphs.size() && graphs.size() <= 12 )
    {
        columnCount   = RiuMultiPlotPage::ColumnCount::COLUMNS_4;
        rowCount      = RimMultiPlot::RowCount::ROWS_3;
        tickmarkCount = RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_VERY_FEW;
    }
    else
    {
        columnCount   = RiuMultiPlotPage::ColumnCount::COLUMNS_4;
        rowCount      = RimMultiPlot::RowCount::ROWS_4;
        tickmarkCount = RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_VERY_FEW;
    }

    plot->setAutoTitlePlot( true );
    plot->setAutoTitleGraphs( showTitleSubGraph );

    plot->setColumnCount( columnCount );
    plot->setRowCount( rowCount );
    plot->setShowPlotTitles( showTitleSubGraph );
    plot->setTickmarkCount( tickmarkCount );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryMultiPlot::fieldsToShowInToolbar()
{
    std::vector<caf::PdmFieldHandle*> toolBarFields;

    auto& sourceObject = m_sourceStepping();
    if ( sourceObject )
    {
        auto fields = sourceObject->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );
    }

    return toolBarFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryMultiPlot::handleGlobalKeyEvent( QKeyEvent* keyEvent )
{
    return RimSummaryPlotControls::handleKeyEvents( m_sourceStepping(), keyEvent );
}
