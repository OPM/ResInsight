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

#include "RigStatistics.h"
#include "RigReservoirCellResults.h"

#include <QDebug>
#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::addNamedResult(RigReservoirCellResults* destinationCellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeUnionCellCount)
{
    // Use time step dates from first result in first source case
    CVF_ASSERT(m_sourceCases.size() > 0);

    QList<QDateTime> sourceTimeStepDates = m_sourceCases[0]->results(RifReaderInterface::MATRIX_RESULTS)->timeStepDates(0);
    size_t destinationScalarResultIndex = destinationCellResults->addEmptyScalarResult(resultType, resultName);
    CVF_ASSERT(destinationScalarResultIndex != cvf::UNDEFINED_SIZE_T);

    destinationCellResults->setTimeStepDates(destinationScalarResultIndex, sourceTimeStepDates);
    std::vector< std::vector<double> >& dataValues = destinationCellResults->cellScalarResults(destinationScalarResultIndex);
    dataValues.resize(sourceTimeStepDates.size());

    // Initializes the size of the destination dataset to active union cell count
    for (int i = 0; i < sourceTimeStepDates.size(); i++)
    {
        dataValues[i].resize(activeUnionCellCount, HUGE_VAL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
// See also RifReaderEclipseOutput::readActiveCellInfo()
//--------------------------------------------------------------------------------------------------
void RigStatistics::computeActiveCellUnion()
{
    if (m_sourceCases.size() == 0)
    {
        return;
    }

    RigMainGrid* mainGrid = m_sourceCases[0]->mainGrid();
    CVF_ASSERT(mainGrid);

    m_destinationCase->activeCellInfo()->setGlobalCellCount(mainGrid->cells().size());
    m_destinationCase->activeCellInfo()->setGridCount(mainGrid->gridCount());

    size_t globalActiveMatrixIndex = 0;
    size_t globalActiveFractureIndex = 0;

    for (size_t gridIdx = 0; gridIdx < mainGrid->gridCount(); gridIdx++)
    {
        RigGridBase* grid = mainGrid->gridByIndex(gridIdx);

        std::vector<char> activeM(grid->cellCount(), 0);
        std::vector<char> activeF(grid->cellCount(), 0);

        for (size_t localGridCellIdx = 0; localGridCellIdx < grid->cellCount(); localGridCellIdx++)
        {
            for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
            {
                size_t globalCellIdx = grid->globalGridCellIndex(localGridCellIdx);

                if (activeM[localGridCellIdx] == 0)
                {
                    if (m_sourceCases[caseIdx]->activeCellInfo()->isActiveInMatrixModel(globalCellIdx))
                    {
                        activeM[localGridCellIdx] = 1;
                    }
                }

                if (activeF[localGridCellIdx] == 0)
                {
                    if (m_sourceCases[caseIdx]->activeCellInfo()->isActiveInFractureModel(globalCellIdx))
                    {
                        activeF[localGridCellIdx] = 1;
                    }
                }
            }
        }

        size_t activeMatrixIndex = 0;
        size_t activeFractureIndex = 0;

        for (size_t localGridCellIdx = 0; localGridCellIdx < grid->cellCount(); localGridCellIdx++)
        {
            size_t globalCellIdx = grid->globalGridCellIndex(localGridCellIdx);

            if (activeM[localGridCellIdx] != 0)
            {
                m_destinationCase->activeCellInfo()->setActiveIndexInMatrixModel(globalCellIdx, globalActiveMatrixIndex++);
                activeMatrixIndex++;
            }

            if (activeF[localGridCellIdx] != 0)
            {
                m_destinationCase->activeCellInfo()->setActiveIndexInFractureModel(globalCellIdx, globalActiveFractureIndex++);
                activeFractureIndex++;
            }
        }

        m_destinationCase->activeCellInfo()->setGridActiveCellCounts(gridIdx, activeMatrixIndex, activeFractureIndex);
    }

    m_destinationCase->activeCellInfo()->computeDerivedData();
    m_destinationCase->computeCachedData();
}

QString createResultNameMin(const QString& resultName)  { return resultName + "_MIN"; }
QString createResultNameMax(const QString& resultName)  { return resultName + "_MAX"; }
QString createResultNameMean(const QString& resultName) { return resultName + "_MEAN"; }
QString createResultNameDev(const QString& resultName)  { return resultName + "_DEV"; }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::evaluateStatistics(RimDefines::ResultCatType resultType, const QStringList& resultNames)
{
    CVF_ASSERT(m_destinationCase.notNull());

    computeActiveCellUnion();

    size_t activeMatrixCellCount = m_destinationCase->activeCellInfo()->globalMatrixModelActiveCellCount();
    RigReservoirCellResults* matrixResults = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS);

    foreach(QString resultName, resultNames)
    {
        QString minResultName = createResultNameMin(resultName);
        QString maxResultName = createResultNameMax(resultName);
        QString meanResultName = createResultNameMean(resultName);
        QString devResultName = createResultNameDev(resultName);

        if (activeMatrixCellCount > 0)
        {
            addNamedResult(matrixResults, resultType, minResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, maxResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, meanResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, devResultName, activeMatrixCellCount);
        }
    }

    if (activeMatrixCellCount > 0)
    {
        caf::ProgressInfo info(m_timeStepIndices.size(), "Computing Statistics");

        for (size_t timeIndicesIdx = 0; timeIndicesIdx < m_timeStepIndices.size(); timeIndicesIdx++)
        {
            size_t timeStepIdx = m_timeStepIndices[timeIndicesIdx];

            size_t gridCount = 0;
            for (size_t gridIdx = 0; gridIdx < m_destinationCase->gridCount(); gridIdx++)
            {
                RigGridBase* grid = m_destinationCase->grid(gridIdx);

                foreach(QString resultName, resultNames)
                {
                    // Build data access objects for source scalar results
                    cvf::Collection<cvf::StructGridScalarDataAccess> dataAccesObjectList;
                    for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
                    {
                        RigEclipseCase* eclipseCase = m_sourceCases.at(caseIdx);

                        size_t scalarResultIndex = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResultForTimeStep(resultType, resultName, timeStepIdx);

                        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                        if (dataAccessObject.notNull())
                        {
                            dataAccesObjectList.push_back(dataAccessObject.p());
                        }
                    }

                    // Build data access objects form destination scalar results
                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectMin = NULL;
                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectMax = NULL;
                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectMean = NULL;
                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectDev = NULL;

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, createResultNameMin(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMin = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, createResultNameMax(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMax = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, createResultNameMean(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMean = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, createResultNameDev(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectDev = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                        }
                    }

                    double min, max, mean, dev;
                    std::vector<double> values(dataAccesObjectList.size(), HUGE_VAL);
                    for (size_t cellIdx = 0; cellIdx < grid->cellCount(); cellIdx++)
                    {
                        size_t globalGridCellIdx = grid->globalGridCellIndex(cellIdx);
                        if (m_destinationCase->activeCellInfo()->isActiveInMatrixModel(globalGridCellIdx))
                        {
                            for (size_t caseIdx = 0; caseIdx < dataAccesObjectList.size(); caseIdx++)
                            {
                                double val = dataAccesObjectList.at(caseIdx)->cellScalar(cellIdx);
                                values[caseIdx] = val;
                            }

                            RigStatisticsEvaluator stat(values);
                            stat.getStatistics(min, max, mean, dev);

                            if (dataAccessObjectMin.notNull())
                            {
                                dataAccessObjectMin->setCellScalar(cellIdx, min);
                            }

                            if (dataAccessObjectMax.notNull())
                            {
                                dataAccessObjectMax->setCellScalar(cellIdx, max);
                            }

                            if (dataAccessObjectMean.notNull())
                            {
                                dataAccessObjectMean->setCellScalar(cellIdx, mean);
                            }

                            if (dataAccessObjectDev.notNull())
                            {
                                dataAccessObjectDev->setCellScalar(cellIdx, dev);
                            }
                        }
                    }
                }
            }

            for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
            {
                RigEclipseCase* eclipseCase = m_sourceCases.at(caseIdx);

                // When one time step is completed, close all result files.
                // Microsoft note: On Windows, the maximum number of files open at the same time is 512
                // http://msdn.microsoft.com/en-us/library/kdfaxaay%28vs.71%29.aspx
                //
                eclipseCase->closeReaderInterface();
            }

            info.setProgress(timeIndicesIdx);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::debugOutput(RimDefines::ResultCatType resultType, const QString& resultName, size_t timeStepIdx)
{
    qDebug() << resultName << "timeIdx : " << timeStepIdx;

    RigReservoirCellResults* matrixResults = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS);
    size_t scalarResultIndex = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(resultName);

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
RigStatistics::RigStatistics(cvf::Collection<RigEclipseCase>& sourceCases, const std::vector<size_t>& timeStepIndices, const RigStatisticsConfig& statisticsConfig, RigEclipseCase* destinationCase) :   m_sourceCases(sourceCases),
    m_statisticsConfig(statisticsConfig),
    m_destinationCase(destinationCase),
    m_globalCellCount(0),
    m_timeStepIndices(timeStepIndices)
{
    if (sourceCases.size() > 0)
    {
        m_globalCellCount = sourceCases[0]->mainGrid()->cells().size();
    }
}

