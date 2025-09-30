/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimcGridView.h"

#include "RiaApplication.h"
#include "RiaKeyValueStoreUtil.h"

#include "RimEclipseView.h"
#include "RimGridView.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cvfArray.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseView, RimcGridView_visibleCellsInternal, "visible_cells_internal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGridView_visibleCellsInternal::RimcGridView_visibleCellsInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Visible Cells Internal", "", "", "Visible Cells Internal" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_visibilityKey, "VisibilityKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGridView_visibleCellsInternal::execute()
{
    auto gridView = self<RimGridView>();
    if ( !gridView )
    {
        return std::unexpected( QString( "Grid view is null." ) );
    }

    if ( m_visibilityKey().isEmpty() )
    {
        return std::unexpected( QString( "Visibility key is empty." ) );
    }

    // Get the cell visibility array
    cvf::ref<cvf::UByteArray> visibleCells = gridView->currentTotalCellVisibility();

    if ( visibleCells.isNull() )
    {
        return std::unexpected( QString( "Failed to get cell visibility data." ) );
    }

    // Convert cvf::UByteArray to std::vector<float> where 1.0 = visible, 0.0 = invisible
    std::vector<float> visibilityValues;
    visibilityValues.reserve( visibleCells->size() );
    for ( size_t i = 0; i < visibleCells->size(); ++i )
    {
        visibilityValues.push_back( ( *visibleCells )[i] ? 1.0f : 0.0f );
    }

    // Store the visibility data in the key-value store
    auto keyValueStore = RiaApplication::instance()->keyValueStore();
    keyValueStore->set( m_visibilityKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( visibilityValues ) );

    return nullptr;
}
