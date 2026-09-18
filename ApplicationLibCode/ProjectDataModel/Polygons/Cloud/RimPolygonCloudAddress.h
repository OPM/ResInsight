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
#include <map>

class RimPolygon;
class RimCloudPolygon;
class RimSumoDataSource;
class RiaSumoConnector;
class Rim3dView;

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
/// The data source/realization/polygon result/name/contact type fields shown in the property panel
/// are a "pending" selection: editing them only updates what is displayed/edited, it does not
/// change this object's identity (name()) or trigger a fetch. The pending selection is copied into
/// a parallel set of "applied" fields -- which loadData()/name()/hasCompleteSelection() actually
/// act on -- only when the user clicks "Apply" (or via setDataSource()/etc. used by the creation
/// command). This avoids fetching from ri-cloud-api (and momentarily showing an unrelated tree
/// name/warning icon) while the user is still mid-way through a cascading dropdown selection.
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

    void setDataSource( RimSumoDataSource* dataSource );
    void setRealization( int realization );
    void setPolygonResult( SumoPolygonResult polygonResult );
    void setPolygonName( const QString& name );
    void setContactType( const QString& contactType );

    void loadData() override;

    // Returns the items a view following the given realization should show. -1 (or the currently
    // Applied realization) returns items() directly -- the tree/property-panel-visible set, no
    // extra fetch. Any other realization is looked up in (and lazily populated into) a per-
    // realization cache of RimCloudPolygon objects, without ever mutating this container's own
    // Applied selection/items() as a side effect -- so two views picking different realization
    // overrides for the same address never interfere with each other. See RimPolygonContainer::
    // itemsForRealization. Realization is now always an explicit, per-view choice (see
    // RimPolygonInViewCollection::m_realizationOverride) -- never derived automatically from a
    // view's own case, since a view's grid case may belong to an entirely different field/
    // ensemble than this address.
    std::vector<RimPolygon*> itemsForRealization( int realization ) const override;

    // A cloud-backed address is the only container whose content genuinely depends on realization
    // -- so it is the only one that offers a per-view realization override, sourced from the
    // Applied data source's available realizations.
    bool              supportsRealizationOverride() const override;
    std::vector<int>  availableRealizationIdsForOverride() const override;

    // Only follows a view's own case realization automatically when that view's case is a
    // RimRoffCaseSumo created from this address's own Applied data source (same case/ensemble) --
    // otherwise returns -1 so the view falls back to this address's Applied realization. See
    // RimPolygonContainer::resolveViewMatchingRealization.
    int resolveViewMatchingRealization( const Rim3dView* view ) const override;

    // See RimPolygonContainer::displayNameForRealization. Builds the same "data source / Real n /
    // polygon result [/ name [/ contact type]]" name used for this address's own tree label
    // (updateName()), but for an arbitrary realization instead of always the Applied one -- so a
    // view showing a different (e.g. auto-followed) realization can display that in its own
    // mirrored tree node's name.
    QString displayNameForRealization( int realization ) const override;

    // Deletes every cached RimCloudPolygon (see itemsForRealization()/m_polygonsByRealization) and
    // clears the cache. Called whenever the applied spec changes (applyPendingSelection()) or on a
    // manual Reload (RicReloadPolygonCloudAddressFeature), since cache entries are keyed by
    // realization only and would otherwise silently keep reflecting the previous spec.
    void clearRealizationCache();

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
    void initAfterRead() override;

