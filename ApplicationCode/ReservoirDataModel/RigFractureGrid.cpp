/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigFractureGrid.h"

#include "RiaLogging.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFractureGrid::RigFractureGrid()
    : m_iCellCount( 0 )
    , m_jCellCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureGrid::setFractureCells( std::vector<RigFractureCell> fractureCells )
{
    m_fractureCells = fractureCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureGrid::setWellCenterFractureCellIJ( std::pair<size_t, size_t> wellCenterFractureCellIJ )
{
    m_wellCenterFractureCellIJ = wellCenterFractureCellIJ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureGrid::setICellCount( size_t iCellCount )
{
    m_iCellCount = iCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureGrid::setJCellCount( size_t jCellCount )
{
    m_jCellCount = jCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigFractureCell>& RigFractureGrid::fractureCells() const
{
    return m_fractureCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFractureGrid::getGlobalIndexFromIJ( size_t i, size_t j ) const
{
    return i * m_jCellCount + j;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFractureCell& RigFractureGrid::cellFromIndex( size_t index ) const
{
    if ( index < m_fractureCells.size() )
    {
        const RigFractureCell& cell = m_fractureCells[index];
        return cell;
    }
    else
    {
        // TODO: Better error handling?
        RiaLogging::error( QString( "Requesting non-existent StimPlanCell" ) );
        RiaLogging::error( QString( "Returning cell 0, results will be invalid" ) );
        const RigFractureCell& cell = m_fractureCells[0];
        return cell;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFractureGrid::jCellCount() const
{
    return m_jCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFractureGrid::iCellCount() const
{
    return m_iCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigFractureGrid::fractureCellAtWellCenter() const
{
    return m_wellCenterFractureCellIJ;
}
