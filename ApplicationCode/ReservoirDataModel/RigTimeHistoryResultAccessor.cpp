/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigTimeHistoryResultAccessor.h"

#include <cmath> // Needed for HUGE_VAL on Linux

#include "RigCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigCaseCellResultsData.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigTimeHistoryResultAccessor::RigTimeHistoryResultAccessor(RigCaseData* eclipseCaseData, size_t gridIndex, size_t cellIndex, size_t scalarResultIndex, RifReaderInterface::PorosityModelResultType porosityModel)
    : m_eclipseCaseData(eclipseCaseData),
      m_gridIndex(gridIndex),
      m_cellIndex(cellIndex),
      m_scalarResultIndex(scalarResultIndex),
      m_porosityModel(porosityModel)
{
    m_face = cvf::StructGridInterface::NO_FACE;

    computeTimeHistoryData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTimeHistoryResultAccessor::setFace(cvf::StructGridInterface::FaceType face)
{
    m_face = face;

    computeTimeHistoryData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigTimeHistoryResultAccessor::timeHistoryValues() const
{
    return m_timeHistoryValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigTimeHistoryResultAccessor::topologyText() const
{
    QString text;

    if (m_eclipseCaseData)
    {
        if (m_cellIndex != cvf::UNDEFINED_SIZE_T)
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if (m_eclipseCaseData->grid(m_gridIndex)->ijkFromCellIndex(m_cellIndex, &i, &j, &k))
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                cvf::StructGridInterface::FaceEnum faceEnum(m_face);

                text += QString("Cell : [%1, %2, %3]").arg(i).arg(j).arg(k);
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTimeHistoryResultAccessor::computeTimeHistoryData()
{
    m_timeHistoryValues.clear();

    if (m_eclipseCaseData)
    {
        size_t timeStepCount = m_eclipseCaseData->results(m_porosityModel)->timeStepCount(m_scalarResultIndex);

        for (size_t i = 0; i < timeStepCount; i++)
        {
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(m_eclipseCaseData, m_gridIndex, m_porosityModel, i, m_scalarResultIndex);

            m_timeHistoryValues.push_back(resultAccessor->cellScalar(m_cellIndex));
        }
    }
}

