/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
/////////////////////////////////////////////////////////////////////////////////
#include "RicDeleteSegmentIntervalFeature.h"

#include "RimProject.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteSegmentIntervalFeature, "RicDeleteSegmentIntervalFeature" );

bool RicDeleteSegmentIntervalFeature::isCommandEnabled() const
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimSegmentInterval>( caf::SelectionManager::FIRST_LEVEL ) ) return true;
    auto* collection = caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>();
    return collection && collection->hasIntervals();
}

void RicDeleteSegmentIntervalFeature::onActionTriggered( bool isChecked )
{
    auto selectedIntervals = caf::SelectionManager::instance()->objectsByType<RimSegmentInterval>( caf::SelectionManager::FIRST_LEVEL );
    RimSegmentCollection* collection = nullptr;

    for ( auto* interval : selectedIntervals )
    {
        collection = interval->firstAncestorOrThisOfTypeAsserted<RimSegmentCollection>();
        collection->removeInterval( interval );
    }

    if ( selectedIntervals.empty() )
    {
        collection = caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>();
        if ( collection ) collection->removeAllIntervals();
    }

    if ( !collection ) return;
    collection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( collection );
    if ( auto* project = RimProject::current() ) project->scheduleCreateDisplayModelAndRedrawAllViews();
}

void RicDeleteSegmentIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    auto intervals = caf::SelectionManager::instance()->objectsByType<RimSegmentInterval>( caf::SelectionManager::FIRST_LEVEL );
    actionToSetup->setText( intervals.empty() ? "Delete All Segment Intervals" : "Delete Segment Interval" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
