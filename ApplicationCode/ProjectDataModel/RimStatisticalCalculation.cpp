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

#include "RimStatisticalCalculation.h"
#include "RimReservoirView.h"
#include "cafPdmUiOrdering.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RigEclipseCase.h"
#include "RifReaderStatisticalCalculation.h"
#include "RigReservoirCellResults.h"
#include "RigStatistics.h"
#include "RigMainGrid.h"


CAF_PDM_SOURCE_INIT(RimStatisticalCalculation, "RimStatisticalCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation::RimStatisticalCalculation()
    : RimReservoir()
{
    CAF_PDM_InitObject("Case Group Statistics", ":/Histogram16x16.png", "", "");
    CAF_PDM_InitField(&m_resultName, "ResultName", QString("PRESSURE"), "ResultName", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation::~RimStatisticalCalculation()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::setMainGrid(RigMainGrid* mainGrid)
{
    CVF_ASSERT(mainGrid);
    CVF_ASSERT(this->reservoirData());

    reservoirData()->setMainGrid(mainGrid);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStatisticalCalculation::openEclipseGridFile()
{
    if (this->reservoirData()) return true;

    cvf::ref<RigEclipseCase> eclipseCase = new RigEclipseCase;

    CVF_ASSERT(parentStatisticalCollection());

    RimIdenticalGridCaseGroup* gridCaseGroup = parentStatisticalCollection()->parentCaseGroup();
    CVF_ASSERT(gridCaseGroup);

    RigMainGrid* mainGrid = gridCaseGroup->mainGrid();

    eclipseCase->setMainGrid(mainGrid);

    eclipseCase->setActiveCellInfo(RifReaderInterface::MATRIX_RESULTS, gridCaseGroup->unionOfActiveCells(RifReaderInterface::MATRIX_RESULTS));
    eclipseCase->setActiveCellInfo(RifReaderInterface::FRACTURE_RESULTS, gridCaseGroup->unionOfActiveCells(RifReaderInterface::FRACTURE_RESULTS));

    this->setReservoirData( eclipseCase.p() );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) const
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCollection* RimStatisticalCalculation::parentStatisticalCollection()
{
    std::vector<RimStatisticalCollection*> parentObjects;
    this->parentObjectsOfType(parentObjects);

    if (parentObjects.size() > 0)
    {
        return parentObjects[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::computeStatistics()
{
    if (this->reservoirData() == NULL)
    {
        openEclipseGridFile();
    }

    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    CVF_ASSERT(gridCaseGroup);
    gridCaseGroup->computeUnionOfActiveCells();

    std::vector<RimReservoir*> sourceCases;

    getSourceCases(sourceCases);

    if (sourceCases.size() == 0)
    {
        return;
    }

    // The first source has been read completely from disk, and contains grid and meta data
    // Use this information for all cases in the case group
    size_t timeStepCount = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->maxTimeStepCount();

    RigStatisticsConfig statisticsConfig;

    std::vector<size_t> timeStepIndices;
    for (size_t i = 0; i < timeStepCount; i++)
    {
        timeStepIndices.push_back(i);
    }

    RigEclipseCase* resultCase = reservoirData();

    QList<QPair<RimDefines::ResultCatType, QString> > resultSpecification;

    //resultSpecification.append(qMakePair(RimDefines::DYNAMIC_NATIVE, QString("PRESSURE")));

    
    {
        QStringList resultNames = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->resultNames(RimDefines::DYNAMIC_NATIVE);
        foreach(QString resultName, resultNames)
        {
            resultSpecification.append(qMakePair(RimDefines::DYNAMIC_NATIVE, resultName));
        }
    }

    {
        QStringList resultNames = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->resultNames(RimDefines::STATIC_NATIVE);
        foreach(QString resultName, resultNames)
        {
            resultSpecification.append(qMakePair(RimDefines::STATIC_NATIVE, resultName));
        }
    }
    

    RigStatistics stat(sourceCases, timeStepIndices, statisticsConfig, resultCase);
    stat.evaluateStatistics(resultSpecification);

    for (size_t i = 0; i < reservoirViews().size(); i++)
    {
        RimReservoirView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::ACTIVE);
        reservoirView->createDisplayModelAndRedraw();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::getSourceCases(std::vector<RimReservoir*>& sourceCases)
{
    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    if (gridCaseGroup)
    {
        size_t caseCount = gridCaseGroup->caseCollection->reservoirs.size();
        for (size_t i = 0; i < caseCount; i++)
        {
            CVF_ASSERT(gridCaseGroup->caseCollection);
            CVF_ASSERT(gridCaseGroup->caseCollection->reservoirs[i]);
            CVF_ASSERT(gridCaseGroup->caseCollection->reservoirs[i]->reservoirData());

            RimReservoir* sourceCase = gridCaseGroup->caseCollection->reservoirs[i];
            sourceCases.push_back(sourceCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimStatisticalCalculation::caseGroup()
{
    RimStatisticalCollection* parentCollection = parentStatisticalCollection();
    if (parentCollection)
    {
        return parentCollection->parentCaseGroup();
    }

    return NULL;
}
