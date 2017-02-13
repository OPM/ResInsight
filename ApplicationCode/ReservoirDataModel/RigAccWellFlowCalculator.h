/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

//==================================================================================================
/// 
/// 
//==================================================================================================
#include "RigActiveCellInfo.h"
#include "RigMainGrid.h"
#include "RigFlowDiagResults.h"

struct RigWellResultPoint;

class RigEclCellIndexCalculator
{
public:
    RigEclCellIndexCalculator(const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo) 
        : m_mainGrid(mainGrid), m_activeCellInfo(activeCellInfo) 
    {}

    size_t resultCellIndex(size_t gridIndex, size_t gridCellIndex) const
    {
        const RigGridBase* grid = m_mainGrid->gridByIndex(gridIndex);
        size_t reservoirCellIndex = grid->reservoirCellIndex(gridCellIndex);

        return m_activeCellInfo->cellResultIndex(reservoirCellIndex);
    }

private:
    const RigMainGrid*       m_mainGrid;
    const RigActiveCellInfo* m_activeCellInfo;
};

//==================================================================================================
/// 
/// 
//==================================================================================================

class RigAccWellFlowCalculator
{
 
public:
    RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                             const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                             const std::map<QString, const std::vector<double>* >& tracerCellFractionValues,
                             const RigEclCellIndexCalculator cellIndexCalculator);

    RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                             const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds);

    const std::vector<double>&   accumulatedTotalFlowPrConnection( size_t branchIdx);// const;
    const std::vector<double>&   accumulatedTracerFlowPrConnection(const QString& tracerName, size_t branchIdx);// const;
    const std::vector<size_t>&   connectionNumbersFromTop(size_t branchIdx) const;
    const std::vector<QString>&  tracerNames() const { return m_tracerNames;}
private:

    void                         calculateAccumulatedFlowPrConnection( size_t branchIdx, size_t startConnectionNumberFromTop);
    std::vector<size_t>          wrpToConnectionIndexFromBottom( const std::vector<RigWellResultPoint> &branchCells);
    static size_t                connectionIndexFromTop( const std::vector<size_t>& resPointToConnectionIndexFromBottom, size_t clSegIdx);
    std::vector<size_t>          findDownstreamBranchIdxs( const RigWellResultPoint& connectionPoint);
    void                         sortTracers();

    const std::vector< std::vector <cvf::Vec3d> >&         m_pipeBranchesCLCoords;
    const std::vector< std::vector <RigWellResultPoint> >& m_pipeBranchesCellIds;
    const std::map<QString, const std::vector<double>* >*  m_tracerCellFractionValues;
    RigEclCellIndexCalculator                              m_cellIndexCalculator;
    std::vector<QString>                                   m_tracerNames;

    struct BranchResult
    {
        std::vector<size_t>                               connectionNumbersFromTop;
        std::map<QString, std::vector<double> >           accConnFlowFractionsPrTracer;
    };

    std::vector< BranchResult >                           m_accConnectionFlowPrBranch;
};


