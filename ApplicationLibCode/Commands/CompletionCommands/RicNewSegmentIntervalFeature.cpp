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
#include "RicNewSegmentIntervalFeature.h"

#include "RimProject.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSegmentIntervalFeature, "RicNewSegmentIntervalFeature" );

bool RicNewSegmentIntervalFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>() != nullptr;
}

void RicNewSegmentIntervalFeature::onActionTriggered( bool isChecked )
{
    auto* collection = caf::SelectionManager::instance()->selectedItemOfType<RimSegmentCollection>();
    if ( !collection ) return;

    collection->setDiameterRoughnessMode( RimSegmentCollection::DiameterRoughnessMode::INTERVALS );
    auto* interval = collection->createInterval( 0.0, 2000.0, collection->linerDiameter(), collection->roughnessFactor() );
    collection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( interval );

    if ( auto* project = RimProject::current() ) project->scheduleCreateDisplayModelAndRedrawAllViews();
}

void RicNewSegmentIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Segment Interval" );
}
