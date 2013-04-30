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
#include "RiaStdInclude.h"

#include "RimStatisticsCaseEvaluator.h"
#include "RigCaseCellResultsData.h"
#include "RimReservoirView.h"
#include "RimCase.h"
#include "RigCaseData.h"
#include "RigStatisticsMath.h"

//#include "RigCaseData.h"
#include <QDebug>
#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::addNamedResult(RigCaseCellResultsData* destinationCellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeUnionCellCount)
{
    // Use time step dates from first result in first source case
    CVF_ASSERT(m_sourceCases.size() > 0);

    std::vector<QDateTime> sourceTimeStepDates = m_sourceCases[0]->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->timeStepDates(0);

    size_t destinationScalarResultIndex = destinationCellResults->addEmptyScalarResult(resultType, resultName, true);
    CVF_ASSERT(destinationScalarResultIndex != cvf::UNDEFINED_SIZE_T);

    destinationCellResults->setTimeStepDates(destinationScalarResultIndex, sourceTimeStepDates);
    std::vector< std::vector<double> >& dataValues = destinationCellResults->cellScalarResults(destinationScalarResultIndex);
    dataValues.resize(sourceTimeStepDates.size());


    // Initializes the size of the destination dataset to active union cell count
    for (size_t i = 0; i < sourceTimeStepDates.size(); i++)
    {
        dataValues[i].resize(activeUnionCellCount, HUGE_VAL);
    }
}


