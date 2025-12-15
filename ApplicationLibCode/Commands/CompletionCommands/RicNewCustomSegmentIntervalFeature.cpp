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
#include "RicNewCustomSegmentIntervalFeature.h"

#include "RimCustomSegmentInterval.h"
#include "RimCustomSegmentIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCustomSegmentIntervalFeature, "RicNewCustomSegmentIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCustomSegmentIntervalFeature::isCommandEnabled() const
{
    return ( caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentIntervalCollection>() ||
             caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() ||
             caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomSegmentIntervalFeature::onActionTriggered( bool isChecked )
{
    // Try to get collection directly
    RimCustomSegmentIntervalCollection* intervalCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimCustomSegmentIntervalCollection>();

    // If not found, try to get it from MSW completion parameters
    if ( !intervalCollection )
    {
        if ( auto mswParams = caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() )
        {
            intervalCollection = mswParams->customSegmentIntervals();
        }
    }

    // If still not found, try to get it from well path completion settings
    if ( !intervalCollection )
    {
        if ( auto completionSettings = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() )
        {
            if ( auto wellPath = completionSettings->firstAncestorOrThisOfType<RimWellPath>() )
            {
                if ( auto mswParams = wellPath->mswCompletionParameters() )
                {
                    intervalCollection = mswParams->customSegmentIntervals();
                }
            }
        }
    }

    if ( intervalCollection )
    {
        RimCustomSegmentInterval* newInterval = new RimCustomSegmentInterval;

        // Set default MD values
        newInterval->setStartMD( 0 );
        newInterval->setEndMD( 100 );

        intervalCollection->addInterval( newInterval );
        intervalCollection->updateConnectedEditors();

        Riu3DMainWindowTools::selectAsCurrentItem( newInterval );

        if ( RimProject* project = RimProject::current() )
        {
            project->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomSegmentIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Segment Interval" );
}
