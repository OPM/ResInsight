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

#include "Cloud/RiaSumoPolygons.h"

#include "cafPdmField.h"
#include "cafSignal.h"

#include <QPointer>

#include <map>
#include <vector>

class RimPolygon;
class RimCloudPolygon;
class RimPolygonCloudSource;
class RiaSumoConnector;

//==================================================================================================
///
/// A leaf polygon container representing exactly one unique polygon table in Sumo: one polygon
/// result category (field outline / structure depth fault lines / fluid contact outline), plus a
/// name and, for fluid contacts, a contact type, as applicable. This identity is fixed once, when
/// RimPolygonCloudSource::buildDirectoryTree() creates it -- it is never edited afterward via the
/// UI (no dropdowns, no Apply/Cancel): the property panel is purely informational.
///
/// Coordinate data (RimCloudPolygon children, see items()) is not persisted in the project file
/// and is not fetched just because this object exists in the tree. It is fetched lazily, the first
/// time any 3D view's visibility checkbox for this leaf is checked on (see
/// RimPolygonInViewCollection::prepareForSync()), and evicted again once no view still shows it
/// checked.
///
/// A second, optional kind of data can exist alongside the base data: an internal, unparented
/// cache entry per additional realization some view is currently "Auto-Follow"-ing (see
/// RimPolygonCloudSource/RimPolygonInViewCollection). Unlike the base data, these are never added
/// as PDM children -- the main project tree (and RimPolygonCloudSource's own browsable structure)
/// therefore always shows only the base realization's data, exactly as it was created; a view
/// showing a different (auto-followed) realization substitutes that cached data directly in place
/// of the base items for its own mirror, with no separate visible node.
///
//==================================================================================================
class RimPolygonCloudAddress : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimPolygonCloudAddress();
    ~RimPolygonCloudAddress() override;

    // Sets the fixed identity of this leaf. Called exactly once, right after construction, by
    // RimPolygonCloudSource::buildDirectoryTree() -- never edited afterward via the UI.
    void configureIdentity( SumoPolygonResult polygonResult, const QString& sumoName, const QString& contactType );

    SumoPolygonResult polygonResult() const;
    QString            sumoName() const;
    QString            contactType() const;

    RimPolygonCloudSource* owningSource() const;

    // Base-realization data: this address's own items(), always fetched for the owning source's
    // base realization. Lazily fetched the first time any view checks this leaf visible (see
    // RimPolygonInViewCollection::prepareForSync()); idempotent (no-op if already fetched).
    bool hasBaseData() const;
    void ensureBaseFetched();
    void evictBaseData();

    // Per-realization data for any realization other than the owning source's own base
    // realization -- used only by a view whose "Auto-Follow View Realization" checkbox resolves
    // to a different realization than the source's base one (see RimPolygonInViewCollection).
    // Unlike the base data above, these RimCloudPolygon objects are plain, unparented cache
    // entries -- never added as PDM children -- so they never show up in the main project tree;
    // they exist purely to be mirrored into whichever view(s) are currently showing that
    // realization.
    bool                     hasDataForRealization( int realization ) const;
    void                     ensureRealizationFetched( int realization );
    std::vector<RimPolygon*> cachedItemsForRealization( int realization ) const;
    void                     evictRealizationIfUnused( int realization );
    std::vector<int>         cachedRealizations() const;

    QString name() const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    RiaSumoConnector* sumoConnector();
    void              updateName();

    // Clicked handler for the tree's download tag (see defineObjectEditorAttribute()) -- fetches
    // this leaf's own base realization directly from the project tree, independent of any 3D
    // view's visibility checkbox (which keeps triggering ensureBaseFetched() exactly as before).
    void onDownloadTagClicked( const caf::SignalEmitter* emitter, size_t index );

    static QString polygonResultLabel( SumoPolygonResult polygonResult );

    // Fetches from Sumo for the given realization -- this address's own fixed polygon result/
    // name/contact type, combined with the owning source's data source -- stamping each returned
    // RimCloudPolygon with its full Sumo identity. Used both for the base realization
    // (ensureBaseFetched()) and for any comparison realization (ensureRealizationGroupFetched()).
    std::vector<RimPolygon*> fetchPolygonsFromSumo( int realization );

private:
    caf::PdmField<QString> m_polygonResult;
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_contactType;

    QPointer<RiaSumoConnector> m_sumoConnector;

    // Unparented cache of RimCloudPolygon objects for realizations other than the owning source's
    // base one -- see the class comment above. Owned/deleted here, never added as PDM children.
    std::map<int, std::vector<RimPolygon*>> m_realizationCache;
};
