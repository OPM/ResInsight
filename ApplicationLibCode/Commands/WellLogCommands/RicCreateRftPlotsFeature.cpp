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
#include "RicCreateRftPlotsFeatureUi.h"

#include "RimMainPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimSummaryCaseCollection.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateRftPlotsFeature, "RicCreateRftPlotsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateRftPlotsFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::onActionTriggered( bool isChecked )
{
    auto wells = wellNames();

    RimRftPlotCollection* rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();
    if ( rftPlotColl )
    {
        for ( const auto& wellName : wells )
        {
            RimWellRftPlot* rftPlot = new RimWellRftPlot();

            rftPlot->setSimWellOrWellPathName( wellName );

            RimWellLogTrack* plotTrack = new RimWellLogTrack();
            rftPlot->addPlot( plotTrack );
            plotTrack->setDescription( QString( "Track %1" ).arg( rftPlot->plotCount() ) );

            rftPlotColl->addPlot( rftPlot );
            rftPlot->applyInitialSelections();

            auto    generatedName = rftPlot->simWellOrWellPathName(); // We may have been given a default well name
            QString plotName      = QString( RimWellRftPlot::plotNameFormatString() ).arg( generatedName );

            rftPlot->nameConfig()->setCustomName( plotName );
            rftPlot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );

            rftPlot->loadDataAndUpdate();
            rftPlotColl->updateConnectedEditors();

            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::onObjectAppended( rftPlot, plotTrack );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Plots" );
    actionToSetup->setIcon( QIcon( ":/FlowCharPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicCreateRftPlotsFeature::wellNames() const
{
    RicCreateRftPlotsFeatureUi ui;

    RimSummaryCaseCollection* defaultEnsemble = nullptr;

    std::vector<RimSummaryCaseCollection*> caseCollection;
    caf::SelectionManager::instance()->objectsByType( &caseCollection );

    if ( caseCollection.size() == 1 )
    {
        defaultEnsemble = caseCollection[0];
    }
    else
    {
        const std::vector<RimSummaryCaseCollection*> rftEnsembles = RimWellPlotTools::rftEnsembles();
        if ( !rftEnsembles.empty() ) defaultEnsemble = rftEnsembles.front();
    }

    ui.setDefaultEnsemble( defaultEnsemble );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, &ui, "Select RFT wells", "" );
    if ( propertyDialog.exec() != QDialog::Accepted ) return {};

    return ui.wellNames();
}
