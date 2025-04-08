/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewPltPlotFeature.h"

#include "RiaLogging.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "Well/RigWellLogCurveData.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPltPlot.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewPltPlotFeature, "RicNewPltPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPltPlotFeature::isCommandEnabled() const
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return false;

    RimSimWellInView* simWell          = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView>();
    RimWellPath*      selectedWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath>();

    bool enable = true;

    if ( selectedWellPath )
    {
        if ( !selectedWellPath->wellPathGeometry() || !RimWellPlotTools::hasFlowData( selectedWellPath ) )
        {
            return false;
        }
    }

    if ( simWell != nullptr )
    {
        RimProject* proj        = RimProject::current();
        QString     simWellName = simWell->name();

        RimWellPath* wellPath = proj->wellPathFromSimWellName( simWellName );
        enable                = wellPath != nullptr;
    }
    return enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPltPlotFeature::onActionTriggered( bool isChecked )
{
    if ( RimWellPlotTools::wellPathsContainingFlow().empty() )
    {
        QString displayMessage =
            "To create a PLT plot, either import a LAS file with observed production data or import a well path trajectory.";

        RiaLogging::errorInMessageBox( nullptr, "No well data available to create a PLT plot", displayMessage );
        return;
    }

    RimPltPlotCollection* pltPlotColl = RimMainPlotCollection::current()->pltPlotCollection();
    if ( pltPlotColl )
    {
        QString           wellPathName;
        RimWellPath*      wellPath    = nullptr;
        RimSimWellInView* eclipseWell = nullptr;

        if ( ( wellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath>() ) != nullptr )
        {
            wellPathName = wellPath->name();
        }
        else if ( ( eclipseWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView>() ) != nullptr )
        {
            wellPath = RimProject::current()->wellPathFromSimWellName( eclipseWell->name() );
            if ( !wellPath ) return;

            wellPathName = wellPath->name();
        }

        QString plotName = QString( RimWellPltPlot::plotNameFormatString() ).arg( wellPathName );

        RimWellPltPlot* pltPlot = new RimWellPltPlot();
        pltPlot->setCurrentWellName( wellPathName );

        RimWellLogTrack* plotTrack = new RimWellLogTrack();
        pltPlot->addPlot( plotTrack );
        plotTrack->setDescription( QString( "Track %1" ).arg( pltPlot->plotCount() ) );

        pltPlotColl->addPlot( pltPlot );
        pltPlot->nameConfig()->setCustomName( plotName );
        pltPlot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );

        pltPlot->loadDataAndUpdate();
        pltPlotColl->updateConnectedEditors();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::onObjectAppended( pltPlot, plotTrack );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPltPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New PLT Plot" );
    actionToSetup->setIcon( QIcon( ":/WellFlowPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewPltPlotFeature::selectedWellPath() const
{
    auto selection = caf::selectedObjectsByType<RimWellPath*>();
    return !selection.empty() ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RicNewPltPlotFeature::selectedSimulationWell( int* branchIndex ) const
{
    RiuSelectionItem*        selItem        = Riu3dSelectionManager::instance()->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>( selItem );
    if ( simWellSelItem )
    {
        ( *branchIndex ) = static_cast<int>( simWellSelItem->m_branchIndex );
        return simWellSelItem->m_simWell;
    }
    else
    {
        ( *branchIndex ) = 0;

        return caf::SelectionManager::instance()->selectedItemOfType<RimSimWellInView>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPltPlotFeature::caseAvailable() const
{
    std::vector<RimCase*> cases = RimProject::current()->allGridCases();
    return !cases.empty();
}
