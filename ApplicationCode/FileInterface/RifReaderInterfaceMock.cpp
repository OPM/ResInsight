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

#include "RifReaderInterfaceMock.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceMock::open(const QString& fileName, RigReservoir* reservoir)
{
    m_reservoirBuilder.populateReservoir(reservoir);
  
    m_reservoir = reservoir;

    m_dynamicResults.clear();
    m_staticResults.clear();
    m_timeStepTexts.clear();

    size_t i;
    for (i = 0; i < m_reservoirBuilder.resultCount(); i++)
    {
        m_dynamicResults.push_back(QString("Dynamic_Result_%1").arg(i));
    }

    for (i = 0; i < m_reservoirBuilder.resultCount(); i++)
    {
        QString varEnd;
        if (i == 0) varEnd = "X";
        if (i == 1) varEnd = "Y";
        int resIndex = 0;
        if (i > 1) resIndex = i;

        m_staticResults.push_back(QString("Static_Result_%1%2").arg(resIndex).arg(varEnd));
    }

    for (i = 0; i < m_reservoirBuilder.timeStepCount(); i++)
    {
        m_timeStepTexts.push_back(QString("Timestep %1").arg(i));
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::close()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceMock::staticResults() const
{

    return m_staticResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceMock::dynamicResults() const
{
    return m_dynamicResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifReaderInterfaceMock::numTimeSteps() const
{
    return m_reservoirBuilder.timeStepCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceMock::staticResult(const QString& result, std::vector<double>* values)
{
    m_reservoirBuilder.staticResult(m_reservoir.p(), result, values);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceMock::dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values)
{
    m_reservoirBuilder.dynamicResult(m_reservoir.p(), result, stepIndex, values);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterfaceMock::RifReaderInterfaceMock()
{
    /*
    m_cellResults.push_back("Dummy results");
    m_cellProperties.push_back("Dummy static result");
    */
}

RifReaderInterfaceMock::~RifReaderInterfaceMock()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::setWorldCoordinates(cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate)
{
    m_reservoirBuilder.setWorldCoordinates(minWorldCoordinate, maxWorldCoordinate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::setGridPointDimensions(const cvf::Vec3st& gridPointDimensions)
{
    m_reservoirBuilder.setGridPointDimensions(gridPointDimensions);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::setResultInfo(size_t resultCount, size_t timeStepCount)
{
    m_reservoirBuilder.setResultInfo(resultCount, timeStepCount);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::addLocalGridRefinement(const cvf::Vec3st& minCellPosition, const cvf::Vec3st& maxCellPosition, const cvf::Vec3st& singleCellRefinementFactors)
{
    m_reservoirBuilder.addLocalGridRefinement(minCellPosition, maxCellPosition, singleCellRefinementFactors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceMock::populateReservoir(RigReservoir* reservoir)
{
    m_reservoirBuilder.populateReservoir(reservoir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceMock::timeStepText() const
{
   return m_timeStepTexts;
}


