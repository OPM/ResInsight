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
#include "RicCreateSegmentIntervalFeature.h"

#include "RimProject.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QIcon>

CAF_CMD_SOURCE_INIT( RicCreateSegmentIntervalFeature, "RicCreateSegmentIntervalFeature" );

bool RicCreateSegmentIntervalFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>() != nullptr;
}

void RicCreateSegmentIntervalFeature::onActionTriggered( bool isChecked )
{
    auto* collection = caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>();
    if ( !collection ) return;

    auto* interval = collection->createInterval( 0.0, 2000.0, collection->linerDiameter(), collection->roughnessFactor() );
    collection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( interval );

    if ( auto* project = RimProject::current() ) project->scheduleCreateDisplayModelAndRedrawAllViews();
}

void RicCreateSegmentIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Segment Interval" );
    actionToSetup->setIcon( QIcon( ":/Segment.svg" ) );
}
