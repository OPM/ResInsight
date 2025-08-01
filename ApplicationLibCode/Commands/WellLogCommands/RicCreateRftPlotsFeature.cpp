/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RicCreateRftPlotsFeature.h"

#include "RiaLogging.h"

#include "RicCreateRftPlotsFeatureUi.h"

#include "RifReaderRftInterface.h"

#include "RimMainPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellRftPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateRftPlotsFeature, "RicCreateRftPlotsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::onActionTriggered( bool isChecked )
{
    auto sourcePlot = dynamic_cast<RimWellRftPlot*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !sourcePlot ) return;

    RimRftPlotCollection* rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();
    if ( !rftPlotColl ) return;

    std::set<QString> wellsWithRftData;

    auto dataSource = sourcePlot->dataSource();
    if ( auto summaryCollection = std::get_if<RimSummaryEnsemble*>( &dataSource ) )
    {
        wellsWithRftData = RimSummaryEnsembleTools::wellsWithRftData( ( *summaryCollection )->allSummaryCases() );
    }
    else if ( auto summaryCase = std::get_if<RimSummaryCase*>( &dataSource ) )
    {
        RifReaderRftInterface* reader = ( *summaryCase )->rftReader();
        if ( reader )
        {
            wellsWithRftData = reader->wellNames();
        }
    }

    if ( wellsWithRftData.empty() ) return;

    RicCreateRftPlotsFeatureUi ui;
    ui.setAllWellNames( { wellsWithRftData.begin(), wellsWithRftData.end() } );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, &ui, "Select RFT wells", "" );
    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    for ( const auto& wellName : ui.selectedWellNames() )
    {
        appendRftPlotForWell( wellName, rftPlotColl, sourcePlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::appendRftPlotForWell( const QString& wellName, RimRftPlotCollection* rftPlotColl, RimWellRftPlot* sourcePlot )
{
    if ( !rftPlotColl )
    {
        RiaLogging::error( "Missing RFT plot collection, no RFT plot created." );
        return;
    }

    if ( !sourcePlot )
    {
        RiaLogging::error( "Missing source RFT plot, no RFT plot created." );
        return;
    }

    // Create a RFT plot based on wellName, and reuse the data source selection in sourcePlot

    auto rftPlot = sourcePlot->copyObject<RimWellRftPlot>();
    if ( !rftPlot ) return;

    rftPlot->setSimWellOrWellPathName( wellName );
    rftPlotColl->addPlot( rftPlot );
    rftPlot->resolveReferencesRecursively();
    rftPlot->initializeDataSources( sourcePlot );

    const auto    generatedName = rftPlot->simWellOrWellPathName(); // We may have been given a default well name
    const QString plotName      = QString( RimWellRftPlot::plotNameFormatString() ).arg( generatedName );

    rftPlot->nameConfig()->setCustomName( plotName );

    rftPlot->loadDataAndUpdate();
    rftPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::appendRftPlotForWell( const QString& wellName, RimRftPlotCollection* rftPlotColl )
{
    if ( !rftPlotColl )
    {
        RiaLogging::error( "Missing RFT plot collection, no RFT plot created." );
        return;
    }

    // Create a default RFT plot based on well name, and toggle on all available data sources in this RFT plot

    auto rftPlot = new RimWellRftPlot();
    rftPlot->setSimWellOrWellPathName( wellName );

    auto plotTrack = new RimWellLogTrack();
    rftPlot->addPlot( plotTrack );
    plotTrack->setDescription( QString( "Track %1" ).arg( rftPlot->plotCount() ) );

    rftPlotColl->addPlot( rftPlot );
    rftPlot->initializeDataSources( nullptr );

    const auto    generatedName = rftPlot->simWellOrWellPathName(); // We may have been given a default well name
    const QString plotName      = QString( RimWellRftPlot::plotNameFormatString() ).arg( generatedName );

    rftPlot->nameConfig()->setCustomName( plotName );
    rftPlot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );
    rftPlot->loadDataAndUpdate();
    rftPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( rftPlot, plotTrack );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Multiple RFT Plots" );
    actionToSetup->setIcon( QIcon( ":/FlowCharPlot16x16.png" ) );
}
