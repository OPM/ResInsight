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

#pragma once

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <type_traits>
#include <utility>
#include <vector>

//==================================================================================================
/// Helpers shared by the cell-filter "New ... Filter" features.
//==================================================================================================
namespace RicCellFilterFeatureTools
{
//--------------------------------------------------------------------------------------------------
/// If a RimCombinedFilter is the current selection, create a new T inside it (configured via init)
/// and select the result. Returns true when a combined filter was the selection — the caller should
/// bail out of its own collection-targeted flow.
//--------------------------------------------------------------------------------------------------
template <typename T, typename Init>
bool addNewFilterIfCombinedSelected( Init&& init )
{
    std::vector<RimCombinedFilter*> combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( combined.empty() ) return false;

    // Combined filters may live under a view OR under the case-level RimDataFilterCollection. The
    // combined filter's own m_srcCase is propagated to new children via addNewFilter, so we don't
    // need a Rim3dView ancestor here.
    RimCombinedFilter* target  = combined.front();
    T*                 created = target->addNewFilter<T>( std::forward<Init>( init ) );
    if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
    return true;
}

//--------------------------------------------------------------------------------------------------
/// If a RimDataFilterCollection (case-level) is the current selection, create a new T inside it,
/// configure it via init, and select the result. Returns true when a data-filter collection was the
/// selection — the caller should bail out of its own collection-targeted flow.
//--------------------------------------------------------------------------------------------------
template <typename T, typename Init>
bool addNewFilterToDataCollectionIfSelected( Init&& init )
{
    static_assert( std::is_base_of_v<RimCellFilter, T>, "T must derive from RimCellFilter" );

    std::vector<RimDataFilterCollection*> selected = caf::selectedObjectsByTypeStrict<RimDataFilterCollection*>();
    if ( selected.empty() ) return false;

    RimDataFilterCollection* target = selected.front();

    auto* created = new T();
    target->addFilter( created );
    std::forward<Init>( init )( created );
    Riu3DMainWindowTools::selectAsCurrentItem( created );
    return true;
}
} // namespace RicCellFilterFeatureTools
