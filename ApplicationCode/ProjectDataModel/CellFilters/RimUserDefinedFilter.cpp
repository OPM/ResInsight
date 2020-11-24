/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimUserDefinedFilter.h"

CAF_PDM_SOURCE_INIT( RimUserDefinedFilter, "UserDefinedFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter::RimUserDefinedFilter()
{
    CAF_PDM_InitObject( "User Defined Filter", ":/CellFilter_UserDefined.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_individualCellIndices,
                                "IndividualCellIndices",
                                "Cells",
                                "",
                                "Use Ctrl-C for copy and Ctrl-V for paste",
                                "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedFilter::~RimUserDefinedFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    auto group = uiOrdering.addNewGroup( QString( "Cell Indexes (I J K) to " ) + modeString() );
    group->setCollapsedByDefault( false );
    group->add( &m_individualCellIndices );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RimUserDefinedFilter::individualCellIndices() const
{
    return m_individualCellIndices.v();
}

void RimUserDefinedFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
}
