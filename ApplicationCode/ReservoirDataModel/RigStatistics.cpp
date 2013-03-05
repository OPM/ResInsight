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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::addNamedResult(RigReservoirCellResults* cellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeCellCount)
{
    // Use time step dates from first result in first source case
    CVF_ASSERT(m_sourceCases.size() > 0);
    QList<QDateTime> timeStepDates = m_sourceCases[0]->results(RifReaderInterface::MATRIX_RESULTS)->timeStepDates(0);

    size_t resultIndexMin = cellResults->addEmptyScalarResult(resultType, resultName);
    cellResults->setTimeStepDates(resultIndexMin, timeStepDates);
    
    std::vector< std::vector<double> >& dataValues = cellResults->cellScalarResults(resultIndexMin);
    dataValues.resize(timeStepDates.size());

    for (size_t i = 0; i < timeStepDates.size(); i++)
    {
        dataValues[i].resize(activeCellCount, HUGE_VAL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::computeActiveCellUnion()
{
    // Early exit if active cell union is already computed
    if (m_destinationCase->activeCellInfo()->globalFractureModelActiveCellCount() + 
        m_destinationCase->activeCellInfo()->globalFractureModelActiveCellCount() > 0)
    {
        return;
    }

    std::vector<char> activeM(m_globalCellCount, 0);
    std::vector<char> activeF(m_globalCellCount, 0);

    for (size_t cellIdx = 0; cellIdx < m_globalCellCount; cellIdx++)
    {
        for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
        {
            if (activeM[cellIdx] == 0)
            {
                if (m_sourceCases[caseIdx]->activeCellInfo()->isActiveInMatrixModel(cellIdx))
                {
                    activeM[cellIdx] = 1;
                }
            }

            if (activeF[cellIdx] == 0)
            {
                if (m_sourceCases[caseIdx]->activeCellInfo()->isActiveInFractureModel(cellIdx))
                {
                    activeF[cellIdx] = 1;
                }
            }
        }
    }

    m_destinationCase->activeCellInfo()->setGlobalCellCount(m_globalCellCount);

    size_t activeMIndex = 0;
    size_t activeFIndex = 0;

    for (size_t cellIdx = 0; cellIdx < m_globalCellCount; cellIdx++)
    {
        if (activeM[cellIdx] != 0)
        {
            m_destinationCase->activeCellInfo()->setActiveIndexInMatrixModel(cellIdx, activeMIndex++);
        }

        if (activeF[cellIdx] != 0)
        {
            m_destinationCase->activeCellInfo()->setActiveIndexInFractureModel(cellIdx, activeFIndex++);
        }
    }

    m_destinationCase->activeCellInfo()->computeDerivedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatistics::evaluateStatistics(RimDefines::ResultCatType resultType, const QString& resultName)
{
    CVF_ASSERT(m_destinationCase.notNull());

    computeActiveCellUnion();

    QString minResultName = resultName + "_MIN";
    QString maxResultName = resultName + "_MAX";
    QString meanResultName = resultName + "_MEAN";
    QString devResultName = resultName + "_DEV";

    RigReservoirCellResults* matrixResults = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS);

    size_t activeMatrixCellCount = m_destinationCase->activeCellInfo()->globalMatrixModelActiveCellCount();
    if (activeMatrixCellCount > 0)
    {
        addNamedResult(matrixResults, resultType, minResultName, activeMatrixCellCount);
        addNamedResult(matrixResults, resultType, maxResultName, activeMatrixCellCount);
        addNamedResult(matrixResults, resultType, meanResultName, activeMatrixCellCount);
        addNamedResult(matrixResults, resultType, devResultName, activeMatrixCellCount);
    }

    if (activeMatrixCellCount > 0)
    {
        for (size_t timeIndicesIdx = 0; timeIndicesIdx < m_timeStepIndices.size(); timeIndicesIdx++)
        {
            size_t timeStepIdx = m_timeStepIndices[timeIndicesIdx];

            size_t gridCount = 0;
            for (size_t gridIdx = 0; gridIdx < m_destinationCase->gridCount(); gridIdx++)
            {
                RigGridBase* grid = m_destinationCase->grid(gridIdx);

                // Build data access objects for source scalar results
                cvf::Collection<cvf::StructGridScalarDataAccess> dataAccesObjectList;
                for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
                {
                    RigEclipseCase* eclipseCase = m_sourceCases.at(caseIdx);

                    size_t scalarResultIndex = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(resultName);

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
                    size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, minResultName);
                    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        dataAccessObjectMin = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                    }
                }

                {
                    size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, maxResultName);
                    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        dataAccessObjectMax = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                    }
                }

                {
                    size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, meanResultName);
                    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        dataAccessObjectMean = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, timeStepIdx, scalarResultIndex);
                    }
                }

                {
                    size_t scalarResultIndex = matrixResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, devResultName);
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
