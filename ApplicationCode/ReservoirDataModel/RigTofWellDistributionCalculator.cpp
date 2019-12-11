/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RigTofWellDistributionCalculator.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"
#include "RiaLogging.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimReservoirCellResultsStorage.h"

#include "cvfTrace.h"

#include <map>



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTofWellDistributionCalculator::RigTofWellDistributionCalculator(RimEclipseResultCase* caseToApply, QString targetWellname, size_t timeStepIndex)
{
    CVF_ASSERT(caseToApply);

    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    CVF_ASSERT(eclipseCaseData);

    RimFlowDiagSolution* flowDiagSolution = caseToApply->defaultFlowDiagSolution();
    CVF_ASSERT(flowDiagSolution);

    RigFlowDiagResults* flowDiagResults = flowDiagSolution->flowDiagResults();
    CVF_ASSERT(flowDiagResults);

    const std::vector<double>* porvResults = eclipseCaseData->resultValues(RiaDefines::MATRIX_MODEL, RiaDefines::STATIC_NATIVE, "PORV", 0);
    const std::vector<double>* swatResults = eclipseCaseData->resultValues( RiaDefines::MATRIX_MODEL,   RiaDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    const std::vector<double>* soilResults = eclipseCaseData->resultValues( RiaDefines::MATRIX_MODEL,   RiaDefines::DYNAMIC_NATIVE, "SOIL", timeStepIndex);
    const std::vector<double>* sgasResults = eclipseCaseData->resultValues( RiaDefines::MATRIX_MODEL,   RiaDefines::DYNAMIC_NATIVE, "SGAS", timeStepIndex);
    if (!porvResults)
    {
        return;
    }

    const RigFlowDiagResultAddress resultAddrTof("TOF", RigFlowDiagResultAddress::PhaseSelection::PHASE_ALL, targetWellname.toStdString());
    const RigFlowDiagResultAddress resultAddrFraction("Fraction", RigFlowDiagResultAddress::PhaseSelection::PHASE_ALL, targetWellname.toStdString());
    const std::vector<double>* tofData =                flowDiagResults->resultValues(resultAddrTof, timeStepIndex);
    const std::vector<double>* targetWellFractionData = flowDiagResults->resultValues(resultAddrFraction, timeStepIndex);
    if (!tofData || !targetWellFractionData)
    {
        return;
    }


    const std::map<double, std::vector<size_t>> tofToCellIndicesMap = buildSortedTofToCellIndicesMap(*tofData);

    const std::vector<QString> candidateContributingWellNames = findCandidateContributingWellNames(*flowDiagSolution, targetWellname, timeStepIndex);
    const size_t numContribWells = candidateContributingWellNames.size();

    for (size_t iContribWell = 0; iContribWell < numContribWells; iContribWell++)
    {
        const QString contribWellName = candidateContributingWellNames[iContribWell];

        const RigFlowDiagResultAddress resultAddrContribWellFraction("Fraction", RigFlowDiagResultAddress::PhaseSelection::PHASE_ALL, contribWellName.toStdString());
        const std::vector<double>* contribWellFractionData = flowDiagResults->resultValues(resultAddrContribWellFraction, timeStepIndex);
        if (!contribWellFractionData)
        {
            continue;
        }

        double accumulatedVol_wat = 0;
        double accumulatedVol_oil = 0;
        double accumulatedVol_gas = 0;

        ContribWellEntry contribWellEntry;
        contribWellEntry.name = contribWellName;

        for (auto mapElement : tofToCellIndicesMap)
        {
            const double tofValue = mapElement.first;
            const std::vector<size_t>& cellIndicesArr = mapElement.second;

            for (size_t cellIndex : cellIndicesArr)
            {
                const double porv = porvResults->at(cellIndex);
                const double targetWellFractionVal = targetWellFractionData->at(cellIndex);
                const double contribWellFractionVal = contribWellFractionData->at(cellIndex);
                if (contribWellFractionVal == HUGE_VAL)
                {
                    continue;
                }

                const double volAllPhasesThisCell = porv * targetWellFractionVal*contribWellFractionVal;

                if (swatResults) accumulatedVol_wat += swatResults->at(cellIndex)*volAllPhasesThisCell;
                if (soilResults) accumulatedVol_oil += soilResults->at(cellIndex)*volAllPhasesThisCell;
                if (sgasResults) accumulatedVol_gas += sgasResults->at(cellIndex)*volAllPhasesThisCell;
            }

            contribWellEntry.accumulatedVolAlongTof_wat.push_back(accumulatedVol_wat);
            contribWellEntry.accumulatedVolAlongTof_oil.push_back(accumulatedVol_oil);
            contribWellEntry.accumulatedVolAlongTof_gas.push_back(accumulatedVol_gas);
        }

        m_contributingWells.push_back(contribWellEntry);
    }

    for (auto mapElement : tofToCellIndicesMap)
    {
        const double tofValue = mapElement.first;
        m_tofInIncreasingOrder.push_back(tofValue);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<double, std::vector<size_t>> RigTofWellDistributionCalculator::buildSortedTofToCellIndicesMap(const std::vector<double>& tofData)
{
    std::map<double, std::vector<size_t>> tofToCellIndicesMap;

    for (size_t i = 0; i < tofData.size(); i++)
    {
        const double tofValue = tofData[i];
        if (tofValue == HUGE_VAL)
        {
            continue;
        }

        std::vector<size_t> vectorOfIndexes{ i };
        auto iteratorBoolFromInsertToMap = tofToCellIndicesMap.insert(std::make_pair(tofValue, vectorOfIndexes));
        if (!iteratorBoolFromInsertToMap.second)
        {
            // Map element for this tofValue already exist => we must add the cell index ourselves
            iteratorBoolFromInsertToMap.first->second.push_back(i);
        }
    }

    return tofToCellIndicesMap;
}

//--------------------------------------------------------------------------------------------------
/// Determine name of the the wells that are candidates for contributing in our calculation
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigTofWellDistributionCalculator::findCandidateContributingWellNames(const RimFlowDiagSolution& flowDiagSolution, QString targetWellname, size_t timeStepIndex)
{
    std::vector<QString> candidateWellNames;

    const RimFlowDiagSolution::TracerStatusType targetWellStatus = flowDiagSolution.tracerStatusInTimeStep(targetWellname, timeStepIndex);
    if (targetWellStatus != RimFlowDiagSolution::INJECTOR &&
        targetWellStatus != RimFlowDiagSolution::PRODUCER)
    {
        RiaLogging::warning("Status of target well is neither INJECTOR nor PRODUCER");
        return candidateWellNames;
    }

    const RimFlowDiagSolution::TracerStatusType oppositeStatus = (targetWellStatus == RimFlowDiagSolution::INJECTOR) ? RimFlowDiagSolution::PRODUCER : RimFlowDiagSolution::INJECTOR;

    const std::vector<QString> allWellNames = flowDiagSolution.tracerNames();
    for (QString name : allWellNames)
    {
        const RimFlowDiagSolution::TracerStatusType status = flowDiagSolution.tracerStatusInTimeStep(name, timeStepIndex);
        if (status == oppositeStatus)
        {
            candidateWellNames.push_back(name);
        }
        else if (status == targetWellStatus)
        {
            if (RimFlowDiagSolution::hasCrossFlowEnding(name))
            {
                candidateWellNames.push_back(name);
            }
        }
    }

    return candidateWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigTofWellDistributionCalculator::sortedUniqueTOFValues() const
{
    return m_tofInIncreasingOrder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigTofWellDistributionCalculator::contributingWellCount() const
{
    return m_contributingWells.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigTofWellDistributionCalculator::contributingWellName(size_t contribWellIndex) const
{
    return m_contributingWells[contribWellIndex].name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigTofWellDistributionCalculator::accumulatedPhaseVolumeForContributingWell(RiaDefines::PhaseType phase, size_t contributingWellIndex) const
{
    CVF_ASSERT(contributingWellIndex < m_contributingWells.size());
    const ContribWellEntry& entry = m_contributingWells[contributingWellIndex];
    
    if (phase == RiaDefines::WATER_PHASE)
    {
        return entry.accumulatedVolAlongTof_wat;
    }
    else if (phase == RiaDefines::OIL_PHASE)
    {
        return entry.accumulatedVolAlongTof_oil;
    }
    else 
    {
        return entry.accumulatedVolAlongTof_gas;
    }
}

