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

    // "Add Folder" should produce a real folder (RimPolygonCollection), not another container
    // shell. Override here so the default base impl (new SelfT) is bypassed for both this class
    // and any derivative that does not override it. The runtime instance is a RimPolygonCollection;
    // the return type stays at RimPolygonContainer* to avoid pulling RimPolygonCollection.h into
    // this header (which would create an include cycle).
    RimPolygonContainer* addNewSubCollection() override;

    // Default behavior recurses into sub-collections. Leaf containers (e.g., file-backed)
    // override to load their own data; folder containers inherit the recursion.
    virtual void loadData();

    // Returns the items a view following the given realization should mirror. realization is -1
    // when the view has none (or the caller has no view context), in which case a container
    // should return its own default items() (its "Applied"/tree-displayed set). Containers whose
    // content is realization-independent (folders, files) can ignore the parameter entirely --
    // the default implementation does exactly that. Only containers whose content genuinely
    // depends on which realization is being shown (e.g. a cloud-backed address) need to override
    // this, and must do so without mutating this container's own persisted/displayed state as a
    // side effect of a view merely asking for its items (multiple views may ask for different
    // realizations of the same container).
    virtual std::vector<RimPolygon*> itemsForRealization( int realization ) const;

    // Whether this container's content genuinely varies per realization (only a cloud-backed
    // address does) -- used by RimPolygonInViewCollection to decide whether to show a per-view
    // realization override field at all. Default: false (folders/files have no such concept).
    virtual bool supportsRealizationOverride() const;

    // The realizations available to pick from for the override above (e.g. the data source's
    // selected realizations for a cloud-backed address). Default: empty.
    virtual std::vector<int> availableRealizationIdsForOverride() const;

    // When no explicit per-view realization override is set (see
    // RimPolygonInViewCollection::m_realizationOverride, sentinel -1), this lets a container
    // resolve an automatic "follow the view's own case" realization, but only when that is
    // clearly safe -- i.e. the view's own case genuinely corresponds to this container's own
    // data source/case/ensemble. Returns -1 (=> fall back to this container's own Applied/
    // default items) whenever the view's case does not match or the concept does not apply.
    // Default: always -1 -- only a cloud-backed address (RimPolygonCloudAddress) overrides
    // this, since only it has a notion of "its own case" to compare the view's case against.
    // A view whose case belongs to a completely different field/ensemble (e.g. a Johan
    // Sverdrup grid case in a mainly-Drogon project) must never have its realization applied
    // to an unrelated address -- that is exactly the bug this matching guards against.
    virtual int resolveViewMatchingRealization( const Rim3dView* view ) const;

    // The name a view mirroring this container for the given realization (-1 meaning "this
    // container's own default/Applied realization") should display. Default: ignores the
    // parameter and returns collectionName() -- only a cloud-backed address, whose tree name
    // embeds a realization number, needs this to vary per realization (e.g. so a view following
    // its own case's realization shows "Real 1" in the tree even while the address itself is
    // Applied to "Real 0").
    virtual QString displayNameForRealization( int realization ) const;

    // Renames the polygon if another polygon in this container already carries the same name.
    void ensureUniquePolygonName( RimPolygon* polygon );

protected:
    // Enforces name uniqueness among sibling folders when the folder is renamed.
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
};
