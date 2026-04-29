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

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

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

    RimCombinedFilter* target = combined.front();
    if ( auto* view = target->firstAncestorOrThisOfType<Rim3dView>() )
    {
        if ( view->ownerCase() )
        {
            T* created = target->addNewFilter<T>( std::forward<Init>( init ) );
            if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        }
    }
    return true;
}
} // namespace RicCellFilterFeatureTools
