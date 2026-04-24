/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimCombinedFilter.h"

#include "RigGridBase.h"
#include "RigReservoirGridTools.h"
#include "RimCase.h"
#include "RimCellIndexFilter.h"
#include "RimCellRangeFilter.h"
#include "RimPolygonFilter.h"
#include "RimUserDefinedFilter.h"
#include "RimUserDefinedIndexFilter.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RiaResultNames.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimCombinedFilter, "CombinedFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCombinedFilter::RimCombinedFilter()
    : RimCellFilter( FilterDefinitionType::INDEX )
{
    CAF_PDM_InitScriptableObject( "Combined Filter", ":/CellFilter.png" );

    setName( "Combined Filter" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filters, "Filters", "Filters" );
    caf::PdmFieldReorderCapability::addToField( &m_filters );

    CAF_PDM_InitScriptableFieldNoDefault( &m_combineMode, "CombineMode", "Combine Mode" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCombinedFilter::~RimCombinedFilter() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::setCase( RimCase* srcCase )
{
    RimCellFilter::setCase( srcCase );
    for ( RimCellFilter* child : m_filters )
    {
        if ( child ) child->setCase( srcCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCombinedFilter::isFilterEnabled() const
{
    if ( !isActive() ) return false;

    for ( RimCellFilter* child : m_filters )
    {
        if ( child && child->isFilterEnabled() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::onGridChanged()
{
    for ( RimCellFilter* child : m_filters )
    {
        if ( child ) child->onGridChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// Evaluate each enabled child to its own per-cell mask (each child respects its own
/// INCLUDE/EXCLUDE mode inside applyToCellVisibility), AND/OR combine the masks, then apply this
/// combined filter's INCLUDE/EXCLUDE mode onto the incoming cellVisibility.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::applyToCellVisibility( cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t timeStepIndex )
{
    if ( cellVisibility == nullptr || grid == nullptr ) return;

    const size_t n = cellVisibility->size();
    if ( n == 0 ) return;

    std::vector<RimCellFilter*> enabledChildren;
    for ( RimCellFilter* child : m_filters )
    {
        if ( child && child->isFilterEnabled() ) enabledChildren.push_back( child );
    }
    if ( enabledChildren.empty() ) return;

    cvf::UByteArray combined( n );
    combined.setAll( m_combineMode() == CombineMode::AND ? 1 : 0 );

    for ( RimCellFilter* child : enabledChildren )
    {
        cvf::UByteArray childMask( n );
        childMask.setAll( 1 );
        child->applyToCellVisibility( &childMask, grid, timeStepIndex );

        if ( m_combineMode() == CombineMode::AND )
        {
            for ( size_t i = 0; i < n; ++i )
                combined[i] = combined[i] && childMask[i];
        }
        else
        {
            for ( size_t i = 0; i < n; ++i )
                combined[i] = combined[i] || childMask[i];
        }
    }

    const bool isInclude = ( filterMode() == INCLUDE );
    for ( size_t i = 0; i < n; ++i )
    {
        if ( isInclude )
        {
            if ( !combined[i] ) ( *cellVisibility )[i] = 0;
        }
        else
        {
            if ( combined[i] ) ( *cellVisibility )[i] = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Bridge for the cell-filter collection's legacy index-based dispatch. Combined filters normally
/// live under the property-filter collection, but this override exists to keep the class usable if
/// a future caller places one in the cell-filter collection. Builds a per-cell mask by running
/// applyToCellVisibility against an all-visible scratch array, then folds the result into the
/// collection's include/exclude arrays respecting this filter's INCLUDE/EXCLUDE mode.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex )
{
    if ( includeVisibility == nullptr || excludeVisibility == nullptr ) return;

    auto* sourceCase = m_srcCase();
    if ( !sourceCase ) return;

    const auto* structGrid = RigReservoirGridTools::gridByIndex( sourceCase, gridIndex );
    const auto* grid       = dynamic_cast<const RigGridBase*>( structGrid );
    if ( grid == nullptr ) return;

    const size_t n = includeVisibility->size();
    if ( n == 0 ) return;

    cvf::UByteArray scratch( n );
    scratch.setAll( 1 );
    applyToCellVisibility( &scratch, grid, 0 );

    if ( filterMode() == INCLUDE )
    {
        for ( size_t i = 0; i < n; ++i )
            if ( scratch[i] ) ( *includeVisibility )[i] = 1;
    }
    else
    {
        for ( size_t i = 0; i < n; ++i )
            if ( !scratch[i] ) ( *excludeVisibility )[i] = 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCombinedFilter::fullName() const
{
    const QString modeStr = ( m_combineMode() == CombineMode::AND ) ? "AND" : "OR";
    return QString( "%1  [%2: %3]" ).arg( name() ).arg( modeStr ).arg( m_filters.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::addFilter( RimCellFilter* child )
{
    if ( !child ) return;
    if ( wouldCreateCycle( child ) ) return;
    m_filters.push_back( child );
    child->setCase( m_srcCase() );
    child->filterChanged.connect( this, &RimCombinedFilter::onChildFilterChanged );
    updateConnectedEditors();
    triggerFilterChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::removeFilter( RimCellFilter* child )
{
    if ( !child ) return;
    m_filters.removeChild( child );
    triggerFilterChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCellFilter*> RimCombinedFilter::filters() const
{
    return m_filters.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimCombinedFilter::addNewCellRangeFilter( RimCase* srcCase, int gridIndex, int sliceDirection, int defaultSlice )
{
    auto* f = new RimCellRangeFilter();
    addFilter( f );
    f->setCase( srcCase );
    f->setGridIndex( gridIndex );
    f->setDefaultValues( sliceDirection, defaultSlice );
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter* RimCombinedFilter::addNewPolygonFilter( RimCase* srcCase, RimPolygon* polygon )
{
    auto* f = new RimPolygonFilter();
    addFilter( f );
    f->setCase( srcCase );
    f->setPolygon( polygon );
    f->configurePolygonEditor();
    if ( polygon )
    {
        f->enableFilter( true );
    }
    else
    {
        f->enablePicking( true );
    }
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter* RimCombinedFilter::addNewCellIndexFilter( RimCase* srcCase )
{
    auto* f = new RimCellIndexFilter();
    addFilter( f );
    f->setCase( srcCase );
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter* RimCombinedFilter::addNewUserDefinedFilter( RimCase* srcCase )
{
    auto* f = new RimUserDefinedFilter();
    addFilter( f );
    f->setCase( srcCase );
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedIndexFilter* RimCombinedFilter::addNewUserDefinedIndexFilter( RimCase* srcCase, const std::vector<size_t>& defCellIndexes )
{
    auto* f = new RimUserDefinedIndexFilter();
    addFilter( f );
    f->setCase( srcCase );
    f->setCellIndexes( defCellIndexes );
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCombinedFilter* RimCombinedFilter::addNewCombinedFilter( RimCase* srcCase )
{
    auto* f = new RimCombinedFilter();
    addFilter( f );
    f->setCase( srcCase );
    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::setCombineMode( CombineMode mode )
{
    m_combineMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCombinedFilter::CombineMode RimCombinedFilter::combineMode() const
{
    return m_combineMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    auto* group = uiOrdering.addNewGroup( "General" );
    group->add( &m_filterMode );
    group->add( &m_combineMode );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    PdmObject::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateIconState();
    triggerFilterChanged();
    notifyHostCollection();
}

//--------------------------------------------------------------------------------------------------
/// Called by CAF when a new object is pushed into m_filters (e.g. via our factory methods or via
/// drag-and-drop). Propagate so the host collection regenerates geometry.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::onChildAdded( caf::PdmFieldHandle* /*containerForNewObject*/ )
{
    triggerFilterChanged();
    notifyHostCollection();
}

//--------------------------------------------------------------------------------------------------
/// Combined filters live only under the Eclipse property-filter collection, which triggers view
/// regen via an explicit updateDisplayModelNotifyManagedViews() call rather than by subscribing to
/// filterChanged. Call that API directly so our edits propagate.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::notifyHostCollection()
{
    if ( auto* propColl = firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
    {
        propColl->updateDisplayModelNotifyManagedViews( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
/// Called after reading from a project file. Re-wire child filterChanged signals so edits made to
/// deserialized children propagate up.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::initAfterRead()
{
    RimCellFilter::initAfterRead();
    for ( RimCellFilter* child : m_filters )
    {
        if ( child ) child->filterChanged.connect( this, &RimCombinedFilter::onChildFilterChanged );
    }
}

//--------------------------------------------------------------------------------------------------
/// Propagate a child's change up so the view re-renders.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::onChildFilterChanged( const caf::SignalEmitter* /*emitter*/ )
{
    triggerFilterChanged();
    notifyHostCollection();
}

//--------------------------------------------------------------------------------------------------
/// Any enabled child that the combined filter would actually evaluate. Property-filter children
/// also need a loaded result; other cell filters are always evaluatable when enabled.
//--------------------------------------------------------------------------------------------------
bool RimCombinedFilter::hasActiveEvaluatableDescendant() const
{
    for ( RimCellFilter* child : m_filters )
    {
        if ( !child || !child->isFilterEnabled() ) continue;

        if ( auto* ep = dynamic_cast<RimEclipsePropertyFilter*>( child ) )
        {
            if ( ep->resultDefinition() && ep->resultDefinition()->hasResult() ) return true;
            continue;
        }
        if ( auto* nested = dynamic_cast<RimCombinedFilter*>( child ) )
        {
            if ( nested->hasActiveEvaluatableDescendant() ) return true;
            continue;
        }
        // Range, polygon, index, user-defined — evaluatable whenever enabled.
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// True if any active descendant is an Eclipse property filter backed by a dynamic result. Drives
/// per-time-step cell-visibility cache invalidation in RimGridView.
//--------------------------------------------------------------------------------------------------
bool RimCombinedFilter::hasActiveDynamicPropertyDescendant() const
{
    for ( RimCellFilter* child : m_filters )
    {
        if ( !child || !child->isFilterEnabled() ) continue;

        if ( auto* ep = dynamic_cast<RimEclipsePropertyFilter*>( child ) )
        {
            if ( ep->resultDefinition() && ep->resultDefinition()->hasDynamicResult() ) return true;
        }
        else if ( auto* nested = dynamic_cast<RimCombinedFilter*>( child ) )
        {
            if ( nested->hasActiveDynamicPropertyDescendant() ) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// True if any active descendant is a formation-names property filter. Matches the collection-level
/// isUsingFormationNames() check.
//--------------------------------------------------------------------------------------------------
bool RimCombinedFilter::hasActiveFormationNamesPropertyDescendant() const
{
    for ( RimCellFilter* child : m_filters )
    {
        if ( !child || !child->isFilterEnabled() ) continue;

        if ( auto* ep = dynamic_cast<RimEclipsePropertyFilter*>( child ) )
        {
            auto* rd = ep->resultDefinition();
            if ( rd && rd->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES &&
                 rd->resultVariable() != RiaResultNames::undefinedResultName() )
                return true;
        }
        else if ( auto* nested = dynamic_cast<RimCombinedFilter*>( child ) )
        {
            if ( nested->hasActiveFormationNamesPropertyDescendant() ) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Right-click "add child" menu on the combined filter. Since combined filters only live under the
/// Eclipse property-filter collection, the menu always offers the full mixed-type set: property
/// filter, polygon, ranges, index, user-defined, and a nested combined filter.
//--------------------------------------------------------------------------------------------------
void RimCombinedFilter::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicEclipsePropertyFilterNewFeature";
    menuBuilder << "Separator";

    menuBuilder.subMenuStart( "Polygon Filter", QIcon( ":/CellFilter_Polygon.png" ) );
    {
        auto polygonCollection = RimTools::polygonCollection();
        if ( polygonCollection )
        {
            for ( auto p : polygonCollection->allPolygons() )
            {
                if ( !p ) continue;
                menuBuilder.addCmdFeatureWithUserData( "RicNewPolygonFilterFeature", p->name(), QVariant::fromValue( static_cast<void*>( p ) ) );
            }
        }
    }
    menuBuilder.subMenuEnd();

    menuBuilder << "RicNewPolygonFilterFeature";
    menuBuilder << "Separator";
    menuBuilder.subMenuStart( "Range Filter" );
    menuBuilder << "RicNewRangeFilterSliceIFeature";
    menuBuilder << "RicNewRangeFilterSliceJFeature";
    menuBuilder << "RicNewRangeFilterSliceKFeature";
    menuBuilder << "RicNewCellRangeFilterFeature";
    menuBuilder.subMenuEnd();
    menuBuilder << "RicNewCellIndexFilterFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicNewUserDefinedFilterFeature";
    menuBuilder << "RicNewUserDefinedIndexFilterFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicEclipseCombinedPropertyFilterNewFeature";
}

//--------------------------------------------------------------------------------------------------
/// Refuse additions that would create a cycle: candidate must not be `this`, and if candidate is a
/// combined filter, `this` must not appear anywhere in its subtree.
//--------------------------------------------------------------------------------------------------
bool RimCombinedFilter::wouldCreateCycle( RimCellFilter* candidate ) const
{
    if ( candidate == this ) return true;

    auto* candidateCombined = dynamic_cast<RimCombinedFilter*>( candidate );
    if ( !candidateCombined ) return false;

    for ( RimCellFilter* grandchild : candidateCombined->filters() )
    {
        if ( grandchild == this ) return true;
        if ( auto* gcCombined = dynamic_cast<RimCombinedFilter*>( grandchild ) )
        {
            if ( wouldCreateCycle( gcCombined ) ) return true;
        }
    }
    return false;
}
