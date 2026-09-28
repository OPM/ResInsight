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

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    RimPolygonInView* findPolygonInView( const RimPolygon* polygon ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    // Per-view choice: follow the source container's own per-view realization resolution (see
    // RimPolygonContainer::resolveViewMatchingRealization). Shown only on the mirror node for a
    // whole RimPolygonCloudSource, governing every leaf beneath it. When checked and resolved to a
    // realization other than the source's base one, every leaf substitutes that realization's
    // cached data for its own base items (see sourceItems()) -- this is also the base-realization
    // comparison mechanism. Disabled with a tooltip when the view's case doesn't match the
    // source's data source.
    caf::PdmField<bool> m_useAutoRealization;

    // Non-persisted UI-only shadow of m_useAutoRealization: when the checkbox is read-only
    // (mismatched view case), it must visually read as unchecked even though the real field keeps
    // its persisted value (so it re-applies once the case matches again). Synced with
    // m_useAutoRealization whenever editable; user edits write back in fieldChangedByUi().
    caf::PdmField<bool> m_useAutoRealizationUiState;

    // Realization the owning view's case matches for the current source, or -1 if no match/no
    // view ancestor. See RimPolygonContainer::resolveViewMatchingRealization.
    int viewMatchingRealizationOrMinusOne() const;

    // Walks up to the ancestor mirror node (this included) whose sourceCollection() is a
    // RimPolygonCloudSource -- the node owning the Auto-Follow checkbox. Nullptr if none.
    const RimPolygonInViewCollection* sourceMirrorAncestorOrThis() const;

    // Realization this node's items should show: the ancestor source's Auto-Follow-resolved
    // realization, or -1 (base/Applied) if Auto-Follow is off, mismatched, or no such ancestor.
    int effectiveRealization() const;
};
