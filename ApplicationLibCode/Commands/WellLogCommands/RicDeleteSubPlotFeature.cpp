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

#include "RimMultiPlot.h"
#include "RimPlot.h"
#include "RimPlotWindow.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteSubPlotFeature, "RicDeleteSubPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubPlotFeature::isCommandEnabled() const
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return false;

    return isAnyDeletablePlotSelected();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::onActionTriggered( bool isChecked )
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return;

    std::vector<RimPlot*> selection = getSelection();

    std::set<RimMultiPlot*> alteredPlotWindows;

    for ( RimPlot* plot : selection )
    {
        if ( !plot ) continue;
        caf::SelectionManager::instance()->removeObjectFromAllSelections( plot );
    }

    for ( RimPlot* plot : selection )
    {
        if ( !plot ) continue;

        RimMultiPlot*   multiPlot   = plot->firstAncestorOrThisOfType<RimMultiPlot>();
        RimWellLogPlot* wellLogPlot = plot->firstAncestorOrThisOfType<RimWellLogPlot>();
        if ( multiPlot )
        {
            alteredPlotWindows.insert( multiPlot );
            multiPlot->removePlotNoUpdate( plot );
            multiPlot->updateConnectedEditors();
            delete plot;
        }
        else if ( wellLogPlot )
        {
            wellLogPlot->removePlot( plot );
            wellLogPlot->updateConnectedEditors();
            delete plot;
        }
    }

    for ( auto mainplot : alteredPlotWindows )
    {
        mainplot->updateAfterPlotRemove();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubPlotFeature::setupActionLook( QAction* actionToSetup )
{
    QString               actionText;
    std::vector<RimPlot*> selection = getSelection();

    size_t tracksSelected = 0u;
    for ( RimPlot* object : selection )
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
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicDeleteSubPlotFeature::getSelection() const
{
    std::vector<RimPlot*> selection;
    if ( sender() )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<void*>() )
        {
            RimPlot* plot = static_cast<RimPlot*>( userData.value<void*>() );
            if ( plot ) selection.push_back( plot );
        }
    }

    if ( selection.empty() )
    {
        selection = caf::SelectionManager::instance()->objectsByType<RimPlot>();
    }

    return selection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubPlotFeature::isAnyDeletablePlotSelected() const
{
    for ( RimPlot* plot : getSelection() )
    {
        if ( !plot ) continue;

        RimMultiPlot*   multiPlot   = plot->firstAncestorOrThisOfType<RimMultiPlot>();
        RimWellLogPlot* wellLogPlot = plot->firstAncestorOrThisOfType<RimWellLogPlot>();
        if ( multiPlot || wellLogPlot ) return true;
    }

    return false;
}
