/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicEclipseCombinedPropertyFilterNewFeature.h"

#include "CellFilterCommands/RicCellFilterFeatureTools.h"
#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseCombinedPropertyFilterNewFeature, "RicEclipseCombinedPropertyFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCombinedPropertyFilterNewFeature::isCommandEnabled() const
{
    // Enable on either the collection, a combined filter (for nesting, regardless of whether the
    // host collection is per-view or case-level), or a case-level data filter collection directly.
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() ) return true;

    if ( RicCellFilterFeatureTools::selectedDataFilterCollection() ) return true;

    if ( auto* target = RicEclipsePropertyFilterFeatureImpl::resolveTargetPropertyFilterCollection() )
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( target );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCombinedPropertyFilterNewFeature::onActionTriggered( bool isChecked )
{
    // Nested combined filter: add the new one inside the currently selected combined filter,
    // regardless of whether the host is per-view or case-level. The combined filter's m_srcCase
    // propagates into new children automatically.
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() )
    {
        RimCombinedFilter* parent  = combined.front();
        RimCombinedFilter* created = parent->addNewFilter<RimCombinedFilter>( []( RimCombinedFilter* ) {} );
        parent->updateConnectedEditors();
        if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        return;
    }

    // Case-level data filter collection (selected directly, or via the owning RimEclipseCase node):
    // add a new combined filter directly there.
    if ( auto* dataColl = RicCellFilterFeatureTools::selectedDataFilterCollection() )
    {
        RimCombinedFilter* created = dataColl->addNewCombinedFilter();
        if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        return;
    }

    auto* target = RicEclipsePropertyFilterFeatureImpl::resolveTargetPropertyFilterCollection();
    if ( !target ) return;

    RimCombinedFilter* created = target->addNewCombinedFilter();
    target->updateConnectedEditors();
    if ( created )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( created );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCombinedPropertyFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "New Combined Property Filter" );
}
