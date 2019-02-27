/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RigEclipseCrossPlotDataExtractor.h"

#include "RiaQDateTimeTools.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultBinSorter.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include <memory>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCrossPlotResult RigEclipseCrossPlotDataExtractor::extract(RigEclipseCaseData*                 caseData,
                                                                    int                                 resultTimeStep,
                                                                    const RigEclipseResultAddress&      xAddress,
                                                                    const RigEclipseResultAddress&      yAddress,
                                                                    RigGridCrossPlotCurveCategorization categorizationType,
                                                                    const RigEclipseResultAddress&      catAddress,
                                                                    int                                 categoryBinCount,
                                                                    std::map<int, cvf::UByteArray>      timeStepCellVisibilityMap)
{
    RigEclipseCrossPlotResult result;
    RigEclipseCrossPlotResult::CategorySamplesMap& categorySamplesMap = result.categorySamplesMap;
    RigEclipseCrossPlotResult::CategoryNameMap&    categoryNameMap    = result.categoryNameMap;

    RigCaseCellResultsData* resultData           = caseData->results(RiaDefines::MATRIX_MODEL);
    RigFormationNames*      activeFormationNames = resultData->activeFormationNames();

    std::unique_ptr<RigEclipseResultBinSorter> catBinSorter;
    const std::vector<std::vector<double>>*    catValuesForAllSteps = nullptr;

    if (xAddress.isValid() && yAddress.isValid())
    {
        RigActiveCellInfo* activeCellInfo = resultData->activeCellInfo();
        const RigMainGrid* mainGrid       = caseData->mainGrid();

        resultData->ensureKnownResultLoaded(xAddress);
        resultData->ensureKnownResultLoaded(yAddress);

        const std::vector<std::vector<double>>& xValuesForAllSteps = resultData->cellScalarResults(xAddress);
        const std::vector<std::vector<double>>& yValuesForAllSteps = resultData->cellScalarResults(yAddress);

        if (categorizationType == RESULT_CATEGORIZATION && catAddress.isValid())
        {
            resultData->ensureKnownResultLoaded(catAddress);
            catValuesForAllSteps = &resultData->cellScalarResults(catAddress);
            catBinSorter.reset(new RigEclipseResultBinSorter(*catValuesForAllSteps, categoryBinCount));
        }

        std::set<int> timeStepsToInclude;
        if (resultTimeStep == -1)
        {
            size_t nStepsInData = std::max(xValuesForAllSteps.size(), yValuesForAllSteps.size());
            bool xValid = xValuesForAllSteps.size() == 1u || xValuesForAllSteps.size() == nStepsInData;
            bool yValid = yValuesForAllSteps.size() == 1u || yValuesForAllSteps.size() == nStepsInData;
            
            if (!(xValid && yValid))
                return result;

            for (size_t i = 0; i < nStepsInData; ++i)
            {
                timeStepsToInclude.insert((int)i);
            }
        }
        else
        {
            timeStepsToInclude.insert(static_cast<size_t>(resultTimeStep));
        }

        for (int timeStep : timeStepsToInclude)
        {
            const cvf::UByteArray* cellVisibility = nullptr;
            if (timeStepCellVisibilityMap.count(timeStep))
            {
                cellVisibility = &timeStepCellVisibilityMap[timeStep];
            }            

            int xIndex = timeStep >= (int)xValuesForAllSteps.size() ? 0 : timeStep;
            int yIndex = timeStep >= (int)yValuesForAllSteps.size() ? 0 : timeStep;

            RigActiveCellsResultAccessor                  xAccessor(mainGrid, &xValuesForAllSteps[xIndex], activeCellInfo);
            RigActiveCellsResultAccessor                  yAccessor(mainGrid, &yValuesForAllSteps[yIndex], activeCellInfo);
            std::unique_ptr<RigActiveCellsResultAccessor> catAccessor;
            if (catValuesForAllSteps)
            {
                int catIndex = timeStep >= (int)catValuesForAllSteps->size() ? 0 : timeStep;
                catAccessor.reset(
                    new RigActiveCellsResultAccessor(mainGrid, &(catValuesForAllSteps->at(catIndex)), activeCellInfo));
            }

            for (size_t globalCellIdx = 0; globalCellIdx < activeCellInfo->reservoirCellCount(); ++globalCellIdx)
            {
                if (cellVisibility && !(*cellVisibility)[globalCellIdx]) continue;

                double xValue = xAccessor.cellScalarGlobIdx(globalCellIdx);
                double yValue = yAccessor.cellScalarGlobIdx(globalCellIdx);

                int category = 0;
                if (categorizationType == TIME_CATEGORIZATION)
                {
                    category = timeStep;
                }
                else if (categorizationType == FORMATION_CATEGORIZATION && activeFormationNames)
                {
                    size_t i(cvf::UNDEFINED_SIZE_T), j(cvf::UNDEFINED_SIZE_T), k(cvf::UNDEFINED_SIZE_T);
                    if (mainGrid->ijkFromCellIndex(globalCellIdx, &i, &j, &k))
                    {
                        category = activeFormationNames->formationIndexFromKLayerIdx(k);
                    }
                }
                else if (catAccessor && catBinSorter)
                {
                    double catValue = catAccessor->cellScalarGlobIdx(globalCellIdx);
                    category        = catBinSorter->binNumber(catValue);
                }
                if (xValue != HUGE_VAL && yValue != HUGE_VAL)
                {
                    categorySamplesMap[category].first.push_back(xValue);
                    categorySamplesMap[category].second.push_back(yValue);
                }
            }
        }
    }

    std::vector<QDateTime> timeStepDates = resultData->timeStepDates();
    QString timeFormatString = RiaQDateTimeTools::createTimeFormatStringFromDates(timeStepDates);

    for (const auto& sampleCategory : categorySamplesMap)
    {
        QString categoryName;
        if (categorizationType == TIME_CATEGORIZATION && categorySamplesMap.size() > 1u)
        {
            if (sampleCategory.first < static_cast<int>(timeStepDates.size()))
            {
                categoryName = RiaQDateTimeTools::toStringUsingApplicationLocale(timeStepDates[sampleCategory.first], timeFormatString);
            }
        }
        else if (categorizationType == FORMATION_CATEGORIZATION && activeFormationNames)
        {
            categoryName = activeFormationNames->formationNameFromKLayerIdx(sampleCategory.first);
        }
        else if (catBinSorter)
        {
            std::pair<double, double> binRange = catBinSorter->binRange(sampleCategory.first);

            categoryName = QString("%1 [%2, %3]")
                               .arg(catAddress.m_resultName)
                               .arg(binRange.first)
                               .arg(binRange.second);
        }
        categoryNameMap.insert(std::make_pair(sampleCategory.first, categoryName));
    }

    return result;
}
