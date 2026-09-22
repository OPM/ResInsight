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

    QString computeDisplayName() const override;
    void    prepareForSync() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    RimPolygonInView* findPolygonInView( const RimPolygon* polygon ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    // Explicit per-view choice: whether this view should follow the source container's own
    // per-view realization resolution (see RimPolygonContainer::resolveViewMatchingRealization).
    // Only relevant (and only shown in the UI) when sourceCollection()->supportsRealizationOverride()
    // is true -- i.e. only on the single mirror node representing a whole RimPolygonCloudSource,
    // governing every RimPolygonCloudAddress leaf beneath it in this view. When checked and the
    // resolved realization differs from the source's own base realization, every RimPolygonCloudAddress
    // leaf beneath this node (in this view) substitutes that realization's cached RimCloudPolygon
    // data directly in place of its own base items (see sourceItems()) -- this is also the "compare
    // with base realization" mechanism, with no separate comparison UI or extra visible tree nodes.
    // Disabled (read-only, with an explanatory tooltip) when the view's own case belongs to a
    // different case/ensemble than the source's own data source, since "follow view" would not be
    // meaningful there.
    caf::PdmField<bool> m_useAutoRealization;

    // Resolves the realization the owning 3D view's own case matches for the current source
    // container, or -1 if there is no view ancestor or the view's case belongs to a different
    // case/ensemble than the source's Applied data source. See
    // RimPolygonContainer::resolveViewMatchingRealization.
    int viewMatchingRealizationOrMinusOne() const;

    // Walks up this mirror node's own ancestor chain (this node included) for the one whose
    // sourceCollection() is a RimPolygonCloudSource -- the node that owns the single Auto-Follow
    // checkbox governing every leaf beneath it. Returns nullptr if this mirror is not nested under
    // a RimPolygonCloudSource mirror at all.
    const RimPolygonInViewCollection* sourceMirrorAncestorOrThis() const;

    // The realization this mirror node's own items should effectively show: the ancestor
    // RimPolygonCloudSource mirror's Auto-Follow-resolved realization, or -1 (meaning: the
    // address's own base/Applied realization) if Auto-Follow is off, doesn't match, or there is no
    // such ancestor.
    int effectiveRealization() const;
};
