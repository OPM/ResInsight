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

    m_readerInterface = new RifReaderStatisticalCalculation;

    openEclipseGridFile();   
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
    CVF_ASSERT(m_rigEclipseCase.notNull());

    m_rigEclipseCase->setMainGrid(mainGrid);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStatisticalCalculation::openEclipseGridFile()
{
    if (m_rigEclipseCase.notNull()) return true;

    cvf::ref<RigEclipseCase> eclipseCase = new RigEclipseCase;

    if (!m_readerInterface->open("dummy", eclipseCase.p()))
    {
        return false;
    }

    m_rigEclipseCase = eclipseCase;

    m_rigEclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(m_readerInterface.p());
    m_rigEclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(m_readerInterface.p());
   
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
RimStatisticalCollection* RimStatisticalCalculation::parent()
{
    std::vector<caf::PdmObject*> parentObjects;
    this->parentObjects(parentObjects);

    RimStatisticalCollection* parentObject = NULL;
    for (size_t i = 0; i < parentObjects.size(); i++)
    {
        if (parentObject) continue;

        caf::PdmObject* obj = parentObjects[i];
        parentObject = dynamic_cast<RimStatisticalCollection*>(obj);
    }

    CVF_ASSERT(parentObject);

    return parentObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::computeStatistics()
{
    if (m_rigEclipseCase.isNull())
    {
        openEclipseGridFile();
    }

    cvf::Collection<RigEclipseCase> sourceCases;

    getSourceCases(sourceCases);

    if (sourceCases.size() == 0)
    {
        return;
    }

    // The first source has been read completely from disk, and contains grid and meta data
    // Use this information for all cases in the case group
    size_t timeStepCount = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount();

    RigStatisticsConfig statisticsConfig;

    std::vector<size_t> timeStepIndices;
    for (size_t i = 0; i < timeStepCount; i++)
    {
        timeStepIndices.push_back(i);
    }

    RigEclipseCase* resultCase = reservoirData();

    RigStatistics stat(sourceCases, timeStepIndices, statisticsConfig, resultCase);
    QStringList resultNames = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->resultNames(RimDefines::DYNAMIC_NATIVE);

    stat.evaluateStatistics(RimDefines::DYNAMIC_NATIVE, resultNames);

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
void RimStatisticalCalculation::getSourceCases(cvf::Collection<RigEclipseCase>& sourceCases)
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

            RigEclipseCase* sourceCase = gridCaseGroup->caseCollection->reservoirs[i]->reservoirData();
            sourceCases.push_back(sourceCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimStatisticalCalculation::caseGroup()
{
    RimStatisticalCollection* statColl = parent();
    if (statColl)
    {
        std::vector<caf::PdmObject*> parentObjects;
        statColl->parentObjects(parentObjects);

        RimIdenticalGridCaseGroup* gridCaseGroup = NULL;
        for (size_t i = 0; i < parentObjects.size(); i++)
        {
            if (gridCaseGroup) continue;

            caf::PdmObject* obj = parentObjects[i];
            gridCaseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>(obj);
        }

        return gridCaseGroup;
    }

    return NULL;
}
