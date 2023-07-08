/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSeismicViewCollection.h"

#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimSeismicView.h"

CAF_PDM_SOURCE_INIT( RimSeismicViewCollection, "SeismicViewCollection", "SeismicViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicViewCollection::RimSeismicViewCollection()
{
    CAF_PDM_InitObject( "Views", ":/SeismicViews24x24.png" );

    CAF_PDM_InitFieldNoDefault( &m_views, "Views", "Seismic Views" );
    m_views.uiCapability()->setUiTreeHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicViewCollection::~RimSeismicViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicView*> RimSeismicViewCollection::views() const
{
    return m_views.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicViewCollection::isEmpty()
{
    return !m_views.hasChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicViewCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicView* RimSeismicViewCollection::addView( RimSeismicData* data, RiaDefines::SeismicSectionType defaultSection )
{
    RimSeismicView* view = new RimSeismicView();

    view->setSeismicData( data );
    view->loadDataAndUpdate();

    view->addSlice( defaultSection );

    m_views.push_back( view );

    return view;
}
