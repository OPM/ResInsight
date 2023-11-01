/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimMainPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimSimWellInView.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellRftPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include "RimSummaryCaseCollection.h"
#include "RimWellPlotTools.h"
#include <vector>

CAF_CMD_SOURCE_INIT( RicCreateRftPlotsFeature, "RicCreateRftPlotsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateRftPlotsFeature::isCommandEnabled() const
{
    /*
        RimRftPlotCollection* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimRftPlotCollection>();
        if ( simWell ) return true;

        if ( selectedWellName().isEmpty() )
        {
            return false;
        }
    */

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftPlotsFeature::onActionTriggered( bool isChecked )
{
    RimRftPlotCollection* rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();
    if ( rftPlotColl )
    {
        auto wellNames = wellNamesWithRft();
        for ( const auto& wellName : wellNames )
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
std::vector<QString> RicCreateRftPlotsFeature::wellNamesWithRft()
{
    std::set<QString> wellNames;

    const std::vector<RimSummaryCaseCollection*> rftEnsembles = RimWellPlotTools::rftEnsembles();
    for ( RimSummaryCaseCollection* summaryCaseColl : rftEnsembles )
    {
        std::set<QString> wellsWithRftData = summaryCaseColl->wellsWithRftData();
        wellNames.insert( wellsWithRftData.begin(), wellsWithRftData.end() );
    }

    return { wellNames.begin(), wellNames.end() };
}
