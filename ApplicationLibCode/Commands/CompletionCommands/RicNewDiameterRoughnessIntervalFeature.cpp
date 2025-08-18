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
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDiameterRoughnessIntervalFeature, "RicNewDiameterRoughnessIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDiameterRoughnessIntervalFeature::isCommandEnabled() const
{
    {
        const auto intervals = caf::SelectionManager::instance()->objectsByType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL );
        if ( !intervals.empty() )
        {
            return true;
        }
    }

    {
        if ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDiameterRoughnessIntervalFeature::onActionTriggered( bool isChecked )
{
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL );
    RimDiameterRoughnessInterval* newInterval = nullptr;
    
    if ( intervals.size() == 1u )
    {
        // Insert before selected interval
        auto intervalCollection = intervals[0]->firstAncestorOrThisOfTypeAsserted<RimDiameterRoughnessIntervalCollection>();

        newInterval = new RimDiameterRoughnessInterval;
        
        // Set some default values
        auto wellPath = intervalCollection->firstAncestorOrThisOfType<RimWellPath>();
        if ( wellPath )
        {
            auto mswParams = wellPath->mswCompletionParameters();
            if ( mswParams )
            {
                newInterval->setDiameter( mswParams->linerDiameter() );
                newInterval->setRoughnessFactor( mswParams->roughnessFactor() );
            }
        }
        
        intervalCollection->insertInterval( intervals[0], newInterval );
        intervalCollection->updateConnectedEditors();
    }
    else
    {
        // Append to collection
        auto intervalCollection = caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>();
        if ( intervalCollection )
        {
            newInterval = new RimDiameterRoughnessInterval;
            
            // Set some default values
            auto wellPath = intervalCollection->firstAncestorOrThisOfType<RimWellPath>();
            if ( wellPath )
            {
                auto mswParams = wellPath->mswCompletionParameters();
                if ( mswParams )
                {
                    newInterval->setDiameter( mswParams->linerDiameter() );
                    newInterval->setRoughnessFactor( mswParams->roughnessFactor() );
                }
            }
            
            intervalCollection->insertInterval( nullptr, newInterval );
            intervalCollection->updateConnectedEditors();
        }
    }

    if ( newInterval )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( newInterval );
        
        RimProject* project = RimProject::current();
        if ( project )
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
    const auto intervals = caf::SelectionManager::instance()->objectsByType<RimDiameterRoughnessInterval>( caf::SelectionManager::FIRST_LEVEL );
    if ( intervals.size() == 1u )
    {
        actionToSetup->setText( "Insert New Interval" );
        actionToSetup->setIcon( QIcon( ":/Plus.svg" ) );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimDiameterRoughnessIntervalCollection>() )
    {
        actionToSetup->setText( "New Interval" );
        actionToSetup->setIcon( QIcon( ":/Plus.svg" ) );
    }
}