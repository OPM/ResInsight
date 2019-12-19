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
#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotWidget.h"

#include "RimMultiPlotWindow.h"
#include "RimPlotWindow.h"
#include "RimWellLogTrack.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteSubPlotFeature, "RicDeleteSubPlotFeature" );

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
        size_t plotsSelected = 0;
        for ( caf::PdmObject* object : selection )
        {
            RimMultiPlotWindow* multiPlot = nullptr;
            object->firstAncestorOrThisOfType( multiPlot );
            if ( dynamic_cast<RimPlotWindow*>( object ) && multiPlot )
            {
                plotsSelected++;
            }
        }
        return plotsSelected == selection.size();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::onActionTriggered( bool isChecked )
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return;

    std::vector<RimPlot*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );
    std::set<RimMultiPlotWindow*> alteredPlotWindows;

    for ( RimPlot* plot : selection )
    {
        RimMultiPlotWindow* plotWindow = nullptr;
        plot->firstAncestorOrThisOfType( plotWindow );
        if ( plot && plotWindow )
        {
            alteredPlotWindows.insert( plotWindow );
            plotWindow->removePlot( plot );
            caf::SelectionManager::instance()->removeObjectFromAllSelections( plot );

            plotWindow->updateConnectedEditors();
            delete plot;
        }
    }

    for ( RimMultiPlotWindow* plotWindow : alteredPlotWindows )
    {
        plotWindow->uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::setupActionLook( QAction* actionToSetup )
{
    QString                      actionText;
    std::vector<caf::PdmObject*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    size_t tracksSelected = 0u;
    for ( caf::PdmObject* object : selection )
    {
        if ( dynamic_cast<RimWellLogTrack*>( object ) )
        {
            tracksSelected++;
        }
    }
    if ( tracksSelected == selection.size() )
    {
        actionText = "Delete Track";
    }
    else
    {
        actionText = "Delete Plot";
    }
    if ( selection.size() > 1u )
    {
        actionText += "s";
    }

    actionToSetup->setText( actionText );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