private:
    RiaSumoConnector* sumoConnector();
    void              ensureDirectoryFetched();
    void              invalidateDirectory();
    void              updateName();

    // Builds the "data source / Real n / polygon result [/ name [/ contact type]]" name for the
    // given realization, from the current Applied selection (data source/polygon result/name/
    // contact type never vary per realization, only the Real n part and the fetched geometry do).
    // Used by both updateName() (always with m_appliedRealization()) and
    // displayNameForRealization() (with whatever realization a view resolves).
    QString composeName( int realization ) const;

    // Handler for the "Apply" button (see defineUiOrdering): explicitly fetches with the current
    // selection and notifies the tree/3D view, since field edits no longer auto-fetch.
    void onApplyClicked();

    // Handler for the "Cancel" button (see defineUiOrdering): discards the pending edits by
    // resetting the pending fields back to the currently Applied selection (or the just-created
    // defaults if nothing has been Applied yet, though the button is disabled in that case -- see
    // hasPendingChanges()/m_hasAppliedSelection).
    void onCancelClicked();

    // True when the pending selection (data source/realization/polygon result/name/contact type)
    // differs from the currently Applied one, or nothing has been Applied yet. Used to disable the
    // "Apply" button once the pending selection is already applied, to prevent redundant re-fetches
    // and make clear that the shown selection is the actual currently-applied one.
    bool hasPendingChanges() const;

    // Copies the pending (currently displayed/edited) selection fields into the applied fields
    // that loadData()/name()/hasCompleteSelection() actually act on. Called from onApplyClicked()
    // and once from initAfterRead() to keep the pending fields (not persisted with meaningful
    // defaults otherwise) in sync with the applied selection restored from a saved project.
    void applyPendingSelection();
    void resetPendingSelectionFromApplied();

    // Picks a first-available name/contact-type option whenever one is required (per
    // m_polygonResult) but currently empty, so the pending selection always has a valid,
    // ready-to-apply default rather than an empty-looking dropdown. See fieldChangedByUi.
    void selectDefaultPendingValues();

    // Used from fieldChangedByUi() when the data source changes: detects whether the pending
    // m_polygonResult category carried over from the previous data source (e.g. a different
    // field's ensemble) is actually offered by the new one, and if not, falls back to the first
    // available category -- so switching to an ensemble with different available polygon results
    // never leaves an invalid/empty-selection category silently stuck in the dropdown.
    bool    isPolygonResultAvailable( const QString& polygonResultKey ) const;
    QString firstAvailablePolygonResultKey() const;

    // True once the current data source/polygon result/name/contact-type combination is complete
    // enough to fetch (e.g. a name is required for every result category except field outline, and
    // a contact type is required in addition for fluid contact outline). Used to avoid fetching
    // (and warning about "no polygons found") while the user is still mid-way through the cascading
    // dropdown selection.
    bool hasCompleteSelection() const;
    static QString polygonResultLabel( SumoPolygonResult polygonResult );

    std::vector<RimPolygon*> fetchPolygonsFromSumo( int realization );

private:
    // "Pending" selection: bound to the property panel, freely editable, not acted upon until
    // applyPendingSelection() copies it into the fields below.
    caf::PdmPtrField<RimSumoDataSource*> m_dataSource;
    caf::PdmField<int>                   m_realization;
    caf::PdmField<QString>               m_polygonResult;
    caf::PdmField<QString>               m_name;
    caf::PdmField<QString>               m_contactType;

    // "Applied" selection: the last selection committed via Apply (or a setter). This is what
    // loadData()/name()/hasCompleteSelection() operate on, so the object's identity and fetched
    // polygons only ever reflect a deliberately-applied selection, never a mid-edit one.
    caf::PdmPtrField<RimSumoDataSource*> m_appliedDataSource;
    caf::PdmField<int>                   m_appliedRealization;
    caf::PdmField<QString>               m_appliedPolygonResult;
    caf::PdmField<QString>               m_appliedName;
    caf::PdmField<QString>               m_appliedContactType;

    // True once Apply has been clicked (or the address was restored from a saved project that had
    // already been applied before saving) -- i.e. there is a deliberately-committed selection to
    // fetch. Gates loadData() (including the automatic reload on project open), so a freshly
    // created address with only default/pre-selected fields (see
    // RicCreateSumoPolygonAddressFeature) never fetches on its own; the very first fetch always
    // requires an explicit Apply click.
    caf::PdmField<bool> m_hasAppliedSelection;

    // Runtime only, not persisted: the result directory of the current data source's case/ensemble,
    // fetched on demand when the property editor asks for name/contact-type options, and the
    // realization this container's current RimPolygon children (items()) were fetched for (used by
    // loadData() to decide whether to update existing polygons in place vs. replace them).
    SumoPolygonDirectory m_cachedDirectory;
    bool                 m_hasCachedDirectory = false;
    int                  m_loadedRealization   = std::numeric_limits<int>::min();

    // Per-realization cache of RimCloudPolygon objects for every realization other than the
    // Applied one that some view has asked for (see itemsForRealization()). These are plain heap
    // objects, not registered as PDM children of m_items -- they exist purely to be pointed at by
    // RimPolygonInView mirrors in whichever view resolved that realization, and are owned/deleted
    // by this map (see clearRealizationCache() and the destructor).
    std::map<int, std::vector<RimCloudPolygon*>> m_polygonsByRealization;

    QPointer<RiaSumoConnector> m_sumoConnector;
};
