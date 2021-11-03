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

#include "cvfBoundingBox.h"
#include "cvfBoundingBoxTree.h"

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureGrid::ensureCellSearchTreeIsBuilt()
{
    if ( m_cellBoundingBoxTree.isNull() )
    {
        size_t itemCount = m_fractureCells.size();

        std::vector<cvf::BoundingBox> cellBoundingBoxes( itemCount );
        std::vector<size_t>           boundingBoxIds( itemCount );

        for ( size_t idx = 0; idx < itemCount; ++idx )
        {
            const RigFractureCell&         cell    = m_fractureCells[idx];
            cvf::BoundingBox&              cellBB  = cellBoundingBoxes[idx];
            const std::vector<cvf::Vec3d>& corners = cell.getPolygon();
            for ( auto c : corners )
                cellBB.add( c );

            boundingBoxIds[idx] = idx;
        }

        m_cellBoundingBoxTree = new cvf::BoundingBoxTree;
        m_cellBoundingBoxTree->buildTreeFromBoundingBoxes( cellBoundingBoxes, &boundingBoxIds );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFractureCell* RigFractureGrid::getCellFromPosition( const cvf::Vec3d& position ) const
{
    cvf::BoundingBox inputBB;
    inputBB.add( position );

    std::vector<size_t> indexes;
    m_cellBoundingBoxTree->findIntersections( inputBB, &indexes );

    if ( !indexes.empty() )
    {
        // Hit: should only one cell since they have no overlap
        return &m_fractureCells[indexes[0]];
    }
    else
    {
        // No hit
        return nullptr;
    }
}
