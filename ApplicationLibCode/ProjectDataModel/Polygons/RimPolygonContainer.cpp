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

#include "RimPolygonContainer.h"

#include "RimPolygon.h"
#include "RimPolygonCollection.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPolygonContainer, "RimPolygonContainer" ); // Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer::RimPolygonContainer()
{
    CAF_PDM_InitObject( "Polygon Container" );

    // m_collectionName, m_subCollections and m_items are initialized by derived classes
    // with derived-specific XML keywords, matching the caf::PdmNestedCollection convention.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonContainer::addNewSubCollection()
{
    auto* sub = new RimPolygonCollection();
    addSubCollection( sub );
    return sub;
}
