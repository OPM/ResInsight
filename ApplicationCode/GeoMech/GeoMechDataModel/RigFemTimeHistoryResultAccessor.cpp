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

#include "RigFemTimeHistoryResultAccessor.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemTypes.h"
#include "RigGeoMechCaseData.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemTimeHistoryResultAccessor::RigFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData, RigFemResultAddress femResultAddress,
    size_t gridIndex, size_t cellIndex, const cvf::Vec3d& intersectionPoint)
    : m_geoMechCaseData(geomData),
    m_femResultAddress(femResultAddress),
    m_gridIndex(gridIndex),
    m_cellIndex(cellIndex),
    m_intersectionPoint(intersectionPoint)
{
    computeTimeHistoryData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigFemTimeHistoryResultAccessor::topologyText() const
{
    QString text;

    if (m_geoMechCaseData)
    {
        RigFemPart* femPart = m_geoMechCaseData->femParts()->part(m_gridIndex);
        int elementId = femPart->elmId(m_cellIndex);
        text += QString("Element : Id[%1]").arg(elementId);

        size_t i = 0;
        size_t j = 0;
        size_t k = 0;
        if (m_geoMechCaseData->femParts()->part(m_gridIndex)->structGrid()->ijkFromCellIndex(m_cellIndex, &i, &j, &k))
        {
            // Adjust to 1-based Eclipse indexing
            i++;
            j++;
            k++;

            cvf::Vec3d domainCoord = m_intersectionPoint;
            text += QString(", ijk[%1, %2, %3]").arg(i).arg(j).arg(k);

            QString formattedText;
            formattedText.sprintf("Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]", domainCoord.x(), domainCoord.y(), -domainCoord.z());

            text += formattedText;
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigFemTimeHistoryResultAccessor::timeHistoryValues() const
{
    return m_timeHistoryValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemTimeHistoryResultAccessor::computeTimeHistoryData()
{
    m_timeHistoryValues.clear();
    
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    // Compute scalar result index from geometry
    {
        RigFemPart* femPart = m_geoMechCaseData->femParts()->part(m_gridIndex);
        RigElementType elmType =  femPart->elementType(m_cellIndex);
        const int* elmentConn = femPart->connectivities(m_cellIndex);
        int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);

        // Find the closest node
        int closestLocalNode = -1;
        float minDist = std::numeric_limits<float>::infinity();
        for (int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx)
        {
            int nodeIdx = elmentConn[lNodeIdx];
            cvf::Vec3f nodePos = femPart->nodes().coordinates[nodeIdx];
            float dist = (nodePos - cvf::Vec3f(m_intersectionPoint)).lengthSquared();
            if (dist < minDist) 
            {
                closestLocalNode = lNodeIdx;
                minDist = dist;
            }
        }

        // Create a text showing the results from the closest node
        if (closestLocalNode >= 0)
        {
            int nodeIdx = elmentConn[closestLocalNode];
            if (m_femResultAddress.resultPosType == RIG_NODAL)
            {
                scalarResultIndex = static_cast<size_t>(nodeIdx);
            } 
            else 
            {
                scalarResultIndex = femPart->elementNodeResultIdx(static_cast<int>(m_cellIndex), closestLocalNode);
            }
        }
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return;
    
    RigFemPartResultsCollection* femPartResultsColl = m_geoMechCaseData->femPartResults();
    for (int frameIdx = 0; frameIdx < femPartResultsColl->frameCount(); frameIdx++)
    {
        const std::vector<float>& scalarResults = m_geoMechCaseData->femPartResults()->resultValues(m_femResultAddress, static_cast<int>(m_gridIndex), frameIdx);
        if (scalarResults.size())
        {
            float scalarValue = scalarResults[scalarResultIndex];

            m_timeHistoryValues.push_back(scalarValue);
        }
    }
}
