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

#include "RigGridBase.h"
#include "RigReservoirGridTools.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimTools.h"
#include "RimViewController.h"

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
/// Is the filter turned on in the explorer tree?
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isActive() const
{
    return m_isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::triggerFilterChanged() const
{
    filterChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Is the cell filter doing active filtering, or is it just showning outline, etc. in the view
/// - isActive == true -> filter enabled in explorer
/// - isFilterEnabled == true -> filter enabled in explorer and is actually filtering cells, too
/// Default implementation just returns the isActive state.
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isFilterEnabled() const
{
    return m_isChecked();
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
void RimCellFilter::setGridIndex( int gridIndex )
{
    m_gridIndex = gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCellFilter::gridIndex() const
{
    return m_gridIndex;
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
void RimCellFilter::updateIconState()
{
    caf::IconProvider iconProvider = uiIconProvider();

    if ( !iconProvider.valid() ) return;

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
    group->add( &m_gridIndex );

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
const cvf::StructGridInterface* RimCellFilter::selectedGrid() const
{
    auto rimCase = firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();
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
void RimCellFilter::applyToCellVisibility( cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t /*timeStepIndex*/ )
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
        cvf::CellRangeFilter rf;
        updateCompundFilter( &rf, gIndx );

        const bool isSubGrid = !grid->isMainGrid();

        for ( size_t cellIdx = 0; cellIdx < n; ++cellIdx )
        {
            size_t i = 0, j = 0, k = 0;
            if ( grid->ijkFromCellIndex( cellIdx, &i, &j, &k ) )
            {
                if ( rf.isCellVisible( i, j, k, isSubGrid ) ) inSet[cellIdx] = 1;
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
        // Unknown/property — subclass must override applyToCellVisibility. Default to no-op.
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
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isFilterControlled() const
{
    auto rimView = firstAncestorOrThisOfTypeAsserted<Rim3dView>();

    bool isFilterControlled = false;
    if ( rimView && rimView->viewController() && rimView->viewController()->isCellFiltersControlled() )
    {
        isFilterControlled = true;
    }

    return isFilterControlled;
}
