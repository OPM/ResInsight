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
    : m_gridIndex( cvf::UNDEFINED_SIZE_T )
    , m_cellIndex( cvf::UNDEFINED_SIZE_T )
    , m_isOpen( false )
    , m_ertBranchId( -1 )
    , m_ertSegmentId( -1 )
    , m_ertOutletBranchId( -1 )
    , m_ertOutletSegmentId( -1 )
    , m_bottomPosition( cvf::Vec3d::UNDEFINED )
    , m_flowRate( 0.0 )
    , m_oilRate( 0.0 )
    , m_gasRate( 0.0 )
    , m_waterRate( 0.0 )
    , m_connectionFactor( 0.0 )
    , m_isConnectedToValve( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setGridIndex( size_t gridIndex )
{
    m_gridIndex = gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setGridCellIndex( size_t cellIndex )
{
    m_cellIndex = cellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setIsOpen( bool isOpen )
{
    m_isOpen = isOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setFlowData( double flowRate, double oilRate, double gasRate, double waterRate )
{
    m_flowRate  = flowRate;
    m_oilRate   = oilRate;
    m_gasRate   = gasRate;
    m_waterRate = waterRate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setConnectionFactor( double connectionFactor )
{
    m_connectionFactor = connectionFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setSegmentData( int branchId, int segmentId )
{
    m_ertBranchId  = branchId;
    m_ertSegmentId = segmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setOutletSegmentData( int outletBranchId, int outletSegmentId )
{
    m_ertOutletBranchId  = outletBranchId;
    m_ertOutletSegmentId = outletSegmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setBottomPosition( const cvf::Vec3d& bottomPosition )
{
    m_bottomPosition = bottomPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setIsConnectedToValve( bool enable )
{
    m_isConnectedToValve = true;
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
    return m_cellIndex != cvf::UNDEFINED_SIZE_T;
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
bool RigWellResultPoint::isOpen() const
{
    return m_isOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isEqual( const RigWellResultPoint& other ) const
{
    return ( m_gridIndex == other.m_gridIndex && m_cellIndex == other.m_cellIndex && m_isOpen == other.m_isOpen &&
             m_ertBranchId == other.m_ertBranchId && m_ertSegmentId == other.m_ertSegmentId && m_flowRate == other.m_flowRate &&
             m_oilRate == other.m_oilRate && m_gasRate == other.m_gasRate && m_waterRate == other.m_waterRate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellResultPoint::isConnectedToValve() const
{
    return m_isConnectedToValve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellResultPoint::flowRate() const
{
    if ( isCell() && m_isOpen )
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
    if ( isCell() && m_isOpen )
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
    if ( isCell() && m_isOpen )
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
    if ( isCell() && m_isOpen )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::clearAllFlow()
{
    m_connectionFactor = 0.0;
    m_flowRate         = 0.0;
    m_oilRate          = 0.0;
    m_gasRate          = 0.0;
    m_waterRate        = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellResultPoint::gridIndex() const
{
    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellResultPoint::cellIndex() const
{
    return m_cellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigWellResultPoint::branchId() const
{
    return m_ertBranchId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigWellResultPoint::segmentId() const
{
    return m_ertSegmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigWellResultPoint::outletBranchId() const
{
    return m_ertOutletBranchId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigWellResultPoint::outletSegmentId() const
{
    return m_ertOutletSegmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellResultPoint::bottomPosition() const
{
    return m_bottomPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk0> RigWellResultPoint::cellIjk() const
{
    return m_cellIjk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultPoint::setIjk( caf::VecIjk0 cellIJK )
{
    m_cellIjk = cellIJK;
}
