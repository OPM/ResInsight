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

#include "SummaryPlotCommands/RicAsciiExportSummaryPlotFeature.h"

#include "RimPlainPlotDataProducer.h"
#include "RimPlot.h"
#include "RimPlotWindow.h"
#include "RimReportPlotGroup.h"
#include "RimTabbedTextProducer.h"
#include "RimTabbedTextProvider.h"

#include "RiuPlotMainWindow.h"
#include "RiuTextDialog.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowPlotDataFeature, "RicShowPlotDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowPlotDataFeature::isCommandEnabled() const
{
    QString content = RiaFeatureCommandContext::instance()->contentString();
    if ( !content.isEmpty() )
    {
        return true;
    }

    std::vector<RimPlotWindow*> selection;
    getSelection( selection );

    for ( auto plot : selection )
    {
        if ( dynamic_cast<const RimTabbedTextProducer*>( plot ) ) return true;
        if ( dynamic_cast<const RimReportPlotGroup*>( plot ) ) return true;
        if ( dynamic_cast<const RimPlainPlotDataProducer*>( plot ) ) return true;
    }
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

    disableModelChangeContribution();

    std::vector<RimPlotWindow*> selection;
    getSelection( selection );

    auto showPlainForProducer = []( RimPlainPlotDataProducer* producer )
    {
        QString title = producer->description();
        QString text  = title + "\n\n" + producer->asciiDataForPlotExport();
        RicShowPlotDataFeature::showTextWindow( title, text );
    };

    for ( auto* plot : selection )
    {
        if ( auto* producer = dynamic_cast<RimTabbedTextProducer*>( plot ) )
        {
            RicShowPlotDataFeature::showTabbedTextWindow( producer->createTabbedTextProvider() );
            continue;
        }

        if ( auto* group = dynamic_cast<RimReportPlotGroup*>( plot ) )
        {
            for ( auto* child : group->childPlotsForTextExport() )
            {
                if ( child ) showPlainForProducer( child );
            }
            continue;
        }

        if ( auto* plainProducer = dynamic_cast<RimPlainPlotDataProducer*>( plot ) )
        {
            showPlainForProducer( plainProducer );
        }
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
void RicShowPlotDataFeature::showTabbedTextWindow( std::unique_ptr<RimTabbedTextProvider> textProvider )
{
    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    CVF_ASSERT( plotwindow );

    auto* textWidget = new RiuTabbedTextDialog( std::move( textProvider ) );
    textWidget->setMinimumSize( 800, 600 );

    QObject::connect( textWidget,
                      &RiuTabbedTextDialog::exportToFileRequested,
                      []( const QString& title, const QString& text )
                      {
                          QString defaultDir = RicAsciiExportSummaryPlotFeature::defaultExportDir();
                          QString fileName   = RicAsciiExportSummaryPlotFeature::getFileNameFromUserDialog( title, defaultDir );
                          if ( fileName.isEmpty() ) return;
                          RicAsciiExportSummaryPlotFeature::exportTextToFile( fileName, text );
                      } );

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

    auto* textWiget = new RiuTextDialog( RiaGuiApplication::widgetToUseAsParent() );
    textWiget->setMinimumSize( 400, 600 );

    textWiget->setWindowTitle( title );
    textWiget->setText( text );

    textWiget->show();

    plotwindow->addToTemporaryWidgets( textWiget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::getSelection( std::vector<RimPlotWindow*>& selection ) const
{
    if ( sender() )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<void*>() )
        {
            auto* plot = static_cast<RimPlot*>( userData.value<void*>() );
            if ( plot ) selection.push_back( plot );
        }
    }

    if ( selection.empty() )
    {
        auto selectedObjects = caf::selectedObjectsByType<caf::PdmObject*>();
        for ( auto obj : selectedObjects )
        {
            if ( !obj ) continue;

            if ( auto plotWindow = obj->firstAncestorOrThisOfType<RimPlotWindow>() )
            {
                selection.push_back( plotWindow );
            }
        }
    }
}
