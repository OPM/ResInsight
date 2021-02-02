/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswSegmentCellIntersection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSegmentCellIntersection::RicMswSegmentCellIntersection( const QString&     gridName,
                                                                    size_t             globalCellIndex,
                                                                    const cvf::Vec3st& gridLocalCellIJK,
                                                                    const cvf::Vec3d&  lengthsInCell )
    : m_gridName( gridName )
    , m_globalCellIndex( globalCellIndex )
    , m_gridLocalCellIJK( gridLocalCellIJK )
    , m_lengthsInCell( lengthsInCell )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RicMswSegmentCellIntersection::gridName() const
{
    return m_gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswSegmentCellIntersection::globalCellIndex() const
{
    return m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicMswSegmentCellIntersection::gridLocalCellIJK() const
{
    return m_gridLocalCellIJK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RicMswSegmentCellIntersection::lengthsInCell() const
{
    return m_lengthsInCell;
}
