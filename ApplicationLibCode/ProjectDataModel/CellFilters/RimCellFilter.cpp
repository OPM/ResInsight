/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimCellFilter.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigReservoirGridTools.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimTools.h"
#include "RimViewController.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"

#include "cvfStructGridGeometryGenerator.h"

namespace caf
{
template <>
void caf::AppEnum<RimCellFilter::FilterModeType>::setUp()
{
    addItem( RimCellFilter::INCLUDE, "INCLUDE", "Include" );
    addItem( RimCellFilter::EXCLUDE, "EXCLUDE", "Exclude" );
    setDefault( RimCellFilter::INCLUDE );
}
} // namespace caf

// CAF_PDM_SOURCE_INIT( RimCellFilter, "CellFilter" );
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimCellFilter, "CellFilter", "CellFilter" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter::RimCellFilter( FilterDefinitionType defType )
    : filterChanged( this )
    , m_filterDefinitionType( defType )
{
    CAF_PDM_InitObject( "Cell Filter" );

    // The base class uses "UserDescription" as field keyword - no alias needed for m_name
    setName( "New filter" );

    m_isChecked.registerKeywordAlias( "Active" );

    CAF_PDM_InitFieldNoDefault( &m_srcCase, "Case", "Case" );
    m_srcCase.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filterMode, "FilterType", "Filter Type" );

    CAF_PDM_InitField( &m_gridIndex, "GridIndex", 0, "Grid" );
    m_gridIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_propagateToSubGrids, "PropagateToSubGrids", true, "Apply to Subgrids" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy" );
    m_nameProxy.registerGetMethod( this, &RimCellFilter::fullName );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter::~RimCellFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilter::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
