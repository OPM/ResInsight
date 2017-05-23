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
#include <vector>
#include <map>
//==================================================================================================
/// 
/// 
//==================================================================================================

class RigMainGrid;
class RigActiveCellInfo;

class RigEclCellIndexCalculator
{
public:
    RigEclCellIndexCalculator(const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo) 
        : m_mainGrid(mainGrid), m_activeCellInfo(activeCellInfo) 
    {}

    size_t resultCellIndex(size_t gridIndex, size_t gridCellIndex) const;

private:
    const RigMainGrid*       m_mainGrid;
    const RigActiveCellInfo* m_activeCellInfo;
};

//==================================================================================================
/// 
/// 
//==================================================================================================

#include <QString>
#include "cvfBase.h"
#include "cvfVector3.h"

struct RigWellResultPoint;

class RigAccWellFlowCalculator
{
 
public:
    RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >&         pipeBranchesCLCoords,
                             const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                             const std::map<QString, const std::vector<double>* >&  tracerCellFractionValues,
                             const RigEclCellIndexCalculator&                       cellIndexCalculator,
                             double smallContribThreshold,
                             bool isProducer);

    RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >&         pipeBranchesCLCoords,
                             const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                             double smallContribThreshold);

    const std::vector<double>&                              connectionNumbersFromTop(size_t branchIdx) const;
    const std::vector<double>&                              accumulatedTracerFlowPrConnection(const QString& tracerName, size_t branchIdx) const;
    const std::vector<double>&                              tracerFlowPrConnection(const QString& tracerName, size_t branchIdx) const;

    const std::vector<double>&                              pseudoLengthFromTop(size_t branchIdx) const;
    const std::vector<double>&                              trueVerticalDepth(size_t branchIdx) const;
    const std::vector<double>&                              accumulatedTracerFlowPrPseudoLength(const QString& tracerName, size_t branchIdx) const;
    const std::vector<double>&                              flowPrPseudoLength( size_t branchIdx) const;
    const std::vector<double>&                              tracerFlowPrPseudoLength(const QString& tracerName, size_t branchIdx) const;


    const std::vector<QString>&                             tracerNames() const { return m_tracerNames;}

    std::vector<std::pair<QString, double> >                totalTracerFractions() const;

private:
    bool                                                    isConnectionFlowConsistent(const RigWellResultPoint &wellCell) const;
    bool                                                    isFlowRateConsistent(double flowRate) const;

    void                                                    calculateAccumulatedFlowPrConnection(size_t branchIdx,
                                                                                                  size_t startConnectionNumberFromTop);
    void                                                    calculateFlowPrPseudoLength(size_t branchIdx,
                                                                                        double startPseudoLengthFromTop);

    std::vector<double>                                     calculateWellCellFlowPrTracer(const RigWellResultPoint& wellCell, 
                                                                                  const std::vector<double>& currentAccumulatedFlowPrTracer ) const;
    void                                                    sortTracers();
    void                                                    groupSmallContributions();

    void                                                    groupSmallTracers(std::map<QString, std::vector<double> >* branchFlowSet, 
                                                                              const std::vector<QString>& tracersToGroup);

    bool                                                    isWellFlowConsistent() const;
    std::vector<double>                                     calculateAccumulatedFractions(const std::vector<double>& accumulatedFlowPrTracer) const;
    std::vector<size_t>                                     wrpToUniqueWrpIndexFromBottom(const std::vector<RigWellResultPoint> &branchCells) const;
    static size_t                                           connectionIndexFromTop( const std::vector<size_t>& resPointToConnectionIndexFromBottom, size_t clSegIdx) ;
    std::vector<size_t>                                     findDownStreamBranchIdxs( const RigWellResultPoint& connectionPoint) const;

    std::vector<std::pair<QString, double> >                totalWellFlowPrTracer() const;


    const std::vector< std::vector <cvf::Vec3d> >&          m_pipeBranchesCLCoords;
    const std::vector< std::vector <RigWellResultPoint> >&  m_pipeBranchesCellIds;
    const std::map<QString, const std::vector<double>* >*   m_tracerCellFractionValues;
    RigEclCellIndexCalculator                               m_cellIndexCalculator;
    std::vector<QString>                                    m_tracerNames;
    double                                                  m_smallContributionsThreshold;
    bool                                                    m_isProducer;

    struct BranchFlow
    {
        std::vector<double>                                 depthValuesFromTop;
        std::vector<double>                                 trueVerticalDepth;
        std::map<QString, std::vector<double> >             accFlowPrTracer;
        std::map<QString, std::vector<double> >             flowPrTracer;
    };                                                      

    void                                                    storeFlowOnDepth(BranchFlow *branchFlow, 
                                                                             double depthValue, 
                                                                             const std::vector<double>& accFlowPrTracer, 
                                                                             const std::vector<double>& flowPrTracer);
    void                                                    storeFlowOnDepthWTvd(BranchFlow *branchFlow,
                                                                                 double depthValue,
                                                                                 double trueVerticalDepth,
                                                                                 const std::vector<double>& accFlowPrTracer,
                                                                                 const std::vector<double>& flowPrTracer);

    std::vector<double>                                     accumulatedDsBranchFlowPrTracer(const BranchFlow &downStreamBranchFlow) const;
    void                                                    addDownStreamBranchFlow(std::vector<double> *accFlowPrTracer,
                                                                                    const std::vector<double>& accBranchFlowPrTracer) const;

    std::vector< BranchFlow >                               m_connectionFlowPrBranch;
    std::vector< BranchFlow >                               m_pseudoLengthFlowPrBranch;

};


