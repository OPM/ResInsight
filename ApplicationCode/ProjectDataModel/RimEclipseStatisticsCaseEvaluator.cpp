/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseStatisticsCaseEvaluator.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigEclipseResultInfo.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"
#include "RigStatisticsMath.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafProgressInfo.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCaseEvaluator::addNamedResult(RigCaseCellResultsData* destinationCellResults, RiaDefines::ResultCatType resultType, const QString& resultName, size_t activeUnionCellCount)
{
    // Use time step dates from first result in first source case
    CVF_ASSERT(m_sourceCases.size() > 0);

    std::vector<RigEclipseTimeStepInfo> sourceTimeStepInfos = m_sourceCases[0]->results(RiaDefines::MATRIX_MODEL)->timeStepInfos(0);

    size_t destinationScalarResultIndex = destinationCellResults->findOrCreateScalarResultIndex(resultType, resultName, true);
    CVF_ASSERT(destinationScalarResultIndex != cvf::UNDEFINED_SIZE_T);

    destinationCellResults->setTimeStepInfos(destinationScalarResultIndex, sourceTimeStepInfos);
    std::vector< std::vector<double> >& dataValues = destinationCellResults->cellScalarResults(destinationScalarResultIndex);
    dataValues.resize(sourceTimeStepInfos.size());


    // Initializes the size of the destination dataset to active union cell count
    for (size_t i = 0; i < sourceTimeStepInfos.size(); i++)
    {
        dataValues[i].resize(activeUnionCellCount, HUGE_VAL);
    }
}


