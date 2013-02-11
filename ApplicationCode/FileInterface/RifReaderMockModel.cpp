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

#include "RifReaderMockModel.h"
#include "RigReservoirCellResults.h"
#include "RifReaderInterface.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::open(const QString& fileName, RigReservoir* reservoir)
{
    m_reservoirBuilder.populateReservoir(reservoir);
  
    m_reservoir = reservoir;

    RigReservoirCellResults* cellResults = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS);


    QList<QDateTime> dates;

    for (int i = 0; i < static_cast<int>(m_reservoirBuilder.timeStepCount()); i++)
    {
        dates.push_back(QDateTime(QDate(2012+i, 6, 1)));
    }

    for (size_t i = 0; i < m_reservoirBuilder.resultCount(); i++)
    {
        size_t resIdx = cellResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, QString("Dynamic_Result_%1").arg(i));
        cellResults->setTimeStepDates(resIdx, dates);
    }

    if (m_reservoirBuilder.timeStepCount() == 0) return true;

    QList<QDateTime> staticDates;
    staticDates.push_back(dates[0]);
    for (int i = 0; i < static_cast<int>(m_reservoirBuilder.resultCount()); i++)
    {
        QString varEnd;
        if (i == 0) varEnd = "X";
        if (i == 1) varEnd = "Y";
        int resIndex = 0;
        if (i > 1) resIndex = i;

        size_t resIdx = cellResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, QString("Static_Result_%1%2").arg(resIndex).arg(varEnd));
        cellResults->setTimeStepDates(resIdx, staticDates);
    }


#define ADD_INPUT_PROPERTY(Name) \
    { \
        size_t resIdx; \
        QString resultName(Name); \
        resIdx = cellResults->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, resultName); \
        cellResults->setTimeStepDates(resIdx, staticDates); \
        cellResults->cellScalarResults(resIdx).resize(1); \
        std::vector<double>& values = cellResults->cellScalarResults(resIdx)[0]; \
        this->inputProperty(resultName, &values); \
    }

    ADD_INPUT_PROPERTY("PORO");
    ADD_INPUT_PROPERTY("PERM");
    ADD_INPUT_PROPERTY("MULTX");
   
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::close()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::inputProperty(const QString& propertyName, std::vector<double>* values)
{
    return m_reservoirBuilder.inputProperty(m_reservoir.p(), propertyName, values);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::staticResult(const QString& result, RifReaderInterface::PorosityModelResultType matrixOrFracture, std::vector<double>* values)
{
    m_reservoirBuilder.staticResult(m_reservoir.p(), result, values);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::dynamicResult(const QString& result, RifReaderInterface::PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values)
{
    m_reservoirBuilder.dynamicResult(m_reservoir.p(), result, stepIndex, values);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderMockModel::RifReaderMockModel()
{
    /*
    m_cellResults.push_back("Dummy results");
    m_cellProperties.push_back("Dummy static result");
    */
}

RifReaderMockModel::~RifReaderMockModel()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setWorldCoordinates(cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate)
{
    m_reservoirBuilder.setWorldCoordinates(minWorldCoordinate, maxWorldCoordinate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setGridPointDimensions(const cvf::Vec3st& gridPointDimensions)
{
    m_reservoirBuilder.setGridPointDimensions(gridPointDimensions);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setResultInfo(size_t resultCount, size_t timeStepCount)
{
    m_reservoirBuilder.setResultInfo(resultCount, timeStepCount);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::addLocalGridRefinement(const cvf::Vec3st& minCellPosition, const cvf::Vec3st& maxCellPosition, const cvf::Vec3st& singleCellRefinementFactors)
{
    m_reservoirBuilder.addLocalGridRefinement(minCellPosition, maxCellPosition, singleCellRefinementFactors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::populateReservoir(RigReservoir* reservoir)
{
    m_reservoirBuilder.populateReservoir(reservoir);
}

