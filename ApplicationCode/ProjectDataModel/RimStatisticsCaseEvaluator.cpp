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
#include "RIStdInclude.h"

#include "RimStatisticsCaseEvaluator.h"
#include "RigReservoirCellResults.h"
#include "RimReservoirView.h"
#include "RimReservoir.h"
#include "RigEclipseCase.h"

//#include "RigEclipseCase.h"
#include <QDebug>
#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
/// An internal class to do the actual computations
//--------------------------------------------------------------------------------------------------
class RimStatisticsEvaluator
{
public:
    RimStatisticsEvaluator(const std::vector<double>& values)
        : m_values(values),
        m_min(HUGE_VAL),
        m_max(-HUGE_VAL),
        m_mean(HUGE_VAL),
        m_dev(HUGE_VAL)
    {
    }


    void getStatistics(double& min, double& max, double& mean, double& dev, double& range)
    {
        evaluate();

        min = m_min;
        max = m_max;
        mean = m_mean;
        dev = m_dev;

        range = m_max - m_min;
    }

private:
    void evaluate()
    {
        double sum = 0.0;
        double sumSquared = 0.0;

        size_t validValueCount = 0;

        for (size_t i = 0; i < m_values.size(); i++)
        {
            double val = m_values[i];
            if (val == HUGE_VAL) continue;

            validValueCount++;

            if (val < m_min) m_min = val;
            if (val > m_max) m_max = val;

            sum += val;
            sumSquared += (val * val);
        }

        if (validValueCount > 0)
        {
            m_mean = sum / validValueCount;


            // http://en.wikipedia.org/wiki/Standard_deviation#Rapid_calculation_methods
            // Running standard deviation

            double s0 = validValueCount;
            double s1 = sum;
            double s2 = sumSquared;

            m_dev = cvf::Math::sqrt( (s0 * s2) - (s1 * s1) ) / s0;
        }
    }


private:
    const std::vector<double>& m_values;

