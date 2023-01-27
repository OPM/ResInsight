/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicShowPlotDataFeature.h"

#include "RiaFeatureCommandContext.h"
#include "RiaGuiApplication.h"
#include "RiaPreferencesSummary.h"
#include "RiaQDateTimeTools.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimPlotWindow.h"
#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlot.h"
#include "RimVfpPlot.h"
#include "RimWellAllocationOverTimePlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindow.h"
#include "RiuTextDialog.h"

#include "cafPdmPointer.h"
#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowPlotDataFeature, "RicShowPlotDataFeature" );

//--------------------------------------------------------------------------------------------------
/// Private text provider class for summary plots
//--------------------------------------------------------------------------------------------------
class RiuTabbedSummaryPlotTextProvider : public RiuTabbedTextProvider
{
public:
    RiuTabbedSummaryPlotTextProvider( RimSummaryPlot* summaryPlot )
        : m_summaryPlot( summaryPlot )
    {
    }

    bool isValid() const override { return m_summaryPlot.notNull(); }

    QString description() const override
    {
        CVF_ASSERT( m_summaryPlot.notNull() && "Need to check that provider is valid" );
        return m_summaryPlot->description();
    }

    QString tabTitle( int tabIndex ) const override
    {
        auto allTabs = tabs();
        CVF_ASSERT( tabIndex < (int)allTabs.size() );
        RiaDefines::DateTimePeriod timePeriod = allTabs[tabIndex];
        if ( timePeriod == RiaDefines::DateTimePeriod::NONE )
        {
            return "No Resampling";
        }
        else
        {
            return QString( "Plot Data, %1" ).arg( RiaQDateTimeTools::dateTimePeriodName( timePeriod ) );
        }
    }

    QString tabText( int tabIndex ) const override
    {
        CVF_ASSERT( m_summaryPlot.notNull() && "Need to check that provider is valid" );

        RiaDefines::DateTimePeriod timePeriod = indexToPeriod( tabIndex );

        if ( m_summaryPlot->containsResamplableCurves() )
        {
            RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

            return m_summaryPlot->asciiDataForSummaryPlotExport( timePeriod, prefs->showSummaryTimeAsLongString() );
        }
        else
        {
            return m_summaryPlot->asciiDataForSummaryPlotExport( RiaDefines::DateTimePeriod::NONE, true );
        }
    }

    int tabCount() const override { return (int)tabs().size(); }

private:
    static RiaDefines::DateTimePeriod indexToPeriod( int tabIndex )
    {
        auto allTabs = tabs();
        CVF_ASSERT( tabIndex < (int)allTabs.size() );
        RiaDefines::DateTimePeriod timePeriod = allTabs[tabIndex];
        return timePeriod;
    }

    static std::vector<RiaDefines::DateTimePeriod> tabs()
    {
        std::vector<RiaDefines::DateTimePeriod> dateTimePeriods = RiaQDateTimeTools::dateTimePeriods();
        dateTimePeriods.erase( std::remove( dateTimePeriods.begin(), dateTimePeriods.end(), RiaDefines::DateTimePeriod::DECADE ),
                               dateTimePeriods.end() );
        return dateTimePeriods;
    }

private:
    caf::PdmPointer<RimSummaryPlot> m_summaryPlot;
};

//--------------------------------------------------------------------------------------------------
/// Private text provider class for grid cross plots
//--------------------------------------------------------------------------------------------------
class RiuTabbedGridCrossPlotTextProvider : public RiuTabbedTextProvider
{
public:
    RiuTabbedGridCrossPlotTextProvider( RimGridCrossPlot* crossPlot )
        : m_crossPlot( crossPlot )
    {
    }

    bool isValid() const override { return m_crossPlot.notNull(); }

    QString description() const override
    {
        CVF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
        return m_crossPlot->createAutoName();
    }

    QString tabTitle( int tabIndex ) const override
    {
        CVF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
        return m_crossPlot->asciiTitleForPlotExport( tabIndex );
    }

    QString tabText( int tabIndex ) const override
    {
        CVF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
        return m_crossPlot->asciiDataForGridCrossPlotExport( tabIndex );
    }

    int tabCount() const override
    {
        CVF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
        return (int)m_crossPlot->dataSets().size();
    }

private:
    caf::PdmPointer<RimGridCrossPlot> m_crossPlot;
};

