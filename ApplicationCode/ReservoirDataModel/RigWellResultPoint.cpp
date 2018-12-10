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

#include "RigWellResultPoint.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellResultPoint::RigWellResultPoint()
    : m_gridIndex(cvf::UNDEFINED_SIZE_T)
    , m_gridCellIndex(cvf::UNDEFINED_SIZE_T)
    , m_isOpen(false)
    , m_ertBranchId(-1)
    , m_ertSegmentId(-1)
    , m_bottomPosition(cvf::Vec3d::UNDEFINED)
    , m_flowRate(0.0)
    , m_oilRate(0.0)
    , m_gasRate(0.0)
    , m_waterRate(0.0)
    , m_connectionFactor(0.0)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isPointValid() const
{
    return m_bottomPosition != cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isCell() const
{
    return m_gridCellIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isValid() const
{
    return isCell() || isPointValid();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isEqual(const RigWellResultPoint& other) const
{
    return (   m_gridIndex      == other.m_gridIndex
            && m_gridCellIndex  == other.m_gridCellIndex
            && m_isOpen         == other.m_isOpen
            && m_ertBranchId    == other.m_ertBranchId
            && m_ertSegmentId   == other.m_ertSegmentId
            && m_flowRate       == other.m_flowRate
            && m_oilRate        == other.m_oilRate
            && m_gasRate        == other.m_gasRate
            && m_waterRate      == other.m_waterRate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::flowRate() const
{
    if (isCell() && m_isOpen)
    {
        return m_flowRate;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::oilRate() const
{
    if (isCell() && m_isOpen)
    {
        return m_oilRate;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::gasRate() const
{
    if (isCell() && m_isOpen)
    {
        return m_gasRate;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::waterRate() const
{
    if (isCell() && m_isOpen)
    {
        return m_waterRate;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::connectionFactor() const
{
    return m_connectionFactor;
}