QString createResultNameMin(const QString& resultName)  { return resultName + "_MIN"; }
QString createResultNameMax(const QString& resultName) { return resultName + "_MAX"; }
QString createResultNameSum(const QString& resultName)  { return resultName + "_SUM"; }
QString createResultNameMean(const QString& resultName) { return resultName + "_MEAN"; }
QString createResultNameDev(const QString& resultName)  { return resultName + "_DEV"; }
QString createResultNameRange(const QString& resultName)  { return resultName + "_RANGE"; }
QString createResultNamePVal(const QString& resultName, double pValPos)  { return resultName + "_P_" + QString::number(pValPos); }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCaseEvaluator::evaluateForResults(const QList<ResSpec>& resultSpecification)
{
    CVF_ASSERT(m_destinationCase);
    
    // First build the destination result data structures to receive the statistics

    for (int i = 0; i < resultSpecification.size(); i++)
    {
        RiaDefines::PorosityModelType poroModel = resultSpecification[i].m_poroModel;
        RiaDefines::ResultCatType resultType = resultSpecification[i].m_resType;
        QString resultName = resultSpecification[i].m_resVarName;

        size_t activeCellCount = m_destinationCase->activeCellInfo(poroModel)->reservoirActiveCellCount();
        RigCaseCellResultsData* destCellResultsData = m_destinationCase->results(poroModel);

        // Placeholder data used to be created here,
        // this is now moved to RimIdenticalGridCaseGroup::loadMainCaseAndActiveCellInfo()


        // Create new result data structures to contain the statistical values
        std::vector<QString> statisticalResultNames;

        statisticalResultNames.push_back(createResultNameMin(resultName));
        statisticalResultNames.push_back(createResultNameMax(resultName));
        statisticalResultNames.push_back(createResultNameSum(resultName));
        statisticalResultNames.push_back(createResultNameMean(resultName));
        statisticalResultNames.push_back(createResultNameDev(resultName));
        statisticalResultNames.push_back(createResultNameRange(resultName));

        if (m_statisticsConfig.m_calculatePercentiles)
        {
            statisticalResultNames.push_back(createResultNamePVal(resultName, m_statisticsConfig.m_pMinPos));
            statisticalResultNames.push_back(createResultNamePVal(resultName, m_statisticsConfig.m_pMidPos));
            statisticalResultNames.push_back(createResultNamePVal(resultName, m_statisticsConfig.m_pMaxPos));
        }

        if (activeCellCount > 0)
        {
            for (size_t i = 0; i < statisticalResultNames.size(); ++i)
            {
                addNamedResult(destCellResultsData, resultType, statisticalResultNames[i], activeCellCount);
            }
        }
    }

    // Start the loop that calculates the statistics

    caf::ProgressInfo progressInfo(m_timeStepIndices.size(), "Computing Statistics");

    for (size_t timeIndicesIdx = 0; timeIndicesIdx < m_timeStepIndices.size(); timeIndicesIdx++)
    {
        size_t timeStepIdx = m_timeStepIndices[timeIndicesIdx];

        for (size_t gridIdx = 0; gridIdx < m_destinationCase->gridCount(); gridIdx++)
        {
            RigGridBase* grid = m_destinationCase->grid(gridIdx);

            for (int resSpecIdx = 0; resSpecIdx < resultSpecification.size(); resSpecIdx++)
            {
                RiaDefines::PorosityModelType poroModel = resultSpecification[resSpecIdx].m_poroModel;
                RiaDefines::ResultCatType resultType = resultSpecification[resSpecIdx].m_resType;
                QString resultName = resultSpecification[resSpecIdx].m_resVarName;

                size_t activeCellCount = m_destinationCase->activeCellInfo(poroModel)->reservoirActiveCellCount();

                if (activeCellCount == 0) continue;

                RigCaseCellResultsData* destCellResultsData = m_destinationCase->results(poroModel);

                size_t dataAccessTimeStepIndex = timeStepIdx;

                // Always evaluate statistics once, and always use time step index zero
                if (resultType == RiaDefines::STATIC_NATIVE)
                {
                    if (timeIndicesIdx > 0) continue;

                    dataAccessTimeStepIndex = 0;
                }

                // Build data access objects for source scalar results

                cvf::Collection<RigResultAccessor> sourceDataAccessList;
                for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
                {
                    RimEclipseCase* sourceCase = m_sourceCases.at(caseIdx);

                    // Trigger loading of dataset
                    sourceCase->results(poroModel)->findOrLoadScalarResultForTimeStep(resultType, resultName, dataAccessTimeStepIndex);

                    cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromNameAndType(sourceCase->eclipseCaseData(), gridIdx, poroModel, dataAccessTimeStepIndex, resultName, resultType);
                    if (resultAccessor.notNull())
                    {
                        sourceDataAccessList.push_back(resultAccessor.p());
                    }
                }

                // Build data access objects for destination scalar results
                // Find the created result container, if any, and put its resultAccessor into the enum indexed destination collection

                cvf::Collection<RigResultModifier> destinationDataAccessList;
                std::vector<QString> statisticalResultNames(STAT_PARAM_COUNT);

                statisticalResultNames[MIN] = createResultNameMin(resultName);
                statisticalResultNames[MAX] = createResultNameMax(resultName);
                statisticalResultNames[SUM] = createResultNameSum(resultName);
                statisticalResultNames[RANGE] = createResultNameRange(resultName);
                statisticalResultNames[MEAN] = createResultNameMean(resultName);
                statisticalResultNames[STDEV] = createResultNameDev(resultName);
                statisticalResultNames[PMIN] = createResultNamePVal(resultName, m_statisticsConfig.m_pMinPos);
                statisticalResultNames[PMID] = createResultNamePVal(resultName, m_statisticsConfig.m_pMidPos);
                statisticalResultNames[PMAX] = createResultNamePVal(resultName, m_statisticsConfig.m_pMaxPos);

                for (size_t stIdx = 0; stIdx < statisticalResultNames.size(); ++stIdx)
                {
                    size_t scalarResultIndex = destCellResultsData->findScalarResultIndex(resultType, statisticalResultNames[stIdx]);

                    cvf::ref<RigResultModifier> resultModifier = RigResultModifierFactory::createResultModifier(m_destinationCase, grid->gridIndex(), poroModel, dataAccessTimeStepIndex, scalarResultIndex);
                    destinationDataAccessList.push_back(resultModifier.p());
                }

                 std::vector<double> statParams(STAT_PARAM_COUNT, HUGE_VAL);
                 std::vector<double> values(sourceDataAccessList.size(), HUGE_VAL);
                // Loop over the cells in the grid, get the case values, and calculate the cell statistics 
#pragma omp parallel for schedule(dynamic) firstprivate(statParams, values)
                for (int cellIdx = 0; static_cast<size_t>(cellIdx) < grid->cellCount(); cellIdx++)
                {

                    size_t reservoirCellIndex = grid->reservoirCellIndex(cellIdx);
                    if (m_destinationCase->activeCellInfo(poroModel)->isActive(reservoirCellIndex))
                    {
                        // Extract the cell values from each of the cases and assemble them into one vector

                        bool foundAnyValidValues = false;
                        for (size_t caseIdx = 0; caseIdx < sourceDataAccessList.size(); caseIdx++)
                        {
                            double val = sourceDataAccessList.at(caseIdx)->cellScalar(cellIdx);

                            // Replace huge_val with zero in the statistical computation for the following case
                            if (m_useZeroAsInactiveCellValue || resultName.toUpper() == "ACTNUM")
                            {
                                if (m_identicalGridCaseGroup->unionOfActiveCells(poroModel)->isActive(reservoirCellIndex) &&
                                    val == HUGE_VAL)
                                {
                                    val = 0.0;
                                }
                            }
                            values[caseIdx] = val;

                            if (val != HUGE_VAL)
                            {
                                foundAnyValidValues = true;
                            }
                        }


                        // Do the real statistics calculations
                       

                        if (foundAnyValidValues)
                        {
                            RigStatisticsMath::calculateBasicStatistics(values, &statParams[MIN], &statParams[MAX], &statParams[SUM], &statParams[RANGE], &statParams[MEAN], &statParams[STDEV]);

                            // Calculate percentiles
                            if (m_statisticsConfig.m_calculatePercentiles )
                            {
                                if (m_statisticsConfig.m_pValMethod == RimEclipseStatisticsCase::NEAREST_OBSERVATION)
                                {
                                    std::vector<double> pValPoss;
                                    pValPoss.push_back(m_statisticsConfig.m_pMinPos);
                                    pValPoss.push_back(m_statisticsConfig.m_pMidPos);
                                    pValPoss.push_back(m_statisticsConfig.m_pMaxPos);
                                    std::vector<double> pVals = RigStatisticsMath::calculateNearestRankPercentiles(values, pValPoss);
                                    statParams[PMIN] = pVals[0];
                                    statParams[PMID] = pVals[1];
                                    statParams[PMAX] = pVals[2];
                                }
                                else if (m_statisticsConfig.m_pValMethod == RimEclipseStatisticsCase::HISTOGRAM_ESTIMATED)
                                {
                                    std::vector<size_t> histogram;
                                    RigHistogramCalculator histCalc(statParams[MIN], statParams[MAX], 100, &histogram);
                                    histCalc.addData(values);
                                    statParams[PMIN] = histCalc.calculatePercentil(m_statisticsConfig.m_pMinPos);
                                    statParams[PMID] = histCalc.calculatePercentil(m_statisticsConfig.m_pMidPos);
                                    statParams[PMAX] = histCalc.calculatePercentil(m_statisticsConfig.m_pMaxPos);
                                }
                                else if (m_statisticsConfig.m_pValMethod == RimEclipseStatisticsCase::INTERPOLATED_OBSERVATION)
                                {
                                    std::vector<double> pValPoss;
                                    pValPoss.push_back(m_statisticsConfig.m_pMinPos);
                                    pValPoss.push_back(m_statisticsConfig.m_pMidPos);
                                    pValPoss.push_back(m_statisticsConfig.m_pMaxPos);
                                    std::vector<double> pVals = RigStatisticsMath::calculateInterpolatedPercentiles(values, pValPoss);
                                    statParams[PMIN] = pVals[0];
                                    statParams[PMID] = pVals[1];
                                    statParams[PMAX] = pVals[2];
                                }
                                else
                                {
                                    CVF_ASSERT(false);
                                }
                            }
                        }

                        // Set the results into the results data structures

                        for (size_t stIdx = 0; stIdx < statParams.size(); ++stIdx)
                        {
                            if (destinationDataAccessList[stIdx].notNull())
                            {
                                destinationDataAccessList[stIdx]->setCellScalar(cellIdx, statParams[stIdx]);
                            }
                        }
                    }
                }
            }
        }

        // When one time step is completed, free memory and clean up
        // Microsoft note: On Windows, the maximum number of files open at the same time is 512
        // http://msdn.microsoft.com/en-us/library/kdfaxaay%28vs.71%29.aspx

        for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
        {
            RimEclipseCase* eclipseCase = m_sourceCases.at(caseIdx);

            if (!eclipseCase->reservoirViews.size())
            {
                eclipseCase->results(RiaDefines::MATRIX_MODEL)->freeAllocatedResultsData();
                eclipseCase->results(RiaDefines::FRACTURE_MODEL)->freeAllocatedResultsData();
            }

        }

        progressInfo.setProgress(timeIndicesIdx);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCaseEvaluator::RimEclipseStatisticsCaseEvaluator(const std::vector<RimEclipseCase*>& sourceCases, const std::vector<size_t>& timeStepIndices, const RimStatisticsConfig& statisticsConfig, RigEclipseCaseData* destinationCase, RimIdenticalGridCaseGroup* identicalGridCaseGroup)
    :   m_sourceCases(sourceCases),
    m_statisticsConfig(statisticsConfig),
    m_destinationCase(destinationCase),
    m_reservoirCellCount(0),
    m_timeStepIndices(timeStepIndices),
    m_identicalGridCaseGroup(identicalGridCaseGroup),
    m_useZeroAsInactiveCellValue(false)
{
    if (sourceCases.size() > 0)
    {
        m_reservoirCellCount = sourceCases[0]->eclipseCaseData()->mainGrid()->globalCellArray().size();
    }

    CVF_ASSERT(m_destinationCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCaseEvaluator::useZeroAsValueForInActiveCellsBasedOnUnionOfActiveCells()
{
    m_useZeroAsInactiveCellValue = true;
}

