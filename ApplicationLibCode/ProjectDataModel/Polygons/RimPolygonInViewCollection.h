/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimNestedMirrorCollectionInView.h"
#include "RimPolygonContainer.h"
#include "RimPolygonInView.h"

#include "cafPdmField.h"

class RimPolygon;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonInViewCollection : public RimNestedMirrorCollectionInView<RimPolygonInViewCollection, RimPolygonContainer, RimPolygonInView>
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonInViewCollection();

    // Refreshes this mirror collection from its RimPolygonCollection source.
    void updateFromPolygonCollection();

    std::vector<RimPolygonInView*> visiblePolygonsInView() const;
    std::vector<RimPolygonInView*> allPolygonsInView() const;

    bool setPolygonVisible( RimPolygon* polygon, bool visible );

protected:
    std::vector<RimPolygonContainer*> sourceSubCollections() const override;
    std::vector<RimPolygon*>          sourceItems() const override;
    RimPolygonInView*                 createItemInView( RimPolygon* source ) override;

    RimPolygonInViewCollection* createSubCollectionInView( RimPolygonContainer* src ) override;

    void onSynced() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

public:
    // Whether the given source container is currently shown checked (as either the base container
    // itself, or -- since a source container can also be a RimPolygonCloudRealizationGroup --
    // matched by pointer identity) in any open view's mirror tree. Used to decide whether fetched
    // RimCloudPolygon data can safely be evicted once a checkbox is unchecked in one view: it may
    // still be needed by another.
    static bool isSourceCheckedInAnyView( const RimPolygonContainer* source );

private:
    RimPolygonInView* findPolygonInView( const RimPolygon* polygon ) const;

    // Recursively searches this mirror node (and its mirrored sub-collections) for the node whose
    // sourceCollection() is exactly the given container. Returns nullptr if not found in this
    // view's tree.
    const RimPolygonInViewCollection* findMirrorForSource( const RimPolygonContainer* source ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    // Explicit per-view choice: whether this view should follow the source container's own
    // per-view realization resolution (see RimPolygonContainer::resolveViewMatchingRealization).
    // Only relevant (and only shown in the UI) when sourceCollection()->supportsRealizationOverride()
    // is true -- i.e. only on the single mirror node representing a whole RimPolygonCloudSource,
    // governing every RimPolygonCloudAddress leaf beneath it in this view. When checked and the
    // resolved realization differs from the source's own base realization, every currently-checked
    // leaf under this source (in this view) gets a RimPolygonCloudRealizationGroup materialized/
    // shown for that realization (see onSynced()) -- this is also the "compare with base
    // realization" mechanism, with no separate comparison UI. Disabled (read-only, with an
    // explanatory tooltip) when the view's own case belongs to a different case/ensemble than the
    // source's own data source, since "follow view" would not be meaningful there.
    caf::PdmField<bool> m_useAutoRealization;

    // Set once this node has applied its one-shot default for m_useAutoRealization (see
    // onSynced()). Persisted so a user's later manual checkbox toggle is never silently
    // overridden again, including across a project save/reload.
    caf::PdmField<bool> m_didApplyDefaultAutoRealization;

    // Resolves the realization the owning 3D view's own case matches for the current source
    // container, or -1 if there is no view ancestor or the view's case belongs to a different
    // case/ensemble than the source's Applied data source. See
    // RimPolygonContainer::resolveViewMatchingRealization.
    int viewMatchingRealizationOrMinusOne() const;

    // All RimPolygonCloudAddress source objects that are checked-visible (m_isChecked) somewhere
    // in this mirror node's own subtree (this node included).
    std::vector<class RimPolygonCloudAddress*> allCheckedAddressesRecursively() const;
};
