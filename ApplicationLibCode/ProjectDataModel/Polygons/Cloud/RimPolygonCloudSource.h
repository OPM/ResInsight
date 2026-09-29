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

class RimSumoDataSource;
class RimPolygonCloudAddress;
class RiaSumoConnector;
class Rim3dView;

//==================================================================================================
///
/// Root of a browsable tree of Sumo polygon results for one (case, ensemble, base realization).
/// Created via "Add Cloud Polygon Source"; the user picks a Data Source and Base Realization and
/// clicks "Apply". The first Apply does one cheap directory metadata fetch and builds the entire
/// folder/leaf structure (Field Outline / Structure Depth Fault Lines + names / Fluid Contact
/// Outline + names + contact types) as RimPolygonCloudFolder/RimPolygonCloudAddress children -- no
/// coordinate data is fetched yet. Each leaf fetches its own coordinate data lazily, the first
/// time a view's visibility checkbox for it is checked on (see RimPolygonInViewCollection).
///
/// Data Source/Base Realization use the same pending/applied Apply-Cancel pattern
/// RimPolygonCloudAddress used to have. Re-applying after the tree is built: if the data source
/// changed, the whole tree is torn down and rebuilt; if only the base realization changed, the
/// tree structure is kept and only already-fetched base data is evicted for lazy re-fetch.
///
/// This is also the only container type offering a per-view "Auto-Follow View Realization" choice
/// (supportsRealizationOverride()), shown once per source and governing every leaf beneath it.
///
//==================================================================================================
class RimPolygonCloudSource : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCloudSource();

    // Emitted whenever Apply changes what should be shown in an open view's mirror. Connected by
    // RimPolygonCollection to trigger view resync + redraw.
    caf::Signal<> objectChanged;

    void setDataSource( RimSumoDataSource* dataSource );
    void setBaseRealization( int realization );

    RimSumoDataSource* dataSource() const;
    int                baseRealization() const;

    bool isDirectoryBuilt() const;

    // Fetches the directory metadata once and builds the folder/leaf tree. No-op if already built
    // or no data source selected.
    void buildDirectoryTree();

    QString name() const;

    // Composes this source's display name for an arbitrary realization, e.g. "iter-0 (case) /
    // Real <n>" -- used by RimPolygonInViewCollection for a view Auto-Following a realization
    // other than this source's own applied base one.
    QString nameForRealization( int realization ) const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

    bool             supportsRealizationOverride() const override;
    std::vector<int> availableRealizationIdsForOverride() const override;
    int              resolveViewMatchingRealization( const Rim3dView* view ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
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

    bool m_directoryBuilt = false;
};
