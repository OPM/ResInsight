/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicDeleteAllEventsFeature.h"

#include "Riu3DMainWindowTools.h"

#include "RimWellEventTimeline.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicDeleteAllEventsFeature, "RicDeleteAllEventsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteAllEventsFeature::isCommandEnabled() const
{
    auto* timeline = caf::SelectionManager::instance()->selectedItemOfType<RimWellEventTimeline>();
    return timeline && timeline->eventCount() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteAllEventsFeature::onActionTriggered( bool isChecked )
{
    auto* timeline = caf::SelectionManager::instance()->selectedItemOfType<RimWellEventTimeline>();
    if ( !timeline ) return;

    const auto answer = QMessageBox::question( Riu3DMainWindowTools::mainWindowWidget(),
                                               "Delete All Events",
                                               QString( "Delete all %1 events from the event timeline?" ).arg( timeline->eventCount() ),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::No );
    if ( answer != QMessageBox::Yes ) return;

    timeline->clearAllEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteAllEventsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete All Events" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
