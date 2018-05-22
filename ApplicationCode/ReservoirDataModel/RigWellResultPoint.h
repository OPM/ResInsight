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

#pragma once

#include "cvfBase.h"
#include "cvfVector3.h"

#include <QDateTime>

#include <vector>


//==================================================================================================
/// Stores the info on a significant point in the well. Either a well-to-grid connection, or the 
/// bottom position of a connection less well-segment
//==================================================================================================
struct RigWellResultPoint
{
    RigWellResultPoint();

    bool   isPointValid() const;
    bool   isCell() const;
    bool   isValid() const;
    bool   isEqual(const RigWellResultPoint& other) const;

    double flowRate() const;
    double oilRate() const;
    double gasRate() const;
    double waterRate() const;
    double connectionFactor() const;

    size_t      m_gridIndex;
    size_t      m_gridCellIndex; //< Index to cell which is included in the well
    bool        m_isOpen; //< Marks the well as open or closed as of Eclipse simulation

    int         m_ertBranchId;
    int         m_ertSegmentId;

    cvf::Vec3d  m_bottomPosition; //< The estimated bottom position of the well segment, when we have no grid cell connections for
                                 //the segment.
    double      m_flowRate; //< Total reservoir rate
    double      m_oilRate; //< Surface oil rate
    double      m_gasRate; //< Surface gas rate For Field-unit, converted to [stb/day] to align with oil and water.
    double      m_waterRate; //< Surface water rate

    double      m_connectionFactor; 
};

//==================================================================================================
/// This class contains the connection information from and including a splitpoint to the end of 
/// that particular branch.
//==================================================================================================
struct RigWellResultBranch
{
    RigWellResultBranch()
        : m_ertBranchId(-1)
    {
    }

    int                               m_ertBranchId;
    std::vector<RigWellResultPoint>   m_branchResultPoints;
};

//==================================================================================================
/// This class contains the well information for one timestep. 
/// The main content is the vector of RigWellResultBranch which contains all the simple pipe 
/// sections that make up the well
//==================================================================================================
class RigWellResultFrame
{
public:
    enum WellProductionType
    {
        PRODUCER,
        OIL_INJECTOR,
        GAS_INJECTOR,
        WATER_INJECTOR,
        UNDEFINED_PRODUCTION_TYPE
    };

public:
    RigWellResultFrame()
        : m_productionType(UNDEFINED_PRODUCTION_TYPE)
        , m_isOpen(false)
    {
    }

    const RigWellResultPoint*         findResultCell(size_t gridIndex, size_t gridCellIndex) const;

    RigWellResultPoint                wellHeadOrStartCell() const;
    WellProductionType                m_productionType;
    bool                              m_isOpen;
    RigWellResultPoint                m_wellHead;
    QDateTime                         m_timestamp;

    std::vector<RigWellResultBranch>  m_wellResultBranches;
};

