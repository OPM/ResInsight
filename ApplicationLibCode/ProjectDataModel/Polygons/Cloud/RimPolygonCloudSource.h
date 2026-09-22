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

#include "Polygons/RimPolygonContainer.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include <QPointer>

class RimSumoDataSource;
class RimPolygonCloudAddress;
class RiaSumoConnector;
class Rim3dView;

//==================================================================================================
///
/// Root of a browsable tree of Sumo polygon results for one (case, ensemble, base realization).
/// Created empty via "Add Cloud Polygon Source" -- the user then picks a Data Source and Base
/// Realization in the property panel and clicks "Apply". The very first Apply does a single, cheap
/// polygon-result-directory metadata fetch and builds the *entire* folder/leaf structure (Field
/// Outline / Structure Depth Fault Lines + names / Fluid Contact Outline + names + contact types)
/// as RimPolygonCloudFolder/RimPolygonCloudAddress children -- no coordinate data is fetched at
/// this point. Each RimPolygonCloudAddress leaf only fetches its own RimCloudPolygon coordinate
/// data lazily, the first time a 3D view's visibility checkbox for that leaf is checked on (see
/// RimPolygonInViewCollection).
///
/// Data Source and Base Realization stay editable afterward via the same "Apply"/"Cancel" pattern
/// RimPolygonCloudAddress used to have: m_dataSource/m_baseRealization are the pending (UI-edited)
/// selection; m_appliedDataSource/m_appliedBaseRealization are the actually-committed identity
/// (used by dataSource()/baseRealization(), buildDirectoryTree(), the tree name, and everything
/// else that needs a fixed, stable identity for this source). Clicking Apply again after the
/// directory has already been built:
/// - if the applied *data source* changed: the whole folder/leaf tree (and any fetched/cached
///   polygon data underneath it) was built for the old ensemble and is no longer valid -- it is
///   torn down and rebuilt from scratch for the new one.
/// - if only the applied *base realization* changed: the folder/leaf structure itself (names/
///   categories) does not depend on realization, so it is left alone; only every address's own
///   already-fetched base-realization data is evicted, so it gets lazily re-fetched for the new
///   base realization value the next time it is needed.
///
/// This is also the only polygon container type offering a per-view "Auto-Follow View Realization"
/// choice (supportsRealizationOverride()) -- it governs every leaf beneath it in a given view, so
/// the choice is only ever shown/asked once per source, not once per leaf.
///
//==================================================================================================
class RimPolygonCloudSource : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCloudSource();

    // Emitted whenever Apply commits a change that may affect what is shown in any open 3D view's
    // RimPolygonInViewCollection mirror (a new tree built, an existing tree rebuilt for a new data
    // source, or a base realization change evicting stale data). RimPolygonCollection connects
    // this to trigger view resync + redraw, mirroring the same signal on RimPolygonCloudAddress/
    // RimPolygonFile/RimPolygon.
    caf::Signal<> objectChanged;

    void setDataSource( RimSumoDataSource* dataSource );
    void setBaseRealization( int realization );

    RimSumoDataSource* dataSource() const;
    int                baseRealization() const;

    bool isDirectoryBuilt() const;

    // Fetches the polygon result directory once (metadata only -- no coordinate arrays) and builds
    // the full nested RimPolygonCloudFolder/RimPolygonCloudAddress tree beneath this object. No-op
    // if already built (see m_directoryBuilt), or if no data source is selected yet.
    void buildDirectoryTree();

    QString name() const;

    // Composes this source's display name for an arbitrary realization -- e.g. "iter-0 (case) /
    // Real <realization>". Used by RimPolygonInViewCollection to show the realization a view is
    // currently Auto-Following in its own mirror node's name, when that differs from this
    // source's own (Applied) base realization -- name()/updateName() always use the base one.
    QString nameForRealization( int realization ) const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

    bool             supportsRealizationOverride() const override;
    std::vector<int> availableRealizationIdsForOverride() const override;
    int              resolveViewMatchingRealization( const Rim3dView* view ) const override;

    // Evicts any fetched RimCloudPolygon data (base or per-realization cache) beneath this source
    // that is not currently shown checked (with that realization resolved as the effective one) in
    // any open view. Called after the Auto-Follow checkbox is toggled in a view, or after Apply
    // changes the base realization; safe to call at any time.
    void evictUnusedRealizationData();

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    RiaSumoConnector*                    sumoConnector();
    void                                 updateName();
    QString                              composeName( int realization ) const;
    std::vector<RimPolygonCloudAddress*> allAddresses() const;
    bool                                 hasPendingChanges() const;
    void                                 onApplyClicked();
    void                                 onCancelClicked();

private:
    caf::PdmPtrField<RimSumoDataSource*> m_dataSource;
    caf::PdmField<int>                   m_baseRealization;

    caf::PdmPtrField<RimSumoDataSource*> m_appliedDataSource;
    caf::PdmField<int>                   m_appliedBaseRealization;

    caf::PdmField<bool> m_directoryBuilt;

    QPointer<RiaSumoConnector> m_sumoConnector;
};
