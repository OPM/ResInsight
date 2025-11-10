/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RigBoundingBoxIjk.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigBoundingBoxIjk::RigBoundingBoxIjk()
    : m_min( cvf::Vec3st::ZERO )
    , m_max( cvf::Vec3st::ZERO )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigBoundingBoxIjk::RigBoundingBoxIjk( const cvf::Vec3st& min, const cvf::Vec3st& max )
    : m_min( min )
    , m_max( max )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigBoundingBoxIjk::isValid() const
{
    return m_min.x() <= m_max.x() && m_min.y() <= m_max.y() && m_min.z() <= m_max.z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigBoundingBoxIjk::overlaps( const RigBoundingBoxIjk& other ) const
{
    // Two boxes overlap if they overlap in all three dimensions
    // No overlap if: this.max < other.min OR this.min > other.max (for any dimension)
    bool overlapsI = m_max.x() >= other.m_min.x() && m_min.x() <= other.m_max.x();
    bool overlapsJ = m_max.y() >= other.m_min.y() && m_min.y() <= other.m_max.y();
    bool overlapsK = m_max.z() >= other.m_min.z() && m_min.z() <= other.m_max.z();

    return overlapsI && overlapsJ && overlapsK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigBoundingBoxIjk> RigBoundingBoxIjk::intersection( const RigBoundingBoxIjk& other ) const
{
    if ( !overlaps( other ) )
    {
        return std::nullopt;
    }

    // Compute intersection box
    cvf::Vec3st intersectMin( std::max( m_min.x(), other.m_min.x() ),
                              std::max( m_min.y(), other.m_min.y() ),
                              std::max( m_min.z(), other.m_min.z() ) );

    cvf::Vec3st intersectMax( std::min( m_max.x(), other.m_max.x() ),
                              std::min( m_max.y(), other.m_max.y() ),
                              std::min( m_max.z(), other.m_max.z() ) );

    return RigBoundingBoxIjk( intersectMin, intersectMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigBoundingBoxIjk> RigBoundingBoxIjk::clamp( const RigBoundingBoxIjk& bounds ) const
{
    // Clamp is the same as intersection for bounding boxes
    return intersection( bounds );
}
