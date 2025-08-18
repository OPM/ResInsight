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
#include "RicNewDiameterRoughnessIntervalFeature.h"

#include "RimDiameterRoughnessInterval.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDiameterRoughnessIntervalFeature, "RicNewDiameterRoughnessIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDiameterRoughnessIntervalFeature::isCommandEnabled() const
{
    return ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>() ||
             caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() ||
             caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletionSettings>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDiameterRoughnessIntervalFeature::onActionTriggered( bool isChecked )
{
    // Try to get collection directly
    RimDiameterRoughnessIntervalCollection* intervalCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>();

    // If not found, try to get it from MSW completion parameters
    if ( !intervalCollection )
    {
        if ( auto mswParams = caf::SelectionManager::instance()->selectedItemOfType<RimMswCompletionParameters>() )
        {
            intervalCollection = mswParams->diameterRoughnessIntervals();
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
                    intervalCollection = mswParams->diameterRoughnessIntervals();
                }
            }
        }
    }

    if ( intervalCollection )
    {
        RimDiameterRoughnessInterval* newInterval = new RimDiameterRoughnessInterval;

        // Set default values
        if ( auto wellPath = intervalCollection->firstAncestorOrThisOfType<RimWellPath>() )
        {
            if ( auto mswParams = wellPath->mswCompletionParameters() )
            {
                newInterval->setStartMD( 0 );
                newInterval->setEndMD( 2000 );
                newInterval->setDiameter( mswParams->linerDiameter() );
                newInterval->setRoughnessFactor( mswParams->roughnessFactor() );
            }
        }

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
void RicNewDiameterRoughnessIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Interval" );
}
