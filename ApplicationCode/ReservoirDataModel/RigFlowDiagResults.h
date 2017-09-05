/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RigFlowDiagResultAddress.h"

#include "RigFlowDiagSolverInterface.h"

#include "RimFlowDiagSolution.h"

#include "cafPdmPointer.h"
#include "cafAppEnum.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"

#include <vector>
#include <map>
#include <string>

class RigFlowDiagResultFrames;
class RigStatisticsDataCache;
class RigActiveCellInfo;

class RigFlowDiagResults: public cvf::Object
{

public:
    enum CellFilter
    {
        CELLS_ACTIVE,
        CELLS_VISIBLE,
        CELLS_COMMUNICATION,
        CELLS_FLOODED,
        CELLS_DRAINED,
    };
    
    typedef caf::AppEnum<CellFilter> CellFilterEnum;
public:
    RigFlowDiagResults(RimFlowDiagSolution* flowSolution, size_t timeStepCount);
    virtual ~RigFlowDiagResults();

    const std::vector<double>*               resultValues(const RigFlowDiagResultAddress& resVarAddr,  size_t frameIndex);
    size_t                                   timeStepCount() { return m_timeStepCount; }
    const RigActiveCellInfo *                activeCellInfo(const RigFlowDiagResultAddress& resVarAddr);

    void                                     minMaxScalarValues (const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* localMin, double* localMax);
    void                                     minMaxScalarValues (const RigFlowDiagResultAddress& resVarAddr, double* globalMin, double* globalMax);
    void                                     posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero);
    void                                     posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero);
    void                                     meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* meanValue);
    void                                     meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* meanValue);
    void                                     p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, double* p10, double* p90);
    void                                     p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* p10, double* p90);
    void                                     sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* sum);
    void                                     sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* sum);
    const std::vector<size_t>&               scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr);
    const std::vector<size_t>&               scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr, int frameIndex);
    const std::vector<int>&                  uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr);
    const std::vector<int>&                  uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex);

    std::pair<double, double>                injectorProducerPairFluxes(const std::string& injTracername, const std::string& prodTracerName, int frameIndex);
    double                                   maxAbsPairFlux(int frameIndex);

    std::vector<int>                         calculatedTimeSteps(RigFlowDiagResultAddress::PhaseSelection phaseSelection);
        

    RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame  flowCharacteristicsResults(int frameIndex,
                                                                                           CellFilter cellSelection,
                                                                                           const std::vector<QString>& tracerNames,
                                                                                           double max_pv_fraction,
                                                                                           double minCommunication,
                                                                                           int maxTof);

    RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame  flowCharacteristicsResults(int frameIndex,
                                                                                           const std::vector<char>& visibleActiveCells,
                                                                                           double max_pv_fraction);

private:
    const std::vector<double>*               findOrCalculateResult (const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    void                                     calculateNativeResultsIfNotPreviouslyAttempted(size_t frameIndex, RigFlowDiagResultAddress::PhaseSelection phaseSelection);

    std::vector<double>*                     calculateDerivedResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);




    std::vector<double>*                     calculateAverageTOFResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    std::vector<double>*                     calculateSumOfFractionsResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    std::vector<double>*                     calculateTracerWithMaxFractionResult(const RigFlowDiagResultAddress &resVarAddr, size_t frameIndex);
    std::vector<double>*                     calculateCommunicationResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    void                                     calculateNumFloodedPV(const RigFlowDiagResultAddress& resVarAddr);


    std::vector<const std::vector<double>* > findResultsForSelectedTracers(const RigFlowDiagResultAddress& resVarAddr, 
                                                                           size_t frameIndex,
                                                                           const std::string& nativeResultName,
                                                                           RimFlowDiagSolution::TracerStatusType wantedTracerType);
    std::vector< std::pair<std::string, const std::vector<double>*> > 
                                             findNamedResultsForSelectedTracers(const RigFlowDiagResultAddress& resVarAddr, 
                                                                                size_t frameIndex, 
                                                                                const std::string& nativeResultName, 
                                                                                RimFlowDiagSolution::TracerStatusType wantedTracerType);

    void                                     calculateSumOfFractionAndFractionMultTOF(size_t activeCellCount,
                                                                                      const std::vector<const std::vector<double> *>& injectorFractions,
                                                                                      const std::vector<const std::vector<double> *>& injectorTOFs,
                                                                                      std::vector<double> *injectorTotalFractions,
                                                                                      std::vector<double> *injectorFractMultTof);

    void                                     calculateSumOfFractions(const std::vector<const std::vector<double> *> &fractions, 
                                                                             size_t activeCellCount, 
                                                                             std::vector<double>* sumOfFractions);

    RigStatisticsDataCache*                  statistics(const RigFlowDiagResultAddress& resVarAddr);
                                             
    RigFlowDiagResultFrames*                 createScalarResult(const RigFlowDiagResultAddress& resVarAddr);
    RigFlowDiagResultFrames*                 findScalarResult  (const RigFlowDiagResultAddress& resVarAddr) ;
    std::vector<double>*                     findScalarResultFrame  (const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);

    //void                                 deleteScalarResult(const RigFlowDiagResultAddress& resVarAddr);

    RigFlowDiagSolverInterface*              solverInterface();

    size_t                                   m_timeStepCount;
    caf::PdmPointer<RimFlowDiagSolution>     m_flowDiagSolution;

    std::vector< std::map<RigFlowDiagResultAddress::PhaseSelection, bool > > m_hasAtemptedNativeResults;

    std::map< RigFlowDiagResultAddress, cvf::ref<RigFlowDiagResultFrames> >  m_resultSets;
    std::map< RigFlowDiagResultAddress, cvf::ref<RigStatisticsDataCache>  >  m_resultStatistics;

    using InjectorProducerCommunicationMap = std::map< std::pair<std::string, std::string>, std::pair<double, double> >;
    std::vector< std::map<RigFlowDiagResultAddress::PhaseSelection, InjectorProducerCommunicationMap> > m_injProdPairFluxCommunicationTimesteps;
};


