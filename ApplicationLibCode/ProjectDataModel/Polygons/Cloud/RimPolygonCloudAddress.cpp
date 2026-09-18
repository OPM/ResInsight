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

#include "RimPolygonCloudAddress.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RimCloudDataSourceCollection.h"
#include "Cloud/RimSumoDataSource.h"

#include "Polygons/Cloud/RimCloudPolygon.h"
#include "Polygons/RimPolygon.h"

#include "Rim3dView.h"
#include "RimRoffCaseSumo.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiButtonBox.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeAttributes.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPolygonCloudAddress, "RimPolygonCloudAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudAddress::RimPolygonCloudAddress()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "Sumo Polygon Address", ":/CloudBlobs.svg" );

    // Inherited field, kept declared here so the derived class's ui/xml keyword for it stays
    // independent of RimPolygonFile's, matching the convention used there.
    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    CAF_PDM_InitField( &m_realization, "Realization", 0, "Realization" );
    m_realization.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_polygonResult, "PolygonResult", QString( "field_outline" ), "Polygon Result" );
    m_polygonResult.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_name, "PolygonName", QString(), "Name" );
    m_name.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_contactType, "ContactType", QString(), "Fluid Contact Type" );
    m_contactType.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    // Applied selection: mirrors the pending fields above, but only ever written to via
    // applyPendingSelection() (Apply button / setters), never directly from the UI. Hidden, since
    // these exist purely to decouple "what is fetched" from "what is being edited".
    CAF_PDM_InitFieldNoDefault( &m_appliedDataSource, "AppliedDataSource", "Applied Data Source" );
    m_appliedDataSource.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_appliedRealization, "AppliedRealization", 0, "Applied Realization" );
    m_appliedRealization.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_appliedPolygonResult, "AppliedPolygonResult", QString( "field_outline" ), "Applied Polygon Result" );
    m_appliedPolygonResult.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_appliedName, "AppliedPolygonName", QString(), "Applied Name" );
    m_appliedName.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_appliedContactType, "AppliedContactType", QString(), "Applied Fluid Contact Type" );
    m_appliedContactType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_hasAppliedSelection, "HasAppliedSelection", false, "Has Applied Selection" );
    m_hasAppliedSelection.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCloudAddress::~RimPolygonCloudAddress()
{
    clearRealizationCache();
}

