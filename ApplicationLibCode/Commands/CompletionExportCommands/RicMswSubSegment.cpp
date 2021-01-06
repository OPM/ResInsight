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

#include "RicMswSubSegment.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSubSegmentCellIntersection::RicMswSubSegmentCellIntersection( const QString&     gridName,
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
const QString& RicMswSubSegmentCellIntersection::gridName() const
{
    return m_gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswSubSegmentCellIntersection::globalCellIndex() const
{
    return m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicMswSubSegmentCellIntersection::gridLocalCellIJK() const
{
    return m_gridLocalCellIJK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RicMswSubSegmentCellIntersection::lengthsInCell() const
{
    return m_lengthsInCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSubSegment::RicMswSubSegment( double startMD, double endMD, double startTVD, double endTVD )
    : m_startMD( startMD )
    , m_endMD( endMD )
    , m_startTVD( startTVD )
    , m_endTVD( endTVD )
    , m_segmentNumber( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::deltaMD() const
{
    return m_endMD - m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::startTVD() const
{
    return m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::endTVD() const
{
    return m_endTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSubSegment::deltaTVD() const
{
    return m_endTVD - m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswSubSegment::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSubSegment::setSegmentNumber( int segmentNumber )
{
    m_segmentNumber = segmentNumber;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSubSegment::addIntersection( std::shared_ptr<RicMswSubSegmentCellIntersection> intersection )
{
    m_intersections.push_back( intersection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<RicMswSubSegmentCellIntersection>>& RicMswSubSegment::intersections() const
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswSubSegmentCellIntersection>>& RicMswSubSegment::intersections()
{
    return m_intersections;
}
