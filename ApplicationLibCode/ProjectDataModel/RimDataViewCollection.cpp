/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimDataViewCollection.h"

#include "RimDataView.h"

#include "cafPdmDocument.h"

CAF_PDM_SOURCE_INIT( RimDataViewCollection, "DataViewCollection", "DataViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataViewCollection::RimDataViewCollection()
{
    CAF_PDM_InitObject( "Data Views", ":/3DWindow.svg" );

    CAF_PDM_InitFieldNoDefault( &m_items, "Views", "Data Views" );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataViewCollection::~RimDataViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDataView*> RimDataViewCollection::views() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataView* RimDataViewCollection::addView()
{
    auto* view = new RimDataView();

    // Add before loadDataAndUpdate() so RimProject::assignViewIdToView() sees the view in the project tree
    addItem( view );
    caf::PdmDocument::updateUiIconStateRecursively( view );

    view->loadDataAndUpdate();

    return view;
}
