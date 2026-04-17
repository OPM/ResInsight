/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "RicNewRefinementRegionFeature.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimRefinementRegion.h"
#include "RimRefinementRegionCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewRefinementRegionFeature, "RicNewRefinementRegionFeature" );

namespace
{
RimRefinementRegionCollection* selectedCollection()
{
    auto collections = caf::selectedObjectsByTypeStrict<RimRefinementRegionCollection*>();
    if ( !collections.empty() ) return collections.front();

    auto views = caf::selectedObjectsByTypeStrict<RimEclipseView*>();
    if ( !views.empty() && views.front() ) return views.front()->refinementRegionCollection();

    auto regions = caf::selectedObjectsByTypeStrict<RimRefinementRegion*>();
    if ( !regions.empty() && regions.front() )
    {
        return regions.front()->firstAncestorOrThisOfType<RimRefinementRegionCollection>();
    }

    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRefinementRegionFeature::isCommandEnabled() const
{
    return selectedCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRefinementRegionFeature::onActionTriggered( bool isChecked )
{
    auto collection = selectedCollection();
    if ( !collection ) return;

    auto* view        = collection->firstAncestorOrThisOfType<RimEclipseView>();
    auto* eclipseCase = view ? view->eclipseCase() : nullptr;

    auto* region = collection->addNewRegion( eclipseCase );

    if ( view ) view->scheduleCreateDisplayModelAndRedraw();

    collection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( region );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRefinementRegionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Refinement Region" );
    actionToSetup->setIcon( QIcon( ":/CellFilter_Range.png" ) );
}
