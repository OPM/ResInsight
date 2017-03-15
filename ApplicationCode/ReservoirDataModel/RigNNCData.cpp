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

#include "RigNNCData.h"
#include "RigMainGrid.h"
#include "cvfGeometryTools.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigNNCData::RigNNCData()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNNCData::processConnections(const RigMainGrid& mainGrid)
{
    //cvf::Trace::show("NNC: Total number: " + cvf::String((int)m_connections.size()));

    for (size_t cnIdx = 0; cnIdx < m_connections.size(); ++cnIdx)
    {
        const RigCell& c1 = mainGrid.globalCellArray()[m_connections[cnIdx].m_c1GlobIdx];
        const RigCell& c2 = mainGrid.globalCellArray()[m_connections[cnIdx].m_c2GlobIdx];

        // Try to find the shared face

        bool isPossibleNeighborInDirection[6]= {true, true, true, true, true, true};

        if (c1.hostGrid() == c2.hostGrid())
        {
            char hasNeighbourInAnyDirection = 0;

            size_t i1, j1, k1;
            c1.hostGrid()->ijkFromCellIndex(c1.gridLocalCellIndex(), &i1, &j1, &k1);
            size_t i2, j2, k2;
            c2.hostGrid()->ijkFromCellIndex(c2.gridLocalCellIndex(), &i2, &j2, &k2);

          
            isPossibleNeighborInDirection[cvf::StructGridInterface::POS_I] = ((i1 + 1) == i2);
            isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_I] = ((i2 + 1) == i1);
            isPossibleNeighborInDirection[cvf::StructGridInterface::POS_J] = ((j1 + 1) == j2);
            isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_J] = ((j2 + 1) == j1);
            isPossibleNeighborInDirection[cvf::StructGridInterface::POS_K] = ((k1 + 1) == k2);
            isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_K] = ((k2 + 1) == k1);

            hasNeighbourInAnyDirection = 
                isPossibleNeighborInDirection[cvf::StructGridInterface::POS_I] 
            + isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_I]
            + isPossibleNeighborInDirection[cvf::StructGridInterface::POS_J]
            + isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_J]
            + isPossibleNeighborInDirection[cvf::StructGridInterface::POS_K]
            + isPossibleNeighborInDirection[cvf::StructGridInterface::NEG_K];


            // If cell 2 is not adjancent with respect to any of the six ijk directions, 
            // assume that we have no overlapping area.

            if (!hasNeighbourInAnyDirection)
            {
                // Add to search map
                //m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c1GlobIdx][cvf::StructGridInterface::NO_FACE].push_back(cnIdx);
                //m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c2GlobIdx][cvf::StructGridInterface::NO_FACE].push_back(cnIdx);

                //cvf::Trace::show("NNC: No direct neighbors : C1: " + cvf::String((int)m_connections[cnIdx].m_c1GlobIdx) + " C2: " + cvf::String((int)m_connections[cnIdx].m_c2GlobIdx));
                continue; // to next connection
            }
        }

        // Possibly do some testing to avoid unneccesary overlap calculations

        cvf::Vec3d normal;
        for (char fIdx = 0; fIdx < 6; ++fIdx)
        {
            if (isPossibleNeighborInDirection[fIdx])
            {
                cvf::Vec3d fc1 = c1.faceCenter((cvf::StructGridInterface::FaceType)(fIdx));
                cvf::Vec3d fc2 = c2.faceCenter(cvf::StructGridInterface::oppositeFace((cvf::StructGridInterface::FaceType)(fIdx)));
                cvf::Vec3d fc1ToFc2 = fc2 - fc1;
                normal = c1.faceNormalWithAreaLenght((cvf::StructGridInterface::FaceType)(fIdx));
                normal.normalize();
                // Check that face centers are approx in the face plane
                if (normal.dot(fc1ToFc2) < 0.01*fc1ToFc2.length()) 
                {

                }
            }
        }

        bool foundAnyOverlap = false;

        for (char fIdx = 0; fIdx < 6; ++fIdx)
        {
            if (!isPossibleNeighborInDirection[fIdx])
            { 
                continue;
            }

            // Calculate connection polygon

            std::vector<size_t> polygon;
            std::vector<cvf::Vec3d> intersections;
            caf::SizeTArray4 face1;
            caf::SizeTArray4 face2;
            c1.faceIndices((cvf::StructGridInterface::FaceType)(fIdx), &face1);
            c2.faceIndices(cvf::StructGridInterface::oppositeFace((cvf::StructGridInterface::FaceType)(fIdx)), &face2);

            bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(
                &polygon, 
                &intersections, 
                (cvf::EdgeIntersectStorage<size_t>*)NULL, 
                cvf::wrapArrayConst(&mainGrid.nodes()), 
                face1.data(), 
                face2.data(), 
                1e-6);

            if (foundOverlap)
            {
                foundAnyOverlap = true;
                // Found an overlap polygon. Store data about connection

                m_connections[cnIdx].m_c1Face = (cvf::StructGridInterface::FaceType)fIdx;
                for (size_t pIdx = 0; pIdx < polygon.size(); ++pIdx)
                {
                    if (polygon[pIdx] < mainGrid.nodes().size())
                        m_connections[cnIdx].m_polygon.push_back(mainGrid.nodes()[polygon[pIdx]]);
                    else
                        m_connections[cnIdx].m_polygon.push_back(intersections[polygon[pIdx] - mainGrid.nodes().size()]);
                }

                // Add to search map, possibly not needed
                //m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c1GlobIdx][fIdx].push_back(cnIdx);
                //m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c2GlobIdx][cvf::StructGridInterface::oppositeFace((cvf::StructGridInterface::FaceType)(fIdx))].push_back(cnIdx);

                break; // The connection face is found. Stop looping over the cell faces. Jump to next connection
            }
        }

        if (!foundAnyOverlap)
        {
            //cvf::Trace::show("NNC: No overlap found for : C1: " + cvf::String((int)m_connections[cnIdx].m_c1GlobIdx) + "C2: " + cvf::String((int)m_connections[cnIdx].m_c2GlobIdx));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigNNCData::makeConnectionScalarResult(size_t scalarResultIndex)
{
    std::vector<double>& results = m_connectionResults[scalarResultIndex];
    results.resize(m_connections.size(), HUGE_VAL);
    return results;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::connectionScalarResult(size_t scalarResultIndex) const
{
    std::map<size_t, std::vector<double> >::const_iterator it = m_connectionResults.find(scalarResultIndex);
    if (it != m_connectionResults.end())
        return &(it->second);
    else
        return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNNCData::setCombTransmisibilityScalarResultIndex(size_t scalarResultIndex)
{
    std::map<size_t, std::vector<double> >::iterator it = m_connectionResults.find(cvf::UNDEFINED_SIZE_T);
    if (it != m_connectionResults.end())
    {
        std::vector<double>& emptyData = m_connectionResults[scalarResultIndex];
        std::vector<double>& realData = m_connectionResults[cvf::UNDEFINED_SIZE_T];
        emptyData.swap(realData);
        m_connectionResults.erase(cvf::UNDEFINED_SIZE_T);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigNNCData::hasScalarValues(size_t scalarResultIndex)
{
    std::map<size_t, std::vector<double> >::iterator it = m_connectionResults.find(scalarResultIndex);
    return (it != m_connectionResults.end());
}

/*
//--------------------------------------------------------------------------------------------------
/// TODO: Possibly not needed !
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigNNCData::findConnectionIndices( size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face) const
{
    ConnectionSearchMap::const_iterator it;
    static std::vector<size_t> empty;

    it = m_cellIdxToFaceToConnectionIdxMap.find(reservoirCellIndex);
    if (it != m_cellIdxToFaceToConnectionIdxMap.end())
    {
        return it->second[face];
    }

    return empty;
}
*/
