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

struct RigWellResultCell
{
    RigWellResultCell() : 
        m_gridIndex(cvf::UNDEFINED_SIZE_T), 
        m_gridCellIndex(cvf::UNDEFINED_SIZE_T), 
        m_isOpen(false) 
    { }

    size_t m_gridIndex;
    size_t m_gridCellIndex;     //< Index to cell which is included in the well

    bool   m_isOpen;            //< Marks the well as open or closed as of Eclipse simulation
};

struct RigWellResultBranch
{
    RigWellResultBranch() :
        m_branchIndex(cvf::UNDEFINED_SIZE_T),
        m_ertBranchId(-1),
        m_useBranchHeadAsCenterLineIntersectionTop(false)
    {}

    size_t                         m_branchIndex;
    int                            m_ertBranchId;

    std::vector<RigWellResultCell> m_wellCells;
    
    // Grid cell from last connection in outlet segment. For MSW wells, this is either well head or a well result cell in another branch
    // For standard wells, this is always well head.
    RigWellResultCell               m_branchHead;

    bool                            m_useBranchHeadAsCenterLineIntersectionTop;

    // If the outlet segment does not have any connections, it is not possible to populate branch head
    // Instead, use the intersection segment outlet branch index and the depth of this segment to identify intersection point
    // when computing centerline coords in RivWellPipesPartMgr
    int                             m_outletErtBranchId;
    double                          m_connectionDepthOnOutletBranch;
};

class RigWellResultFrame
{
public:
    enum WellProductionType { PRODUCER, OIL_INJECTOR, GAS_INJECTOR, WATER_INJECTOR, UNDEFINED_PRODUCTION_TYPE };

public:
    RigWellResultFrame() :
        m_isOpen(false),
        m_productionType(UNDEFINED_PRODUCTION_TYPE)
    { }

    const RigWellResultCell* findResultCell(size_t gridIndex, size_t gridCellIndex) const
    {
        if (m_wellHead.m_gridCellIndex == gridCellIndex && m_wellHead.m_gridIndex == gridIndex )
        {
            return &m_wellHead;
        }

        for (size_t wb = 0; wb < m_wellResultBranches.size(); ++wb)
        {
            for (size_t wc = 0; wc < m_wellResultBranches[wb].m_wellCells.size(); ++wc)
            {
                if (   m_wellResultBranches[wb].m_wellCells[wc].m_gridCellIndex == gridCellIndex  
                    && m_wellResultBranches[wb].m_wellCells[wc].m_gridIndex == gridIndex  )
                {
                    return &(m_wellResultBranches[wb].m_wellCells[wc]);
                }
            }
        }

        return NULL;
    }

    WellProductionType  m_productionType;
    bool                m_isOpen;
    RigWellResultCell   m_wellHead;
    QDateTime           m_timestamp;
    
    std::vector<RigWellResultBranch> m_wellResultBranches;
};


//==================================================================================================
/// 
//==================================================================================================
class RigSingleWellResultsData : public cvf::Object
{
public:
    RigSingleWellResultsData() { m_isMultiSegmentWell = false; }

    void setMultiSegmentWell(bool isMultiSegmentWell);
    bool isMultiSegmentWell() const;

    bool hasWellResult(size_t resultTimeStepIndex) const;
    size_t firstResultTimeStep() const;

    const RigWellResultFrame& wellResultFrame(size_t resultTimeStepIndex) const;

    void computeStaticWellCellPath();

    void computeMappingFromResultTimeIndicesToWellTimeIndices(const std::vector<QDateTime>& resultTimes);

public:
    QString                             m_wellName;
    bool                                m_isMultiSegmentWell;

    std::vector<size_t>                 m_resultTimeStepIndexToWellTimeStepIndex;   // Well result timesteps may differ from result timesteps
    std::vector< RigWellResultFrame >   m_wellCellsTimeSteps;
    RigWellResultFrame                  m_staticWellCells;


};

