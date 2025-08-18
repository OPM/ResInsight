/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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
#include "RicDeleteDiameterRoughnessIntervalFeature.h"

#include "RimDiameterRoughnessInterval.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimProject.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteDiameterRoughnessIntervalFeature, "RicDeleteDiameterRoughnessIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteDiameterRoughnessIntervalFeature::isCommandEnabled() const
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL ) )
    {
        return true;
    }
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>() )
    {
        auto collection = caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>();
        return collection && !collection->intervals().empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteDiameterRoughnessIntervalFeature::onActionTriggered( bool isChecked )
{
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL );
    RimDiameterRoughnessIntervalCollection* intervalCollection = nullptr;
    
    if ( !intervals.empty() )
    {
        // Delete selected intervals
        intervalCollection = intervals[0]->firstAncestorOrThisOfTypeAsserted<RimDiameterRoughnessIntervalCollection>();
        for ( RimDiameterRoughnessInterval* intervalToDelete : intervals )
        {
            intervalCollection->removeInterval( intervalToDelete );
        }
        intervalCollection->updateConnectedEditors();
    }
    else
    {
        // Delete all intervals in collection
        intervalCollection = caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>();
        if ( intervalCollection )
        {
            intervalCollection->removeAllIntervals();
            intervalCollection->updateConnectedEditors();
        }
    }

    if ( intervalCollection )
    {
        if ( intervalCollection->intervals().empty() )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( intervalCollection );
        }

        RimProject* proj = RimProject::current();
        if ( proj )
        {
            proj->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteDiameterRoughnessIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL );
    if ( !intervals.empty() )
    {
        if ( intervals.size() == 1 )
        {
            actionToSetup->setText( "Delete Interval" );
        }
        else
        {
            actionToSetup->setText( QString( "Delete %1 Intervals" ).arg( intervals.size() ) );
        }
        actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>() )
    {
        actionToSetup->setText( "Delete All Intervals" );
        actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
    }
}