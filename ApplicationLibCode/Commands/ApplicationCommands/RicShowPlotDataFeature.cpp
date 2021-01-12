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
#include "RiaPreferences.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlot.h"
#include "RimVfpPlot.h"
#include "RimWellLogPlot.h"

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
        RiaQDateTimeTools::DateTimePeriod timePeriod = allTabs[tabIndex];
        if ( timePeriod == RiaQDateTimeTools::DateTimePeriod::NONE )
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

        RiaQDateTimeTools::DateTimePeriod timePeriod = indexToPeriod( tabIndex );

        if ( m_summaryPlot->containsResamplableCurves() )
        {
            RiaPreferences* prefs = RiaApplication::instance()->preferences();

            return m_summaryPlot->asciiDataForSummaryPlotExport( timePeriod, prefs->showSummaryTimeAsLongString() );
        }
        else
        {
            return m_summaryPlot->asciiDataForSummaryPlotExport( RiaQDateTimeTools::DateTimePeriod::NONE, true );
        }
    }

    int tabCount() const override { return (int)tabs().size(); }

private:
    static RiaQDateTimeTools::DateTimePeriod indexToPeriod( int tabIndex )
    {
        auto allTabs = tabs();
        CVF_ASSERT( tabIndex < (int)allTabs.size() );
        RiaQDateTimeTools::DateTimePeriod timePeriod = allTabs[tabIndex];
        return timePeriod;
    }

    static std::vector<RiaQDateTimeTools::DateTimePeriod> tabs()
    {
        std::vector<RiaQDateTimeTools::DateTimePeriod> dateTimePeriods = RiaQDateTimeTools::dateTimePeriods();
        dateTimePeriods.erase( std::remove( dateTimePeriods.begin(),
                                            dateTimePeriods.end(),
                                            RiaQDateTimeTools::DateTimePeriod::DECADE ),
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

    auto selectedSummaryPlots = caf::selectedObjectsByType<RimSummaryPlot*>();
    if ( selectedSummaryPlots.size() > 0 )
    {
        for ( auto c : selectedSummaryPlots )
        {
            if ( dynamic_cast<RimSummaryCrossPlot*>( c ) )
            {
                return false;
            }
        }

        return true;
    }

    auto wellLogPlots = caf::selectedObjectsByType<RimWellLogPlot*>();
    if ( wellLogPlots.size() > 0 ) return true;

    auto gridCrossPlots = caf::selectedObjectsByType<RimGridCrossPlot*>();
    if ( gridCrossPlots.size() > 0 ) return true;

    auto vfpPlots = caf::selectedObjectsByType<RimVfpPlot*>();
    if ( vfpPlots.size() > 0 ) return true;

    return false;
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

    std::vector<RimSummaryPlot*>   selectedSummaryPlots = caf::selectedObjectsByType<RimSummaryPlot*>();
    std::vector<RimWellLogPlot*>   wellLogPlots         = caf::selectedObjectsByType<RimWellLogPlot*>();
    std::vector<RimGridCrossPlot*> crossPlots           = caf::selectedObjectsByType<RimGridCrossPlot*>();
    std::vector<RimVfpPlot*>       vfpPlots             = caf::selectedObjectsByType<RimVfpPlot*>();
    if ( selectedSummaryPlots.empty() && wellLogPlots.empty() && crossPlots.empty() && vfpPlots.empty() )
    {
        CVF_ASSERT( false );

        return;
    }

    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    CVF_ASSERT( plotwindow );

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
