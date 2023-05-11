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

#include "RicDeleteSubItemsFeature.h"

#include "RimProject.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteSubItemsFeature, "RicDeleteSubItemsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubItemsFeature::isCommandEnabled()
{
    return RicDeleteSubItemsFeature::canCommandBeEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubItemsFeature::canCommandBeEnabled()
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems( items );

    if ( items.empty() ) return false;

    for ( auto* item : items )
    {
        if ( !RicDeleteSubItemsFeature::hasDeletableSubItems( item ) ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubItemsFeature::onActionTriggered( bool isChecked )
{
    bool onlyUnchecked = false;
    RicDeleteSubItemsFeature::deleteSubItems( onlyUnchecked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubItemsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Sub Items" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubItemsFeature::hasDeletableSubItems( caf::PdmUiItem* uiItem )
{
    {
        auto multiPlot = dynamic_cast<RimSummaryMultiPlot*>( uiItem );
        if ( multiPlot && !multiPlot->summaryPlots().empty() )
        {
            return true;
        }
    }

    {
        auto collection = dynamic_cast<RimSummaryMultiPlotCollection*>( uiItem );
        if ( collection && !collection->multiPlots().empty() )
        {
            return true;
        }
    }

    {
        auto collection = dynamic_cast<RimWellPathCollection*>( uiItem );
        if ( collection && !collection->allWellPaths().empty() )
        {
            return true;
        }
    }

    {
        auto collection = dynamic_cast<RimWellPathFractureCollection*>( uiItem );
        if ( collection && !collection->allFractures().empty() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubItemsFeature::deleteSubItems( bool onlyDeleteUnchecked )
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems( items );

    CVF_ASSERT( items.size() > 0 );

    for ( auto item : items )
    {
        if ( !RicDeleteSubItemsFeature::hasDeletableSubItems( item ) ) continue;

        {
            auto multiPlot = dynamic_cast<RimSummaryMultiPlot*>( item );
            if ( multiPlot )
            {
                if ( onlyDeleteUnchecked )
                {
                    auto plots = multiPlot->plots();
                    for ( auto plot : plots )
                    {
                        if ( plot->showWindow() ) continue;
                        multiPlot->removePlotNoUpdate( plot );
                        delete plot;
                    }
                }
                else
                {
                    multiPlot->deleteAllPlots();
                }

                multiPlot->updateConnectedEditors();
            }
        }

        {
            auto collection = dynamic_cast<RimSummaryMultiPlotCollection*>( item );
            if ( collection )
            {
                if ( onlyDeleteUnchecked )
                {
                    auto plots = collection->multiPlots();
                    for ( auto plot : plots )
                    {
                        if ( plot->showWindow() ) continue;
                        collection->removePlotNoUpdate( plot );
                        delete plot;
                    }
                }
                else
                {
                    collection->deleteAllPlots();
                }

                collection->updateConnectedEditors();
            }
        }

        {
            auto collection = dynamic_cast<RimWellPathCollection*>( item );
            if ( collection )
            {
                if ( onlyDeleteUnchecked )
                {
                    auto paths = collection->allWellPaths();
                    for ( auto path : paths )
                    {
                        if ( path->showWellPath() ) continue;
                        collection->removeWellPath( path );
                        delete path;
                    }
                }
                else
                {
                    collection->deleteAllWellPaths();
                }

                collection->updateConnectedEditors();
                collection->scheduleRedrawAffectedViews();
            }
        }

        {
            auto collection = dynamic_cast<RimWellPathFractureCollection*>( item );
            if ( collection )
            {
                if ( onlyDeleteUnchecked )
                {
                    auto items = collection->allFractures();
                    for ( auto item : items )
                    {
                        if ( item->isChecked() ) continue;
                        collection->removeFracture( item );
                        delete item;
                    }
                }
                else
                {
                    collection->deleteFractures();
                }

                collection->updateConnectedEditors();

                RimProject* proj = RimProject::current();
                if ( proj ) proj->reloadCompletionTypeResultsInAllViews();
            }
        }
    }
}
