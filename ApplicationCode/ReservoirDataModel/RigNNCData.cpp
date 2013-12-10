/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
//#include "../ModelVisualization/cvfGeometryTools.h"


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
    /*
    for (size_t cnIdx = 0; cnIdx < 0; ++cnIdx)
    {
        const RigCell& c1 = mainGrid.cells()[m_connections[cnIdx].m_c1GlobIdx];
        const RigCell& c2 = mainGrid.cells()[m_connections[cnIdx].m_c2GlobIdx];

        // Try to find the shared face
        char hasNeighbourInAnyDirection = 0;

        if (c1.hostGrid() == c2.hostGrid())
        {
            size_t i1, j1, k1;
            c1.hostGrid()->ijkFromCellIndex(c1.cellIndex(), &i1, &j1, &k1);
            size_t i2, j2, k2;
            c2.hostGrid()->ijkFromCellIndex(c2.cellIndex(), &i2, &j2, &k2);

            bool isPossibleNeighborInDirection[6]= {false, false, false, false, false, false};
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
                m_connections[cnIdx].m_hasNoSharedArea = true;
                continue; // to next connection
            }

            if (hasNeighbourInAnyDirection == 1)
            {
                for (char fIdx = 0; fIdx < 6; ++fIdx)
                {
                    if (isPossibleNeighborInDirection[fIdx])
                    {
                        m_connections[cnIdx].m_c1Face = (cvf::StructGridInterface::FaceType)fIdx;
                        break; // the face loop
                    }
                }

                // calculate polygon for this face

            }
            else
            {

                cvf::Vec3d normal;
                for (char fIdx = 0; fIdx < 6; ++fIdx)
                {
                    if (isPossibleNeighborInDirection[fIdx])
                    {
                        cvf::Vec3d fc1 = c1.faceCenter((cvf::StructGridInterface::FaceType)(fIdx));
                        cvf::Vec3d fc2 = c2.faceCenter(cvf::StructGridInterface::oppositeFace((cvf::StructGridInterface::FaceType)(fIdx)));
                        cvf::Vec3d fc1ToFc2 = fc2 - fc1;
                        normal = c1.faceNormal((cvf::StructGridInterface::FaceType)(fIdx));
                        normal.normalize();
                        // Check that face centers are approx in the face plane
                        if (normal.dot(fc1ToFc2) < 0.01*fc1ToFc2.length()) 
                        {

                        }

                        // Calculate connection polygon

                        std::vector<size_t> polygon;
                        std::vector<cvf::Vec3d> intersections;
                        bool isOk = false;
                        caf::SizeTArray4 face1;
                        caf::SizeTArray4 face2;
                        c1.faceIndices((cvf::StructGridInterface::FaceType)(fIdx), &face1);
                        c2.faceIndices(cvf::StructGridInterface::oppositeFace((cvf::StructGridInterface::FaceType)(fIdx)), &face2);

                        isOk = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(
                            &polygon, 
                            &intersections, 
                            (cvf::EdgeIntersectStorage<size_t>*)NULL, 
                            cvf::wrapArrayConst(&mainGrid.nodes()), 
                            face1.data(), 
                            face2.data(), 
                            1e-6);

                    }

                    }
                }

            }
        }
        */
}
