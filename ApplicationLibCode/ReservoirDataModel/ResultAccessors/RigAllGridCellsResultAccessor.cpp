/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigAllGridCellsResultAccessor.h"

#include "RigGridBase.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigAllGridCellsResultAccessor::RigAllGridCellsResultAccessor( const RigGridBase* grid, const std::vector<double>* reservoirResultValues )
    : m_grid( grid )
    , m_reservoirResultValues( reservoirResultValues )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessor::cellScalar( size_t gridLocalCellIndex ) const
{
    if ( m_reservoirResultValues->empty() ) return HUGE_VAL;

    size_t reservoirCellIndex = m_grid->reservoirCellIndex( gridLocalCellIndex );
    if ( reservoirCellIndex >= m_reservoirResultValues->size() ) return HUGE_VAL;

    return ( *m_reservoirResultValues )[reservoirCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessor::cellFaceScalar( size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId ) const
{
    return cellScalar( gridLocalCellIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessor::cellScalarGlobIdx( size_t globCellIndex ) const
{
    if ( m_reservoirResultValues->empty() ) return HUGE_VAL;

    // The cell index can come from another case, see RimGridCalculation::getActiveCellValues(). Avoid at(), which throws: an
    // exception escaping an OpenMP loop calls std::terminate.
    if ( globCellIndex >= m_reservoirResultValues->size() ) return HUGE_VAL;

    return ( *m_reservoirResultValues )[globCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessor::cellFaceScalarGlobIdx( size_t globCellIndex, cvf::StructGridInterface::FaceType faceId ) const
{
    return cellScalarGlobIdx( globCellIndex );
}
