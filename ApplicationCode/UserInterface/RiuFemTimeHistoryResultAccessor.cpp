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

#include "RiuFemTimeHistoryResultAccessor.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemTypes.h"
#include "RigGeoMechCaseData.h"
#include "RiuGeoMechXfTensorResultAccessor.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFemTimeHistoryResultAccessor::RiuFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData, 
                                                                 RigFemResultAddress femResultAddress,
                                                                 size_t gridIndex, 
                                                                 int elementIndex, 
                                                                 int face,
                                                                 const cvf::Vec3d& intersectionPoint)
    : m_geoMechCaseData(geomData),
    m_femResultAddress(femResultAddress),
    m_gridIndex(gridIndex),
    m_elementIndex(elementIndex),
    m_face(face),
    m_intersectionPoint(intersectionPoint),
    m_hasIntersectionTriangle(false)
{
    computeTimeHistoryData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFemTimeHistoryResultAccessor::RiuFemTimeHistoryResultAccessor(RigGeoMechCaseData* geomData, 
                                                                 RigFemResultAddress femResultAddress, 
                                                                 size_t gridIndex, 
                                                                 int elementIndex, 
                                                                 int face, 
                                                                 const cvf::Vec3d& intersectionPoint, 
                                                                 const std::array<cvf::Vec3f, 3>& intersectionTriangle)
    : m_geoMechCaseData(geomData),
    m_femResultAddress(femResultAddress),
    m_gridIndex(gridIndex),
    m_elementIndex(elementIndex),
    m_face(face),
    m_intersectionPoint(intersectionPoint),
    m_hasIntersectionTriangle(true),
    m_intersectionTriangle(intersectionTriangle)
{
    computeTimeHistoryData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuFemTimeHistoryResultAccessor::geometrySelectionText() const
{
    QString text;

    if (m_geoMechCaseData)
    {
        RigFemPart* femPart = m_geoMechCaseData->femParts()->part(m_gridIndex);
        int elementId = femPart->elmId(m_elementIndex);
        text += QString("Element : Id[%1]").arg(elementId);

        size_t i = 0;
        size_t j = 0;
        size_t k = 0;
        if (m_geoMechCaseData->femParts()->part(m_gridIndex)->structGrid()->ijkFromCellIndex(m_elementIndex, &i, &j, &k))
        {
            // Adjust to 1-based Eclipse indexing
            i++;
            j++;
            k++;

            cvf::Vec3d domainCoord = m_intersectionPoint;
            text += QString(", ijk[%1, %2, %3] ").arg(i).arg(j).arg(k);

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
std::vector<double> RiuFemTimeHistoryResultAccessor::timeHistoryValues() const
{
    return m_timeHistoryValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFemTimeHistoryResultAccessor::computeTimeHistoryData()
{
    m_timeHistoryValues.clear();
    
    RigFemClosestResultIndexCalculator closestCalc(m_geoMechCaseData->femParts()->part(m_gridIndex),
                                                   m_femResultAddress.resultPosType,
                                                   m_elementIndex,
                                                   m_face,
                                                   m_intersectionPoint );
    
    int scalarResultIndex = closestCalc.resultIndexToClosestResult();
    m_closestNodeId = closestCalc.closestNodeId();

    RigFemPartResultsCollection* femPartResultsColl = m_geoMechCaseData->femPartResults();

    if (m_femResultAddress.resultPosType == RIG_ELEMENT_NODAL_FACE && m_hasIntersectionTriangle)
    {
        int closestElmNodeResIndex = closestCalc.closestElementNodeResIdx();

        for ( int frameIdx = 0; frameIdx < femPartResultsColl->frameCount(); frameIdx++ )
        {
            RiuGeoMechXfTensorResultAccessor stressXfAccessor(femPartResultsColl, m_femResultAddress, frameIdx);
            float scalarValue = stressXfAccessor.calculateElmNodeValue(m_intersectionTriangle, closestElmNodeResIndex);
            m_timeHistoryValues.push_back(scalarValue);
        }
    }
    else
    {
        if ( scalarResultIndex < 0 ) return;

        for ( int frameIdx = 0; frameIdx < femPartResultsColl->frameCount(); frameIdx++ )
        {
            const std::vector<float>& scalarResults = m_geoMechCaseData->femPartResults()->resultValues(m_femResultAddress, static_cast<int>(m_gridIndex), frameIdx);
            if ( scalarResults.size() )
            {
                float scalarValue = scalarResults[scalarResultIndex];

                m_timeHistoryValues.push_back(scalarValue);
            }
        }
    }

}
