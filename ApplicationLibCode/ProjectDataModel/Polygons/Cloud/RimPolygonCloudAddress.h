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

class RimPolygon;
class RimCloudPolygon;
class RimPolygonCloudSource;
class RimPolygonCloudRealizationGroup;
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
/// RimPolygonInViewCollection::onSynced()), and evicted again once no view still shows it checked.
///
/// A second, optional kind of data can exist alongside the base data: one
/// RimPolygonCloudRealizationGroup child per additional realization some view is currently
/// "Auto-Follow"-ing (see RimPolygonCloudSource/RimPolygonInViewCollection) -- this is what lets a
/// user compare a polygon against its own base realization, with zero bespoke comparison UI.
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
    // RimPolygonInViewCollection::onSynced()); idempotent (no-op if already fetched).
    bool hasBaseData() const;
    void ensureBaseFetched();
    void evictBaseData();

    // Per-realization comparison data (see RimPolygonCloudRealizationGroup). Created/evicted on
    // demand by RimPolygonInViewCollection when a view's Auto-Follow resolves a realization other
    // than the base one.
    bool                             hasRealizationGroup( int realization ) const;
    RimPolygonCloudRealizationGroup* ensureRealizationGroupFetched( int realization );
    void                             evictRealizationGroup( int realization );
    std::vector<int>                 fetchedRealizationGroupRealizations() const;
    std::vector<RimPolygonCloudRealizationGroup*> realizationGroups() const;

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
};
