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

    CAF_PDM_InitField( &m_name, "UserDescription", QString( "New filter" ), "Name" );
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

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
    m_nameProxy.xmlCapability()->disableIO();
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
    return QString( "%1" ).arg( m_name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellFilter::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setName( QString filtername )
{
    m_name = filtername;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::setActive( bool active )
{
    m_isActive = active;
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// Is the filter turned on in the explorer tree?
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isActive() const
{
    return m_isActive();
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
    return m_isActive();
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
    m_isActive.uiCapability()->setUiReadOnly( isControlled );
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

    iconProvider.setActive( m_isActive && !m_isActive.uiCapability()->isUiReadOnly() );

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellFilter::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
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
