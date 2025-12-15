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
#include "RicDeleteCustomSegmentIntervalFeature.h"

#include "RimCustomSegmentInterval.h"
#include "RimCustomSegmentIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteCustomSegmentIntervalFeature, "RicDeleteCustomSegmentIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteCustomSegmentIntervalFeature::isCommandEnabled() const
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentInterval>( caf::SelectionManager::FIRST_LEVEL ) )
    {
        return true;
    }

    if ( auto collection = caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentIntervalCollection>() )
    {
        return collection && !collection->intervals().empty();
    }

    if ( auto mswParams = caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() )
    {
        if ( mswParams->customSegmentIntervals() )
        {
            return !mswParams->customSegmentIntervals()->intervals().empty();
        }
    }

    if ( auto completionSettings = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() )
    {
        if ( auto wellPath = completionSettings->firstAncestorOrThisOfType<RimWellPath>() )
        {
            auto mswParams = wellPath->mswCompletionParameters();
            if ( mswParams && mswParams->customSegmentIntervals() )
            {
                return !mswParams->customSegmentIntervals()->intervals().empty();
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomSegmentIntervalFeature::onActionTriggered( bool isChecked )
{
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimCustomSegmentInterval>( caf::SelectionManager::FIRST_LEVEL );
    RimCustomSegmentIntervalCollection* intervalCollection = nullptr;

    if ( !intervals.empty() )
    {
        // Delete selected intervals
        intervalCollection = intervals[0]->firstAncestorOrThisOfTypeAsserted<RimCustomSegmentIntervalCollection>();
        for ( RimCustomSegmentInterval* intervalToDelete : intervals )
        {
            intervalCollection->removeInterval( intervalToDelete );
        }
        intervalCollection->updateConnectedEditors();
    }
    else
    {
        // Delete all intervals in collection
        intervalCollection = caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentIntervalCollection>();
        if ( !intervalCollection )
        {
            // Handle selection of MSW completion parameters
            auto mswParams = caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>();
            if ( mswParams && mswParams->customSegmentIntervals() )
            {
                intervalCollection = mswParams->customSegmentIntervals();
            }
            else
            {
                // Handle selection of well path completion settings
                if ( auto completionSettings = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() )
                {
                    if ( auto wellPath = completionSettings->firstAncestorOrThisOfType<RimWellPath>() )
                    {
                        auto mswParams = wellPath->mswCompletionParameters();
                        if ( mswParams && mswParams->customSegmentIntervals() )
                        {
                            intervalCollection = mswParams->customSegmentIntervals();
                        }
                    }
                }
            }
        }

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

        if ( RimProject* proj = RimProject::current() )
        {
            proj->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomSegmentIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimCustomSegmentInterval>( caf::SelectionManager::FIRST_LEVEL );
    if ( !intervals.empty() )
    {
        if ( intervals.size() == 1 )
        {
            actionToSetup->setText( "Delete Segment Interval" );
        }
        else
        {
            actionToSetup->setText( QString( "Delete %1 Segment Intervals" ).arg( intervals.size() ) );
        }
        actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentIntervalCollection>() ||
              caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() ||
              caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() )
    {
        actionToSetup->setText( "Delete All Segment Intervals" );
        actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
    }
}
