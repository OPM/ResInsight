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

#include "cvfBase.h"

#include "RigMainGrid.h"
#include "RigEclipseCase.h"
#include "RigReservoirCellResults.h"

#include "RifReaderStatisticalCalculation.h"


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderStatisticalCalculation::RifReaderStatisticalCalculation()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderStatisticalCalculation::~RifReaderStatisticalCalculation()
{
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderStatisticalCalculation::open(const QString& fileName, RigEclipseCase* eclipseCase)
{
    buildMetaData(eclipseCase);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderStatisticalCalculation::buildMetaData(RigEclipseCase* eclipseCase)
{
    RigReservoirCellResults* matrixModelResults = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS);
    RigReservoirCellResults* fractureModelResults = eclipseCase->results(RifReaderInterface::FRACTURE_RESULTS);

    // Dynamic results
    for (int i = 0; i < m_matrixDynamicResultNames.size(); ++i)
    {
        size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, m_matrixDynamicResultNames[i], true);
        matrixModelResults->setTimeStepDates(resIndex, m_timeSteps);
    }

    for (int i = 0; i < m_fractureDynamicResultNames.size(); ++i)
    {
        size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, m_fractureDynamicResultNames[i], true);
        fractureModelResults->setTimeStepDates(resIndex, m_timeSteps);
    }

    std::vector<QDateTime> staticDate;
    if (m_timeSteps.size() > 0)
    {
        staticDate.push_back(m_timeSteps.front());
    }

    // Static results
    for (int i = 0; i < m_fractureStaticResultNames.size(); ++i)
    {
        size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, m_fractureStaticResultNames[i], true);
        fractureModelResults->setTimeStepDates(resIndex, staticDate);
    }

    for (int i = 0; i < m_matrixStaticResultNames.size(); ++i)
    {
        size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, m_matrixStaticResultNames[i], true);
        matrixModelResults->setTimeStepDates(resIndex, staticDate);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderStatisticalCalculation::setMatrixResultNames(const QStringList& staticResultNames, const QStringList& dynamicResultNames)
{
    m_matrixStaticResultNames = staticResultNames;
    m_matrixDynamicResultNames = dynamicResultNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderStatisticalCalculation::setFractureResultNames(const QStringList& staticResultNames, const QStringList& dynamicResultNames)
{
    m_fractureStaticResultNames = staticResultNames;
    m_fractureDynamicResultNames = dynamicResultNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderStatisticalCalculation::setTimeSteps(const std::vector<QDateTime>& timesteps)
{
    m_timeSteps = timesteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderStatisticalCalculation::staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values)
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderStatisticalCalculation::dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values)
{
    return false;
}