QString createResultNameMin(const QString& resultName)  { return resultName + "_MIN"; }
QString createResultNameMax(const QString& resultName)  { return resultName + "_MAX"; }
QString createResultNameMean(const QString& resultName) { return resultName + "_MEAN"; }
QString createResultNameDev(const QString& resultName)  { return resultName + "_DEV"; }
QString createResultNameRange(const QString& resultName)  { return resultName + "_RANGE"; }
QString createResultNamePVal(const QString& resultName, double pValPos)  { return resultName + "_P_" + QString::number(pValPos); }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::buildSourceMetaData(RifReaderInterface::PorosityModelResultType poroModel, RimDefines::ResultCatType resultType, const QString& resultName)
{
    if (m_sourceCases.size() == 0) return;

    std::vector<QDateTime> timeStepDates = m_sourceCases[0]->results(poroModel)->cellResults()->timeStepDates(0);

    for (size_t caseIdx = 1; caseIdx < m_sourceCases.size(); caseIdx++)
    {
        RimReservoirCellResultsStorage* cellResultsStorage = m_sourceCases[caseIdx]->results(poroModel);
        size_t scalarResultIndex = cellResultsStorage->findOrLoadScalarResult(resultType, resultName);
        if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
        {
            size_t scalarResultIndex = cellResultsStorage->cellResults()->addEmptyScalarResult(resultType, resultName, false);
            cellResultsStorage->cellResults()->setTimeStepDates(scalarResultIndex, timeStepDates);

            std::vector< std::vector<double> >& dataValues = cellResultsStorage->cellResults()->cellScalarResults(scalarResultIndex);
            dataValues.resize(timeStepDates.size());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::evaluateForResults(const QList<ResSpec>& resultSpecification)
{
    CVF_ASSERT(m_destinationCase);
    
    // First build the destination result data structures to receive the statistics

    for (int i = 0; i < resultSpecification.size(); i++)
    {
        RifReaderInterface::PorosityModelResultType poroModel = resultSpecification[i].m_poroModel;
        RimDefines::ResultCatType resultType = resultSpecification[i].m_resType;
        QString resultName = resultSpecification[i].m_resVarName;

        size_t activeCellCount = m_destinationCase->activeCellInfo(poroModel)->globalActiveCellCount();
        RigCaseCellResultsData* destCellResultsData = m_destinationCase->results(poroModel);

        // Special handling if SOIL is asked for
        // Build SGAS/SWAT meta data, SOIL is automatically generated as part of RigCaseCellResultsData::findOrLoadScalarResultForTimeStep
        if (resultName.toUpper() == "SOIL")
        {
            size_t swatIndex = m_sourceCases.at(0)->results(poroModel)->cellResults()->findScalarResultIndex(resultType, "SWAT");
            if (swatIndex != cvf::UNDEFINED_SIZE_T)
            {
                buildSourceMetaData(poroModel, resultType, "SWAT");
            }

            size_t sgasIndex = m_sourceCases.at(0)->results(poroModel)->cellResults()->findScalarResultIndex(resultType, "SGAS");
            if (sgasIndex != cvf::UNDEFINED_SIZE_T)
            {
                buildSourceMetaData(poroModel, resultType, "SGAS");
            }
        }
        else
        {
            // Meta info is loaded from disk for first case only
            // Build metadata for all other source cases
            buildSourceMetaData(poroModel, resultType, resultName);
        }

        // Create new result data structures to contain the statistical values
        std::vector<QString> statisticalResultNames;

        statisticalResultNames.push_back(createResultNameMin(resultName));
        statisticalResultNames.push_back(createResultNameMax(resultName));
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

            for (int i = 0; i < resultSpecification.size(); i++)
            {
                RifReaderInterface::PorosityModelResultType poroModel = resultSpecification[i].m_poroModel;
                RimDefines::ResultCatType resultType = resultSpecification[i].m_resType;
                QString resultName = resultSpecification[i].m_resVarName;

                size_t activeCellCount = m_destinationCase->activeCellInfo(poroModel)->globalActiveCellCount();

                if (activeCellCount == 0) continue;

                RigCaseCellResultsData* destCellResultsData = m_destinationCase->results(poroModel);

                size_t dataAccessTimeStepIndex = timeStepIdx;

                // Always evaluate statistics once, and always use time step index zero
                if (resultType == RimDefines::STATIC_NATIVE)
                {
                    if (timeIndicesIdx > 0) continue;

                    dataAccessTimeStepIndex = 0;
                }

                // Build data access objects for source scalar results

                cvf::Collection<cvf::StructGridScalarDataAccess> sourceDataAccessList;
                for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
                {
                    RimCase* eclipseCase = m_sourceCases.at(caseIdx);

                    size_t scalarResultIndex = eclipseCase->results(poroModel)->findOrLoadScalarResultForTimeStep(resultType, resultName, dataAccessTimeStepIndex);

                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->reservoirData()->dataAccessObject(grid, poroModel, dataAccessTimeStepIndex, scalarResultIndex);
                    if (dataAccessObject.notNull())
                    {
                        sourceDataAccessList.push_back(dataAccessObject.p());
                    }
                }

                // Build data access objects for destination scalar results
                // Find the created result container, if any, and put its dataAccessObject into the enum indexed destination collection

                cvf::Collection<cvf::StructGridScalarDataAccess> destinationDataAccessList;
                std::vector<QString> statisticalResultNames(STAT_PARAM_COUNT);

                statisticalResultNames[MIN] = createResultNameMin(resultName);
                statisticalResultNames[MAX] = createResultNameMax(resultName);
                statisticalResultNames[RANGE] = createResultNameRange(resultName);
                statisticalResultNames[MEAN] = createResultNameMean(resultName);
                statisticalResultNames[STDEV] = createResultNameDev(resultName);
                statisticalResultNames[PMIN] = createResultNamePVal(resultName, m_statisticsConfig.m_pMinPos);
                statisticalResultNames[PMID] = createResultNamePVal(resultName, m_statisticsConfig.m_pMidPos);
                statisticalResultNames[PMAX] = createResultNamePVal(resultName, m_statisticsConfig.m_pMaxPos);

                for (size_t stIdx = 0; stIdx < statisticalResultNames.size(); ++stIdx)
                {
                    size_t scalarResultIndex = destCellResultsData->findScalarResultIndex(resultType, statisticalResultNames[stIdx]);
                    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        destinationDataAccessList.push_back(m_destinationCase->dataAccessObject(grid, poroModel, dataAccessTimeStepIndex, scalarResultIndex).p());
                    }
                    else
                    {
                        destinationDataAccessList.push_back(NULL);
                    }
                }

                // Loop over the cells in the grid, get the case values, and calculate the cell statistics 

                for (size_t cellIdx = 0; cellIdx < grid->cellCount(); cellIdx++)
                {

                    size_t globalGridCellIdx = grid->globalGridCellIndex(cellIdx);
                    if (m_destinationCase->activeCellInfo(poroModel)->isActive(globalGridCellIdx))
                    {
                        // Extract the cell values from each of the cases and assemble them into one vector

                        std::vector<double> values(sourceDataAccessList.size(), HUGE_VAL);

                        bool foundAnyValidValues = false;
                        for (size_t caseIdx = 0; caseIdx < sourceDataAccessList.size(); caseIdx++)
                        {
                            double val = sourceDataAccessList.at(caseIdx)->cellScalar(cellIdx);
                            values[caseIdx] = val;

                            if (val != HUGE_VAL)
                            {
                                foundAnyValidValues = true;
                            }
                        }

                        // Do the real statistics calculations
                        std::vector<double> statParams(STAT_PARAM_COUNT, HUGE_VAL);

                        if (foundAnyValidValues)
                        {
                            RigStatisticsMath::calculateBasicStatistics(values, &statParams[MIN], &statParams[MAX], &statParams[RANGE], &statParams[MEAN], &statParams[STDEV]);

                            // Calculate percentiles
                            if (m_statisticsConfig.m_calculatePercentiles )
                            {
                                if (m_statisticsConfig.m_pValMethod == RimStatisticsCase::NEAREST_OBSERVATION)
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
                                else if (m_statisticsConfig.m_pValMethod == RimStatisticsCase::HISTOGRAM_ESTIMATED)
                                {
                                    std::vector<size_t> histogram;
                                    RigHistogramCalculator histCalc(statParams[MIN], statParams[MAX], 100, &histogram);
                                    histCalc.addData(values);
                                    statParams[PMIN] = histCalc.calculatePercentil(m_statisticsConfig.m_pMinPos);
                                    statParams[PMID] = histCalc.calculatePercentil(m_statisticsConfig.m_pMidPos);
                                    statParams[PMAX] = histCalc.calculatePercentil(m_statisticsConfig.m_pMaxPos);
                                }
                                else if (m_statisticsConfig.m_pValMethod == RimStatisticsCase::INTERPOLATED_OBSERVATION)
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

        // When one time step is completed, close all result files.
        // Microsoft note: On Windows, the maximum number of files open at the same time is 512
        // http://msdn.microsoft.com/en-us/library/kdfaxaay%28vs.71%29.aspx

        for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
        {
            RimCase* eclipseCase = m_sourceCases.at(caseIdx);
            eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->readerInterface()->close();
            eclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->readerInterface()->close();
        }

        progressInfo.setProgress(timeIndicesIdx);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::debugOutput(RimDefines::ResultCatType resultType, const QString& resultName, size_t timeStepIdx)
{
    CVF_ASSERT(m_destinationCase);

    qDebug() << resultName << "timeIdx : " << timeStepIdx;

    size_t scalarResultIndex = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(resultType, resultName);

    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = m_destinationCase->dataAccessObject(m_destinationCase->mainGrid(), RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
    if (dataAccessObject.isNull()) return;

    for (size_t cellIdx = 0; cellIdx < m_globalCellCount; cellIdx++)
    {
        qDebug() << dataAccessObject->cellScalar(cellIdx);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCaseEvaluator::RimStatisticsCaseEvaluator(const std::vector<RimCase*>& sourceCases, const std::vector<size_t>& timeStepIndices, const RimStatisticsConfig& statisticsConfig, RigCaseData* destinationCase) 
    :   m_sourceCases(sourceCases),
    m_statisticsConfig(statisticsConfig),
    m_destinationCase(destinationCase),
    m_globalCellCount(0),
    m_timeStepIndices(timeStepIndices)
{
    if (sourceCases.size() > 0)
    {
        m_globalCellCount = sourceCases[0]->reservoirData()->mainGrid()->cells().size();
    }

    CVF_ASSERT(m_destinationCase);
}

