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

#include <map>



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTofWellDistributionCalculator::RigTofWellDistributionCalculator(RimEclipseResultCase* caseToApply, QString targetWellname, size_t timeStepIndex, RiaDefines::PhaseType phase)
{
    CVF_ASSERT(caseToApply);

    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    CVF_ASSERT(eclipseCaseData);

    RimFlowDiagSolution* flowDiagSolution = caseToApply->defaultFlowDiagSolution();
    CVF_ASSERT(flowDiagSolution);

    RigFlowDiagResults* flowDiagResults = flowDiagSolution->flowDiagResults();
    CVF_ASSERT(flowDiagResults);

    const std::vector<double>* porvResults = eclipseCaseData->resultValues(RiaDefines::MATRIX_MODEL, RiaDefines::STATIC_NATIVE, "PORV", 0);
    if (!porvResults)
    {
        return;
    }

    QString phaseResultName;
    if      (phase == RiaDefines::WATER_PHASE) phaseResultName = "SWAT";
    else if (phase == RiaDefines::OIL_PHASE)   phaseResultName = "SOIL";
    else if (phase == RiaDefines::GAS_PHASE)   phaseResultName = "SGAS";
    const std::vector<double>* phaseResults = eclipseCaseData->resultValues(RiaDefines::MATRIX_MODEL, RiaDefines::DYNAMIC_NATIVE, phaseResultName, timeStepIndex);
    if (!phaseResults)
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

        double accumulatedVolForSpecifiedPhase = 0;

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
                accumulatedVolForSpecifiedPhase += phaseResults->at(cellIndex)*volAllPhasesThisCell;
            }

            contribWellEntry.accumulatedVolAlongTof.push_back(accumulatedVolForSpecifiedPhase);
        }

        if (accumulatedVolForSpecifiedPhase > 0)
        {
            m_contributingWells.push_back(contribWellEntry);
        }
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
void RigTofWellDistributionCalculator::groupSmallContributions(double smallContribThreshold)
{
    if (m_tofInIncreasingOrder.size() == 0)
    {
        return;
    }

    double totalVolAtLastTof = 0;
    for (const ContribWellEntry& entry : m_contributingWells)
    {
        totalVolAtLastTof += entry.accumulatedVolAlongTof.back();
    }

    std::vector<ContribWellEntry> sourceEntryArr = std::move(m_contributingWells);

    ContribWellEntry groupingEntry;
    groupingEntry.name = "Other";
    groupingEntry.accumulatedVolAlongTof.resize(m_tofInIncreasingOrder.size(), 0);
    bool anySmallContribsDetected = false;

    for (const ContribWellEntry& sourceEntry : sourceEntryArr)
    {
        const double volAtLastTof = sourceEntry.accumulatedVolAlongTof.back();
        if (volAtLastTof >= totalVolAtLastTof*smallContribThreshold)
        {
            m_contributingWells.push_back(sourceEntry);
        }
        else
        {
            for (size_t i = 0; i < groupingEntry.accumulatedVolAlongTof.size(); i++)
            {
                groupingEntry.accumulatedVolAlongTof[i] += sourceEntry.accumulatedVolAlongTof[i];
            }
            anySmallContribsDetected = true;
        }
    }

    if (anySmallContribsDetected)
    {
        m_contributingWells.push_back(groupingEntry);
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

        // Also filter out special TOF values greater than 73000 days (~200 years)
        if (tofValue > 73000.0)
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
const std::vector<double>& RigTofWellDistributionCalculator::accumulatedVolumeForContributingWell(size_t contributingWellIndex) const
{
    CVF_ASSERT(contributingWellIndex < m_contributingWells.size());
    const ContribWellEntry& entry = m_contributingWells[contributingWellIndex];
    return entry.accumulatedVolAlongTof;
}

