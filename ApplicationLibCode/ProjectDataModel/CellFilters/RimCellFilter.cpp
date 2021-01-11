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
#include "RimViewController.h"

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
RimCellFilter::RimCellFilter()
    : filterChanged( this )
{
    CAF_PDM_InitObject( "Cell Filter", "", "", "" );

    CAF_PDM_InitField( &m_name, "UserDescription", QString( "New filter" ), "Name", "", "", "" );
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filterMode, "FilterType", "Filter Type", "", "", "" );

    CAF_PDM_InitField( &m_gridIndex, "GridIndex", 0, "Grid", "", "", "" );
    CAF_PDM_InitField( &m_propagateToSubGrids, "PropagateToSubGrids", true, "Apply to Subgrids", "", "", "" );
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
    return &m_name;
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
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isActive() const
{
    return m_isActive();
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
    caf::IconProvider iconProvider = this->uiIconProvider();

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

    this->setUiIcon( iconProvider );
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
    group->add( &m_gridIndex );
    group->add( &m_propagateToSubGrids );

    bool readOnlyState = isFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields;
    this->fields( objFields );
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
    RimCase* rimCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( rimCase );

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
QList<caf::PdmOptionItemInfo> RimCellFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                    bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( useOptionsOnly ) ( *useOptionsOnly ) = true;

    if ( &m_gridIndex == fieldNeedingOptions )
    {
        RimCase* rimCase = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( rimCase );

        for ( int gIdx = 0; gIdx < RigReservoirGridTools::gridCount( rimCase ); ++gIdx )
        {
            QString gridName;

            gridName += RigReservoirGridTools::gridName( rimCase, gIdx );
            if ( gIdx == 0 )
            {
                if ( gridName.isEmpty() )
                    gridName += "Main Grid";
                else
                    gridName += " (Main Grid)";
            }

            caf::PdmOptionItemInfo item( gridName, (int)gIdx );
            options.push_back( item );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isFilterControlled() const
{
    Rim3dView* rimView = nullptr;
    firstAncestorOrThisOfTypeAsserted( rimView );

    bool isFilterControlled = false;
    if ( rimView && rimView->viewController() && rimView->viewController()->isCellFiltersControlled() )
    {
        isFilterControlled = true;
    }

    return isFilterControlled;
}
