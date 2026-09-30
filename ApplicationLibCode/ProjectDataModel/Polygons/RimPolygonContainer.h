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
class Rim3dView;

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

    // "Add Folder" should produce a real RimPolygonCollection, not another container shell.
    // Return type stays RimPolygonContainer* to avoid an include cycle.
    RimPolygonContainer* addNewSubCollection() override;

    // Default recurses into sub-collections; leaf containers override to load their own data.
    virtual void loadData();

    // Whether this container's content genuinely varies per realization (only a cloud-backed
    // address does). Default: false.
    virtual bool supportsRealizationOverride() const;

    // Realizations available for the override above. Default: empty.
    virtual std::vector<int> availableRealizationIdsForOverride() const;

    // Resolves an automatic "follow the view's own case" realization, only when the view's case
    // genuinely matches this container's own data source. Returns -1 otherwise (fall back to this
    // container's own default items). Default: always -1; only RimPolygonCloudAddress overrides
    // this, to avoid applying an unrelated view's realization to a different field/ensemble.
    virtual int resolveViewMatchingRealization( const Rim3dView* view ) const;

    // Renames the polygon if another polygon in this container already carries the same name.
    void ensureUniquePolygonName( RimPolygon* polygon );

protected:
    // Enforces name uniqueness among sibling folders when the folder is renamed.
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
};
