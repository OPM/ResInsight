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

#include "RimDataAnalyticsCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultDistance.h"
#include "RimFaultDistanceCollection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewFaultDistanceResultFeature, "RicNewFaultDistanceResultFeature" );

namespace
{
RimEclipseView* firstViewOfCase( const caf::PdmObjectHandle* object )
{
    if ( !object ) return nullptr;
    RimEclipseCase* eclipseCase = object->firstAncestorOrThisOfType<RimEclipseCase>();
    if ( !eclipseCase ) return nullptr;
    const auto views = eclipseCase->reservoirViews();
    return views.empty() ? nullptr : views.front();
}

RimEclipseView* findHostView()
{
    const auto faultCollections = caf::SelectionManager::instance()->objectsByType<RimFaultInViewCollection>();
    if ( !faultCollections.empty() ) return faultCollections.front()->firstAncestorOrThisOfType<RimEclipseView>();

    const auto analyticsCollections = caf::SelectionManager::instance()->objectsByType<RimDataAnalyticsCollection>();
    if ( !analyticsCollections.empty() ) return firstViewOfCase( analyticsCollections.front() );

    const auto distanceCollections = caf::SelectionManager::instance()->objectsByType<RimFaultDistanceCollection>();
    if ( !distanceCollections.empty() ) return firstViewOfCase( distanceCollections.front() );

    const auto distanceResults = caf::SelectionManager::instance()->objectsByType<RimFaultDistance>();
    if ( !distanceResults.empty() ) return firstViewOfCase( distanceResults.front() );

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

    RimFaultDistanceCollection* distanceCollection = eclipseView->faultDistanceCollection();
    if ( !distanceCollection ) return;

    const auto                   selectedFaultPointers = caf::SelectionManager::instance()->objectsByType<RimFaultInView>();
    std::vector<RimFaultInView*> selectedFaults( selectedFaultPointers.begin(), selectedFaultPointers.end() );

    RimFaultDistance* newResult = distanceCollection->addResult();
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
