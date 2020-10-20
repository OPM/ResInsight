/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemClosestResultIndexCalculator.h"

#include "RigFemPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemClosestResultIndexCalculator::RigFemClosestResultIndexCalculator( RigFemPart*         femPart,
                                                                        RigFemResultPosEnum resultPosition,
                                                                        int                 elementIndex,
                                                                        int                 m_face,
                                                                        const cvf::Vec3d&   intersectionPointInDomain )
{
    m_resultIndexToClosestResult = -1;
    m_closestNodeId              = -1;
    m_closestElementNodeResIdx   = -1;

    if ( resultPosition != RIG_ELEMENT_NODAL_FACE || m_face == -1 )
    {
        RigElementType elmType      = femPart->elementType( elementIndex );
        const int*     elementConn  = femPart->connectivities( elementIndex );
        int            elmNodeCount = RigFemTypes::elementNodeCount( elmType );

        // Find the closest node
        int   closestLocalNode = -1;
        float minDist          = std::numeric_limits<float>::infinity();
        for ( int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx )
        {
            int        nodeIdx         = elementConn[lNodeIdx];
            cvf::Vec3f nodePosInDomain = femPart->nodes().coordinates[nodeIdx];
            float      dist            = ( nodePosInDomain - cvf::Vec3f( intersectionPointInDomain ) ).lengthSquared();
            if ( dist < minDist )
            {
                closestLocalNode = lNodeIdx;
                minDist          = dist;
            }
        }

        if ( closestLocalNode >= 0 )
        {
            int nodeIdx = elementConn[closestLocalNode];
            m_closestElementNodeResIdx =
                static_cast<int>( femPart->elementNodeResultIdx( elementIndex, closestLocalNode ) );

            if ( resultPosition == RIG_NODAL )
            {
                m_resultIndexToClosestResult = nodeIdx;
            }
            else if ( resultPosition == RIG_ELEMENT_NODAL_FACE )
            {
                m_resultIndexToClosestResult = -1;
            }
            else if ( resultPosition == RIG_ELEMENT )
            {
                m_resultIndexToClosestResult = elementIndex;
            }
            else
            {
                m_resultIndexToClosestResult = m_closestElementNodeResIdx;
            }

            m_closestNodeId = femPart->nodes().nodeIds[nodeIdx];
        }
    }
    else if ( m_face != -1 )
    {
        int elmNodFaceResIdx = -1;
        int closestNodeIdx   = -1;
        {
            int closestLocFaceNode  = -1;
            int closestLocalElmNode = -1;
            {
                RigElementType elmType        = femPart->elementType( elementIndex );
                const int*     elmNodeIndices = femPart->connectivities( elementIndex );
                int            faceNodeCount  = 0;
                const int*     localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( elmType, m_face, &faceNodeCount );

                float minDist = std::numeric_limits<float>::infinity();
                for ( int faceNodIdx = 0; faceNodIdx < faceNodeCount; ++faceNodIdx )
                {
                    int        nodeIdx         = elmNodeIndices[localElmNodeIndicesForFace[faceNodIdx]];
                    cvf::Vec3f nodePosInDomain = femPart->nodes().coordinates[nodeIdx];
                    float      dist = ( nodePosInDomain - cvf::Vec3f( intersectionPointInDomain ) ).lengthSquared();
                    if ( dist < minDist )
                    {
                        closestLocFaceNode  = faceNodIdx;
                        closestNodeIdx      = nodeIdx;
                        closestLocalElmNode = localElmNodeIndicesForFace[faceNodIdx];
                        minDist             = dist;
                    }
                }
            }

            int elmNodFaceResIdxElmStart  = elementIndex * 24; // HACK should get from part
            int elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + 4 * m_face;

            if ( closestLocFaceNode >= 0 )
            {
                elmNodFaceResIdx = elmNodFaceResIdxFaceStart + closestLocFaceNode;
                m_closestElementNodeResIdx =
                    static_cast<int>( femPart->elementNodeResultIdx( elementIndex, closestLocalElmNode ) );
            }
        }

        m_resultIndexToClosestResult = elmNodFaceResIdx;
        m_closestNodeId              = femPart->nodes().nodeIds[closestNodeIdx];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemClosestResultIndexCalculator::resultIndexToClosestResult() const
{
    return m_resultIndexToClosestResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemClosestResultIndexCalculator::closestNodeId() const
{
    return m_closestNodeId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemClosestResultIndexCalculator::closestElementNodeResIdx() const
{
    return m_closestElementNodeResIdx;
}