//--------------------------------------------------------------------------------------------------
///
///
/// RicShowPlotDataFeature
///
///
//--------------------------------------------------------------------------------------------------
bool RicShowPlotDataFeature::isCommandEnabled()
{
    QString content = RiaFeatureCommandContext::instance()->contentString();
    if ( !content.isEmpty() )
    {
        return true;
    }

    std::vector<RimPlotWindow*> selection;
    getSelection( selection );

    int validPlots = 0;

    for ( auto plot : selection )
    {
        if ( dynamic_cast<RimSummaryCrossPlot*>( plot ) )
        {
            return false;
        }

        if ( dynamic_cast<RimSummaryPlot*>( plot ) || dynamic_cast<RimWellLogPlot*>( plot ) || dynamic_cast<RimWellLogTrack*>( plot ) ||
             dynamic_cast<RimGridCrossPlot*>( plot ) || dynamic_cast<RimVfpPlot*>( plot ) ||
             dynamic_cast<RimWellAllocationOverTimePlot*>( plot ) )
        {
            validPlots++;
        }
    }
    return ( validPlots > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::onActionTriggered( bool isChecked )
{
    QString content = RiaFeatureCommandContext::instance()->contentString();
    if ( !content.isEmpty() )
    {
        QString title = "Data Content";
        {
            QString titleCandidate = RiaFeatureCommandContext::instance()->titleString();
            if ( !titleCandidate.isEmpty() ) title = titleCandidate;
        }

        RicShowPlotDataFeature::showTextWindow( title, content );

        return;
    }

    this->disableModelChangeContribution();

    std::vector<RimPlotWindow*> selection;
    getSelection( selection );

    std::vector<RimSummaryPlot*>                selectedSummaryPlots;
    std::vector<RimWellLogPlot*>                wellLogPlots;
    std::vector<RimGridCrossPlot*>              crossPlots;
    std::vector<RimVfpPlot*>                    vfpPlots;
    std::vector<RimWellLogTrack*>               depthTracks;
    std::vector<RimWellAllocationOverTimePlot*> wellAllocationOverTimePlots;

    for ( auto plot : selection )
    {
        if ( auto sumPlot = dynamic_cast<RimSummaryPlot*>( plot ) )
        {
            selectedSummaryPlots.push_back( sumPlot );
            continue;
        }

        if ( auto wellPlot = dynamic_cast<RimWellLogPlot*>( plot ) )
        {
            wellLogPlots.push_back( wellPlot );
            continue;
        }

        if ( auto xPlot = dynamic_cast<RimGridCrossPlot*>( plot ) )
        {
            crossPlots.push_back( xPlot );
            continue;
        }

        if ( auto vfpPlot = dynamic_cast<RimVfpPlot*>( plot ) )
        {
            vfpPlots.push_back( vfpPlot );
            continue;
        }

        if ( auto depthTrack = dynamic_cast<RimWellLogTrack*>( plot ) )
        {
            depthTracks.push_back( depthTrack );
            continue;
        }

        if ( auto wellAllocationOverTimePlot = dynamic_cast<RimWellAllocationOverTimePlot*>( plot ) )
        {
            wellAllocationOverTimePlots.push_back( wellAllocationOverTimePlot );
            continue;
        }
    }

    for ( RimSummaryPlot* summaryPlot : selectedSummaryPlots )
    {
        auto textProvider = new RiuTabbedSummaryPlotTextProvider( summaryPlot );
        RicShowPlotDataFeature::showTabbedTextWindow( textProvider );
    }

    for ( RimWellLogPlot* wellLogPlot : wellLogPlots )
    {
        QString title = wellLogPlot->description();
        QString text  = wellLogPlot->asciiDataForPlotExport();
        RicShowPlotDataFeature::showTextWindow( title, text );
    }

    for ( auto* plot : depthTracks )
    {
        QString title = plot->description();
        QString text  = plot->asciiDataForPlotExport();
        RicShowPlotDataFeature::showTextWindow( title, text );
    }

    for ( RimVfpPlot* vfpPlot : vfpPlots )
    {
        QString title = vfpPlot->description();
        QString text  = vfpPlot->asciiDataForPlotExport();
        RicShowPlotDataFeature::showTextWindow( title, text );
    }

    for ( RimGridCrossPlot* crossPlot : crossPlots )
    {
        auto textProvider = new RiuTabbedGridCrossPlotTextProvider( crossPlot );
        RicShowPlotDataFeature::showTabbedTextWindow( textProvider );
    }

    for ( RimWellAllocationOverTimePlot* wellAllocationOverTimePlot : wellAllocationOverTimePlots )
    {
        QString title = wellAllocationOverTimePlot->description();
        QString text  = wellAllocationOverTimePlot->asciiDataForPlotExport();
        RicShowPlotDataFeature::showTextWindow( title, text );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show Plot Data" );
    actionToSetup->setIcon( QIcon( ":/PlotWindow.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::showTabbedTextWindow( RiuTabbedTextProvider* textProvider )
{
    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    CVF_ASSERT( plotwindow );

    RiuTabbedTextDialog* textWidget = new RiuTabbedTextDialog( textProvider );
    textWidget->setMinimumSize( 800, 600 );
    plotwindow->addToTemporaryWidgets( textWidget );
    textWidget->show();
    textWidget->redrawText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::showTextWindow( const QString& title, const QString& text )
{
    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    CVF_ASSERT( plotwindow );

    RiuTextDialog* textWiget = new RiuTextDialog();
    textWiget->setMinimumSize( 400, 600 );

    textWiget->setWindowTitle( title );
    textWiget->setText( text );

    textWiget->show();

    plotwindow->addToTemporaryWidgets( textWiget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::getSelection( std::vector<RimPlotWindow*>& selection )
{
    if ( sender() )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<void*>() )
        {
            RimPlot* plot = static_cast<RimPlot*>( userData.value<void*>() );
            if ( plot ) selection.push_back( plot );
        }
    }

    if ( selection.empty() )
    {
        caf::SelectionManager::instance()->objectsByType( &selection );
    }
}
