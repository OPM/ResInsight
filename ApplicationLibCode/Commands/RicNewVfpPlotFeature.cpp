/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020  Equinor ASA
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

#include "RicNewVfpPlotFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"

#include "RimMainPlotCollection.h"

#include "VerticalFlowPerformance/RimVfpDeck.h"
#include "VerticalFlowPerformance/RimVfpPlot.h"
#include "VerticalFlowPerformance/RimVfpPlotCollection.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicNewVfpPlotFeature, "RicNewVfpPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewVfpPlotFeature::isCommandEnabled() const
{
    auto plotColl = caf::firstAncestorOfTypeFromSelectedObject<RimVfpPlotCollection>();
    return ( plotColl != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::onActionTriggered( bool isChecked )
{
    RimVfpPlotCollection* vfpPlotColl = RimMainPlotCollection::current()->vfpPlotCollection();
    if ( !vfpPlotColl ) return;

    RiaApplication*    app = RiaGuiApplication::instance();
    RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();

    const QString vfpDataKey = "VFP_DATA";
    QString       defaultDir = app->lastUsedDialogDirectory( vfpDataKey );
    QStringList   fileNames =
        RiuFileDialogTools::getOpenFileNames( mpw, "Import VFP Files", defaultDir, "VFP Text Files (*.ecl *.vfp *.data);;All Files (*.*)" );

    if ( fileNames.isEmpty() ) return;

    app->setLastUsedDialogDirectory( vfpDataKey, QFileInfo( fileNames.last() ).absolutePath() );

    std::vector<RimVfpPlot*> vfpPlots;
    std::vector<RimVfpDeck*> vfpDecks;

    for ( const auto& fileName : fileNames )
    {
        if ( fileName.contains( ".DATA" ) )
        {
            auto vfpDeck = vfpPlotColl->addDeck( fileName );
            vfpDecks.push_back( vfpDeck );
        }
        else
        {
            auto vfpPlot = new RimVfpPlot();
            vfpPlot->setFileName( fileName );
            vfpPlotColl->addPlot( vfpPlot );

            vfpPlots.push_back( vfpPlot );
        }
    }

    vfpPlotColl->updateConnectedEditors();

    for ( auto deck : vfpDecks )
    {
        deck->loadDataAndUpdate();
        deck->updateConnectedEditors();
    }

    for ( auto plot : vfpPlots )
    {
        plot->loadDataAndUpdate();
    }

    RiuPlotMainWindowTools::showPlotMainWindow();

    if ( !vfpPlots.empty() )
    {
        RiuPlotMainWindowTools::onObjectAppended( vfpPlots.front() );
    }

    if ( !vfpDecks.empty() )
    {
        RiuPlotMainWindowTools::onObjectAppended( vfpDecks.front() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New VFP Plots" );
    actionToSetup->setIcon( QIcon( ":/VfpPlot.svg" ) );
}
