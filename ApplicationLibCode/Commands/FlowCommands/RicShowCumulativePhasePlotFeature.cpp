/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicShowCumulativePhasePlotFeature.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSimWellInView.h"
#include "RimWellAllocationPlot.h"
#include "RimWellDistributionPlotCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowCumulativePhasePlotFeature, "RicShowCumulativePhasePlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowCumulativePhasePlotFeature::isCommandEnabled() const
{
    auto plot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellAllocationPlot>();
    if ( plot != nullptr ) return true;

    auto simWell = caf::SelectionManager::instance()->selectedItemOfType<RimSimWellInView>();

    return simWell != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowCumulativePhasePlotFeature::onActionTriggered( bool isChecked )
{
    RimEclipseResultCase* eclipseResultCase = nullptr;
    int                   timeStep          = 0;
    QString               wellName;

    {
        auto plot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellAllocationPlot>();

        if ( plot )
        {
            eclipseResultCase = getDataFromWellAllocation( plot, wellName, timeStep );
        }
    }

    {
        auto simWell = caf::SelectionManager::instance()->selectedItemOfType<RimSimWellInView>();
        if ( simWell )
        {
            eclipseResultCase = getDataFromSimWell( simWell, wellName, timeStep );
        }
    }

    RimFlowPlotCollection* flowPlotColl = RimMainPlotCollection::current()->flowPlotCollection();
    if ( !flowPlotColl ) return;

    RimWellDistributionPlotCollection* wdp = flowPlotColl->wellDistributionPlotCollection();
    if ( wdp && eclipseResultCase )
    {
        RiuPlotMainWindowTools::showPlotMainWindow();

        wdp->setData( eclipseResultCase, wellName, timeStep );
        wdp->setShowWindow( true );
        wdp->loadDataAndUpdate();

        wdp->updateConnectedEditors();

        RiuPlotMainWindowTools::onObjectAppended( wdp );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowCumulativePhasePlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CumulativePhaseDist16x16.png" ) );
    actionToSetup->setText( "Show Cumulative Phase Distribution Plot" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RicShowCumulativePhasePlotFeature::getDataFromSimWell( RimSimWellInView* simWell, QString& wellName, int& timeStepIndex )
{
    if ( simWell )
    {
        wellName = simWell->name();

        auto eclView = simWell->firstAncestorOfType<RimEclipseView>();
        if ( eclView )
        {
            timeStepIndex = eclView->currentTimeStep();
        }

        return simWell->firstAncestorOfType<RimEclipseResultCase>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase*
    RicShowCumulativePhasePlotFeature::getDataFromWellAllocation( RimWellAllocationPlot* wap, QString& wellName, int& timeStepIndex )
{
    RimEclipseResultCase* resultCase = nullptr;

    if ( wap )
    {
        wellName = wap->wellName();

        timeStepIndex = wap->timeStep();

        resultCase = wap->rimCase();
    }

    return resultCase;
}
