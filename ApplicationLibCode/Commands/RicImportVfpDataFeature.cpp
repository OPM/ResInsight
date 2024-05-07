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

#include "RicImportVfpDataFeature.h"

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

CAF_CMD_SOURCE_INIT( RicImportVfpDataFeature, "RicImportVfpDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportVfpDataFeature::isCommandEnabled() const
{
    auto plotColl = caf::firstAncestorOfTypeFromSelectedObject<RimVfpPlotCollection>();
    return ( plotColl != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportVfpDataFeature::onActionTriggered( bool isChecked )
{
    RimVfpPlotCollection* vfpPlotColl = RimMainPlotCollection::current()->vfpPlotCollection();
    if ( !vfpPlotColl ) return;

    RiaApplication*    app = RiaGuiApplication::instance();
    RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();

    const QString vfpDataKey = "VFP_DATA";
    QString       defaultDir = app->lastUsedDialogDirectory( vfpDataKey );

    QString vfpTextFileFilter    = "VFP Text Files (*.ecl *.vfp)";
    QString simulatorInputFilter = "Simulator Input Files (*.data)";
    QString allFilters           = vfpTextFileFilter + ";;" + simulatorInputFilter + ";;" + "All Files (*.*)";

    QString selectedFilter = simulatorInputFilter;

    QVariant userData = this->userData();
    if ( !userData.isNull() && userData.canConvert<bool>() )
    {
        bool isVfpFiles = userData.toBool();
        if ( isVfpFiles ) selectedFilter = vfpTextFileFilter;
    }

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( mpw, "Import VFP Files", defaultDir, allFilters, &selectedFilter );

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
void RicImportVfpDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import VFP Data" );
    actionToSetup->setIcon( QIcon( ":/VfpPlot.svg" ) );
}
