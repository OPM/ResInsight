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

#pragma once

#include "cafPdmNestedCollection.h"

class RimPolygon;

//==================================================================================================
///
/// Common base for polygon containers (folders and files).
///
/// Both RimPolygonCollection (a user-managed folder) and RimPolygonFile (a file-backed
/// folder of polygons) inherit from this. They live polymorphically in the inherited
/// m_subCollections, so the in-view mirror tree picks both up uniformly.
///
//==================================================================================================
class RimPolygonContainer : public caf::PdmNestedCollection<RimPolygonContainer, RimPolygon>
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonContainer();

    // "Add Folder" should produce a real folder (RimPolygonCollection), not another container
    // shell. Override here so the default base impl (new SelfT) is bypassed for both this class
    // and any derivative that does not override it. The runtime instance is a RimPolygonCollection;
    // the return type stays at RimPolygonContainer* to avoid pulling RimPolygonCollection.h into
    // this header (which would create an include cycle).
    RimPolygonContainer* addNewSubCollection() override;

    // Default behavior recurses into sub-collections. Leaf containers (e.g., file-backed)
    // override to load their own data; folder containers inherit the recursion.
    virtual void loadData();

    // Renames the polygon if another polygon in this container already carries the same name.
    void ensureUniquePolygonName( RimPolygon* polygon );

protected:
    // Enforces name uniqueness among sibling folders when the folder is renamed.
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
};
