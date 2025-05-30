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

#include "RicShowTotalAllocationDataFeature.h"

#include "ApplicationCommands/RicShowPlotDataFeature.h"

#include "RimTotalWellAllocationPlot.h"
#include "RimWellAllocationPlot.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

#include <set>

CAF_CMD_SOURCE_INIT( RicShowTotalAllocationDataFeature, "RicShowTotalAllocationDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowTotalAllocationDataFeature::isCommandEnabled() const
{
    std::set<RimWellAllocationPlot*> wellAllocPlots = RicShowTotalAllocationDataFeature::selectedWellAllocationPlots();

    return !wellAllocPlots.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowTotalAllocationDataFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    std::set<RimWellAllocationPlot*> wellAllocPlots = RicShowTotalAllocationDataFeature::selectedWellAllocationPlots();
    CVF_ASSERT( wellAllocPlots.size() > 0 );

    for ( auto wellAllocPlot : wellAllocPlots )
    {
        QString txt = wellAllocPlot->description();
        txt += "\n";
        txt += "\n";
        txt += wellAllocPlot->totalWellFlowPlot()->totalAllocationAsText();

        QString title = "Total Allocation (" + wellAllocPlot->wellName() + ")";

        RicShowPlotDataFeature::showTextWindow( title, txt );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowTotalAllocationDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show Total Allocation Data" );
    // actionToSetup->setIcon(QIcon(":/PlotWindow.svg"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimWellAllocationPlot*> RicShowTotalAllocationDataFeature::selectedWellAllocationPlots()
{
    std::set<RimWellAllocationPlot*> wellAllocPlots;

    for ( auto obj : caf::SelectionManager::instance()->objectsByType<caf::PdmObject>() )
    {
        CVF_ASSERT( obj );

        RimWellAllocationPlot* parentPlot = obj->firstAncestorOrThisOfType<RimWellAllocationPlot>();
        if ( parentPlot )
        {
            wellAllocPlots.insert( parentPlot );
        }
    }

    return wellAllocPlots;
}
