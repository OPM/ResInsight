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


CAF_PDM_SOURCE_INIT(RimStatisticalCalculation, "RimStatisticalCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation::RimStatisticalCalculation()
    : RimReservoir()
{
    CAF_PDM_InitField(&statisticsMin,       "StatisticsMin",    true, "Minimum", "", "" ,"");
    CAF_PDM_InitField(&statisticsMax,       "StatisticsMax",    true, "Maximum", "", "" ,"");
    CAF_PDM_InitField(&statisticsMean,      "StatisticsMean",   true, "Mean", "", "" ,"");
    CAF_PDM_InitField(&statisticsStdDev,    "StatisticsStdDev", true, "Std dev", "", "" ,"");

    m_readerInterface = new RifReaderStatisticalCalculation;
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
bool RimStatisticalCalculation::openEclipseGridFile()
{
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
    // Fields declared in RimCellFilter
    uiOrdering.add(&caseName);

    // Fields declared in RimResultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Statistical parameters");
    group1->add(&statisticsMin);
    group1->add(&statisticsMax);
    group1->add(&statisticsMean);
    group1->add(&statisticsStdDev);

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
    if (statisticsMin)
    {
        createAndComputeMin();
    }
    
    if (statisticsMax)
    {
        createAndComputeMax();
    }

    if (statisticsMean)
    {
        createAndComputeMean();
    }

    if (statisticsStdDev)
    {
        createAndComputeStdDev();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::createAndComputeMin()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::createAndComputeMax()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::createAndComputeMean()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticalCalculation::createAndComputeStdDev()
{

}
