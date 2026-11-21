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

#include "RicDeleteSelectedEventsFeature.h"

#include "Riu3DMainWindowTools.h"

#include "RimWellEvent.h"
#include "RimWellEventTimeline.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

#include <map>

CAF_CMD_SOURCE_INIT( RicDeleteSelectedEventsFeature, "RicDeleteSelectedEventsFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Selected events grouped by owning timeline. Empty if the selection contains anything but events.
//--------------------------------------------------------------------------------------------------
std::map<RimWellEventTimeline*, std::vector<RimWellEvent*>> selectedEventsByTimeline()
{
    std::map<RimWellEventTimeline*, std::vector<RimWellEvent*>> result;

    for ( caf::PdmUiItem* item : caf::SelectionManager::instance()->selectedItems() )
    {
        auto* event = dynamic_cast<RimWellEvent*>( item );
        if ( !event ) return {};

        auto* timeline = event->firstAncestorOfType<RimWellEventTimeline>();
        if ( !timeline ) return {};

        result[timeline].push_back( event );
    }

    return result;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSelectedEventsFeature::isCommandEnabled() const
{
    return !selectedEventsByTimeline().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSelectedEventsFeature::onActionTriggered( bool isChecked )
{
    const auto eventsByTimeline = selectedEventsByTimeline();

    size_t count = 0;
    for ( const auto& [timeline, events] : eventsByTimeline )
        count += events.size();
    if ( count == 0 ) return;

    const auto answer = QMessageBox::question( Riu3DMainWindowTools::mainWindowWidget(),
                                               "Delete Selected Events",
                                               QString( "Delete %1 selected event(s)?" ).arg( count ),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::No );
    if ( answer != QMessageBox::Yes ) return;

    for ( const auto& [timeline, events] : eventsByTimeline )
    {
        timeline->removeEvents( events );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSelectedEventsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Selected Events" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
