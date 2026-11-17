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

#include "RicCombineSelectedFiltersFeature.h"

#include "RimCellFilter.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <algorithm>

CAF_CMD_SOURCE_INIT( RicCombineSelectedFiltersFeature, "RicCombineSelectedFiltersFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// The collection owning all selected filters directly, or nullptr if fewer than two filters are
/// selected or they do not share a data filter collection.
//--------------------------------------------------------------------------------------------------
RimDataFilterCollection* commonDataFilterCollection( const std::vector<RimCellFilter*>& selected )
{
    if ( selected.size() < 2 ) return nullptr;

    auto* collection = selected.front()->firstAncestorOfType<RimDataFilterCollection>();
    if ( !collection ) return nullptr;

    const auto items = collection->filters();
    for ( RimCellFilter* filter : selected )
    {
        if ( std::find( items.begin(), items.end(), filter ) == items.end() ) return nullptr;
    }
    return collection;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCombineSelectedFiltersFeature::isCommandEnabled() const
{
    return commonDataFilterCollection( caf::selectedObjectsByTypeStrict<RimCellFilter*>() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCombineSelectedFiltersFeature::onActionTriggered( bool isChecked )
{
    auto selected   = caf::selectedObjectsByTypeStrict<RimCellFilter*>();
    auto collection = commonDataFilterCollection( selected );
    if ( !collection ) return;

    if ( auto* combined = collection->combineFilters( selected ) )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( combined );
        Riu3DMainWindowTools::setExpanded( combined, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCombineSelectedFiltersFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "Combine Selected Filters" );
}
