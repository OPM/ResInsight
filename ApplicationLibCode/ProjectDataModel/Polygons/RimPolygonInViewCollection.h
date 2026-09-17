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

    // Refreshes this mirror collection from its RimPolygonCollection source. Never derives a
    // realization from the owning 3D view's own case -- see m_realizationOverride below.
    void updateFromPolygonCollection();

    std::vector<RimPolygonInView*> visiblePolygonsInView() const;
    std::vector<RimPolygonInView*> allPolygonsInView() const;

    bool setPolygonVisible( RimPolygon* polygon, bool visible );

protected:
    std::vector<RimPolygonContainer*> sourceSubCollections() const override;
    std::vector<RimPolygon*>          sourceItems() const override;
    RimPolygonInView*                 createItemInView( RimPolygon* source ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    RimPolygonInView* findPolygonInView( const RimPolygon* polygon ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    // Explicit per-view choice: whether this view should follow the source container's own
    // per-view realization resolution (see RimPolygonContainer::resolveViewMatchingRealization) or
    // always show the source's own Applied/base realization. Only relevant (and only shown in the
    // UI) when sourceCollection()->supportsRealizationOverride() is true, i.e. for a cloud-backed
    // address. There is deliberately no separate per-view realization override dropdown here --
    // either this view shows the address exactly as configured (its own Applied realization,
    // edited on RimPolygonCloudAddress itself), or it opts in to following the view's own case
    // realization when that is meaningful (same Sumo case/ensemble as the address's Applied data
    // source). Disabled (read-only, with an explanatory tooltip) when the view's own case belongs
    // to a different case/ensemble than the source's Applied data source, since "follow view"
    // would not be meaningful there.
    caf::PdmField<bool> m_useAutoRealization;

    // Resolves the realization the owning 3D view's own case matches for the current source
    // container, or -1 if there is no view ancestor or the view's case belongs to a different
    // case/ensemble than the source's Applied data source. See
    // RimPolygonContainer::resolveViewMatchingRealization.
    int viewMatchingRealizationOrMinusOne() const;
};