//--------------------------------------------------------------------------------------------------
/// Setters used by RicCreateSumoPolygonAddressFeature to pre-select a convenient default. They set
/// both the pending (edited) and applied (fetched-from) fields directly, since these are explicit,
/// deliberate calls, not stray UI edits -- unlike a user typing/picking in the property panel, this
/// is a one-shot setup and does not need a subsequent Apply click to take effect. loadData() is
/// still not called here; the caller decides when (if at all) to fetch.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setDataSource( RimSumoDataSource* dataSource )
{
    m_dataSource        = dataSource;
    m_appliedDataSource = dataSource;
    invalidateDirectory();
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setRealization( int realization )
{
    m_realization        = realization;
    m_appliedRealization = realization;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setPolygonResult( SumoPolygonResult polygonResult )
{
    m_polygonResult        = RiaSumoPolygons::polygonResultKey( polygonResult );
    m_appliedPolygonResult = m_polygonResult();
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setPolygonName( const QString& name )
{
    m_name        = name;
    m_appliedName = name;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::setContactType( const QString& contactType )
{
    m_contactType        = contactType;
    m_appliedContactType = contactType;
    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::loadData()
{
    if ( !m_hasAppliedSelection() )
    {
        // Nothing has ever been applied for this address yet (e.g. it was just created via the
        // context menu, with only a convenience default data source/realization pre-selected): do
        // not fetch until the user explicitly clicks "Apply", even though the default selection
        // (field outline) may already be "complete" per hasCompleteSelection().
        if ( !m_items.empty() )
        {
            m_items.deleteChildren();
        }
        return;
    }

    if ( !hasCompleteSelection() )
    {
        // Selection is not complete yet (e.g. polygon result just changed and name/contact type
        // have not been chosen): clear any stale polygons from a previous selection, but do not
        // fetch or warn -- this is a normal, expected mid-selection state, not an error.
        if ( !m_items.empty() )
        {
            m_items.deleteChildren();
        }
        m_loadedRealization = m_appliedRealization();
        return;
    }

    auto fetchedPolygons = fetchPolygonsFromSumo( m_appliedRealization() );

    auto existingPolygons = items();
    if ( !fetchedPolygons.empty() && existingPolygons.size() == fetchedPolygons.size() )
    {
        // Same number of polygons as before (the common case: same realization/result, refetched
        // data): update the existing objects in place instead of replacing them, so a view is not
        // forced to throw away and recreate its mirrored RimPolygonInView objects (and any
        // selection/visibility state on them) on every reload.
        for ( size_t i = 0; i < existingPolygons.size(); i++ )
        {
            auto* existingPolygon = existingPolygons[i];
            auto* fetchedPolygon  = fetchedPolygons[i];

            existingPolygon->setDeletable( false );
            existingPolygon->setName( fetchedPolygon->name() );
            existingPolygon->setPointsInDomainCoords( fetchedPolygon->pointsInDomainCoords() );
            existingPolygon->coordinatesChanged.send();
            existingPolygon->objectChanged.send();

            // Keep the Sumo identity stamped on the (persisted) RimCloudPolygon in sync, so a
            // stale realization/spec is never left behind after an in-place update.
            if ( auto* existingCloudPolygon = dynamic_cast<RimCloudPolygon*>( existingPolygon ) )
            {
                if ( auto* fetchedCloudPolygon = dynamic_cast<RimCloudPolygon*>( fetchedPolygon ) )
                {
                    existingCloudPolygon->setSumoIdentity( fetchedCloudPolygon->caseId(),
                                                           fetchedCloudPolygon->ensembleName(),
                                                           fetchedCloudPolygon->realization(),
                                                           fetchedCloudPolygon->polygonResult(),
                                                           fetchedCloudPolygon->sumoName(),
                                                           fetchedCloudPolygon->contactType() );
                }
            }

            delete fetchedPolygon;
        }
    }
    else
    {
        m_items.deleteChildren();
        m_items.setValue( fetchedPolygons );

        for ( auto* polygon : fetchedPolygons )
        {
            ensureUniquePolygonName( polygon );
        }
    }

    if ( fetchedPolygons.empty() )
    {
        RiaLogging::warning( "No polygons found for Sumo polygon address: " + name().toStdString() );
    }
    else
    {
        RiaLogging::info( std::format( "Fetched {} polygon(s) from Sumo for address: {}", fetchedPolygons.size(), name() ) );
    }

    m_loadedRealization = m_appliedRealization();
}

//--------------------------------------------------------------------------------------------------
/// Explicit fetch trigger for the "Apply" button in the property panel (see defineUiOrdering).
/// Commits the pending selection into the applied fields, then fetches for that applied selection.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::onApplyClicked()
{
    applyPendingSelection();

    updateName();
    loadData();

    // updateAllRequiredEditors() (not just updateConnectedEditors()) so the project tree picks up
    // the RimPolygon children just (re)created by loadData() -- this node already exists and is
    // rendered in the tree from creation time, so its new children need an explicit structural
    // refresh; updateConnectedEditors() alone only refreshes the property panel's own fields.
    updateAllRequiredEditors();
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Explicit discard trigger for the "Cancel" button in the property panel (see defineUiOrdering):
/// discards the pending edits and restores the pending fields to the currently Applied selection,
/// without touching m_items/loadData() -- there is nothing to (re)fetch, since the Applied
/// selection/its fetched polygons are unaffected by pending edits in the first place.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::onCancelClicked()
{
    resetPendingSelectionFromApplied();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Commits the pending selection into the applied fields. Also clears the per-realization cache
/// (see itemsForRealization()): cache entries are keyed by realization only, so if the spec
/// (data source/result/name/contact type) changes, stale entries would otherwise silently keep
/// reflecting the previous spec.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::applyPendingSelection()
{
    m_appliedDataSource    = m_dataSource();
    m_appliedRealization   = m_realization();
    m_appliedPolygonResult = m_polygonResult();
    m_appliedName          = m_name();
    m_appliedContactType   = m_contactType();
    m_hasAppliedSelection  = true;

    clearRealizationCache();
}

//--------------------------------------------------------------------------------------------------
/// Restores the pending (editable) fields from the applied selection, so the property panel shows
/// what is actually loaded rather than stale in-progress edits, e.g. right after a project is
/// opened and the pending fields still have their just-constructed defaults.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::resetPendingSelectionFromApplied()
{
    // Only invalidate the cached directory if the pending data source actually differs from the
    // one being restored -- e.g. right after Cancel, if the user never touched the data source
    // field, the cached directory (fetched for the applied data source) is still valid and does
    // not need to be re-fetched from Sumo just because the property panel is being refreshed.
    const bool dataSourceChanges = m_dataSource() != m_appliedDataSource();

    m_dataSource    = m_appliedDataSource();
    m_realization   = m_appliedRealization();
    m_polygonResult = m_appliedPolygonResult();
    m_name          = m_appliedName();
    m_contactType   = m_appliedContactType();

    if ( dataSourceChanges )
    {
        invalidateDirectory();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::initAfterRead()
{
    resetPendingSelectionFromApplied();
    updateName();
}

//--------------------------------------------------------------------------------------------------
/// Deletes every cached RimCloudPolygon and clears the cache map. See m_polygonsByRealization.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::clearRealizationCache()
{
    for ( auto& [realization, polygons] : m_polygonsByRealization )
    {
        for ( auto* polygon : polygons )
        {
            delete polygon;
        }
    }
    m_polygonsByRealization.clear();
}

//--------------------------------------------------------------------------------------------------
/// See RimPolygonContainer::itemsForRealization.
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::itemsForRealization( int realization ) const
{
    if ( !m_hasAppliedSelection() ) return {};

    if ( realization < 0 || realization == m_appliedRealization() )
    {
        // No view context, or the view's realization already matches the Applied one: return the
        // Applied/tree-displayed items directly, no extra fetch/cache entry needed.
        return items();
    }

    auto it = m_polygonsByRealization.find( realization );
    if ( it != m_polygonsByRealization.end() )
    {
        return std::vector<RimPolygon*>( it->second.begin(), it->second.end() );
    }

    // Not cached yet: fetch for this realization (reusing the Applied data source/result/name/
    // contact type, only substituting realization) and cache the result. These RimCloudPolygon
    // objects are plain heap objects, not added as children of m_items / not part of the PDM child
    // hierarchy -- they exist purely to be pointed at by RimPolygonInView mirrors in whichever view
    // resolved this realization, and are owned/deleted by m_polygonsByRealization (see
    // clearRealizationCache() and the destructor).
    auto* mutableThis      = const_cast<RimPolygonCloudAddress*>( this );
    auto  fetchedPolygons  = mutableThis->fetchPolygonsFromSumo( realization );
    auto& cachedForCasting = mutableThis->m_polygonsByRealization[realization];
    for ( auto* polygon : fetchedPolygons )
    {
        if ( auto* cloudPolygon = dynamic_cast<RimCloudPolygon*>( polygon ) )
        {
            cachedForCasting.push_back( cloudPolygon );
        }
    }

    return std::vector<RimPolygon*>( cachedForCasting.begin(), cachedForCasting.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::supportsRealizationOverride() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Realizations available to pick from for a per-view override: the Applied data source's
// selected realizations, i.e. the same set the "Realization" dropdown in the property panel
// offers -- not tied in any way to any 3D view's own case.
//--------------------------------------------------------------------------------------------------
std::vector<int> RimPolygonCloudAddress::availableRealizationIdsForOverride() const
{
    std::vector<int> realizations;

    if ( auto* dataSource = m_appliedDataSource() )
    {
        for ( const auto& realizationId : dataSource->selectedRealizationIds() )
        {
            bool ok    = false;
            int  value = realizationId.toInt( &ok );
            if ( ok ) realizations.push_back( value );
        }
    }

    return realizations;
}

//--------------------------------------------------------------------------------------------------
/// Only follows a view's own case realization when that case is a RimRoffCaseSumo created from
/// this address's own Applied data source -- i.e. the same Sumo case/ensemble. A view whose case
/// belongs to a different field/data source (e.g. a Johan Sverdrup grid case in an otherwise
/// Drogon-configured project) must never have its realization silently applied to this address.
//--------------------------------------------------------------------------------------------------
int RimPolygonCloudAddress::resolveViewMatchingRealization( const Rim3dView* view ) const
{
    if ( !view || !m_appliedDataSource() ) return -1;

    if ( auto* sumoCase = dynamic_cast<const RimRoffCaseSumo*>( view->ownerCase() ) )
    {
        if ( sumoCase->dataSource() == m_appliedDataSource() )
        {
            return sumoCase->realization();
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::polygons() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::name() const
{
    QString nameCandidate = m_collectionName.value();
    if ( !nameCandidate.isEmpty() ) return nameCandidate;

    return "Sumo Polygon Address";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonCloudAddress::addNewSubCollection()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
    uiOrdering.add( &m_dataSource );
    uiOrdering.add( &m_realization );
    uiOrdering.add( &m_polygonResult );

    const bool isFieldOutline = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) );
    const bool isFluidContact = ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) );

    m_name.uiCapability()->setUiHidden( isFieldOutline );
    uiOrdering.add( &m_name );

    m_contactType.uiCapability()->setUiHidden( !isFluidContact );
    uiOrdering.add( &m_contactType );

    // A real QDialogButtonBox -- the same mechanism a QDialog's OK/Cancel/Help row uses (see
    // RicEditPreferencesFeature.cpp) -- lays out its buttons via its own internal QHBoxLayout
    // (leading stretch, then packed buttons), independent of this form's QGridLayout column
    // accounting. caf::PdmUiButtonBox (a thin PdmUiItem wrapper) hides its own label slot, so it
    // occupies the same single "field" grid column any other item on its own row would (no
    // inflation of the form's shared column count), while filling that column's full width -- and
    // then the buttons pack themselves snugly at the right edge, exactly matching the Preferences
    // dialog's look, with no per-button alignment/column-span tuning needed at all.
    caf::PdmUiButtonBox* buttonBox = uiOrdering.addNewButtonBox();

    auto& applySpec = buttonBox->addButton( "Apply", [this]() { onApplyClicked(); } );
    if ( !hasPendingChanges() )
    {
        applySpec.enabled = false;
        applySpec.toolTip = "The selection shown is already applied.";
    }

    auto& cancelSpec = buttonBox->addButton( "Cancel", [this]() { onCancelClicked(); } );
    if ( !m_hasAppliedSelection() )
    {
        cancelSpec.enabled = false;
        cancelSpec.toolTip = "Nothing has been applied yet, so there is no selection to revert to.";
    }
    else if ( !hasPendingChanges() )
    {
        cancelSpec.enabled = false;
        cancelSpec.toolTip = "The selection shown is already applied.";
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // No auto-fetch (or even name/identity update) on every field change: this cascading
    // selection (data source, realization, polygon result, name, contact type) is purely "pending"
    // until the user clicks "Apply" (see defineUiOrdering / onApplyClicked). Dependent dropdown
    // fields are reset and re-defaulted here, so cascading combo boxes stay valid -- and always
    // have some option pre-selected -- while the user edits.
    if ( changedField == &m_dataSource )
    {
        invalidateDirectory();
        ensureDirectoryFetched();

        if ( !isPolygonResultAvailable( m_polygonResult() ) )
        {
            m_polygonResult = firstAvailablePolygonResultKey();
        }

        m_name        = "";
        m_contactType = "";
        selectDefaultPendingValues();
    }
    else if ( changedField == &m_polygonResult )
    {
        m_name        = "";
        m_contactType = "";
        selectDefaultPendingValues();
    }
    else if ( changedField == &m_name )
    {
        m_contactType = "";
        selectDefaultPendingValues();
    }

    RimPolygonContainer::fieldChangedByUi( changedField, oldValue, newValue );

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Picks a first-available option for m_name/m_contactType whenever they are empty but a value is
/// required for the current m_polygonResult -- so the pending selection always has something valid
/// pre-filled (ready to Apply) instead of presenting an empty-looking dropdown the user must fill
/// in themselves.
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::selectDefaultPendingValues()
{
    ensureDirectoryFetched();

    const std::vector<SumoPolygonMeta>* metaList = nullptr;
    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        metaList = &m_cachedDirectory.structureDepthFaultLines;
    }
    else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        metaList = &m_cachedDirectory.fluidContactOutline;
    }

    if ( metaList && m_name().isEmpty() && !metaList->empty() )
    {
        m_name = metaList->front().name;
    }

    if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) && m_contactType().isEmpty() )
    {
        for ( const auto& meta : m_cachedDirectory.fluidContactOutline )
        {
            if ( meta.name != m_name() || meta.contactType.isEmpty() ) continue;
            m_contactType = meta.contactType;
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// True if the given polygon result category has at least one entry in the currently cached
/// directory (see ensureDirectoryFetched()) -- i.e. it is actually offered by the current data
/// source's ensemble. Used when the data source changes to detect a stale m_polygonResult
/// selection carried over from a previous (e.g. different field's) ensemble.
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::isPolygonResultAvailable( const QString& polygonResultKey ) const
{
    if ( polygonResultKey == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) )
    {
        return !m_cachedDirectory.fieldOutline.empty();
    }
    if ( polygonResultKey == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        return !m_cachedDirectory.structureDepthFaultLines.empty();
    }
    if ( polygonResultKey == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        return !m_cachedDirectory.fluidContactOutline.empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// First polygon result category (field outline / structure depth fault lines / fluid contact
/// outline, in that order) that has at least one entry in the currently cached directory. Falls
/// back to field outline if the directory is empty/not yet fetched, matching the constructor's
/// default.
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::firstAvailablePolygonResultKey() const
{
    if ( !m_cachedDirectory.fieldOutline.empty() ) return RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline );
    if ( !m_cachedDirectory.structureDepthFaultLines.empty() )
        return RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines );
    if ( !m_cachedDirectory.fluidContactOutline.empty() )
        return RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline );

    return RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonCloudAddress::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_dataSource )
    {
        for ( auto* dataSource : RimCloudDataSourceCollection::instance()->sumoDataSources() )
        {
            // Matches the display name already used for the data source itself under the cloud
            // data sources tree (RimSumoDataSource::updateName(): "ensemble (case)" unless the
            // user set a custom name), so the same ensemble/case reads identically in both places.
            options.push_back( caf::PdmOptionItemInfo( dataSource->name(), dataSource ) );
        }
    }
    else if ( fieldNeedingOptions == &m_realization )
    {
        if ( auto* dataSource = m_dataSource() )
        {
            for ( const auto& realizationId : dataSource->selectedRealizationIds() )
            {
                bool ok    = false;
                int  value = realizationId.toInt( &ok );
                if ( ok ) options.push_back( caf::PdmOptionItemInfo( realizationId, value ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_polygonResult )
    {
        ensureDirectoryFetched();

        if ( !m_cachedDirectory.fieldOutline.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::FieldOutline ),
                                                       RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) ) );
        }
        if ( !m_cachedDirectory.structureDepthFaultLines.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::StructureDepthFaultLines ),
                                                       RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) ) );
        }
        if ( !m_cachedDirectory.fluidContactOutline.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo( polygonResultLabel( SumoPolygonResult::FluidContactOutline ),
                                                       RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_name )
    {
        ensureDirectoryFetched();

        const std::vector<SumoPolygonMeta>* metaList = nullptr;
        if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
        {
            metaList = &m_cachedDirectory.structureDepthFaultLines;
        }
        else if ( m_polygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
        {
            metaList = &m_cachedDirectory.fluidContactOutline;
        }

        if ( metaList )
        {
            std::vector<QString> namesAdded;
            for ( const auto& meta : *metaList )
            {
                if ( std::ranges::find( namesAdded, meta.name ) != namesAdded.end() ) continue;
                namesAdded.push_back( meta.name );
                options.push_back( caf::PdmOptionItemInfo( meta.name, meta.name ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_contactType )
    {
        ensureDirectoryFetched();

        for ( const auto& meta : m_cachedDirectory.fluidContactOutline )
        {
            if ( meta.name != m_name() ) continue;
            if ( meta.contactType.isEmpty() ) continue;

            bool alreadyAdded = false;
            for ( const auto& option : options )
            {
                if ( option.value().toString() == meta.contactType )
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if ( !alreadyAdded ) options.push_back( caf::PdmOptionItemInfo( meta.contactType, meta.contactType ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicReloadPolygonCloudAddressFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_items.empty() )
    {
        caf::PdmUiTreeViewItemAttribute::appendTagToTreeViewItemAttribute( attribute, ":/warning.svg" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector* RimPolygonCloudAddress::sumoConnector()
{
    if ( !m_sumoConnector )
    {
        m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
    }

    return m_sumoConnector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::ensureDirectoryFetched()
{
    if ( m_hasCachedDirectory ) return;

    auto* dataSource = m_dataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return;

    m_cachedDirectory    = connector->polygons().polygonResultDirectory( dataSource->caseId(), dataSource->ensembleName() );
    m_hasCachedDirectory = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::invalidateDirectory()
{
    m_cachedDirectory    = SumoPolygonDirectory();
    m_hasCachedDirectory = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCloudAddress::updateName()
{
    setCollectionName( composeName( m_appliedRealization() ) );
}

//--------------------------------------------------------------------------------------------------
/// See RimPolygonContainer::displayNameForRealization. realization < 0 means "this address's own
/// Applied realization" (matching itemsForRealization()'s convention), so it always renders the
/// same as updateName()/name() in that case.
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::displayNameForRealization( int realization ) const
{
    return composeName( realization < 0 ? m_appliedRealization() : realization );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::composeName( int realization ) const
{
    QStringList parts;

    if ( auto* dataSource = m_appliedDataSource() )
    {
        parts << dataSource->name();
    }

    parts << QString( "Real %1" ).arg( realization );

    if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FieldOutline ) )
    {
        parts << polygonResultLabel( SumoPolygonResult::FieldOutline );
    }
    else
    {
        if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
        {
            parts << polygonResultLabel( SumoPolygonResult::StructureDepthFaultLines );
        }
        else if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
        {
            parts << polygonResultLabel( SumoPolygonResult::FluidContactOutline );
        }

        if ( !m_appliedName().isEmpty() ) parts << m_appliedName();
        if ( !m_appliedContactType().isEmpty() ) parts << m_appliedContactType();
    }

    return parts.join( " / " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonCloudAddress::polygonResultLabel( SumoPolygonResult polygonResult )
{
    switch ( polygonResult )
    {
        case SumoPolygonResult::FieldOutline:
            return "Field Outline";
        case SumoPolygonResult::StructureDepthFaultLines:
            return "Structure Depth Fault Lines";
        case SumoPolygonResult::FluidContactOutline:
            return "Fluid Contact Outline";
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasCompleteSelection() const
{
    if ( !m_appliedDataSource() ) return false;

    SumoPolygonResult polygonResult = SumoPolygonResult::FieldOutline;
    if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        polygonResult = SumoPolygonResult::StructureDepthFaultLines;
    }
    else if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        polygonResult = SumoPolygonResult::FluidContactOutline;
    }

    if ( polygonResult != SumoPolygonResult::FieldOutline && m_appliedName().isEmpty() ) return false;
    if ( polygonResult == SumoPolygonResult::FluidContactOutline && m_appliedContactType().isEmpty() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// True when the pending selection differs from the Applied one (including "nothing has been
/// Applied yet"). Used to disable the "Apply" button once its pending selection is already the
/// current Applied selection, so re-clicking it can't trigger a redundant fetch, and the button's
/// disabled state itself signals that the panel shows the actual currently-applied selection.
//--------------------------------------------------------------------------------------------------
bool RimPolygonCloudAddress::hasPendingChanges() const
{
    if ( !m_hasAppliedSelection() ) return true;

    return m_dataSource() != m_appliedDataSource() || m_realization() != m_appliedRealization() ||
           m_polygonResult() != m_appliedPolygonResult() || m_name() != m_appliedName() || m_contactType() != m_appliedContactType();
}

//--------------------------------------------------------------------------------------------------
/// Fetches from Sumo for the given realization (reusing the Applied data source/result/name/
/// contact type, only substituting realization) -- not necessarily m_appliedRealization(), so
/// this can be used both for the Applied realization itself (loadData()) and for any other
/// realization a view resolves (itemsForRealization()).
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCloudAddress::fetchPolygonsFromSumo( int realization )
{
    std::vector<RimPolygon*> polygons;

    auto* dataSource = m_appliedDataSource();
    auto* connector  = sumoConnector();
    if ( !dataSource || !connector ) return polygons;

    SumoPolygonResult polygonResult = SumoPolygonResult::FieldOutline;
    if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::StructureDepthFaultLines ) )
    {
        polygonResult = SumoPolygonResult::StructureDepthFaultLines;
    }
    else if ( m_appliedPolygonResult() == RiaSumoPolygons::polygonResultKey( SumoPolygonResult::FluidContactOutline ) )
    {
        polygonResult = SumoPolygonResult::FluidContactOutline;
    }

    auto polygonDataList = connector->polygons().polygonsData( dataSource->caseId(),
                                                               dataSource->ensembleName(),
                                                               realization,
                                                               polygonResult,
                                                               m_appliedName(),
                                                               m_appliedContactType() );

    const bool nameGroupsAreDistinct = polygonDataList.size() > 1;

    for ( const auto& data : polygonDataList )
    {
        auto* polygon = new RimCloudPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );
        polygon->setDeletable( false );
        polygon->setSumoIdentity( dataSource->caseId().get(),
                                  dataSource->ensembleName(),
                                  realization,
                                  m_appliedPolygonResult(),
                                  m_appliedName(),
                                  m_appliedContactType() );

        QString polygonName = data.name.isEmpty() ? name() : data.name;
        if ( nameGroupsAreDistinct ) polygonName = QString( "%1 (%2)" ).arg( polygonName ).arg( data.polyId );
        polygon->setName( polygonName );

        std::vector<cvf::Vec3d> points;
        const size_t            pointCount = std::min( { data.xArr.size(), data.yArr.size(), data.zArr.size() } );
        points.reserve( pointCount );
        for ( size_t i = 0; i < pointCount; i++ )
        {
            // Sumo's zTvdSSArray is a positive-down TVDSS depth. ResInsight's domain z convention is
            // elevation (negative down), matching the sign flip RifPolygonReader applies when parsing
            // depth values from a polygon file -- so invert here for the same reason.
            points.emplace_back( data.xArr[i], data.yArr[i], -data.zArr[i] );
        }
        polygon->setPointsInDomainCoords( points );

        polygons.push_back( polygon );
    }

    return polygons;
}
