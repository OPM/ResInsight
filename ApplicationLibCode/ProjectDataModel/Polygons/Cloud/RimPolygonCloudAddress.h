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
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include <QPointer>

#include <limits>

class RimPolygon;
class RimSumoDataSource;
class RiaSumoConnector;

//==================================================================================================
///
/// A leaf polygon container, like RimPolygonFile, backed by a polygon result fetched from Sumo
/// through ri-cloud-api instead of a file on disk. Points at an existing RimSumoDataSource (an
/// already-configured case/ensemble under the cloud data sources) and a realization, and lets the
/// user pick one polygon result (field outline / structure depth fault lines / fluid contact
/// outline, with name and, for fluid contacts, contact type) from that ensemble's polygon result
/// directory.
///
/// Coordinate data is not persisted in the project file: loadData() re-fetches from Sumo, both on
/// an explicit "Reload" and once automatically when the project is (re)loaded (see
/// RiaApplication::loadProject calling RimPolygonCollection::loadData() recursively). RimPolygon
/// children are read-only and marked non-deletable, matching how RimPolygonFile treats the
/// polygons it imports from a file.
///
//==================================================================================================
class RimPolygonCloudAddress : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimPolygonCloudAddress();

    void setDataSource( RimSumoDataSource* dataSource );
    void setRealization( int realization );
    void setPolygonResult( SumoPolygonResult polygonResult );
    void setPolygonName( const QString& name );
    void setContactType( const QString& contactType );

    void loadData() override;

    // Called before a view reads this container's items(), see
    // RimPolygonContainer::prepareItemsForRealization. Switches to and (re)loads the given
    // realization when it differs from the one currently loaded; a negative realization (no view
    // context) leaves whatever is currently loaded untouched.
    //
    // NOTE: the fetched RimPolygon children are shared by every view showing this address, so two
    // views following different realizations of the same address will keep re-triggering each
    // other's reload. Acceptable for now since one address is expected to be used from one
    // realization context at a time; revisit (e.g. per-realization caching) if that stops holding.
    void prepareItemsForRealization( int realization ) override;

    std::vector<RimPolygon*> polygons() const;

    QString name() const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    RiaSumoConnector* sumoConnector();
    void              ensureDirectoryFetched();
    void              invalidateDirectory();
    void              updateName();

    // True once the current data source/polygon result/name/contact-type combination is complete
    // enough to fetch (e.g. a name is required for every result category except field outline, and
    // a contact type is required in addition for fluid contact outline). Used to avoid fetching
    // (and warning about "no polygons found") while the user is still mid-way through the cascading
    // dropdown selection.
    bool hasCompleteSelection() const;

    static QString polygonResultLabel( SumoPolygonResult polygonResult );

    std::vector<RimPolygon*> fetchPolygonsFromSumo();

private:
    caf::PdmPtrField<RimSumoDataSource*> m_dataSource;
    caf::PdmField<int>                   m_realization;
    caf::PdmField<QString>               m_polygonResult;
    caf::PdmField<QString>               m_name;
    caf::PdmField<QString>               m_contactType;

    // Runtime only, not persisted: the result directory of the current data source's case/ensemble,
    // fetched on demand when the property editor asks for name/contact-type options, and the
    // realization this container's current RimPolygon children were fetched for (used by
    // prepareItemsForRealization to avoid refetching when nothing changed).
    SumoPolygonDirectory m_cachedDirectory;
    bool                 m_hasCachedDirectory = false;
    int                  m_loadedRealization   = std::numeric_limits<int>::min();

    QPointer<RiaSumoConnector> m_sumoConnector;
};
