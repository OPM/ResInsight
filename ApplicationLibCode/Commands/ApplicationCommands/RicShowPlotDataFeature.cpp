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
#include "RiaReportDataProducer.h"

#include "RimPlot.h"
#include "RimPlotWindow.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
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
        if ( dynamic_cast<const RiaReportDataProducer*>( plot ) ) return true;
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

    // The report window shows one plot at a time
    for ( auto* plot : selection )
    {
        if ( dynamic_cast<RiaReportDataProducer*>( plot ) )
        {
            RiuPlotMainWindowTools::showReportWindow( plot );
            return;
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
void RicShowPlotDataFeature::showTextWindow( const QString& title, const QString& text )
{
    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    CAF_ASSERT( plotwindow );

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