/// Filters listed under an ensemble-level data filter collection are always on, and have no checkbox
/// in the project tree. Per-view on/off is controlled by the RimDataFilterInView wrappers.
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilter::objectToggleField()
{
    if ( parentField() )
    {
        auto* collection = dynamic_cast<RimDataFilterCollection*>( parentField()->ownerObject() );
        if ( collection && collection->isEnsembleLevel() ) return nullptr;
    }

    return RimCheckableNamedObject::objectToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicCombineSelectedFiltersFeature";
}

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the tree selector
//--------------------------------------------------------------------------------------------------
QString RimCellFilter::fullName() const
{
    return name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setActive( bool active )
{
    m_isChecked = active;
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// Is the cell filter doing active filtering, or is it just showing outline, etc. in the view
/// - isChecked == true -> filter enabled in explorer
/// - isActive == true -> filter enabled in explorer and is actually filtering cells, too
/// Default implementation just returns the checked state.
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isActive() const
{
    return isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::triggerFilterChanged() const
{
    filterChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Renames go through the inherited m_name field; emit filterChanged so any RimCombinedFilter
/// hosting this filter can refresh its auto-derived display name.
//--------------------------------------------------------------------------------------------------
void RimCellFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& /*oldValue*/, const QVariant& /*newValue*/ )
{
    if ( changedField == nameField() ) triggerFilterChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isRangeFilter() const
{
    return m_filterDefinitionType == FilterDefinitionType::RANGE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isIndexFilter() const
{
    return m_filterDefinitionType == FilterDefinitionType::INDEX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setCase( RimCase* srcCase )
{
    m_srcCase = srcCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimCellFilter::eclipseCase() const
{
    return dynamic_cast<RimEclipseCase*>( m_srcCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimCellFilter::geoMechCase() const
{
    return dynamic_cast<RimGeoMechCase*>( m_srcCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::updateActiveState( bool isControlled )
{
    m_isChecked.uiCapability()->setUiReadOnly( isControlled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RimCellFilter::FilterModeType> RimCellFilter::filterMode() const
{
    return m_filterMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setFilterMode( FilterModeType filterMode )
{
    m_filterMode = filterMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setGridIndex( int gridIndex )
{
    m_gridIndex = gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCellFilter::gridIndex() const
{
    // The original IJK of a nested hybrid grid is defined on the main grid only
    if ( isRangeFilter() && usesNestedHybridOriginalIjk() ) return 0;

    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::usesNestedHybridOriginalIjk() const
{
    auto eclCase = dynamic_cast<RimEclipseCase*>( sourceCase() );
    if ( !eclCase ) return false;

    auto mainGrid = eclCase->mainGrid();
    return mainGrid && mainGrid->nestedHybridLayout().has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::propagateToSubGrids() const
{
    return m_propagateToSubGrids();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellFilter::iconResourceString() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::updateIconState()
{
    caf::IconProvider iconProvider = uiIconProvider();

    if ( !iconProvider.valid() ) return;

    if ( const QString iconResource = iconResourceString(); !iconResource.isEmpty() )
    {
        iconProvider.setIconResourceString( iconResource );
    }

    if ( filterMode() == INCLUDE )
    {
        iconProvider.setOverlayResourceString( ":/Plus.png" );
    }
    else
    {
        iconProvider.setOverlayResourceString( ":/Minus.png" );
    }

    iconProvider.setActive( m_isChecked && !m_isChecked.uiCapability()->isUiReadOnly() );

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    auto group = uiOrdering.addNewGroup( "General" );
    group->add( &m_filterMode );

    if ( geoMechCase() != nullptr )
    {
        m_gridIndex.uiCapability()->setUiName( "Part" );
    }
    // Nested hybrid grids: range filters always use the original grid IJK, so there is no grid to choose
    if ( !( isRangeFilter() && usesNestedHybridOriginalIjk() ) ) group->add( &m_gridIndex );

    bool readOnlyState = isFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields = fields();
    for ( auto& objField : objFields )
    {
        objField->uiCapability()->setUiReadOnly( readOnlyState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellFilter::modeString() const
{
    if ( m_filterMode == RimCellFilter::FilterModeType::INCLUDE ) return "include";
    return "exclude";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimCellFilter::sourceCase() const
{
    // Case-level data filters have no Rim3dView ancestor; fall back to m_srcCase, which the
    // owning RimDataFilterCollection / RimCombinedFilter propagates on add.
    RimCase* rimCase = nullptr;
    if ( auto* rimView = firstAncestorOrThisOfType<Rim3dView>() )
    {
        rimCase = rimView->ownerCase();
    }
    if ( !rimCase ) rimCase = m_srcCase();

    return rimCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::StructGridInterface* RimCellFilter::selectedGrid() const
{
    RimCase* rimCase = sourceCase();
    if ( !rimCase ) return nullptr;

    int clampedIndex = gridIndex();
    if ( clampedIndex >= RigReservoirGridTools::gridCount( rimCase ) )
    {
        clampedIndex = 0;
    }

    return RigReservoirGridTools::gridByIndex( rimCase, clampedIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCellFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_gridIndex )
    {
        RimTools::eclipseGridOptionItems( &options, eclipseCase() );
        RimTools::geoMechPartOptionItems( &options, geoMechCase() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::updateCellVisibility( cvf::UByteArray*   cellVisibility,
                                          const RigGridBase* grid,
                                          size_t /*timeStepIndex*/,
                                          RimEclipseCase* /*sourceCaseOverride*/ )
{
    if ( cellVisibility == nullptr || grid == nullptr ) return;

    const size_t n = cellVisibility->size();
    if ( n == 0 ) return;

    // Determine which cells are "in this filter's set" — independent of the filter's INCLUDE/EXCLUDE mode.
    cvf::UByteArray inSet( n );
    inSet.setAll( 0 );

    const int gIndx = static_cast<int>( grid->gridIndex() );

    if ( isRangeFilter() )
    {
        // Nested hybrid grid: the filter is defined on the original grid IJK, and every cell in every
        // grid is evaluated using the original IJK it maps to (LGR cells via their parent cells).
        const RigMainGrid* mainGrid               = grid->mainGrid();
        const bool         useNestedHybridOrigIjk = mainGrid && mainGrid->nestedHybridLayout().has_value();

        // updateCompundFilter is a no-op when the range filter targets a different grid than the
        // one being evaluated. Skip the cellVisibility mutation entirely so LGR cells are not
        // erroneously blanked by a main-grid-targeted filter (legacy: parentGridVisibilities).
        const int filterGridIndex = useNestedHybridOrigIjk ? 0 : gIndx;
        if ( gridIndex() != filterGridIndex ) return;

        cvf::CellRangeFilter rf;
        updateCompundFilter( &rf, filterGridIndex );

        const bool isSubGrid = !grid->isMainGrid();

        for ( size_t cellIdx = 0; cellIdx < n; ++cellIdx )
        {
            std::optional<caf::VecIjk0> ijk = useNestedHybridOrigIjk ? mainGrid->nestedHybridOriginalIjk( grid, cellIdx )
                                                                     : grid->ijkFromCellIndex( cellIdx );

            if ( ijk )
            {
                // isCellVisible returns false when there are no include ranges (pure EXCLUDE
                // mode), so also consult isCellExcluded to mark those cells as "in the set".
                if ( rf.isCellVisible( ijk->i(), ijk->j(), ijk->k(), isSubGrid ) ||
                     rf.isCellExcluded( ijk->i(), ijk->j(), ijk->k(), isSubGrid ) )
                {
                    inSet[cellIdx] = 1;
                }
            }
        }
    }
    else if ( isIndexFilter() )
    {
        // updateCellIndexFilter writes into include[] for INCLUDE-mode filters and exclude[] for
        // EXCLUDE-mode filters. A cell is "in the set" iff either array was mutated at that index.
        cvf::UByteArray incArr( n );
        cvf::UByteArray excArr( n );
        incArr.setAll( 0 );
        excArr.setAll( 1 );
        updateCellIndexFilter( &incArr, &excArr, gIndx );
        for ( size_t i = 0; i < n; ++i )
        {
            if ( incArr[i] || !excArr[i] ) inSet[i] = 1;
        }
    }
    else
    {
        // Unknown/property — subclass must override updateCellVisibility. Default to no-op.
        return;
    }

    const bool isInclude = ( filterMode() == INCLUDE );
    for ( size_t i = 0; i < n; ++i )
    {
        const bool in = ( inSet[i] != 0 );
        if ( isInclude )
        {
            if ( !in ) ( *cellVisibility )[i] = 0;
        }
        else
        {
            if ( in ) ( *cellVisibility )[i] = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Evaluate the filter against the given case for all grids, and return the visibility indexed by
/// reservoir cell index. Property filters are evaluated against the given case's own result values.
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimCellFilter::computeReservoirCellVisibility( RimEclipseCase* eclipseCase, size_t timeStepIndex )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() || !eclipseCase->eclipseCaseData()->mainGrid() ) return nullptr;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();

    cvf::ref<cvf::UByteArray> reservoirVisibility = new cvf::UByteArray( caseData->mainGrid()->totalCellCount() );
    reservoirVisibility->setAll( false );

    const bool isInclude = ( filterMode() == INCLUDE );

    // Nested hybrid grid: range filters use the original grid IJK and are evaluated on every grid
    const bool usesNestedHybridOriginalIjk = caseData->mainGrid()->nestedHybridLayout().has_value();

    // Grid local masks are kept to allow propagation of parent grid visibility into LGRs
    std::vector<cvf::ref<cvf::UByteArray>> gridMasks( caseData->gridCount() );

    for ( size_t gridIdx = 0; gridIdx < caseData->gridCount(); gridIdx++ )
    {
        RigGridBase* grid = caseData->grid( gridIdx );

        gridMasks[gridIdx]        = new cvf::UByteArray( grid->cellCount() );
        cvf::UByteArray& gridMask = *gridMasks[gridIdx];
        gridMask.setAll( true );

        if ( isRangeFilter() )
        {
            // Range filters evaluate only on their target grid. On other grids an INCLUDE filter
            // contributes no cells, while an EXCLUDE filter removes none.
            const bool isTargetGrid = usesNestedHybridOriginalIjk || ( gridIndex() == static_cast<int>( gridIdx ) );
            if ( isTargetGrid )
            {
                updateCellVisibility( &gridMask, grid, timeStepIndex, eclipseCase );
            }
            else
            {
                gridMask.setAll( !isInclude );
            }
        }
        else
        {
            // Index filters (polygon, user defined) and property filters evaluate on all grids
            updateCellVisibility( &gridMask, grid, timeStepIndex, eclipseCase );
        }

        // Cells in LGRs follow the visibility of their parent grid cell, as in the filtered geometry
        // of a 3d view. Geometry based filters (range and index, e.g. an INDEX_K polygon) can fail to
        // select the refined cells directly on a fine LGR, so propagate the parent grid visibility.
        // Property filters evaluate each cell against its own result value and must be left untouched.
        const bool isGeometryFilter = isRangeFilter() || isIndexFilter();
        if ( isGeometryFilter && !grid->isMainGrid() )
        {
            auto                   localGrid  = static_cast<const RigLocalGrid*>( grid );
            const cvf::UByteArray& parentMask = *gridMasks[localGrid->parentGrid()->gridIndex()];

            for ( size_t localIdx = 0; localIdx < grid->cellCount(); localIdx++ )
            {
                const size_t parentCellIndex = grid->cell( localIdx ).parentCellIndex();
                if ( isInclude )
                {
                    gridMask[localIdx] = gridMask[localIdx] || parentMask[parentCellIndex];
                }
                else
                {
                    gridMask[localIdx] = gridMask[localIdx] && parentMask[parentCellIndex];
                }
            }
        }

        for ( size_t localIdx = 0; localIdx < grid->cellCount(); localIdx++ )
        {
            reservoirVisibility->set( grid->reservoirCellIndex( localIdx ), gridMask[localIdx] );
        }
    }

    return reservoirVisibility;
}

//--------------------------------------------------------------------------------------------------
/// Static filters select the same cells regardless of time step. Property filters on dynamic results
/// and combined filters containing them override this.
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isDynamic() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigEclipseResultAddress> RimCellFilter::dynamicResultAddress() const
{
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isFilterControlled() const
{
    // Case-level data filters have no view ancestor; treat as not controlled.
    auto rimView = firstAncestorOrThisOfType<Rim3dView>();

    bool isFilterControlled = false;
    if ( rimView && rimView->viewController() && rimView->viewController()->isCellFiltersControlled() )
    {
        isFilterControlled = true;
    }

    return isFilterControlled;
}
