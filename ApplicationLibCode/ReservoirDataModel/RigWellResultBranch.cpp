/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RigWellResultBranch.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultBranch::RigWellResultBranch()
    : m_ertBranchId( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigWellResultBranch::ertBranchId() const
{
    return m_ertBranchId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultBranch::setErtBranchId( int id )
{
    m_ertBranchId = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellResultPoint> RigWellResultBranch::branchResultPoints() const
{
    return m_branchResultPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultBranch::clearBranchResultPoints()
{
    m_branchResultPoints.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultBranch::addBranchResultPoint( const RigWellResultPoint& point )
{
    m_branchResultPoints.push_back( point );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultBranch::setBranchResultPoints( const std::vector<RigWellResultPoint>& points )
{
    m_branchResultPoints = points;
}