    double m_min;
    double m_max;
    double m_mean;
    double m_dev;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::addNamedResult(RigReservoirCellResults* destinationCellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeUnionCellCount)
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::buildSourceMetaData(RimDefines::ResultCatType resultType, const QString& resultName)
{
    if (m_sourceCases.size() == 0) return;

    std::vector<QDateTime> timeStepDates = m_sourceCases[0]->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->timeStepDates(0);

    for (size_t caseIdx = 1; caseIdx < m_sourceCases.size(); caseIdx++)
    {
        RigEclipseCase* eclipseCase = m_sourceCases.at(caseIdx)->reservoirData();

        RimReservoirCellResultsStorage* matrixResults = m_sourceCases[caseIdx]->results(RifReaderInterface::MATRIX_RESULTS);
        size_t scalarResultIndex = matrixResults->findOrLoadScalarResult(resultType, resultName);
        if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
        {
            size_t scalarResultIndex = matrixResults->cellResults()->addEmptyScalarResult(resultType, resultName, false);
            matrixResults->cellResults()->setTimeStepDates(scalarResultIndex, timeStepDates);

            std::vector< std::vector<double> >& dataValues = matrixResults->cellResults()->cellScalarResults(scalarResultIndex);
            dataValues.resize(timeStepDates.size());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCaseEvaluator::evaluateForResults(const QList<QPair<RimDefines::ResultCatType, QString> >& resultSpecification)
{
    CVF_ASSERT(m_destinationCase);

    size_t activeMatrixCellCount = m_destinationCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->globalActiveCellCount();
    RigReservoirCellResults* matrixResults = m_destinationCase->results(RifReaderInterface::MATRIX_RESULTS);

    for (int i = 0; i < resultSpecification.size(); i++)
    {
        RimDefines::ResultCatType resultType = resultSpecification[i].first;
        QString resultName = resultSpecification[i].second;

        // Special handling if SOIL is asked for
        // Build SGAS/SWAT meta data, SOIL is automatically generated as part of RigReservoirCellResults::findOrLoadScalarResultForTimeStep
        if (resultName.toUpper() == "SOIL")
        {
            size_t swatIndex = m_sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->findScalarResultIndex(resultType, "SWAT");
            if (swatIndex != cvf::UNDEFINED_SIZE_T)
            {
                buildSourceMetaData(resultType, "SWAT");
            }

            size_t sgasIndex = m_sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->findScalarResultIndex(resultType, "SGAS");
            if (sgasIndex != cvf::UNDEFINED_SIZE_T)
            {
                buildSourceMetaData(resultType, "SGAS");
            }
        }
        else
        {
        // Meta info is loaded from disk for first case only
        // Build metadata for all other source cases
        buildSourceMetaData(resultType, resultName);
        }

        QString minResultName = createResultNameMin(resultName);
        QString maxResultName = createResultNameMax(resultName);
        QString meanResultName = createResultNameMean(resultName);
        QString devResultName = createResultNameDev(resultName);
        QString rangeResultName = createResultNameRange(resultName);

        if (activeMatrixCellCount > 0)
        {
            addNamedResult(matrixResults, resultType, minResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, maxResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, meanResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, devResultName, activeMatrixCellCount);
            addNamedResult(matrixResults, resultType, rangeResultName, activeMatrixCellCount);
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

                for (int i = 0; i < resultSpecification.size(); i++)
                {
                    RimDefines::ResultCatType resultType = resultSpecification[i].first;
                    QString resultName = resultSpecification[i].second;

                    size_t dataAccessTimeStepIndex = timeStepIdx;

                    // Always evaluate statistics once, and always use time step index zero
                    if (resultType == RimDefines::STATIC_NATIVE)
                    {
                        if (timeIndicesIdx > 0) continue;

                        dataAccessTimeStepIndex = 0;
                    }

                    // Build data access objects for source scalar results
                    cvf::Collection<cvf::StructGridScalarDataAccess> dataAccesObjectList;
                    for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
                    {
                        RimReservoir* eclipseCase = m_sourceCases.at(caseIdx);

                        size_t scalarResultIndex = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResultForTimeStep(resultType, resultName, dataAccessTimeStepIndex);

                        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->reservoirData()->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
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
                    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectRange = NULL;

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(resultType, createResultNameMin(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMin = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(resultType, createResultNameMax(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMax = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(resultType, createResultNameMean(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectMean = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(resultType, createResultNameDev(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectDev = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
                        }
                    }

                    {
                        size_t scalarResultIndex = matrixResults->findScalarResultIndex(resultType, createResultNameRange(resultName));
                        if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
                        {
                            dataAccessObjectRange = m_destinationCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, dataAccessTimeStepIndex, scalarResultIndex);
                        }
                    }

                    double min, max, mean, dev, range;
                    for (size_t cellIdx = 0; cellIdx < grid->cellCount(); cellIdx++)
                    {
                        std::vector<double> values(dataAccesObjectList.size(), HUGE_VAL);

                        size_t globalGridCellIdx = grid->globalGridCellIndex(cellIdx);
                        if (m_destinationCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->isActive(globalGridCellIdx))
                        {
                            bool foundAnyValidValues = false;
                            for (size_t caseIdx = 0; caseIdx < dataAccesObjectList.size(); caseIdx++)
                            {
                                double val = dataAccesObjectList.at(caseIdx)->cellScalar(cellIdx);
                                values[caseIdx] = val;

                                if (val != HUGE_VAL)
                                {
                                    foundAnyValidValues = true;
                                }
                            }

                            min = HUGE_VAL;
                            max = HUGE_VAL;
                            mean = HUGE_VAL;
                            dev = HUGE_VAL;
                            range = HUGE_VAL;
                            
                            if (foundAnyValidValues)
                            {
                                RimStatisticsEvaluator stat(values);
                                stat.getStatistics(min, max, mean, dev, range);
                            }

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

                            if (dataAccessObjectRange.notNull())
                            {
                                dataAccessObjectRange->setCellScalar(cellIdx, range);
                            }
                        }
                    }
                }
            }

            for (size_t caseIdx = 0; caseIdx < m_sourceCases.size(); caseIdx++)
            {
                RimReservoir* eclipseCase = m_sourceCases.at(caseIdx);

                // When one time step is completed, close all result files.
                // Microsoft note: On Windows, the maximum number of files open at the same time is 512
                // http://msdn.microsoft.com/en-us/library/kdfaxaay%28vs.71%29.aspx
                //
                eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->readerInterface()->close();
            }

            info.setProgress(timeIndicesIdx);
        }
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
RimStatisticsCaseEvaluator::RimStatisticsCaseEvaluator(const std::vector<RimReservoir*>& sourceCases, const std::vector<size_t>& timeStepIndices, const RimStatisticsConfig& statisticsConfig, RigEclipseCase* destinationCase) 
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

