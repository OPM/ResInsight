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

//#include "RiaGuiApplication.h"

//#include <QPainter>

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

CAF_PDM_SOURCE_INIT( RimCellFilter, "CellFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter::RimCellFilter()
{
    CAF_PDM_InitObject( "Cell Filter", "", "", "" );

    CAF_PDM_InitField( &m_name, "UserDescription", QString( "Filter Name" ), "Name", "", "", "" );
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filterMode, "FilterType", "Filter Type", "", "", "" );
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
QString RimCellFilter::name()
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilter::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RimCellFilter::FilterModeType> RimCellFilter::filterMode()
{
    return m_filterMode();
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
    uiOrdering.add( &m_filterMode );
}

QString RimCellFilter::modeString() const
{
    if ( m_filterMode == RimCellFilter::FilterModeType::INCLUDE ) return "include";
    return "exclude";
}
