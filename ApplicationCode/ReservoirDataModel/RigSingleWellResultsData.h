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
#include "cvfObject.h"
#include "cvfMath.h"

#include "RimDefines.h"
#include <QDateTime>
#include <vector>
#include "cvfVector3.h"

//==================================================================================================
/// Stores the info on a significant point in the well. Either a well-to-grid connection, or the 
/// bottom position of a connection less well-segment
//==================================================================================================
struct RigWellResultPoint
{
    RigWellResultPoint() : 
        m_gridIndex(cvf::UNDEFINED_SIZE_T), 
        m_gridCellIndex(cvf::UNDEFINED_SIZE_T), 
        m_isOpen(false),
        m_ertBranchId(-1),
        m_ertSegmentId(-1),
        m_bottomPosition(cvf::Vec3d::UNDEFINED)
    { }

    bool isPointValid() const
    {
        return m_bottomPosition != cvf::Vec3d::UNDEFINED;
    }

    bool isCell() const
    {
        return m_gridCellIndex != cvf::UNDEFINED_SIZE_T;
    }

    bool isValid() const
    {
        return isCell() || isPointValid();
    }

    size_t                            m_gridIndex;
    size_t                            m_gridCellIndex;     //< Index to cell which is included in the well

    bool                              m_isOpen;            //< Marks the well as open or closed as of Eclipse simulation

    int                               m_ertBranchId;
    int                               m_ertSegmentId;

    cvf::Vec3d                        m_bottomPosition;    //< The estimated bottom position of the well segment, when we have no grid cell connections for the segment.
};

//==================================================================================================
/// This class contains the connection information from and including a splitpoint to the end of 
/// that particular branch.
//==================================================================================================
struct RigWellResultBranch
{
    RigWellResultBranch() :
        m_ertBranchId(-1)
    {}

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
    enum WellProductionType { PRODUCER, OIL_INJECTOR, GAS_INJECTOR, WATER_INJECTOR, UNDEFINED_PRODUCTION_TYPE };

public:
    RigWellResultFrame() :
        m_isOpen(false),
        m_productionType(UNDEFINED_PRODUCTION_TYPE)
    { }

    const RigWellResultPoint*         findResultCell(size_t gridIndex, size_t gridCellIndex) const;

    WellProductionType                m_productionType;
    bool                              m_isOpen;
    RigWellResultPoint                m_wellHead;
    QDateTime                         m_timestamp;
    
    std::vector<RigWellResultBranch>  m_wellResultBranches;
};


//==================================================================================================
/// 
//==================================================================================================
class RigSingleWellResultsData : public cvf::Object
{
public:
    RigSingleWellResultsData() { m_isMultiSegmentWell = false; }

    void                              setMultiSegmentWell(bool isMultiSegmentWell);
    bool                              isMultiSegmentWell() const;

    bool                              hasWellResult(size_t resultTimeStepIndex) const;
    size_t                            firstResultTimeStep() const;

    const RigWellResultFrame&         wellResultFrame(size_t resultTimeStepIndex) const;

    void                              computeStaticWellCellPath();
                                      
    void                              computeMappingFromResultTimeIndicesToWellTimeIndices(const std::vector<QDateTime>& resultTimes);
                                      
public:                               
    QString                           m_wellName;
    bool                              m_isMultiSegmentWell;
                                      
    std::vector<size_t>               m_resultTimeStepIndexToWellTimeStepIndex;   // Well result timesteps may differ from result timesteps
    std::vector< RigWellResultFrame > m_wellCellsTimeSteps;
    RigWellResultFrame                m_staticWellCells;
};

