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

#pragma once

#include "RiaDefines.h"

#include "RigWellResultPoint.h"

#include "cvfVector3.h"

#include <vector>

//==================================================================================================
/// This class contains the connection information from and including a split point to the end of
/// that particular branch.
//==================================================================================================
struct RigWellResultBranch
{
    RigWellResultBranch();

    int  ertBranchId() const;
    void setErtBranchId( int id );

    std::vector<RigWellResultPoint> branchResultPoints() const;
    void                            addBranchResultPoint( const RigWellResultPoint& point );
    void                            setBranchResultPoints( const std::vector<RigWellResultPoint>& points );

private:
    int                             m_ertBranchId;
    std::vector<RigWellResultPoint> m_branchResultPoints;
};

using SimulationWellCellBranch = std::pair<std::vector<cvf::Vec3d>, std::vector<RigWellResultPoint>>;
