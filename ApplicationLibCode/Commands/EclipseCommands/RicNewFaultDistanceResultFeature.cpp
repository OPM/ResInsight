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

#include "RicNewFaultDistanceResultFeature.h"

#include "RimEclipseView.h"
#include "RimFaultDistanceResult.h"
#include "RimFaultDistanceResultCollection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewFaultDistanceResultFeature, "RicNewFaultDistanceResultFeature" );

namespace
{
RimEclipseView* findHostView()
{
    const auto faultCollections = caf::SelectionManager::instance()->objectsByType<RimFaultInViewCollection>();
    if ( !faultCollections.empty() ) return faultCollections.front()->firstAncestorOrThisOfType<RimEclipseView>();

    const auto distanceCollections = caf::SelectionManager::instance()->objectsByType<RimFaultDistanceResultCollection>();
    if ( !distanceCollections.empty() ) return distanceCollections.front()->firstAncestorOrThisOfType<RimEclipseView>();

    const auto distanceResults = caf::SelectionManager::instance()->objectsByType<RimFaultDistanceResult>();
    if ( !distanceResults.empty() ) return distanceResults.front()->firstAncestorOrThisOfType<RimEclipseView>();

    const auto faults = caf::SelectionManager::instance()->objectsByType<RimFaultInView>();
    if ( !faults.empty() ) return faults.front()->firstAncestorOrThisOfType<RimEclipseView>();

    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFaultDistanceResultFeature::isCommandEnabled() const
{
    return findHostView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultDistanceResultFeature::onActionTriggered( bool isChecked )
{
    RimEclipseView* eclipseView = findHostView();
    if ( !eclipseView ) return;

    RimFaultDistanceResultCollection* distanceCollection = eclipseView->faultDistanceResults();
    if ( !distanceCollection ) return;

    const auto                   selectedFaultPointers = caf::SelectionManager::instance()->objectsByType<RimFaultInView>();
    std::vector<RimFaultInView*> selectedFaults( selectedFaultPointers.begin(), selectedFaultPointers.end() );

    RimFaultDistanceResult* newResult = distanceCollection->addResult();
    if ( !newResult ) return;

    if ( !selectedFaults.empty() )
    {
        newResult->setSelectedFaults( selectedFaults );
    }
    else if ( eclipseView->faultCollection() )
    {
        newResult->setSelectedFaults( eclipseView->faultCollection()->faults() );
    }

    eclipseView->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( newResult );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultDistanceResultFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Fault Distance" );
    actionToSetup->setIcon( QIcon( ":/draw_style_faults_24x24.png" ) );
}
