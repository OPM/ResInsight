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

#include "RicDeleteSubPlotFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RiaGuiApplication.h"
#include "RiuGridPlotWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotWidget.h"

#include "RimGridPlotWindow.h"
#include "RimPlotInterface.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteSubPlotFeature, "RicDeleteWellLogPlotTrackFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubPlotFeature::isCommandEnabled()
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return false;

    std::vector<caf::PdmObject*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() > 0 )
    {
        RimGridPlotWindow* wellLogPlot = nullptr;
        selection[0]->firstAncestorOrThisOfType( wellLogPlot );
        if ( dynamic_cast<RimPlotInterface*>( selection[0] ) && wellLogPlot && wellLogPlot->plotCount() > 1 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::onActionTriggered( bool isChecked )
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return;

    std::vector<caf::PdmObject*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );
    RiuPlotMainWindow*           plotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    std::set<RimGridPlotWindow*> alteredWellLogPlots;

    for ( size_t i = 0; i < selection.size(); i++ )
    {
        RimPlotInterface* plot = dynamic_cast<RimPlotInterface*>( selection[i] );

        RimGridPlotWindow* wellLogPlot = nullptr;
        selection[i]->firstAncestorOrThisOfType( wellLogPlot );
        if ( plot && wellLogPlot && wellLogPlot->plotCount() > 1 )
        {
            alteredWellLogPlots.insert( wellLogPlot );
            wellLogPlot->removePlot( plot );
            caf::SelectionManager::instance()->removeObjectFromAllSelections( selection[i] );

            wellLogPlot->updateConnectedEditors();
            delete plot;
        }
    }

    for ( RimGridPlotWindow* wellLogPlot : alteredWellLogPlots )
    {
        RiuGridPlotWindow* viewWidget = dynamic_cast<RiuGridPlotWindow*>( wellLogPlot->viewWidget() );
        plotWindow->setWidthOfMdiWindow( viewWidget, viewWidget->preferredWidth() );
        // TODO: add back with virtual methods
        // wellLogPlot->calculateAvailableDepthRange();
        // wellLogPlot->updateDepthZoom();
        wellLogPlot->uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Track" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
