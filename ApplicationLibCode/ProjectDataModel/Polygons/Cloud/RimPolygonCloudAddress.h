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

#include <map>
#include <vector>

class RimPolygon;
class RimCloudPolygon;
class RimPolygonCloudSource;
class RiaSumoConnector;

//==================================================================================================
///
/// Leaf polygon container for one unique polygon table in Sumo (polygon result category, name,
/// and for fluid contacts a contact type). Identity is fixed once by
/// RimPolygonCloudSource::buildDirectoryTree() and never edited afterward.
///
/// Coordinate data (RimCloudPolygon children) is not persisted and is fetched lazily, the first
/// time a view's visibility checkbox for this leaf is checked (see
/// RimPolygonInViewCollection::prepareForSync()), and evicted once no view shows it checked.
///
/// An additional, unparented cache holds data for other realizations a view is "Auto-Follow"-ing.
/// These are never PDM children, so the tree always shows only the base realization.
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

    // Fixed identity, set once by RimPolygonCloudSource::buildDirectoryTree().
    void configureIdentity( SumoPolygonResult polygonResult, const QString& sumoName, const QString& contactType );

    SumoPolygonResult polygonResult() const;
    QString           sumoName() const;
    QString           contactType() const;

    RimPolygonCloudSource* owningSource() const;

    // Base-realization data: items() for the owning source's base realization. Lazily fetched,
    // idempotent.
    bool hasBaseData() const;
    void ensureBaseFetched();
    void evictBaseData();

    // Cache for other realizations (Auto-Follow). Never added as PDM children.
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

    // Tree download-tag click handler: fetches base data directly, independent of view checkboxes.
    void onDownloadTagClicked( const caf::SignalEmitter* emitter, size_t index );

    static QString polygonResultLabel( SumoPolygonResult polygonResult );

    // Fetches from Sumo for the given realization, stamping each RimCloudPolygon with its Sumo
    // identity. Used for both base and comparison realizations.
    std::vector<RimPolygon*> fetchPolygonsFromSumo( int realization );

private:
    caf::PdmField<QString> m_polygonResult;
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_contactType;

    // Cache of RimCloudPolygon objects for non-base realizations. Owned/deleted here, never PDM
    // children.
    std::map<int, std::vector<RimPolygon*>> m_realizationCache;
};
